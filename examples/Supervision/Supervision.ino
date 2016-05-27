#include <DeepSleepScheduler.h>

#define LED_PIN 13

void block() {
  digitalWrite(LED_PIN, HIGH);
  while (1);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  scheduler.setTaskTimeout(TIMEOUT_2S);
  scheduler.scheduleDelayed(block, 1000);
}

void loop() {
  scheduler.execute();
}

