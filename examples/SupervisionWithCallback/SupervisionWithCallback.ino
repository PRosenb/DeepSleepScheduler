#define SUPERVISION_CALLBACK
#define SUPERVISION_CALLBACK_TIMEOUT WDTO_1S
#include <DeepSleepScheduler.h>

#ifdef ESP32
#define LED_PIN 2
#else
#define LED_PIN 13
#endif

void block() {
  while (1);
}

class SupervisionCallback: public Runnable {
    void run() {
      digitalWrite(LED_PIN, HIGH);
      // this method is called from the interrupt so
      // delay() does not work
      
      // to see that the LED is on,
      // we block until the watchdog resets the CPU what
      // is configured with SUPERVISION_CALLBACK_TIMEOUT
      // and defaults to 1s
      while (1);
    }
};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  scheduler.setTaskTimeout(TIMEOUT_2S);
  scheduler.setSupervisionCallback(new SupervisionCallback());
  scheduler.scheduleDelayed(block, 1000);
}

void loop() {
  scheduler.execute();
}

