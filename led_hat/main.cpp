#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <FastLED.h>
#include "led.h"

void setup() {
    delay( 3000 ); // power-up safety delay for LEDs, TODO is this necessary?
    LED_Setup();
}

void loop() {
    LED_Update();
}