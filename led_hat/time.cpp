#include <Arduino.h>

static long offset = 0;
void Time_SetTime(unsigned long t) {
    offset = t - millis();
}

unsigned long Time_GetTime() {
    return millis() + offset;

}