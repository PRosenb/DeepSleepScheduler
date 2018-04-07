#include <DeepSleepScheduler.h>

void toggleLed() {
  if (digitalRead(LED_BUILTIN) == HIGH) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  scheduler.scheduleAt(toggleLed, scheduler.getScheduleTimeOfCurrentTask() + 1000);
}

void setup() {
#ifdef ESP32
  // ESP_PD_DOMAIN_RTC_PERIPH needs to be kept on
  // in order for the LED to stay on during sleep
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
#endif

  pinMode(LED_BUILTIN, OUTPUT);
  scheduler.schedule(toggleLed);
}

void loop() {
  scheduler.execute();
}

