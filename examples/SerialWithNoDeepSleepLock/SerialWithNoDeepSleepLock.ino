#define AWAKE_INDICATION_PIN 13
#include <DeepSleepScheduler.h>

void releaseNoDeepSleepLock() {
  scheduler.releaseNoDeepSleepLock();
}

void printMillis() {
  // usage of F() puts the strings to the flash and therefore saves main memory
  Serial.print(F("millis(): "));
  Serial.print(millis());
  Serial.print(F(", scheduler.getMillis(): "));
  Serial.println(scheduler.getMillis());

  // If you use around 1 second or less, millis() and scheduler.getMillis()
  // will be equal because the CPU is not put to SLEEP_MODE_PWR_DOWN in that case.
  scheduler.scheduleDelayed(printMillis, 2000);

  // Allow to finish serial before CPU is put to deep sleep.
  // Please make sure you also call releaseNoDeepSleepLock() to let the CPU sleep again.
  // You can see if it is properly done by checking if AWAKE_INDICATION_PIN is low some time
  // what indicates that the CPU is in deep sleep.
  // See example ShowSleep for more details.
  scheduler.acquireNoDeepSleepLock();
  scheduler.scheduleDelayed(releaseNoDeepSleepLock, 100);

  // can also be done with a small delay instead,
  // see example PrintMillis
  // delay(100);
}

void setup() {
  Serial.begin(9600);

  scheduler.schedule(printMillis);
}

void loop() {
  scheduler.execute();
}

