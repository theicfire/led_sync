#pragma once
#include <unistd.h>
//#include <stdint.h>

#include "ring_buffer.h"

class AngleEstimate {
  public:
  AngleEstimate() {
    start_mag_ = 0;
    current_angle_ = 0;
    current_index_ = 0;
    last_smaller_data_index_ = 0;
    last_larger_data_index_ = 0;
    avg_mag_ = 0;

    last_period_ = 0;
    switch_index_ = 0;
    last_angle_ = 0;
  }
  void add_accel(int16_t x, int16_t y, int16_t z);
  // From -1 to 1. 0 for no angle. Starts at 0.
  double get_angle();

  private:
  double get_mag(int16_t x, int16_t y, int16_t z);
  void recalc_angle();

  double start_mag_;
  double current_angle_;
  unsigned long current_index_;
  unsigned long last_smaller_data_index_;
  unsigned long last_larger_data_index_;
  double avg_mag_;

  int last_period_;
  int switch_index_;
  double last_angle_;
};
