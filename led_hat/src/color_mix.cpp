#include "color_mix.h"
#include <cstdint>
#include <cstdio>

float lerp(float s, float e, float t) { return s + (e - s) * t; }
float bilinear_interp(float c00, float c10, float c01, float c11, float tx,
                      float ty) {
  return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
}

// Learn more why I should stay in RGB. Interesting, see
// https://stackoverflow.com/questions/52376506/applying-smoothing-filters-bilateral-gauss-vs-and-colorspaces
CRGB calculate_color_mix(CRGB c00, CRGB c10, CRGB c01, CRGB c11, float x,
                         float y) {
  CRGB ret = {0, 0, 0};
  if (x > 1.0f || y > 1.0f || x < 0 || y < 0) {
    return ret;
  }
  ret.r = bilinear_interp(c00.r, c10.r, c01.r, c11.r, x, y);
  ret.g = bilinear_interp(c00.g, c10.g, c01.g, c11.g, x, y);
  ret.b = bilinear_interp(c00.b, c10.b, c01.b, c11.b, x, y);
  return ret;
}
