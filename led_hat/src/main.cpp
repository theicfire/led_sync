#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
// #include <HardwareSerial.h>
#include <Wire.h>

#include "angle_estimate.h"
#include "led.h"
#include "radio.h"

//#define USE_RECORDED_DATA
#ifdef USE_RECORDED_DATA
#include "data.h"
#endif

#define MAX_DATA_LEN (5000)

Adafruit_MMA8451 mma = Adafruit_MMA8451();
WiFiClient client;
// TODO make enum
const bool RECORD_DATA = true;
const bool IS_LEADER = false;

void waitForSerial() {
  while (!Serial) {
    delay(1);
  }
}

#ifdef USE_RECORDED_DATA
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
#endif

void setup_accel() {
  if (!mma.begin()) {
    Serial.println("Failed to start accelerometer");
    while (1)
      ;
  }
  Serial.println("MMA8451 found!");

  mma.setRange(MMA8451_RANGE_2_G);
  mma.setDataRate(MMA8451_DATARATE_100_HZ);

  Serial.print("Range = ");
  Serial.print(2 << mma.getRange());
  Serial.println("G");
  Serial.print("Rate = ");
  Serial.println(mma.getDataRate());
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
  // if (client.connect("10.0.0.8", 9000))
  // client.setNoDelay(true);
}

void setup(void) {
  delay(3000);  // power-up safety delay for LEDs, TODO is this necessary?
  Serial.begin(115200);
  waitForSerial();

  Serial.println("Swing!");

#ifdef USE_RECORDED_DATA
  // calc_on_recorded_data();
#else
  if (RECORD_DATA) {
    setup_accel();
    setup_wifi();
  } else if (IS_LEADER) {
    setup_accel();
    Radio_Init();
  } else {
    Radio_Init();
    LED_Init();
  }
#endif
}

int MSG_LEN = 7;
uint8_t data[MAX_DATA_LEN];
int data_loc = 0;
uint8_t skipped_data_count = 0;

void add_data(uint16_t x, uint16_t y, uint16_t z) {
  if (data_loc >= MAX_DATA_LEN - MSG_LEN - 1) {
    if (skipped_data_count < 0xFF) {
      skipped_data_count += 1;
    }
    return;
  }
  data[data_loc + 0] = skipped_data_count;
  data[data_loc + 1] = x >> 8;
  data[data_loc + 2] = x & 0xFF;
  data[data_loc + 3] = y >> 8;
  data[data_loc + 4] = y & 0xFF;
  data[data_loc + 5] = z >> 8;
  data[data_loc + 6] = z & 0xFF;
  data_loc += MSG_LEN;
}

void send_data(uint16_t x, uint16_t y, uint16_t z) {
  add_data(x, y, z);

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

AngleEstimate estimator;
int count = 0;
unsigned long last__us;
void loop() {
#ifdef USE_RECORDED_DATA
  return;
#endif
  if (RECORD_DATA) {
    const char* addr = "192.168.40.16";
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
    mma.read();
    send_data(mma.x, mma.y, mma.z);
    delay(10);
  } else if (IS_LEADER) {
    mma.read();
    uint16_t swing_mag = AngleEstimate::get_mag(mma.x, mma.y, mma.z);
    Radio_Update(swing_mag);
    delay(8);

    // unsigned long end__us = micros();
    // if (count % 10 == 0) {
    // Serial.println(mag);
    // Serial.println((uint16_t) mag);
    //}
    // last__us = end__us;
  } else {
    uint16_t mag = Radio_GetRecentMag();
    if (mag != 0) {
      // Serial.print("Mag: ");
      // Serial.println(mag);
      estimator.add_mag(mag);
      LED_Update(estimator.get_angle());
    }
  }
}
