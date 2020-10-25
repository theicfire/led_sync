#ifdef ARDUINO
#include <Arduino.h>
#else
#include "math.h"
#define min fmin
#define max fmax
#endif
#include "angle_estimate.h"

double AngleEstimate::get_mag(int16_t x, int16_t y, int16_t z) {
  return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

void AngleEstimate::add_accel(int16_t x, int16_t y, int16_t z) {
  double mag = AngleEstimate::get_mag(x, y, z);
  add_mag(mag);
}

void AngleEstimate::add_mag(double mag) {
  double alpha = 0.05;
  avg_mag_ = (alpha * mag) + (1 - alpha) * avg_mag_;
  if (swing_state_ == VALLEY_SEARCHING) {
    if (mag < 3300 && current_index_ - previous_lowest_index_ > 50) {
      swing_state_ = MAYBE_VALLEY;
      lowest_index = current_index_;
      lowest_mag_ = mag;
    }
  } else if (swing_state_ == MAYBE_VALLEY) {
    if (mag > 5000) {
      swing_state_ = VALLEY_SEARCHING;
      lowest_index = -1;
      lowest_mag_ = -1;
    } else if (mag < lowest_mag_) {
      lowest_index = current_index_;
      lowest_mag_ = mag;
    } else if (current_index_ - lowest_index > 5) {
      // found lowest. It was at lowest_index. Calculate period.
      last_period_ = lowest_index - previous_lowest_index_;
      // printf("Period now %d\n", last_period_);
      previous_lowest_index_ = lowest_index;
      angle_increasing_ = !angle_increasing_;
      swing_state_ = VALLEY_SEARCHING;
      lowest_index = -1;
      lowest_mag_ = -1;
    }
  }
  recalc_angle();
  current_index_ += 1;
}

void AngleEstimate::recalc_angle() {}

double AngleEstimate::get_angle() {
  double angle = inner_get_angle();
  double MAX_STEP = 0.05;
  if (last_angle_ > angle && last_angle_ - angle > MAX_STEP) {
    last_angle_ -= MAX_STEP;
    return last_angle_;
  }
  if (last_angle_ < angle && last_angle_ - angle < -MAX_STEP) {
    last_angle_ += MAX_STEP;
    return last_angle_;
  }
  last_angle_ = angle;
  return last_angle_;
}

double AngleEstimate::inner_get_angle() {
  if (last_period_ != 0) {
    int since_last = current_index_ - previous_lowest_index_;
    if (since_last > last_period_) {
      if (since_last > 300) {
        return 0;
      }
      return angle_increasing_ ? 1 : -1;
    }
    if (angle_increasing_) {
      return (since_last * 1.0 / last_period_) * 2 - 1;
    }
    return (since_last * 1.0 / last_period_) * -2 + 1;
  }
  return 0;
}

void AngleEstimate::recalc_angle2() {
  double before_angle = get_angle();
  const int COMPARE_WINDOW = 10;
  bool smaller = false;
  bool larger = false;
  // printf("Check switch %lu %d %d\n", current_index_,
  // last_smaller_data_index_, last_larger_data_index_);
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

  if (prev_angle != current_angle_) {
    if (current_angle_ == 0) {
      switch_index_ = 0;
      last_period_ = 0;
      last_angle_ = 0;
    } else {
      if (switch_index_ > 0) {
        last_angle_ = before_angle;
        last_period_ = current_index_ - switch_index_;
        // printf("Period now %d\n", last_period_);
      }
      switch_index_ = current_index_;
    }
  }
}

double AngleEstimate::get_angle2() {
  int TIME_SHIFT = 0;  // TODO change..
  int steps_past_last = current_index_ - switch_index_;
  if (last_period_ > 0 && current_angle_ == 1) {
    if (steps_past_last < last_period_ / 2 - TIME_SHIFT) {
      double rise = (1 - last_angle_);
      double run = (last_period_ / 2) - TIME_SHIFT;
      double m = rise / run;
      double b = last_angle_;
      return min(1.0, m * steps_past_last + b);
    } else {
      double rise = -2;
      double run = (last_period_);
      double m = rise / run;
      double b = 1;
      double steps = (steps_past_last - (last_period_ / 2 - TIME_SHIFT));
      return fmax(-1, m * steps + b);
    }
  } else if (last_period_ > 0 && current_angle_ == -1) {
    if (steps_past_last < last_period_ / 2 - TIME_SHIFT) {
      double rise = (-1 - last_angle_);
      double run = (last_period_ / 2) - TIME_SHIFT;
      double m = rise / run;
      double b = last_angle_;
      return fmax(-1, m * steps_past_last + b);
    } else {
      double rise = 2;
      double run = (last_period_);
      double m = rise / run;
      double b = -1;
      double steps = (steps_past_last - (last_period_ / 2 - TIME_SHIFT));
      return min(1.0, m * steps + b);
    }
  }
  return 0;
}

#ifndef ARDUINO
#include "data.h"
int main() {
  AngleEstimate estimator;
  for (unsigned int i = 0;
       i < sizeof(recorded_accels) / sizeof(recorded_accels[0]); i++) {
    estimator.add_accel(recorded_accels[i][0], recorded_accels[i][1],
                        recorded_accels[i][2]);
    printf("%.3f\t%f\n",
           AngleEstimate::get_mag(recorded_accels[i][0], recorded_accels[i][1],
                                  recorded_accels[i][2]),
           estimator.get_angle());
  }
  return 0;
}
#endif
