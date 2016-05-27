#include <DeepSleepScheduler.h>

#define INTERRUPT_PIN 2
#define LED_PIN 13

void ledOn() {
  digitalWrite(LED_PIN, HIGH);
  scheduler.scheduleDelayed(ledOff, 2000);
}

void ledOff() {
  digitalWrite(LED_PIN, LOW);
}

void isrInterruptPin() {
  scheduler.schedule(ledOn);
}

void setup() {
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), isrInterruptPin, FALLING);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  scheduler.execute();
}

