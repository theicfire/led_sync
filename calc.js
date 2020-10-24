const csv = require('csv-parser');
const fs = require('fs');

class AngleEstimate {
  COMPARE_WINDOW = 10;
  constructor() {
    this.start_mag = 0;
    this.current_angle = 0;
    this.current_index = 0;
    this.last_smaller_data_index = 0;
    this.last_larger_data_index = 0;
    this.avg_mag = 0;

    this.last_period = 0;
    this.switch_index = 0;
    this.last_angle = 0;
  }

  add_accel(x, y, z) {
    const mag = this.get_mag(x, y, z);
    this.avg_mag = .4 * mag + .6 * this.avg_mag;
    if (this.current_index < 90) {
      this.start_mag = mag;
      this.current_index += 1;
      return;
    }

    if (mag < this.start_mag) {
      this.last_smaller_data_index = this.current_index;
    } else {
      this.last_larger_data_index = this.current_index;
    }
    this.recalc_angle();
    this.current_index += 1;
  }

  recalc_angle() {
    const before_angle = this.get_angle();
    let smaller = false;
    let larger = false;

    if (this.last_smaller_data_index >= this.current_index - this.COMPARE_WINDOW) {
      smaller = true;
    }
    if (this.last_larger_data_index >= this.current_index - this.COMPARE_WINDOW) {
      larger = true;
    }
    const prev_angle = this.current_angle;
    if (!smaller) {
      this.current_angle = 1;
    } else if (!larger) {
      this.current_angle = -1;
    }

    if (this.avg_mag < 300 && prev_angle == 0) {
      this.current_angle = 0;
    } else if (this.avg_mag < 100) {
      this.current_angle = 0;
    }
    if (prev_angle !== this.current_angle) {
      if (this.current_angle === 0) {
        this.switch_index = 0;
        this.last_period = 0;
        this.last_angle = 0;
      } else {
        if (this.switch_index > 0) {
          this.last_angle = before_angle;
          this.last_period = this.current_index - this.switch_index;
        }
        this.switch_index = this.current_index;
      }
    }
  }

  get_mag(x, y, z) {
    return Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2) + Math.pow(z, 2));
  }

  get_angle() {
    const TIME_SHIFT = this.COMPARE_WINDOW;
    const steps_past_last = this.current_index - this.switch_index;
    if (this.last_period > 0 && this.current_angle === 1) {
      if (steps_past_last < this.last_period / 2 - TIME_SHIFT) {
        const rise = (1 - this.last_angle);
        const run = (this.last_period / 2) - TIME_SHIFT;
        const m = rise / run;
        const b = this.last_angle;
        return Math.min(1, m * steps_past_last + b);
      } else {
        const rise = -2;
        const run = (this.last_period);
        const m = rise / run;
        const b = 1;
        const steps = (steps_past_last - (this.last_period / 2 - TIME_SHIFT));
        return Math.max(-1, m * steps + b);
      }
    } else if (this.last_period > 0 && this.current_angle === -1) {
      if (steps_past_last < this.last_period / 2 - TIME_SHIFT) {
        const rise = (-1 - this.last_angle);
        const run = (this.last_period / 2) - TIME_SHIFT;
        const m = rise / run;
        const b = this.last_angle;
        return Math.max(-1, m * steps_past_last + b);
      } else {
        const rise = 2;
        const run = (this.last_period);
        const m = rise / run;
        const b = -1;
        const steps = (steps_past_last - (this.last_period / 2 - TIME_SHIFT));
        return Math.min(1, m * steps + b);
      }
    }
    return 0;
  }
}

function get_angle(accel_a, accel_b) {
      const numer = accel_a.x * accel_b.x + accel_a.y * accel_b.y + accel_a.z * accel_b.z;
      const b = Math.sqrt(Math.pow(accel_a.x, 2) + Math.pow(accel_a.y, 2) + Math.pow(accel_a.z, 2));
      const c = Math.sqrt(Math.pow(accel_b.x, 2) + Math.pow(accel_b.y, 2) + Math.pow(accel_b.z, 2));
      return Math.acos(numer / (b * c));
}

const smooth_alpha = 0.2;
let smoothed = 0;
function get_smoothed_angle(accel_a, accel_b) {
      const theta = get_angle(accel_a, accel_b);
      smoothed = (1 - smooth_alpha) * smoothed + smooth_alpha * theta;
      return smoothed;
}

function get_length(accel) {
  return Math.sqrt(Math.pow(accel.x, 2) + Math.pow(accel.y, 2) + Math.pow(accel.z, 2));
}

function beginning_avg(points) {
  let sum = 0;
  for (let i = 0; i < 50; i++) {
    sum += points[i];
  }
  return sum / 50;
}

function find_patterns(rows) {
  const estimator = new AngleEstimate();
  for (let i = 0; i < rows.length; i++) {
    estimator.add_accel(rows[i].x, rows[i].y, rows[i].z);
    console.log(estimator.get_angle());
  }
}

let first;
const rows = [];
fs.createReadStream('entries2.csv')
  .pipe(csv())
  .on('data', (row) => {
    if (!first) {
      first = row;
    } else {
      //console.log(get_smoothed_angle(first, row));
      const length = get_length(row);
      console.log(length);
      rows.push(row);
      //console.log(`{${row.x},${row.y},${row.z}},`);
    }
  })
  .on('end', () => {
    //find_patterns(rows);
  });
