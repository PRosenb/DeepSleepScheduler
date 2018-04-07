#include <DeepSleepScheduler.h>

#ifdef ESP32
#define INTERRUPT_PIN 4
#else
#define INTERRUPT_PIN 2
#endif

void ledOn() {
  digitalWrite(LED_BUILTIN, HIGH);
  scheduler.scheduleDelayed(ledOff, 2000);
}

void ledOff() {
  digitalWrite(LED_BUILTIN, LOW);
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
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  scheduler.execute();
}

