#pragma once
#include <FastLED.h>
#include <cstdint>

CRGB calculate_color_mix(CRGB c00, CRGB c10, CRGB c01, CRGB c11, float x,
                         float y);