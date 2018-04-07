
#ifdef ESP32
#include <esp_sleep.h>
#include <esp32-hal-timer.h>
#include <soc/rtc.h>
#endif

// -------------------------------------------------------------------------------------------------
// Definition (usually in H file)
// -------------------------------------------------------------------------------------------------

class SchedulerEsp: public Scheduler {
#ifdef ESP32
    // ---------------------------------------------------------------------------------------------
  public:
    virtual unsigned long getMillis() const {
      // https://forum.makehackvoid.com/t/playing-with-the-esp-32/1144/11
      uint64_t rtcTime = rtc_time_get();
      uint64_t rtcTimeUs = rtcTime * 20 / 3;  // ticks -> us 1,000,000/150,000
      return rtcTimeUs / 1000;
    }

    /**
        Do not call this method, it is used by the watchdog interrupt.
    */
    static void isrWatchdogExpiredStatic();
  private:
    hw_timer_t *timer = NULL;
#elif ESP8266
  public:
    virtual unsigned long getMillis() const {
      // on ESP8266 we do not support sleep, so millis() stays correct.
      return millis();
    }
#endif
    // ---------------------------------------------------------------------------------------------

  private:
    virtual void taskWdtEnable(const uint8_t value);
    virtual void taskWdtDisable();
    virtual void taskWdtReset();
    inline unsigned long wdtTimeoutToDurationMs(const uint8_t value);
    virtual void sleepIfRequired();
    inline void sleep(unsigned long durationMs, bool queueEmpty);
    virtual bool isWakeupByOtherInterrupt();
    inline SleepMode evaluateSleepMode();
};

extern SchedulerEsp scheduler;

#ifndef LIBCALL_DEEP_SLEEP_SCHEDULER
// -------------------------------------------------------------------------------------------------
// Implementation (usuallly in CPP file)
// -------------------------------------------------------------------------------------------------
/**
   the one and only instance of Scheduler
*/
SchedulerEsp scheduler;

#ifdef ESP32
// -------------------------------------------------------------------------------------------------
void SchedulerEsp::isrWatchdogExpiredStatic() {
#ifdef SUPERVISION_CALLBACK
  if (supervisionCallbackRunnable != NULL) {
    // No need to supervise this call as this interrupt has a time limit.
    // When it expires, the system is restarted.
    supervisionCallbackRunnable->run();
  }
#endif

  Serial.println(F("watchdog reboot"));
  esp_restart_noos();
}

/**
   Interrupt service routine called when the timer expires.
*/
void IRAM_ATTR isrWatchdogExpired() {
  SchedulerEsp::isrWatchdogExpiredStatic();
}

void SchedulerEsp::taskWdtEnable(const uint8_t value) {
  if (value != NO_SUPERVISION) {
    const unsigned long durationMs = wdtTimeoutToDurationMs(value);
    if (timer == NULL) {
      //timer 0, div 80
      timer = timerBegin(0, 80, true);
      timerAttachInterrupt(timer, &isrWatchdogExpired, true);
    }
    //set time in us
    timerAlarmWrite(timer, durationMs * 1000, false);
    //enable interrupt
    timerAlarmEnable(timer);
  } else {
    taskWdtDisable();
  }
}

void SchedulerEsp::taskWdtDisable() {
  if (timer != NULL) {
    //disable interrupt
    timerDetachInterrupt(timer);
    timerEnd(timer);
    timer = NULL;
  }
}

void SchedulerEsp::taskWdtReset() {
  //reset timer (feed watchdog)
  if (timer != NULL) {
    timerWrite(timer, 0);
  }
}

bool SchedulerEsp::isWakeupByOtherInterrupt() {
  esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
  return wakeupCause != 0 && wakeupCause != ESP_SLEEP_WAKEUP_TIMER;
}

#elif ESP8266
// -------------------------------------------------------------------------------------------------
void SchedulerEsp::taskWdtEnable(const uint8_t value) {
  const unsigned long durationMs = wdtTimeoutToDurationMs(value);
  ESP.wdtEnable(durationMs);
}

void SchedulerEsp::taskWdtDisable() {
  ESP.wdtDisable();
}

void SchedulerEsp::taskWdtReset() {
  ESP.wdtFeed();
}

bool SchedulerEsp::isWakeupByOtherInterrupt() {
  // TODO support for ESP8266
  return false;
}
#endif
// -------------------------------------------------------------------------------------------------

inline unsigned long SchedulerEsp::wdtTimeoutToDurationMs(const uint8_t value) {
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
  }
  return durationMs;
}

void SchedulerEsp::sleepIfRequired() {
  noInterrupts();
  bool queueEmpty = first == NULL;
  interrupts();
  SleepMode sleepMode = IDLE;
  if (!queueEmpty) {
    sleepMode = evaluateSleepMode();
  } else {
    // nothing in the queue
    if (doesSleep()
#ifdef DEEP_SLEEP_DELAY
        && millis() >= lastTaskFinishedMillis + DEEP_SLEEP_DELAY
#endif
       ) {
      taskWdtDisable();
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

inline Scheduler::SleepMode SchedulerEsp::evaluateSleepMode() {
  noInterrupts();
  unsigned long currentSchedulerMillis = getMillis();

  unsigned long firstScheduledUptimeMillis = 0;
  if (first != NULL) {
    firstScheduledUptimeMillis = first->scheduledUptimeMillis;
  }
  interrupts();

  SleepMode sleepMode = NO_SLEEP;
  if (!isWakeupByOtherInterrupt()) {
    // not woken up during sleep

    unsigned long maxWaitTimeMillis = 0;
    if (firstScheduledUptimeMillis > currentSchedulerMillis) {
      maxWaitTimeMillis = firstScheduledUptimeMillis - currentSchedulerMillis;
    }

    if (maxWaitTimeMillis == 0) {
      sleepMode = NO_SLEEP;
    } else if (!doesSleep() || maxWaitTimeMillis < BUFFER_TIME
#ifdef DEEP_SLEEP_DELAY
               || millis() < lastTaskFinishedMillis + DEEP_SLEEP_DELAY
#endif
              ) {
      // use IDLE for values less then BUFFER_TIME
      sleepMode = IDLE;
    } else {
      sleepMode = SLEEP;
      firstRegularlyScheduledUptimeAfterSleep = firstScheduledUptimeMillis;
    }
  } else {
    // we woke up due to an other interrupt.
    // continue sleepting
    sleepMode = SLEEP;

    // A special case is when the other interrupt scheduled a task between now and before the WDT interrupt occurs.
    // In this case, we prevent sleeping until it is scheduled.
    // If the WDT interrupt occurs before that, it is executed earlier as expected because getMillis() will be
    // corrected when the WTD occurs.

    if (firstScheduledUptimeMillis < firstRegularlyScheduledUptimeAfterSleep) {
      sleepMode = IDLE;
    } else {
#ifdef DEEP_SLEEP_DELAY
      // The CPU was woken up by an interrupt other than WDT.
      // The interrupt may have scheduled a task to run immediatelly. In that case we delay deep sleep.
      if (millis() < lastTaskFinishedMillis + DEEP_SLEEP_DELAY) {
        sleepMode = IDLE;
      } else {
        sleepMode = SLEEP;
      }
#endif
    }
  }
  return sleepMode;
}

#ifdef ESP32
// -------------------------------------------------------------------------------------------------
void SchedulerEsp::sleep(unsigned long durationMs, bool queueEmpty) {
  if (durationMs > 0) {
    esp_sleep_enable_timer_wakeup(durationMs * 1000L);
  } else if (queueEmpty) {
    // workaround for bug
    esp_sleep_enable_timer_wakeup(UINT64_MAX);

#ifdef ESP_DEEP_SLEEP_FOR_INFINITE_SLEEP
    esp_deep_sleep_start(); // does not return
#endif
  } else {
    // should not happen
    esp_sleep_enable_timer_wakeup(1);
  }

  esp_light_sleep_start();
}
#elif ESP8266
// -------------------------------------------------------------------------------------------------
void SchedulerEsp::sleep(unsigned long durationMs, bool queueEmpty) {
#ifdef ESP_DEEP_SLEEP_FOR_INFINITE_SLEEP
  if (queueEmpty) {
    ESP.deepSleep(0); // does not return
  }
#endif
  delay(durationMs);
}
#endif
// -------------------------------------------------------------------------------------------------

#endif // #ifndef LIBCALL_DEEP_SLEEP_SCHEDULER
