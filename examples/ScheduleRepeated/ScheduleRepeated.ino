#include <DeepSleepScheduler.h>

#ifdef ESP32
#define LED_PIN 2
#else
#define LED_PIN 13
#endif

void toggleLed() {
  if (digitalRead(LED_PIN) == HIGH) {
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
  scheduler.scheduleAt(toggleLed, scheduler.getScheduleTimeOfCurrentTask() + 1000);
}

void setup() {
#ifdef ESP32
  // ESP_PD_DOMAIN_RTC_PERIPH needs to be kept on
  // in order for the LED to stay on during sleep
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
#endif

  pinMode(LED_PIN, OUTPUT);
  scheduler.schedule(toggleLed);
}

void loop() {
  scheduler.execute();
}

