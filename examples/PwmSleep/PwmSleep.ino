// do not use SLEEP_MODE_PWR_DOWN to prevent that
// the asynchronous clock is stopped
#define SLEEP_MODE SLEEP_MODE_PWR_SAVE
// show then the CPU is active on PIN 13
#define AWAKE_INDICATION_PIN 13
#include <DeepSleepScheduler.h>

// the PWM signal can only be used in sleep mode on
// Arduino UNO PINs 3 and 11 because only these two
// use an asynchronous clock that is not stopped
// with SLEEP_MODE_PWR_SAVE
#define PWN_PIN 3

void highValue() {
  analogWrite(PWN_PIN, 200);
  scheduler.scheduleDelayed(lowValue, 5000);
}

void lowValue() {
  analogWrite(PWN_PIN, 100);
  scheduler.scheduleDelayed(highValue, 5000);
}

void setup() {
  pinMode(PWN_PIN, OUTPUT);
  scheduler.schedule(lowValue);
}

void loop() {
  scheduler.execute();
}

