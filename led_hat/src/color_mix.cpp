#include "color_mix.h"
#include <cstdint>
#include <cstdio>

float lerp(float s, float e, float t) { return s + (e - s) * t; }
float blerp(float c00, float c10, float c01, float c11, float tx, float ty) {
  return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
}

float calculate_single_mix(uint8_t tl, uint8_t bl, uint8_t tr, uint8_t br,
                           float x, float y) {
  float ret = blerp(tl, tr, bl, br, x, y);
  //   printf("call blerp: %d %d %d %d %f %f = %f", tl, bl, tr, br, x, y, ret);
  return ret;
}

ColorMix::ColorMix(Color tl, Color bl, Color tr, Color br)
    : tl(tl), bl(bl), tr(tr), br(br) {}

Color ColorMix::calculate_color_mix(float x, float y) {
  Color ret = {0, 0, 0};
  if (x > 1.0f || y > 1.0f) {
    return ret;
  }
  ret.r = calculate_single_mix(tl.r, bl.r, tr.r, br.r, x, y);
  ret.g = calculate_single_mix(tl.g, bl.g, tr.g, br.g, x, y);
  ret.b = calculate_single_mix(tl.b, bl.b, tr.b, br.b, x, y);
  return ret;
}
