#include <DeepSleepScheduler.h>

#define LED_PIN 13

void printMillis() {
  digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);

  // usage of F() puts the strings to the flash and therefore saves main memory
  Serial.print(F("millis(): "));
  Serial.print(millis());
  Serial.print(F(", scheduler.getMillis(): "));
  Serial.println(scheduler.getMillis());

  // If you use around 1 second or less, millis() and scheduler.getMillis()
  // will be equal because the CPU is not put to SLEEP_MODE_PWR_DOWN in that case.
  scheduler.scheduleDelayed(printMillis, 2000);

  // allow to finish serial before CPU is put to sleep
  delay(100);
  // can also be with NoDeepSleepLock, see example SerialWithNoDeepSleepLock
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);

  scheduler.schedule(printMillis);
}

void loop() {
  scheduler.execute();
}

