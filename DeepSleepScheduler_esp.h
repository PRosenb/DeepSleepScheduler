
#include <esp_sleep.h>
#include <esp32-hal-timer.h>

// -------------------------------------------------------------------------------------------------
// Definition (usually in H file)
// -------------------------------------------------------------------------------------------------

class SchedulerEsp: public Scheduler {
  public:
    /**
        Do not call this method, it is used by the watchdog interrupt.
    */
    static void isrWatchdogExpiredStatic();
  private:
    hw_timer_t *timer = NULL;

    virtual void taskWdtEnable(const uint8_t value);
    virtual void taskWdtDisable();
    virtual void taskWdtReset();
    virtual void sleepIfRequired();
    virtual bool isOurWakeupInterrupt();
    inline SleepMode evaluateSleepMode();
};

// -------------------------------------------------------------------------------------------------
// Implementation (usuallly in CPP file)
// -------------------------------------------------------------------------------------------------
/**
   the one and only instance of Scheduler
*/
SchedulerEsp scheduler;

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

  //timer 0, div 80
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &isrWatchdogExpired, true);
  //set time in us
  timerAlarmWrite(timer, durationMs * 1000, false);
  //enable interrupt
  timerAlarmEnable(timer);
}

void SchedulerEsp::taskWdtDisable() {
  if (timer != NULL) {
    //disable interrupt
    timerDetachInterrupt(timer);
    timerEnd(timer);
    timer = NULL;
  }
}

inline void SchedulerEsp::taskWdtReset() {
  //reset timer (feed watchdog)
  if (timer != NULL) {
    timerWrite(timer, 0);
  }
}

bool SchedulerEsp::isOurWakeupInterrupt() {
  esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
  return wakeupCause == ESP_SLEEP_WAKEUP_TIMER;
}

inline void SchedulerEsp::sleepIfRequired() {
  noInterrupts();
  bool queueEmpty = first == NULL;
  interrupts();
  SleepMode sleepMode = IDLE;
  if (!queueEmpty) {
    sleepMode = evaluateSleepMode();
  } else {
    // nothing in the queue
    if (doesDeepSleep()
#ifdef DEEP_SLEEP_DELAY
        && millis() >= lastTaskFinishedMillis + DEEP_SLEEP_DELAY
#endif
       ) {
      //      wdt_disable();
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

      if (maxWaitTimeMillis > 0) {
        esp_sleep_enable_timer_wakeup(maxWaitTimeMillis * 1000L);
      } else {
        if (queueEmpty) {
          // workaround for bug
          esp_sleep_enable_timer_wakeup(UINT64_MAX);

#ifdef ESP_DEEP_SLEEP_FOR_INFINITE_SLEEP
          esp_deep_sleep_start(); // does not return
#endif
        } else {
          // should not happen
          esp_sleep_enable_timer_wakeup(1);
        }
      }
      esp_light_sleep_start();

      // correct millisInDeepSleep after wake up
      millisInDeepSleep += maxWaitTimeMillis;
    } else { // IDLE
      yield();
    }
    // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
#ifdef AWAKE_INDICATION_PIN
    digitalWrite(AWAKE_INDICATION_PIN, HIGH);
#endif
  }
}

Scheduler::SleepMode SchedulerEsp::evaluateSleepMode() {
  noInterrupts();
  unsigned long currentSchedulerMillis = getMillis();

  unsigned long firstScheduledUptimeMillis = 0;
  if (first != NULL) {
    firstScheduledUptimeMillis = first->scheduledUptimeMillis;
  }
  interrupts();

  SleepMode sleepMode = NO_SLEEP;
  if (isOurWakeupInterrupt()) {
    // not woken up during sleep

    unsigned long maxWaitTimeMillis = 0;
    if (firstScheduledUptimeMillis > currentSchedulerMillis) {
      maxWaitTimeMillis = firstScheduledUptimeMillis - currentSchedulerMillis;
    }

    if (maxWaitTimeMillis == 0) {
      sleepMode = NO_SLEEP;
    } else if (!doesDeepSleep() || maxWaitTimeMillis < BUFFER_TIME
#ifdef DEEP_SLEEP_DELAY
               || millis() < lastTaskFinishedMillis + DEEP_SLEEP_DELAY
#endif
              ) {
      // use IDLE for values less then BUFFER_TIME
      sleepMode = IDLE;
    } else {
      sleepMode = SLEEP;
      firstRegularlyScheduledUptimeAfterSleep = firstScheduledUptimeMillis;

      noInterrupts();
      millisBeforeDeepSleep = millis();
      interrupts();
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

