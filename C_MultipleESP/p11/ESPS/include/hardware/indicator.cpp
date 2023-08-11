#include "Arduino.h"
#include "indicator.h"

const int LED_PIN = LED_BUILTIN;
const unsigned long BLINK_DELAY = 500;

void blinkLED(int nbrOfBlink) {
  for (int i = 0; i < nbrOfBlink; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(LED_PIN, LOW);
    delay(BLINK_DELAY);
  }
}

void blinkLEDERROR() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }

}
  
