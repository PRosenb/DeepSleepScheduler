
// The watchdog timer can be configured to sleep from 15ms to 8s at a time.
// As the sleep times differ slighly from the directed times, the scheduler
// adds the correction values to improve the callback times.
// Example:
// When the CPU is supposed to sleep 15ms but it sleeps 3ms too long (18ms),
// you configure
// #define SLEEP_TIME_15MS_CORRECTION 3
// The sleep time will then be calculated as 15ms + 3ms = 18ms.
// See example WdtTiemsMeasurer.

// Please be aware that the CPU is not set to SLEEP_MODE_PWR_DOWN
// when the total sleep time is less than about a second.

#define SLEEP_TIME_15MS_CORRECTION 3
#define SLEEP_TIME_30MS_CORRECTION 4
#define SLEEP_TIME_60MS_CORRECTION 7
#define SLEEP_TIME_120MS_CORRECTION 13
#define SLEEP_TIME_250MS_CORRECTION 15
#define SLEEP_TIME_500MS_CORRECTION 28
#define SLEEP_TIME_1S_CORRECTION 54
#define SLEEP_TIME_2S_CORRECTION 106
#define SLEEP_TIME_4S_CORRECTION 209
#define SLEEP_TIME_8S_CORRECTION 415

#include <DeepSleepScheduler.h>

#define LED_PIN 13

void ledOn() {
  digitalWrite(LED_PIN, HIGH);
  scheduler.scheduleDelayed(ledOff, 5000);
}

void ledOff() {
  digitalWrite(LED_PIN, LOW);
  scheduler.scheduleDelayed(ledOn, 5000);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  scheduler.schedule(ledOn);
}

void loop() {
  scheduler.execute();
}

