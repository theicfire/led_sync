#pragma once
#include <unistd.h>
//#include <stdint.h>

#include "ring_buffer.h"

class AngleEstimate {
  public:
  AngleEstimate() {}
  void add_accel(int16_t x, int16_t y, int16_t z);
  // From -1 to 1. 0 for no angle. Starts at 0.
  double get_angle();

  private:
  double get_mag(int16_t x, int16_t y, int16_t z);
  void recalc_angle();

  double start_mag_;
  double current_angle_ = 0;
  unsigned long current_index_ = 0;
  unsigned long last_smaller_data_index_;
  unsigned long last_larger_data_index_;
  double avg_mag_;
};
