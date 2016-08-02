#include <DeepSleepScheduler.h>

#define LED_PIN 13

void toggleLed() {
  if (digitalRead(LED_PIN) == HIGH) {
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
  scheduler.scheduleAt(toggleLed, scheduler.getScheduleTimeOfCurrentTask() + 1000);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  scheduler.schedule(toggleLed);
}

void loop() {
  scheduler.execute();
}

