/*
   Shows when the CPU is in sleep mode (LED off) and active (LED on).
   Please not that the CPU is not put to deep sleep for schedule times
   shorter than 1 second. For these, it uses SLEEP_MODE_IDLE.

   You will notice, that it's not on in full brightness because
   the CPU is sleeping shortly almost all the time when it is set
   to SLEEP_MODE_IDLE. In that case, it wakes up on every timer tick
   what is every around 1 ms. It also wakes up on all kinds of other
   interrupts (see CPU description for more details).

   It can further be seen, that it's not off for the whole time it's
   set to be in deep sleep. That is because only part of the schedule
   time is in deep sleep. About one second before a schedule callback,
   it wakes up to handle that callback.
*/

#define AWAKE_INDICATION_PIN LED_BUILTIN
#include <DeepSleepScheduler.h>

void keepCpuOn() {
  // As many times as aquireNoSleepLock() is called, we also need
  // to call releaseNoSleepLock() to let the CPU fall into deep sleep
  // again.
  scheduler.acquireNoSleepLock();
  scheduler.scheduleDelayed(allowCpuToSleep, 3000);
}

void allowCpuToSleep() {
  scheduler.releaseNoSleepLock();
  scheduler.scheduleDelayed(keepCpuOn, 3000);
}

void setup() {
  scheduler.schedule(keepCpuOn);
}

void loop() {
  scheduler.execute();
}

