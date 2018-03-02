#include <DeepSleepScheduler.h>

#ifdef ESP32
#define LED_PIN 2
#else
#define LED_PIN 13
#endif

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

