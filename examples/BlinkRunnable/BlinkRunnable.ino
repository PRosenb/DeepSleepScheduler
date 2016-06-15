#include <DeepSleepScheduler.h>

#define LED_PIN1 12
#define LED_PIN2 13

class BlinkRunnable: public Runnable {
  private:
    bool ledOn = true;
    const byte ledPin;
    const int delay;
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
  BlinkRunnable *blinkRunnable1 = new BlinkRunnable(LED_PIN1, 1000);
  BlinkRunnable *blinkRunnable2 = new BlinkRunnable(LED_PIN2, 500);
  scheduler.schedule(blinkRunnable1);
  scheduler.schedule(blinkRunnable2);
}

void loop() {
  scheduler.execute();
}

