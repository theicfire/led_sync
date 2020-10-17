#include <Arduino.h>
#include "angle_estimate.h"

void AngleEstimate::add_accel(int16_t x, int16_t y, int16_t z) {
  double mag = get_mag(x, y, z);
  // TODO random angles..
  avg_mag_ = .4 * mag + .6 * avg_mag_;
  if (current_index_ < 90) {
    start_mag_ = mag;
    current_index_ += 1;
    return;
  }

  if (mag < start_mag_) {
    last_smaller_data_index_ = current_index_;
  } else {
    last_larger_data_index_ = current_index_;
  }
  recalc_angle();
  current_index_ += 1;
}

void AngleEstimate::recalc_angle() {
  const int COMPARE_WINDOW = 10;
  bool smaller = false;
  bool larger = false;
  if (last_smaller_data_index_ >= current_index_ - COMPARE_WINDOW) {
    smaller = true;
  }
  if (last_larger_data_index_ >= current_index_ - COMPARE_WINDOW) {
    larger = true;
  }
  double prev_angle = current_angle_;
  if (!smaller) {
    current_angle_ = 1;
  } else if (!larger) {
    current_angle_ = -1;
  }

  if (avg_mag_ < 300 && prev_angle == 0) {
    current_angle_ = 0;
  } else if (avg_mag_ < 100) {
    current_angle_ = 0;
  }
}


double AngleEstimate::get_mag(int16_t x, int16_t y, int16_t z) {
  return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

double AngleEstimate::get_angle() {
  return current_angle_;

}
