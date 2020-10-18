//#include <Arduino.h>
#include "angle_estimate.h"
#include "math.h"

void AngleEstimate::add_accel(int16_t x, int16_t y, int16_t z) {
  double mag = get_mag(x, y, z);
  // TODO random angles..
  avg_mag_ = .4 * mag + .6 * avg_mag_;
  if (current_index_ < 90) {
    start_mag_ = mag;
    current_index_ += 1;
    return;
  }

  //printf("index %d has mag %f\n", current_index_, mag);
  if (mag < start_mag_) {
    last_smaller_data_index_ = current_index_;
  } else {
    last_larger_data_index_ = current_index_;
  }
  recalc_angle();
  current_index_ += 1;
}

void AngleEstimate::recalc_angle() {
  double before_angle = get_angle();
  const int COMPARE_WINDOW = 10;
  bool smaller = false;
  bool larger = false;
  //printf("Check switch %lu %d %d\n", current_index_, last_smaller_data_index_, last_larger_data_index_);
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
      }
      switch_index_ = current_index_;
    }
  }
}


double AngleEstimate::get_mag(int16_t x, int16_t y, int16_t z) {
  return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

double AngleEstimate::get_angle() {
    int TIME_SHIFT = 10; // TODO change..
    int steps_past_last = current_index_ - switch_index_;
    if (last_period_ > 0 && current_angle_ == 1) {
      if (steps_past_last < last_period_ / 2 - TIME_SHIFT) {
        double rise = (1 - last_angle_);
        double run = (last_period_ / 2) - TIME_SHIFT;
        double m = rise / run;
        double b = last_angle_;
        return fmin(1, m * steps_past_last + b);
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
        return fmin(1, m * steps + b);
      }
    }
    return 0;
}

#include "data.h"
int main() {
  AngleEstimate estimator;
  for (unsigned int i = 0; i < sizeof(recorded_accels) / sizeof(recorded_accels[0]); i++) {
    estimator.add_accel(recorded_accels[i][0], recorded_accels[i][1],recorded_accels[i][2]);
    printf("%f\n", estimator.get_angle());
  }
  return 0;
}