const csv = require('csv-parser');
const fs = require('fs');

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

function find_patterns(points) {
  const COMPARE_WINDOW = 10;
  const ZERO_WINDOW = COMPARE_WINDOW;
  const start = beginning_avg(points);
  let direction = 0;
  let last_switch = 0;
  for (let i = ZERO_WINDOW; i < points.length; i++) {
    let below = false;
    let above = false;
    for (let j = 0; j < COMPARE_WINDOW; j++) {
      if (points[i - j] < start) {
        below = true;
      } else {
        above = true;
      }
    }
    const prev_direction = direction;
    if (!below && direction !== 1) {
      last_switch = i;
      direction = 1;
    }
    if (!above && direction !== -1) {
      last_switch = i;
      direction = -1;
    }

    let sum = 0;
    for (let j = 0; j < ZERO_WINDOW; j++) {
      sum += Math.pow(points[i] - points[i - j], 2);
    }
    // Hysterisis...
    if (Math.sqrt(sum / ZERO_WINDOW) < 300 && prev_direction == 0) {
      direction = 0;
    } else if (Math.sqrt(sum / ZERO_WINDOW) < 100) {
      direction = 0;
    }
    console.log(direction);
  }
}

let first;
const lengths = [];
fs.createReadStream('entries1.csv')
  .pipe(csv())
  .on('data', (row) => {
    if (!first) {
      first = row;
    } else {
      //console.log(get_smoothed_angle(first, row));
      const length = get_length(row);
      //console.log(length);
      lengths.push(length);
      console.log(`{${row.x},${row.y},${row.z}},`);
    }
  })
  .on('end', () => {
    //find_patterns(lengths);
  });
