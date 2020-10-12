const csv = require('csv-parser');
const fs = require('fs');

const smooth_alpha = 0.2;
let first;
let smoothed = 0;
fs.createReadStream('entries1.csv')
  .pipe(csv())
  .on('data', (row) => {
    if (!first) {
      first = row;
      const b = Math.sqrt(Math.pow(first.x, 2) + Math.pow(first.y, 2) + Math.pow(first.z, 2));
    } else {
      const numer = first.x * row.x + first.y * row.y + first.z * row.z;
      const b = Math.sqrt(Math.pow(first.x, 2) + Math.pow(first.y, 2) + Math.pow(first.z, 2));
      const c = Math.sqrt(Math.pow(row.x, 2) + Math.pow(row.y, 2) + Math.pow(row.z, 2));
      const theta = Math.acos(numer / (b * c));
      smoothed = (1 - smooth_alpha) * smoothed + smooth_alpha * theta;
      console.log(smoothed);
    }
    //console.log(row);
  })
  .on('end', () => {
    //console.log('CSV file successfully processed');
  });
