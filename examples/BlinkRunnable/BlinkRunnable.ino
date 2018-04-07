#include <DeepSleepScheduler.h>

class BlinkRunnable: public Runnable {
  private:
    bool ledOn = true;
    const byte ledPin;
    const unsigned long delay;
  public:
    BlinkRunnable(byte ledPin, int delay) : ledPin(ledPin), delay(delay) {
      pinMode(ledPin, OUTPUT);
    }
    virtual void run() {
      if (ledOn) {
        ledOn = false;
        digitalWrite(ledPin, HIGH);
      } else {
        ledOn = true;
        digitalWrite(ledPin, LOW);
      }
      scheduler.scheduleDelayed(this, delay);
    }
};

void setup() {
#ifdef ESP32
  // ESP_PD_DOMAIN_RTC_PERIPH needs to be kept on
  // in order for the LED to stay on during sleep
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
#endif

  BlinkRunnable *blinkRunnable = new BlinkRunnable(LED_BUILTIN, 1000);
  scheduler.schedule(blinkRunnable);
}

void loop() {
  scheduler.execute();
}

