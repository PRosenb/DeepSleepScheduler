#include <DeepSleepScheduler.h>

#define LED_PIN 13

void ledOn() {
  digitalWrite(LED_PIN, HIGH);
  scheduler.scheduleDelayed(ledOff, 1000);
}

void ledOff() {
  digitalWrite(LED_PIN, LOW);
  scheduler.scheduleDelayed(ledOn, 1000);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  scheduler.schedule(ledOn);
}

void loop() {
  scheduler.execute();
}

