
// -------------------------------------------------------------------------------------------------
// Definition of AVR, included inside of class Scheduler
// -------------------------------------------------------------------------------------------------

// values changeable by the user
#ifndef SLEEP_MODE
#define SLEEP_MODE SLEEP_MODE_PWR_DOWN
#endif

#ifndef SUPERVISION_CALLBACK_TIMEOUT
#define SUPERVISION_CALLBACK_TIMEOUT WDTO_1S
#endif

#ifndef MIN_WAIT_TIME_FOR_SLEEP
#define MIN_WAIT_TIME_FOR_SLEEP SLEEP_TIME_1S
#endif

#ifndef SLEEP_TIME_15MS_CORRECTION
#define SLEEP_TIME_15MS_CORRECTION 3
#endif
#ifndef SLEEP_TIME_30MS_CORRECTION
#define SLEEP_TIME_30MS_CORRECTION 4
#endif
#ifndef SLEEP_TIME_60MS_CORRECTION
#define SLEEP_TIME_60MS_CORRECTION 7
#endif
#ifndef SLEEP_TIME_120MS_CORRECTION
#define SLEEP_TIME_120MS_CORRECTION 13
#endif
#ifndef SLEEP_TIME_250MS_CORRECTION
#define SLEEP_TIME_250MS_CORRECTION 15
#endif
#ifndef SLEEP_TIME_500MS_CORRECTION
#define SLEEP_TIME_500MS_CORRECTION 28
#endif
#ifndef SLEEP_TIME_1S_CORRECTION
#define SLEEP_TIME_1S_CORRECTION 54
#endif
#ifndef SLEEP_TIME_2S_CORRECTION
#define SLEEP_TIME_2S_CORRECTION 106
#endif
#ifndef SLEEP_TIME_4S_CORRECTION
#define SLEEP_TIME_4S_CORRECTION 209
#endif
#ifndef SLEEP_TIME_8S_CORRECTION
#define SLEEP_TIME_8S_CORRECTION 415
#endif

// Constants
// =========
#define SLEEP_TIME_15MS 15 + SLEEP_TIME_15MS_CORRECTION
#define SLEEP_TIME_30MS 30 + SLEEP_TIME_30MS_CORRECTION
#define SLEEP_TIME_60MS 60 + SLEEP_TIME_60MS_CORRECTION
#define SLEEP_TIME_120MS 120 + SLEEP_TIME_120MS_CORRECTION
#define SLEEP_TIME_250MS 250 + SLEEP_TIME_250MS_CORRECTION
#define SLEEP_TIME_500MS 500 + SLEEP_TIME_500MS_CORRECTION
#define SLEEP_TIME_1S 1000 + SLEEP_TIME_1S_CORRECTION
#define SLEEP_TIME_2S 2000 + SLEEP_TIME_2S_CORRECTION
#define SLEEP_TIME_4S 4000 + SLEEP_TIME_4S_CORRECTION
#define SLEEP_TIME_8S 8000 + SLEEP_TIME_8S_CORRECTION

private:
void init();
public:
/**
  Do not call this method, it is used by the watchdog interrupt.
*/
static void isrWdt();
private:
// variables used in the interrupt
static volatile unsigned int wdtSleepTimeMillis;
static volatile unsigned long millisInDeepSleep;
static volatile unsigned long millisBeforeDeepSleep;
/**
   Stores the time of the task from which the sleep time of the WDT is
   calculated when it is put to sleep.
   In case an interrupt schedules a new time, this time is compared against
   it to check if the new time is before the WDT would wake up anyway.
*/
unsigned long firstRegularlyScheduledUptimeAfterSleep;

void taskWdtEnable(const uint8_t value);
inline void taskWdtDisable();
inline void sleepIfRequired();
bool isWakeupByOtherInterrupt();

void wdtEnableInterrupt();
inline SleepMode evaluateSleepModeAndEnableWdtIfRequired();
inline unsigned long wdtEnableForSleep(unsigned long maxWaitTimeMillis);

