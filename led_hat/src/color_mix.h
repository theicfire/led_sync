#pragma once
#include <cstdint>

struct vec3 {
  vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  float x, y, z;
};
struct Color {
  Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
  uint8_t r, g, b;
};

class ColorMix {
 private:
  Color tl, bl, tr, br;

 public:
  ColorMix(Color tl, Color bl, Color tr, Color br);
  // x, y: numbers between 0 and 1, to find the mix color
  // Return: Mixed color, or all 0's in the case of an error
  Color calculate_color_mix(float x, float y);
};
