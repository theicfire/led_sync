#pragma once
#include <unistd.h>
//#include <stdint.h>

#include "ring_buffer.h"

enum SwingState {
  VALLEY_SEARCHING,
  MAYBE_VALLEY,
};

class AngleEstimate {
 public:
  AngleEstimate() {
    start_mag_ = 0;
    current_angle_ = 0;
    current_index_ = 0;
    last_smaller_data_index_ = 0;
    last_larger_data_index_ = 0;
    avg_mag_ = 0;

    switch_index_ = 0;
    last_angle_ = 0;

    swing_state_ = VALLEY_SEARCHING;
    previous_lowest_index_ = 0;
    lowest_index = -1;
    lowest_mag_ = 0;
    last_period_ = 0;
    angle_increasing_ = false;
  }
  void add_accel(int16_t x, int16_t y, int16_t z);
  void add_mag(double mag);
  // From -1 to 1. 0 for no angle. Starts at 0.
  double get_angle();
  double inner_get_angle();
  double get_angle2();
  static double get_mag(int16_t x, int16_t y, int16_t z);

 private:
  void recalc_angle();
  void recalc_angle2();

  SwingState swing_state_;
  int previous_lowest_index_;
  int lowest_index;
  double lowest_mag_;
  int last_period_;
  bool angle_increasing_;

  double start_mag_;
  double current_angle_;
  unsigned long current_index_;
  unsigned long last_smaller_data_index_;
  unsigned long last_larger_data_index_;
  double avg_mag_;

  int switch_index_;
  double last_angle_;
};
