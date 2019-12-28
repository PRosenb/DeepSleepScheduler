
#include <avr/sleep.h>
#include <avr/wdt.h>

#ifndef LIBCALL_DEEP_SLEEP_SCHEDULER
// -------------------------------------------------------------------------------------------------
// Implementation (usuallly in CPP file)
// -------------------------------------------------------------------------------------------------

volatile unsigned int Scheduler::wdtSleepTimeMillis;
volatile unsigned long Scheduler::millisInDeepSleep;
volatile unsigned long Scheduler::millisBeforeDeepSleep;

void Scheduler::init() {
  wdtSleepTimeMillis = 0;
  millisInDeepSleep = 0;
  millisBeforeDeepSleep = 0;
  firstRegularlyScheduledUptimeAfterSleep = 0;
}

unsigned long Scheduler::getMillis() const {
  unsigned long value;
  noInterrupts();
  value = millis() + millisInDeepSleep;
  interrupts();
  return value;
}

void Scheduler::taskWdtEnable(const uint8_t value) {
  wdt_enable(value);
}

void Scheduler::taskWdtDisable() {
  wdt_disable();
}

void Scheduler::taskWdtReset() {
  wdt_reset();
}

bool Scheduler::isWakeupByOtherInterrupt() {
  noInterrupts();
  unsigned long wdtSleepTimeMillisLocal = wdtSleepTimeMillis;
  interrupts();
  return wdtSleepTimeMillisLocal != 0;
}

void Scheduler::sleepIfRequired() {
  // Enable sleep bit with sleep_enable() before the sleep time evaluation because it can happen
  // that the WDT interrupt occurs during sleep time evaluation but before the CPU
  // sleeps. In that case, the WDT interrupt clears the sleep bit and the CPU will not sleep
  // but continue execution immediatelly.
  sleep_enable(); // enables the sleep bit, a safety pin
  noInterrupts();
  bool queueEmpty = first == NULL;
  interrupts();
  SleepMode sleepMode = IDLE;
  if (!queueEmpty) {
    sleepMode = evaluateSleepModeAndEnableWdtIfRequired();
  } else {
    // nothing in the queue
    if (doesSleep()
#ifdef SLEEP_DELAY
        && millis() >= lastTaskFinishedMillis + SLEEP_DELAY
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
    byte adcsraSave = 0;
    if (sleepMode == SLEEP) {
      noInterrupts();
      set_sleep_mode(SLEEP_MODE);
      adcsraSave = ADCSRA;
      ADCSRA = 0;  // disable ADC
      // turn off brown-out in software
#if defined(BODS) && defined(BODSE)
      sleep_bod_disable();
#endif
      interrupts ();             // guarantees next instruction executed
      sleep_cpu(); // here the device is actually put to sleep
    } else { // IDLE
      set_sleep_mode(SLEEP_MODE_IDLE);
      sleep_cpu(); // here the device is actually put to sleep
    }
    // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
#ifdef AWAKE_INDICATION_PIN
    digitalWrite(AWAKE_INDICATION_PIN, HIGH);
#endif
    if (adcsraSave != 0) {
      // re-enable ADC
      ADCSRA = adcsraSave;
    }
  }
  sleep_disable();
}

inline Scheduler::SleepMode Scheduler::evaluateSleepModeAndEnableWdtIfRequired() {
  noInterrupts();
  unsigned long wdtSleepTimeMillisLocal = wdtSleepTimeMillis;
  unsigned long currentSchedulerMillis = getMillis();

  unsigned long firstScheduledUptimeMillis = 0;
  if (first != NULL) {
    firstScheduledUptimeMillis = first->scheduledUptimeMillis;
  }
  interrupts();

  SleepMode sleepMode = NO_SLEEP;
  if (!isWakeupByOtherInterrupt()) {
    // not woken up during WDT sleep

    unsigned long maxWaitTimeMillis = 0;
    if (firstScheduledUptimeMillis > currentSchedulerMillis) {
      maxWaitTimeMillis = firstScheduledUptimeMillis - currentSchedulerMillis;
    }

    if (maxWaitTimeMillis == 0) {
      sleepMode = NO_SLEEP;
    } else if (!doesSleep() || maxWaitTimeMillis < MIN_WAIT_TIME_FOR_SLEEP + BUFFER_TIME
#ifdef SLEEP_DELAY
               || millis() < lastTaskFinishedMillis + SLEEP_DELAY
#endif
              ) {
      // use SLEEP_MODE_IDLE for values less then MIN_WAIT_TIME_FOR_SLEEP
      sleepMode = IDLE;
    } else {
      sleepMode = SLEEP;
      firstRegularlyScheduledUptimeAfterSleep = firstScheduledUptimeMillis;

      wdtSleepTimeMillisLocal = wdtEnableForSleep(maxWaitTimeMillis);

      noInterrupts();
      wdtSleepTimeMillis = wdtSleepTimeMillisLocal;
      wdtEnableInterrupt();
      millisBeforeDeepSleep = millis();
      interrupts();
    }
  } else {
    // wdt already running, so we woke up due to an other interrupt then WDT.
    // continue sleepting without enabling wdt again
    sleepMode = SLEEP;
    wdtEnableInterrupt();
    // A special case is when the other interrupt scheduled a task between now and before the WDT interrupt occurs.
    // In this case, we prevent SLEEP_MODE_PWR_DOWN until it is scheduled.
    // If the WDT interrupt occurs before that, it is executed earlier as expected because getMillis() will be
    // corrected when the WTD occurs.

    if (firstScheduledUptimeMillis < firstRegularlyScheduledUptimeAfterSleep) {
      sleepMode = IDLE;
    } else {
#ifdef SLEEP_DELAY
      // The CPU was woken up by an interrupt other than WDT.
      // The interrupt may have scheduled a task to run immediatelly. In that case we delay deep sleep.
      if (millis() < lastTaskFinishedMillis + SLEEP_DELAY) {
        sleepMode = IDLE;
      } else {
        sleepMode = SLEEP;
      }
#endif
    }
  }
  return sleepMode;
}

inline unsigned long Scheduler::wdtEnableForSleep(const unsigned long maxWaitTimeMillis) {
  unsigned long wdtSleepTimeMillis;
  if (maxWaitTimeMillis >= SLEEP_TIME_8S + BUFFER_TIME) {
    wdtSleepTimeMillis = SLEEP_TIME_8S;
    wdt_enable(WDTO_8S);
  } else if (maxWaitTimeMillis >= SLEEP_TIME_4S + BUFFER_TIME) {
    wdtSleepTimeMillis = SLEEP_TIME_4S;
    wdt_enable(WDTO_4S);
  } else if (maxWaitTimeMillis >= SLEEP_TIME_2S + BUFFER_TIME) {
    wdtSleepTimeMillis = SLEEP_TIME_2S;
    wdt_enable(WDTO_2S);
  } else if (maxWaitTimeMillis >= SLEEP_TIME_1S + BUFFER_TIME) {
    wdtSleepTimeMillis = SLEEP_TIME_1S;
    wdt_enable(WDTO_1S);
  } else if (maxWaitTimeMillis >= SLEEP_TIME_500MS + BUFFER_TIME) {
    wdtSleepTimeMillis = SLEEP_TIME_500MS;
    wdt_enable(WDTO_500MS);
  } else if (maxWaitTimeMillis >= SLEEP_TIME_250MS + BUFFER_TIME) {
    wdtSleepTimeMillis = SLEEP_TIME_250MS;
    wdt_enable(WDTO_250MS);
  } else if (maxWaitTimeMillis >= SLEEP_TIME_120MS + BUFFER_TIME) {
    wdtSleepTimeMillis = SLEEP_TIME_120MS;
    wdt_enable(WDTO_120MS);
  } else if (maxWaitTimeMillis >= SLEEP_TIME_60MS + BUFFER_TIME) {
    wdtSleepTimeMillis = SLEEP_TIME_60MS;
    wdt_enable(WDTO_60MS);
  } else if (maxWaitTimeMillis >= SLEEP_TIME_30MS + BUFFER_TIME) {
    wdtSleepTimeMillis = SLEEP_TIME_30MS;
    wdt_enable(WDTO_30MS);
  } else { // maxWaitTimeMs >= 17
    wdtSleepTimeMillis = SLEEP_TIME_15MS;
    wdt_enable(WDTO_15MS);
  }
  return wdtSleepTimeMillis;
}

void Scheduler::isrWdt() {
  sleep_disable();
  millisInDeepSleep += wdtSleepTimeMillis;
  millisInDeepSleep -= millis() - millisBeforeDeepSleep;
#ifdef SUPERVISION_CALLBACK
  const unsigned int wdtSleepTimeMillisBefore = wdtSleepTimeMillis;
#endif
  wdtSleepTimeMillis = 0;
#ifdef SUPERVISION_CALLBACK
  if (wdtSleepTimeMillisBefore == 0 && supervisionCallbackRunnable != NULL) {
    wdt_reset();
    // give the callback some time but reset if it fails
    wdt_enable(SUPERVISION_CALLBACK_TIMEOUT);
    supervisionCallbackRunnable->run();
    // trigger restart
    wdt_enable(WDTO_15MS);
    while (1);
  }
#endif
}

/**
  first timeout will be the interrupt, second system reset
*/
void Scheduler::wdtEnableInterrupt() {
  // http://forum.arduino.cc/index.php?topic=108870.0
#if defined( __AVR_ATtiny25__ ) || defined( __AVR_ATtiny45__ ) || defined( __AVR_ATtiny85__ ) || defined( __AVR_ATtiny87__ ) || defined( __AVR_ATtiny167__ )
  WDTCR |= (1 << WDCE) | (1 << WDIE);
#else
  WDTCSR |= (1 << WDCE) | (1 << WDIE);
#endif
}

ISR (WDT_vect) {
  // WDIE & WDIF is cleared in hardware upon entering this ISR
  Scheduler::isrWdt();
}

#endif // #ifndef LIBCALL_DEEP_SLEEP_SCHEDULER
