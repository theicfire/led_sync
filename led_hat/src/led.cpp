
#include <Arduino.h>
#include <FastLED.h>

#include "color_mix.h"
#include "first_image.h"
#include "friend.h"
#include "gamma.h"
#include "time.h"

constexpr int NUM_COLORS = 3;
constexpr int ANIMATION_SECONDS = 10;
constexpr int LED_PIN = 5;
constexpr int NUM_LEDS = 300;
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
// TODO remove globals..
CRGB leds[NUM_LEDS];
static uint8_t led_buff1[300][3];
static uint8_t led_buff2[300][3];
static int count = 0;
static CRGB tl = {0, 0, 0};
static CRGB tr = {0, 255, 0};
static CRGB bl = {255, 0, 0};
static CRGB br = {0, 0, 255};

constexpr float WIDTH_MULTIPLY = 4.f;
constexpr float HEIGHT_MULTIPLY = 16.f;
constexpr int IMAGE_HEIGHT = 300;
constexpr int IMAGE_WIDTH = 300;

// Different for each micro
constexpr int LED_OFFSET = 0;

CRGB getColorBuff1(int index) {
  CRGB ret = {.r = 0, .b = 0, .g = 0};
  ret.r = pgm_read_byte(&gamma8[led_buff1[index][0]]);
  ret.g = pgm_read_byte(&gamma8[led_buff1[index][1]]);
  ret.b = pgm_read_byte(&gamma8[led_buff1[index][2]]);
  return ret;
}

CRGB getColorBuff2(int index) {
  CRGB ret = {.r = 0, .b = 0, .g = 0};
  ret.r = pgm_read_byte(&gamma8[led_buff2[index][0]]);
  ret.g = pgm_read_byte(&gamma8[led_buff2[index][1]]);
  ret.b = pgm_read_byte(&gamma8[led_buff2[index][2]]);
  return ret;
}

// Time in ms. Starts at 0.
void playImage(unsigned long time) {
  count += 1;
  constexpr int FRAME_RATE = 1000 / 60;
  int loc = (time / FRAME_RATE) %
            ((IMAGE_HEIGHT - 1) *
             ((int)HEIGHT_MULTIPLY));  // - 1 because we want to read the last
                                       // two row of flash
  float y = loc / HEIGHT_MULTIPLY;
  int yi = y;
  memcpy_P(led_buff1, first_image[yi], NUM_LEDS * 3);
  memcpy_P(led_buff2, first_image[yi + 1], NUM_LEDS * 3);
  // TODO check that we don't go over the width
  for (int i = 0; i < NUM_LEDS; i++) {
    float x = i / WIDTH_MULTIPLY;
    int xi = x;
    CRGB tl = getColorBuff1(x);
    CRGB tr = getColorBuff1(x + 1);
    CRGB bl = getColorBuff2(x);
    CRGB br = getColorBuff2(x + 1);
    leds[i] = calculate_color_mix(tl, tr, bl, br, (x - xi), (y - yi));
  }
}

void LED_Init() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
  // FastLED.setBrightness(BRIGHTNESS);
}

void LED_Update() {
  unsigned long time = Time_GetTime();
  playImage(time);
  FastLED.show();
}
