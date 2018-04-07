#include <DeepSleepScheduler.h>

#ifdef ESP32
#include <esp_sleep.h>
#endif

void ledOn() {
  digitalWrite(LED_BUILTIN, HIGH);
  scheduler.scheduleDelayed(ledOff, 1000);
}

void ledOff() {
  digitalWrite(LED_BUILTIN, LOW);
  scheduler.scheduleDelayed(ledOn, 1000);
}

void setup() {
#ifdef ESP32
  // ESP_PD_DOMAIN_RTC_PERIPH needs to be kept on
  // in order for the LED to stay on during sleep
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
#endif

  pinMode(LED_BUILTIN, OUTPUT);
  scheduler.schedule(ledOn);
}

void loop() {
  scheduler.execute();
}

