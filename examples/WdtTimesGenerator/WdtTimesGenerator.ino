/**
   This sketch is used together with WdtTimesMeasuerer, please read the description there.
*/
#include <avr/sleep.h>
#include <avr/wdt.h>

// ports to use
#define OUTPUT_PIN 2
#define LED_PIN 13

// constants and variables
#define BEFORE_FIRST -1
#define FINISHED -2

volatile unsigned int wdtTime =  BEFORE_FIRST;

void setup() {
  Serial.begin(115200);
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);

  Serial.println(F("WdtTimesGenerator"));
  delay(3000);
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  noInterrupts();
  const byte outputPinState = digitalRead(OUTPUT_PIN);
  interrupts();
  if (outputPinState == LOW) {
    wdtTime = getAndPrintNextWdtTime(wdtTime);
    delay(100); // finish serial out

    if (wdtTime == FINISHED) {
      digitalWrite(LED_PIN, LOW);
      pwrDownSleep();
    }

    digitalWrite(OUTPUT_PIN, HIGH);
    wdt_enable(wdtTime);
    // enable interrupt
    // first timeout will be the interrupt, second system reset
    WDTCSR |= (1 << WDCE) | (1 << WDIE);
    pwrDownSleep();
    wdt_disable();
  }

  // gap between different times
  delay(1000);
}

void pwrDownSleep() {
  sleep_enable(); // enables the sleep bit, a safety pin
  noInterrupts();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  byte adcsraSave = ADCSRA;
  ADCSRA = 0;  // disable ADC
  // turn off brown-out in software
#if defined(BODS) && defined(BODSE)
  sleep_bod_disable();
#endif
  interrupts (); // guarantees next instruction executed
  sleep_cpu(); // here the device is actually put to sleep

  if (adcsraSave != 0) {
    // re-enable ADC
    ADCSRA = adcsraSave;
  }
}

ISR (WDT_vect) {
  // WDIE & WDIF is cleared in hardware upon entering this ISR
  sleep_disable();
  digitalWrite(OUTPUT_PIN, LOW);
}

unsigned int getAndPrintNextWdtTime(unsigned int last) {
  switch (last) {
    case BEFORE_FIRST:
      Serial.println(F("WDTO_15MS"));
      return WDTO_15MS;
    case WDTO_15MS:
      Serial.println(F("WDTO_30MS"));
      return WDTO_30MS;
    case WDTO_30MS:
      Serial.println(F("WDTO_60MS"));
      return WDTO_60MS;
    case WDTO_60MS:
      Serial.println(F("WDTO_120MS"));
      return WDTO_120MS;
    case WDTO_120MS:
      Serial.println(F("WDTO_250MS"));
      return WDTO_250MS;
    case WDTO_250MS:
      Serial.println(F("WDTO_500MS"));
      return WDTO_500MS;
    case WDTO_500MS:
      Serial.println(F("WDTO_1S"));
      return WDTO_1S;
    case WDTO_1S:
      Serial.println(F("WDTO_2S"));
      return WDTO_2S;
    case WDTO_2S:
      Serial.println(F("WDTO_4S"));
      return WDTO_4S;
    case WDTO_4S:
      Serial.println(F("WDTO_8S"));
      return WDTO_8S;
    case WDTO_8S:
    case FINISHED:
      Serial.println(F("finished"));
      return FINISHED;
  }
}

