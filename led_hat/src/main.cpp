#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>


void waitForSerial() {
  while (!Serial) {
    delay(1);
  }
}

Adafruit_MMA8451 mma = Adafruit_MMA8451();
WiFiClient client;

void setup(void) {
  delay( 3000 ); // power-up safety delay for LEDs, TODO is this necessary?
  Serial.begin(115200);
  waitForSerial();

  Serial.println("Adafruit MMA8451 test!");


  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("MMA8451 found!");

  mma.setRange(MMA8451_RANGE_2_G);
  mma.setDataRate(MMA8451_DATARATE_100_HZ);

  Serial.print("Range = "); Serial.print(2 << mma.getRange());
  Serial.println("G");
  Serial.print("Rate = "); Serial.println(mma.getDataRate());

  //WiFi.begin("Baba Ganoush (SS)", "PlsNoTorrent");
  WiFi.begin("chaseme", "gobubbles");

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    //WiFi.printDiag(Serial);
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  //if (client.connect("10.0.0.8", 9000))
  if (client.connect("192.168.137.16", 9000))
  {
    Serial.println("connected!");
  } else {
    Serial.println("Failed to connect!");
  }
  //client.setNoDelay(true);

}

int count = 0;
uint8_t data[1400];
int data_loc = 0;

void add_data(uint16_t x, uint16_t y, uint16_t z) {
  if (data_loc >= 1400 - 7) {
    return;
  }
  data[data_loc + 0] = x >> 8;
  data[data_loc + 1] = x & 0xFF;
  data[data_loc + 2] = y >> 8;
  data[data_loc + 3] = y & 0xFF;
  data[data_loc + 4] = z >> 8;
  data[data_loc + 5] = z & 0xFF;
  data_loc += 6;
}

void loop() {
  count += 1;
  // Read the 'raw' data in 14-bit counts
  //Serial.print("read ");
  //Serial.println(count);
  mma.read();
  add_data(mma.x, mma.y, mma.z);
  Serial.print("X:\t"); Serial.print(mma.x);
  //Serial.print("\tY:\t"); Serial.print(mma.y);
  //Serial.print("\tZ:\t"); Serial.print(mma.z);
  Serial.println();

  /* Get a new sensor event */
  //sensors_event_t event;
  //mma.getEvent(&event);

  //[> Display the results (acceleration is measured in m/s^2) <]
  //Serial.print(event.acceleration.x); Serial.print("\t");
  //Serial.print(event.acceleration.y); Serial.print("\t");
  //Serial.print(event.acceleration.z); Serial.print("\t");
  //Serial.println("m/s^2 ");

  int to_send = min(client.availableForWrite(), (size_t) data_loc);
  if (client.connected() && to_send > 0) {
    Serial.print("send: ");
    Serial.println(to_send);
    client.write(data, to_send);
    Serial.println("done send");
    data_loc = 0;
  }
  delay(10);
}
