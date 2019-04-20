
// -------------------------------------------------------------------------------------------------
// Definition of ESP, included inside of class Scheduler
// -------------------------------------------------------------------------------------------------
#ifndef ESP32_TASK_WDT_TIMER_NUMBER
#define ESP32_TASK_WDT_TIMER_NUMBER 3
#endif

#ifdef ESP32
// ---------------------------------------------------------------------------------------------
public:

/**
    Do not call this method, it is used by the watchdog interrupt.
*/
static void isrWatchdogExpiredStatic();
private:
hw_timer_t *timer = NULL;
#elif ESP8266
public:
unsigned long getMillis() const {
  // on ESP8266 we do not support sleep, so millis() stays correct.
  return millis();
}
#endif
// ---------------------------------------------------------------------------------------------

private:
void taskWdtEnable(const uint8_t value);
void taskWdtDisable();
void taskWdtReset();
inline unsigned long wdtTimeoutToDurationMs(const uint8_t value);
void sleepIfRequired();
inline void sleep(unsigned long durationMs, bool queueEmpty);
inline SleepMode evaluateSleepMode();

// unused here, only used for AVR
bool isWakeupByOtherInterrupt() {
  return false;
}
void wdtEnableInterrupt() {}

