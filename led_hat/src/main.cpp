#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <FastLED.h>
#include "led.h"
#include "radio.h"

void waitForSerial() {
    while (!Serial) {
        delay(1);
    }
}

void setup() {
    Serial.begin(115200);
    delay( 3000 ); // power-up safety delay for LEDs, TODO is this necessary?
    Radio_Init();
    LED_Init();
   waitForSerial();
}

void loop() {
    LED_Update();
    Radio_Update();
}
