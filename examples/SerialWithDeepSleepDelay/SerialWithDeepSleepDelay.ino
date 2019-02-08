
// show the awake times of the CPU on output LED_BUILTIN
#define AWAKE_INDICATION_PIN LED_BUILTIN
// delay sleep by 100 milli seconds to allow finishing serial write
#define SLEEP_DELAY 100
#include <DeepSleepScheduler.h>

void printMillis() {
  // usage of F() puts the strings to the flash and therefore saves main memory
  Serial.print(F("millis(): "));
  Serial.print(millis());
  Serial.print(F(", scheduler.getMillis(): "));
  Serial.println(scheduler.getMillis());

  // If you use around 1 second or less, millis() and scheduler.getMillis()
  // will be equal because the CPU is not put to SLEEP_MODE_PWR_DOWN in that case.
  scheduler.scheduleDelayed(printMillis, 2000);
}

void setup() {
  Serial.begin(115200);
  scheduler.schedule(printMillis);
}

void loop() {
  scheduler.execute();
}

