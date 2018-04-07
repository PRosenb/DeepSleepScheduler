#include <DeepSleepScheduler.h>

void block() {
  digitalWrite(LED_BUILTIN, HIGH);
  while (1);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  scheduler.setTaskTimeout(TIMEOUT_2S);
  scheduler.scheduleDelayed(block, 1000);
}

void loop() {
  scheduler.execute();
}

