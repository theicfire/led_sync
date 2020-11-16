// #include <Adafruit_MMA8451.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <HardwareSerial.h>
#include <Wire.h>

#include "angle_estimate.h"
#include "led.h"
#include "radio.h"

// #define USE_RECORDED_DATA // Uncommen to use recorded data in data.h
#ifdef USE_RECORDED_DATA
#include "data.h"
#else
int16_t recorded_accels[][3] = {};
#endif

#define MAX_DATA_LEN (5000)
#define BNO055_SAMPLERATE_DELAY_MS (10)

// Adafruit_MMA8451 mma = Adafruit_MMA8451();
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
WiFiClient client;
enum RunType {
  DO_USE_RECORDED_DATA,
  RECORD_DATA,
  IS_LEADER,
  IS_FOLLOWER,
};
#ifdef USE_RECORDED_DATA
RunType run_type = DO_USE_RECORDED_DATA;
#else
RunType run_type = RECORD_DATA;
#endif

AngleEstimate estimator;

void waitForSerial() {
  while (!Serial) {
    delay(1);
  }
}

void calc_on_recorded_data() {
  AngleEstimate estimator;
  Serial.println("Calculate angle based on recorded data");
  for (unsigned int i = 0;
       i < sizeof(recorded_accels) / sizeof(recorded_accels[0]); i++) {
    estimator.add_accel(recorded_accels[i][0], recorded_accels[i][1],
                        recorded_accels[i][2]);
    Serial.print(AngleEstimate::get_mag(
        recorded_accels[i][0], recorded_accels[i][1], recorded_accels[i][2]));
    Serial.print("\t");
    Serial.print(estimator.get_angle());
    Serial.println();
    delay(10);
  }
  Serial.println("Done");
}

void displaySensorDetails(void) {
  sensor_t sensor;
  bno.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print("Sensor:       ");
  Serial.println(sensor.name);
  Serial.print("Driver Ver:   ");
  Serial.println(sensor.version);
  Serial.print("Unique ID:    ");
  Serial.println(sensor.sensor_id);
  Serial.print("Max Value:    ");
  Serial.print(sensor.max_value);
  Serial.println(" xxx");
  Serial.print("Min Value:    ");
  Serial.print(sensor.min_value);
  Serial.println(" xxx");
  Serial.print("Resolution:   ");
  Serial.print(sensor.resolution);
  Serial.println(" xxx");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void displaySensorStatus(void) {
  /* Get the system status values (mostly for debugging purposes) */
  uint8_t system_status, self_test_results, system_error;
  system_status = self_test_results = system_error = 0;
  bno.getSystemStatus(&system_status, &self_test_results, &system_error);

  /* Display the results in the Serial Monitor */
  Serial.println("");
  Serial.print("System Status: 0x");
  Serial.println(system_status, HEX);
  Serial.print("Self Test:     0x");
  Serial.println(self_test_results, HEX);
  Serial.print("System Error:  0x");
  Serial.println(system_error, HEX);
  Serial.println("");
  delay(500);
}

void displayCalStatus(void) {
  /* Get the four calibration values (0..3) */
  /* Any sensor data reporting 0 should be ignored, */
  /* 3 means 'fully calibrated" */
  uint8_t system, gyro, accel, mag;
  system = gyro = accel = mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);

  /* The data should be ignored until the system calibration is > 0 */
  Serial.print("\t");
  if (!system) {
    Serial.print("! ");
  }

  /* Display the individual values */
  Serial.print("Sys:");
  Serial.print(system, DEC);
  Serial.print(" G:");
  Serial.print(gyro, DEC);
  Serial.print(" A:");
  Serial.print(accel, DEC);
  Serial.print(" M:");
  Serial.print(mag, DEC);
}

void setup_bno(void) {
  /* Initialise the sensor */
  if (!bno.begin()) {
    Serial.print(
        "Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1) {
    }
  }

  adafruit_bno055_offsets_t calibrationData;
  calibrationData.accel_offset_x = -19;
  calibrationData.accel_offset_y = 3;
  calibrationData.accel_offset_z = -20;
  calibrationData.gyro_offset_x = 0;
  calibrationData.gyro_offset_y = -3;
  calibrationData.gyro_offset_z = 0;
  calibrationData.mag_offset_x = -118;
  calibrationData.mag_offset_y = 8;
  calibrationData.mag_offset_z = -99;
  calibrationData.accel_radius = 1000;
  calibrationData.mag_radius = 763;

  Serial.println("\n\nRestoring Calibration data to the BNO055...");
  bno.setSensorOffsets(calibrationData);
  Serial.println("\n\nCalibration data loaded into BNO055");

  delay(1000);  // TODO required?

  displaySensorDetails();
  displaySensorStatus();

  /* Crystal must be configured AFTER loading calibration data into BNO055. */
  bno.setExtCrystalUse(true);

  sensors_event_t event;
  bno.getEvent(&event);
  Serial.println("Please finish calibrating sensor: ");
  while (!bno.isFullyCalibrated()) {
    bno.getEvent(&event);

    Serial.print("X: ");
    Serial.print(event.orientation.x, 4);
    Serial.print("\tY: ");
    Serial.print(event.orientation.y, 4);
    Serial.print("\tZ: ");
    Serial.print(event.orientation.z, 4);

    /* Optional: Display calibration status */
    displayCalStatus();

    /* New line for the next sample */
    Serial.println("");

    /* Wait the specified delay before requesting new data */
    delay(BNO055_SAMPLERATE_DELAY_MS);
  }

  Serial.println("\nFully calibrated!");
  Serial.println("--------------------------------");
  //   delay(500); // TODO needed?
}

void setup_wifi() {
  // WiFi.begin("Baba Ganoush (SS)", "PlsNoTorrent");
  WiFi.begin("chaseme", "gobubbles");

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // WiFi.printDiag(Serial);
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  // client.setNoDelay(true);
}

void setup(void) {
  delay(3000);  // power-up safety delay for LEDs, TODO is this necessary?
  Serial.begin(115200);
  waitForSerial();

  Serial.println("Swing!");

  if (run_type == DO_USE_RECORDED_DATA) {
    calc_on_recorded_data();
  } else if (run_type == RECORD_DATA) {
    setup_bno();
    setup_wifi();
  } else if (run_type == IS_LEADER) {
    setup_bno();
    Radio_Init();
  } else if (run_type == IS_FOLLOWER) {
    Radio_Init();
    LED_Init();
  }
}

int MSG_LEN = 9;
uint8_t data[MAX_DATA_LEN];
int data_loc = 0;
uint8_t skipped_data_count = 0;

void add_data(uint16_t w, uint16_t x, uint16_t y, uint16_t z) {
  if (data_loc >= MAX_DATA_LEN - MSG_LEN - 1) {
    if (skipped_data_count < 0xFF) {
      skipped_data_count += 1;
    }
    return;
  }
  data[data_loc + 0] = skipped_data_count;
  data[data_loc + 1] = w >> 8;
  data[data_loc + 2] = w & 0xFF;
  data[data_loc + 3] = x >> 8;
  data[data_loc + 4] = x & 0xFF;
  data[data_loc + 5] = y >> 8;
  data[data_loc + 6] = y & 0xFF;
  data[data_loc + 7] = z >> 8;
  data[data_loc + 8] = z & 0xFF;
  data_loc += MSG_LEN;
}

void send_data(uint16_t w, uint16_t x, uint16_t y, uint16_t z) {
  add_data(w, x, y, z);

  int to_send = min(client.availableForWrite(), (size_t)data_loc);
  to_send = (to_send / MSG_LEN) * MSG_LEN;  // Full messages at a time
  if (to_send > 0) {
    Serial.print("send: ");
    Serial.println(to_send);
    client.write(data, to_send);
    Serial.println("done send");
    if (data_loc > to_send) {
      Serial.println("reordering data");
      for (int i = to_send; i < data_loc; i++) {
        data[i - to_send] = data[i];
      }
    }
    data_loc = 0;
  }
}

// Accelerometer is running at 100hz, so poll the accelerometer a bit faster
// than that (not slower)
const int LOOP_DELAY__ms = 10;
void loop() {
  if (run_type == DO_USE_RECORDED_DATA) {
    return;
  } else if (run_type == RECORD_DATA) {
    const char* addr = "192.168.111.16";
    if (!client.connected()) {
      if (client.connect(addr, 9000)) {
        Serial.println("connected!");
      } else {
        Serial.print("Failed to connect to ");
        Serial.println(addr);
        delay(500);
        return;
      }
    }
    adafruit_bno055_raw_quat_t raw_quat;
    bno.getRawQuat(raw_quat);
    send_data(raw_quat.w, raw_quat.x, raw_quat.y, raw_quat.z);
    // delay(LOOP_DELAY__ms);
    delay(BNO055_SAMPLERATE_DELAY_MS);
  } else if (run_type == IS_LEADER) {
    // mma.read(); TODO..
    // uint16_t swing_mag = AngleEstimate::get_mag(mma.x, mma.y, mma.z);
    // Radio_Update(swing_mag);
    delay(LOOP_DELAY__ms);
  } else if (run_type == IS_FOLLOWER) {
    uint16_t mag = Radio_GetRecentMag();
    if (mag != 0) {
      estimator.add_mag(mag);
      LED_Update(estimator.get_angle());
    }
  }
}
