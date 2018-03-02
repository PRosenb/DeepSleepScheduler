#include <DeepSleepScheduler.h>

#ifdef ESP32
#define LED_PIN1 2
#define LED_PIN2 4
#else
#define LED_PIN1 12
#define LED_PIN2 13
#endif

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

  BlinkRunnable *blinkRunnable1 = new BlinkRunnable(LED_PIN1, 1000);
  BlinkRunnable *blinkRunnable2 = new BlinkRunnable(LED_PIN2, 500);
  scheduler.schedule(blinkRunnable1);
  scheduler.schedule(blinkRunnable2);
}

void loop() {
  scheduler.execute();
}

