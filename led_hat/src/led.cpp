
#include <Arduino.h>
#include <FastLED.h>

#include "color_mix.h"
#include "friend.h"
#include "gamma.h"
#include "image_merge.h"
#include "time.h"

// Different for each micro
constexpr int LED_OFFSET = 0;

constexpr int NUM_COLORS = 3;
constexpr int ANIMATION_SECONDS = 10;
constexpr int LED_PIN = 5;
constexpr int NUM_LEDS = 300;
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
// TODO remove globals..
CRGB leds[NUM_LEDS];
static int count = 0;
static CRGB tl = {0, 0, 0};
static CRGB tr = {0, 255, 0};
static CRGB bl = {255, 0, 0};
static CRGB br = {0, 0, 255};

constexpr float WIDTH_MULTIPLY = 8.f;
constexpr float HEIGHT_MULTIPLY = 16.f;
constexpr int IMAGE_WIDTH = 300;
static uint8_t led_buff1[IMAGE_WIDTH][3];
static uint8_t led_buff2[IMAGE_WIDTH][3];
constexpr int NUM_MOVEMENTS = 7;
constexpr int MS_PER_MOVEMENT = 250000;
// static int movements[NUM_MOVEMENTS][2] = {{0, 0}, {0, 50}};
static int movements[NUM_MOVEMENTS][2] = {
    {0, 0}, {0, 763}, {0, 400}, {150, 400}, {150, 0}, {150, 763}, {100, 300}};

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
  int movement_index = (time / MS_PER_MOVEMENT) % NUM_MOVEMENTS;
  int previous_movement_index = ((time / MS_PER_MOVEMENT) - 1) % NUM_MOVEMENTS;
  float fraction = ((time - movement_index * MS_PER_MOVEMENT) %
                    (NUM_MOVEMENTS * MS_PER_MOVEMENT)) /
                   (MS_PER_MOVEMENT * 1.f);
  float y = movements[movement_index][1] * fraction +
            movements[previous_movement_index][1] * (1 - fraction);
  int yi = y;
  printf("yi: %d\n", yi);
  float x_start = movements[movement_index][0] * fraction +
                  movements[previous_movement_index][0] * (1 - fraction);

  memcpy_P(led_buff1, image_merge[yi], IMAGE_WIDTH * 3);
  memcpy_P(led_buff2, image_merge[yi + 1], IMAGE_WIDTH * 3);
  // TODO check that we don't go over the width
  for (int i = 0; i < NUM_LEDS; i++) {
    int x = ((i + ((int)x_start * (int)WIDTH_MULTIPLY) + LED_OFFSET) %
             (NUM_LEDS * (int)WIDTH_MULTIPLY)) /
            WIDTH_MULTIPLY;
    float x_fraction = x_start - ((int)x_start);
    CRGB tl = getColorBuff1(x);
    CRGB tr = getColorBuff1(x + 1);
    CRGB bl = getColorBuff2(x);
    CRGB br = getColorBuff2(x + 1);
    leds[i] = calculate_color_mix(tl, tr, bl, br, x_fraction, (y - yi));
  }
}

void LED_Init() {
  printf("hello\n");
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
  // FastLED.setBrightness(BRIGHTNESS);
}

void LED_Update() {
  unsigned long time = Time_GetTime();
  playImage(time);
  FastLED.show();
}
