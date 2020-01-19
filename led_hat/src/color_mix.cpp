#include "color_mix.h"
#include <cstdint>
#include <cstdio>

// I can speed this up.. the determinate never changes, for example.
// Inspired by
// https://www.gamedev.net/forums/topic/597393-getting-the-height-of-a-point-on-a-triangle/
float calcZ(vec3 p1, vec3 p2, vec3 p3, float x, float y) {
  float det = (p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y);

  float l1 = ((p2.y - p3.y) * (x - p3.x) + (p3.x - p2.x) * (y - p3.y)) / det;
  if (l1 < 0 || l1 > 1.0f) return 0;
  float l2 = ((p3.y - p1.y) * (x - p3.x) + (p1.x - p3.x) * (y - p3.y)) / det;
  if (l2 < 0 || l2 > 1.0f) return 0;
  float l3 = 1.0f - l1 - l2;

  return l1 * p1.z + l2 * p2.z + l3 * p3.z;
}

float calculate_single_mix(uint8_t tl, uint8_t bl, uint8_t tr, uint8_t br,
                           float x, float y) {
  vec3 tl_vec(0, 0, tl);
  vec3 bl_vec(0, 100, bl);
  vec3 tr_vec(100, 0, tr);
  vec3 br_vec(100, 100, br);
  float ret = calcZ(tl_vec, bl_vec, br_vec, x, y);
  if (ret == 0) {
    ret = calcZ(tl_vec, br_vec, tr_vec, x, y);
  }
  return ret;
}

ColorMix::ColorMix(Color tl, Color bl, Color tr, Color br)
    : tl(tl), bl(bl), tr(tr), br(br) {}

Color ColorMix::calculate_color_mix(uint8_t x, uint8_t y) {
  Color ret = {0, 0, 0};
  if (x > 100 || y > 100) {
    return ret;
  }
  ret.r = calculate_single_mix(tl.r, bl.r, tr.r, br.r, x, y);
  ret.g = calculate_single_mix(tl.g, bl.g, tr.g, br.g, x, y);
  ret.b = calculate_single_mix(tl.b, bl.b, tr.b, br.b, x, y);
  return ret;
}
