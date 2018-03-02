/**
   This sketch is used together with WdtTimesGenerator to find out what
   correction times can be used for the used CPU.

   Required hardware fo the measurement:
   - 2 Arduino boards (e.g. Arduino Uno)
   - 4 wires to connect them

   Program the Arduino board under test with the sketch WdtTimesGenerator and
   the other Arduino board with WdtTimesMeasurer.

   Then hook them up with 4 cables as follows:
   WdtTimesGenerator PIN 5V -- WdtTimesMeasurer PIN 5V
   WdtTimesGenerator PIN GND -- WdtTimesMeasurer PIN GND
   WdtTimesGenerator PIN 2 -- WdtTimesMeasurer PIN 2
   WdtTimesGenerator PIN RESET -- WdtTimesMeasurer PIN 3

   Then connect the WdtTimesMeasurer board with USB and open Serial Monitor
   to see the measurements.
   The values are calculated as average values when the test is run multiple
   repeats. So let it run for some repeats and then take the last output.
*/
#include <avr/wdt.h>

// ports to use
#define INTERRUPT_PIN 2
#define RESET_PIN 3
#define LED_PIN 13

// constants and variables
#define FINISHED -1
#define CORRECTIONS_COUNT 10

volatile boolean ignoreInterrupts = false;
volatile unsigned int wdtTime =  WDTO_15MS;
volatile unsigned long startTime = 0;
volatile unsigned long endTime = 0;

volatile unsigned int repeat = 0;
volatile unsigned int avgCorrectionSumIndex = 0;
volatile signed long avgCorrectionSum[CORRECTIONS_COUNT];

void setup() {
  Serial.begin(115200);
  delay(150);
  Serial.println(F("WdtTimesMeasurer"));

  pinMode(LED_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);

  for (int i = 0; i < CORRECTIONS_COUNT; i++) {
    avgCorrectionSum[i] = 0;
  }

  initMeasurement();
}

void loop() {
  noInterrupts();
  const unsigned long startTimeLocal = startTime;
  const unsigned long endTimeLocal = endTime;
  interrupts();
  if (endTimeLocal != 0) {
    printNextCorrection(endTime - startTime);
    noInterrupts();
    startTime = 0;
    endTime = 0;
    interrupts();

    if (wdtTime == FINISHED) {
      Serial.println();
      initMeasurement();
    }
  }
  yield();
}

void initMeasurement() {
  wdtTime =  WDTO_15MS;
  repeat++;
  avgCorrectionSumIndex = 0;

  noInterrupts();
  ignoreInterrupts = true;
  interrupts();
  detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN));

  Serial.println(F("resetting.."));
  digitalWrite(RESET_PIN, LOW);
  delay(150);
  digitalWrite(RESET_PIN, HIGH);

  // await startup
  delay(2000);

  Serial.print(F("start repeat "));
  Serial.println(repeat);
  Serial.println();
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), isrInterruptPin, CHANGE);

  // ignore pending interrupts
  delay(100);
  noInterrupts();
  ignoreInterrupts = false;
  interrupts();
}

void isrInterruptPin() {
  if (ignoreInterrupts) {
    return;
  }
  if (digitalRead(INTERRUPT_PIN) == HIGH) {
    if (startTime == 0) {
      startTime = millis();
      digitalWrite(LED_PIN, HIGH);
    }
  } else {
    if (endTime == 0) {
      endTime = millis();
      digitalWrite(LED_PIN, LOW);
    }
  }
}

void printNextCorrection(unsigned long duration) {
  Serial.print(F("#define SLEEP_TIME_"));
  signed long correction = 0;
  switch (wdtTime) {
    case WDTO_15MS:
      Serial.print(F("15MS"));
      correction = duration - 15;
      wdtTime =  WDTO_30MS;
      break;
    case WDTO_30MS:
      Serial.print(F("30MS"));
      correction = duration - 30;
      wdtTime = WDTO_60MS;
      break;
    case WDTO_60MS:
      Serial.print(F("60MS"));
      correction = duration - 60;
      wdtTime = WDTO_120MS;
      break;
    case WDTO_120MS:
      Serial.print(F("120MS"));
      correction = duration - 120;
      wdtTime = WDTO_250MS;
      break;
    case WDTO_250MS:
      Serial.print(F("250MS"));
      correction = duration - 250;
      wdtTime = WDTO_500MS;
      break;
    case WDTO_500MS:
      Serial.print(F("500MS"));
      correction = duration - 500;
      wdtTime = WDTO_1S;
      break;
    case WDTO_1S:
      Serial.print(F("1S"));
      correction = duration - 1000;
      wdtTime = WDTO_2S;
      break;
    case WDTO_2S:
      Serial.print(F("2S"));
      correction = duration - 2000;
      wdtTime = WDTO_4S;
      break;
    case WDTO_4S:
      Serial.print(F("4S"));
      correction = duration - 4000;
      wdtTime = WDTO_8S;
      break;
    case WDTO_8S:
      Serial.print(F("8S"));
      correction = duration - 8000;
      wdtTime = FINISHED;
      break;
    default:
      wdtTime = FINISHED;
  }

  avgCorrectionSum[avgCorrectionSumIndex] += correction;
  Serial.print(F("_CORRECTION "));
  Serial.print(avgCorrectionSum[avgCorrectionSumIndex] / repeat);
  //  Serial.print(F(" "));
  //  Serial.print(duration);
  Serial.println();
  avgCorrectionSumIndex++;
}

