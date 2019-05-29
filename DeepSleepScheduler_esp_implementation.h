
#ifdef ESP32
#include <esp_sleep.h>
#include <esp32-hal-timer.h>
#include <soc/rtc.h>
#elif ESP8266
#include <limits.h>
#endif

#ifndef LIBCALL_DEEP_SLEEP_SCHEDULER
// -------------------------------------------------------------------------------------------------
// Implementation (usuallly in CPP file)
// -------------------------------------------------------------------------------------------------
#define ESP8266_MAX_DELAY_TIME_WDT_MS 7500
void Scheduler::init() {}

#ifdef ESP32
// -------------------------------------------------------------------------------------------------
unsigned long Scheduler::getMillis() const {
  // read RTC clock which runs from initial boot/reset (also during sleep)
  // https://forum.makehackvoid.com/t/playing-with-the-esp-32/1144/11
  uint64_t rtcTime = rtc_time_get();
  uint64_t rtcTimeUs = rtcTime * 20 / 3;  // ticks -> us 1,000,000/150,000
  return rtcTimeUs / 1000;
}

void Scheduler::isrWatchdogExpiredStatic() {
#ifdef SUPERVISION_CALLBACK
  if (supervisionCallbackRunnable != NULL) {
    // No need to supervise this call as this interrupt has a time limit.
    // When it expires, the system is restarted.
    supervisionCallbackRunnable->run();
  }
#endif

  Serial.println(F("watchdog reboot"));
  esp_restart();
}

/**
   Interrupt service routine called when the timer expires.
*/
void IRAM_ATTR isrWatchdogExpired() {
  Scheduler::isrWatchdogExpiredStatic();
}

void Scheduler::taskWdtEnable(const uint8_t value) {
  if (value != NO_SUPERVISION) {
    const unsigned long durationMs = wdtTimeoutToDurationMs(value);
    if (timer == NULL) {
      // div 80
      timer = timerBegin(ESP32_TASK_WDT_TIMER_NUMBER, 80, true);
      timerAttachInterrupt(timer, &isrWatchdogExpired, true);
    }
    //set time in us
    timerAlarmWrite(timer, durationMs * 1000, false);
    //enable interrupt
    //only works after taskWdtDisable() if yield() is done before
    yield();
    timerAlarmEnable(timer);
  } else {
    taskWdtDisable();
  }
}

void Scheduler::taskWdtDisable() {
  if (timer != NULL) {
    //disable interrupt
    timerAlarmDisable(timer);
    timerDetachInterrupt(timer);
    timerEnd(timer);
    timer = NULL;
  }
}

void Scheduler::taskWdtReset() {
  //reset timer (feed watchdog)
  if (timer != NULL) {
    timerWrite(timer, 0);
  }
}

#elif ESP8266
// -------------------------------------------------------------------------------------------------
unsigned long Scheduler::getMillis() const {
  // on ESP8266 we do not support sleep, so millis() stays correct.
  return millis();
}

void Scheduler::taskWdtEnable(const uint8_t value) {
  const unsigned long durationMs = wdtTimeoutToDurationMs(value);
  ESP.wdtEnable(durationMs);
}

void Scheduler::taskWdtDisable() {
  ESP.wdtDisable();
}

void Scheduler::taskWdtReset() {
  ESP.wdtFeed();
}
#endif
// -------------------------------------------------------------------------------------------------

inline unsigned long Scheduler::wdtTimeoutToDurationMs(const uint8_t value) {
  unsigned long durationMs;
  switch (value) {
    case TIMEOUT_15Ms: {
        durationMs = 15;
        break;
      }
    case TIMEOUT_30MS: {
        durationMs = 30;
        break;
      }
    case TIMEOUT_60MS: {
        durationMs = 60;
        break;
      }
    case TIMEOUT_120MS: {
        durationMs = 120;
        break;
      }
    case TIMEOUT_250MS: {
        durationMs = 250;
        break;
      }
    case TIMEOUT_500MS: {
        durationMs = 500;
        break;
      }
    case TIMEOUT_1S: {
        durationMs = 1000;
        break;
      }
    case TIMEOUT_2S: {
        durationMs = 2000;
        break;
      }
    case TIMEOUT_4S: {
        durationMs = 4000;
        break;
      }
    case TIMEOUT_8S: {
        durationMs = 8000;
        break;
      }
    default: {
        // should not happen
        durationMs = 15;
      }
  }
  return durationMs;
}

void Scheduler::sleepIfRequired() {
  noInterrupts();
  bool queueEmpty = first == NULL;
  interrupts();
  SleepMode sleepMode = IDLE;
  if (!queueEmpty) {
    sleepMode = evaluateSleepMode();
  } else {
    // nothing in the queue
    if (doesSleep()
#ifdef SLEEP_DELAY
        && millis() >= lastTaskFinishedMillis + SLEEP_DELAY
#endif
       ) {
      sleepMode = SLEEP;
    } else {
      sleepMode = IDLE;
    }
  }
  if (sleepMode != NO_SLEEP) {
#ifdef AWAKE_INDICATION_PIN
    digitalWrite(AWAKE_INDICATION_PIN, LOW);
#endif
    if (sleepMode == SLEEP) {
      taskWdtDisable();
      noInterrupts();
      unsigned long currentSchedulerMillis = getMillis();

      unsigned long firstScheduledUptimeMillis = 0;
      if (first != NULL) {
        firstScheduledUptimeMillis = first->scheduledUptimeMillis;
      }

      unsigned long maxWaitTimeMillis = 0;
      if (firstScheduledUptimeMillis > currentSchedulerMillis) {
        maxWaitTimeMillis = firstScheduledUptimeMillis - currentSchedulerMillis;
      }
      interrupts();

      sleep(maxWaitTimeMillis, queueEmpty);
    } else { // IDLE
      yield();
    }
    // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
#ifdef AWAKE_INDICATION_PIN
    digitalWrite(AWAKE_INDICATION_PIN, HIGH);
#endif
  }
}

inline Scheduler::SleepMode Scheduler::evaluateSleepMode() {
  noInterrupts();
  unsigned long currentSchedulerMillis = getMillis();

  unsigned long firstScheduledUptimeMillis = 0;
  if (first != NULL) {
    firstScheduledUptimeMillis = first->scheduledUptimeMillis;
  }
  interrupts();

  SleepMode sleepMode = NO_SLEEP;
  unsigned long maxWaitTimeMillis = 0;
  if (firstScheduledUptimeMillis > currentSchedulerMillis) {
    maxWaitTimeMillis = firstScheduledUptimeMillis - currentSchedulerMillis;
  }

  if (maxWaitTimeMillis == 0) {
    sleepMode = NO_SLEEP;
  } else if (!doesSleep() || maxWaitTimeMillis < BUFFER_TIME
#ifdef SLEEP_DELAY
             || millis() < lastTaskFinishedMillis + SLEEP_DELAY
#endif
            ) {
    // use IDLE for values less then BUFFER_TIME
    sleepMode = IDLE;
  } else {
    sleepMode = SLEEP;
  }
  return sleepMode;
}

#ifdef ESP32
// -------------------------------------------------------------------------------------------------
void Scheduler::sleep(unsigned long durationMs, bool queueEmpty) {
  bool timerWakeup;
  if (durationMs > 0) {
    esp_sleep_enable_timer_wakeup(durationMs * 1000L);
    timerWakeup = true;
  } else if (queueEmpty) {
#ifdef ESP_DEEP_SLEEP_FOR_INFINITE_SLEEP
    esp_deep_sleep_start(); // does not return
#endif
    timerWakeup = false;
  } else {
    // should not happen
    esp_sleep_enable_timer_wakeup(1);
    timerWakeup = true;
  }

  esp_light_sleep_start();

  if (timerWakeup) {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
  }
}
#elif ESP8266
// -------------------------------------------------------------------------------------------------
void Scheduler::sleep(unsigned long durationMs, bool queueEmpty) {
#ifdef ESP_DEEP_SLEEP_FOR_INFINITE_SLEEP
  if (queueEmpty) {
    ESP.deepSleep(0); // does not return
  }
#endif

  if (durationMs > ESP8266_MAX_DELAY_TIME_MS) {
    durationMs = ESP8266_MAX_DELAY_TIME_MS;
  }

  delay(durationMs);
  ESP.wdtFeed();
}
#endif
// -------------------------------------------------------------------------------------------------

#endif // #ifndef LIBCALL_DEEP_SLEEP_SCHEDULER

