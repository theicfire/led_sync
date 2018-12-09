// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX
 
#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include "time.h"
 
// for Feather32u4 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
 
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 868.0
 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

unsigned long lastTimeSent = 0;
bool isMaster = false;
 
// Blinky on receipt
#define LED 13

#define SENDING_PERIOD__ms 1000

static void Radio_ResetModule() {
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
}

void Radio_Init()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
 
  delay(100); // TODO why?
 
  Serial.println("Feather LoRa RX Test!");
 
  Radio_ResetModule(); // TODO why?
 
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }

  Serial.println("LoRa radio init OK!");
 
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
 
  rf95.setTxPower(23, false);
}

static unsigned long Radio_BytesToLong(uint8_t* buf, uint8_t len) {
    unsigned long ret = 0;
    for (uint8_t i = 0; i < len; i++) {
        ret += ((unsigned long) buf[i]) << (i * 8);
    }
    return ret;
}

// TODO statically require buf to be size 4?
static void Radio_LongToBytes(unsigned long x, uint8_t* buf) {
    for (uint8_t i = 0; i < 4; i++) {
        buf[i] = (x >> (i * 8)) & 0xFF;
    }
}

 
static void Radio_SlaveUpdate()
{
  if (rf95.available())
  {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
 
    if (rf95.recv(buf, &len))
    {
        RH_RF95::printBuffer("Received: ", buf, len);
        unsigned long time__ms = Radio_BytesToLong(buf, len);
        Time_SetTime(time__ms);
        Serial.print("Setting time: "); Serial.println(time__ms);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}

static void Radio_MasterUpdate() {
  if (lastTimeSent + SENDING_PERIOD__ms > Time_GetTime()) {
    return;
  }
  lastTimeSent = Time_GetTime();

  Serial.println("Transmitting...");
  
  uint8_t radiopacket[5];
  Radio_LongToBytes(Time_GetTime(), radiopacket);
  radiopacket[4] = 0; // TODO, is this needed?

  RH_RF95::printBuffer("Sending: ", radiopacket, 5);
  delay(10); // TODO, is this needed?
  rf95.send(radiopacket, 5);

  Serial.println("Waiting for packet to complete..."); 
  delay(10); // TODO, is this needed?
  rf95.waitPacketSent();
}

void Radio_Update() {
    if (isMaster) {
        Radio_MasterUpdate();
    } else {
        Radio_SlaveUpdate();
    }
}