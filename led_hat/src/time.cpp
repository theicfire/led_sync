#include <Arduino.h>

static long offset = 0;
void Time_SetTime(unsigned long t) {
  offset = (t % 4294967295) - (millis() % 4294967295);
}

// Will wrap every 49 days
unsigned long Time_GetTime() {
  // Serial.print("Time is: "); Serial.println(millis() + offset);
  return (millis() + offset) % 4294967295;
}
