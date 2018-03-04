#include <DeepSleepScheduler.h>

#ifdef ESP32
#define INTERRUPT_PIN 4
#define LED_PIN 2
#else
#define INTERRUPT_PIN 2
#define LED_PIN 13
#endif

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
#ifdef ESP32
  // wake up using ext0 (4 low)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, LOW);
#endif

  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), isrInterruptPin, FALLING);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  scheduler.execute();
}

