#include <Arduino.h>

static long offset = 0;
void Time_SetTime(unsigned long t) {
    offset = (t % 100000) - (millis() % 100000);
}

unsigned long Time_GetTime() {
    // Serial.print("Time is: "); Serial.println(millis() + offset);
    return (millis() + offset) % 100000;
}
