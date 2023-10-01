#include "led.h"
#include "radio.h"
#include <Arduino.h>
#include <FastLED.h>
#include <stdio.h>
#include <string.h>

// Derives from https://github.com/HarringayMakerSpace/ESP-Now
#include <ESP8266WiFi.h>
#include <_types/_uint8_t.h>
#include <ios>
extern "C" {
#include <espnow.h>
#include <user_interface.h>
}

#define WIFI_CHANNEL 4

const bool isMaster = false; // True for only one device

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

enum States_t {
  SLEEP_LISTEN = 1,
  DOOR_DASH_WAITING = 2,
  DOOR_DASH_WINNER = 3,
  DOOR_DASH_LOSER = 4
} States;
States_t globalState = SLEEP_LISTEN;

const unsigned long SLEEP_DURATION__us = 1e6;
const unsigned long LISTEN_TIME__ms = 50;

void waitForSerial() {
  while (!Serial) {
    delay(1);
  }
}

bool isMacAddressSelf(uint8_t *mac) {
  uint8_t *selfMacAddress = WiFi.macAddress();
  for (int i = 0; i < 6; i++) {
    if (mac[i] != selfMacAddress[i]) {
      return false;
    }
  }
  return true;
}

long wakeTime = 0;

void setup() {
  // delay( 3000 ); // power-up safety delay for LEDs, TODO is this necessary?
  Serial.begin(115200);
  waitForSerial();
  Radio_Init();
  Serial.println("Hello world!");

  wakeTime = millis();

  while (millis() - wakeTime < LISTEN_TIME__ms) {}
  if (globalState == SLEEP_LISTEN) {
    Serial.println("Back to sleep");
    ESP.deepSleep(SLEEP_DURATION__us);
  } else {
    // TODO
  }
}

void loop() { Serial.println("ERROR, this should never run"); }

struct __attribute__((packed)) DataStruct {
  // Device sending to master that the device's button was pressed
  uint8_t button_pressed_mac[6]; // M1

  // Master sends a message to everyone about who the winner
  uint8_t winner_mac[6]; // M2
};

bool hasWinnerMsg(DataStruct data) {
  for (int i = 0; i < 6; i++) {
    if (data.winner_mac[i] != 0) {
      return true;
    }
  }
  return false;
}

unsigned long lastSentMillis = 0;
unsigned long sendIntervalMillis = 3000;
bool friendNewlyFound = false;
bool quickRespond = false;
unsigned long friendTimeout = 5000;
unsigned long lastSeenFriendTime = 0;

void sendButtonPressed() {
  DataStruct sendingData = {};
  sendingData.button_pressed_mac = WiFi.macAddress();
  // uint8_t byteArray[sizeof(sendingData)];
  // memcpy(byteArray, &sendingData, sizeof(sendingData));
  esp_now_send(broadcastMac, (uint8_t *)sendingData,
               sizeof(sendingData)); // NULL means send to all peers
}

// Only master sends this
void sendWinnerSelected(uint8_t[] winnerMac) {
  DataStruct sendingData = {};
  sendingData.winner_mac = winnerMac;
  esp_now_send(broadcastMac, (uint8_t *)sendingData,
               sizeof(sendingData)); // NULL means send to all peers
}

void receiveCallBackFunction(uint8_t *senderMac, uint8_t *incomingData,
                             uint8_t len) {

  DataStruct receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  // Relaying messages
  // Changing state
  if (hasWinnerMsg(receivedData)) {
    // M2 message
    if isMacAddressSelf (receivedData.winner_mac) {
      globalState = DOOR_DASH_WINNER;
    } else {
      globalState = DOOR_DASH_LOSER
    }
  } else {
    // M1 message
    if (globalState == SLEEP_LISTEN) {
      globalState = DOOR_DASH_WAITING;
    }
  }

  // TODO relay messages to others
}

void PrintMac(uint8_t *macaddr) {
  for (byte n = 0; n < 6; n++) {
    Serial.print(macaddr[n], HEX);
  }
}

void Radio_Init() {
  if (esp_now_init() != 0) {
    Serial.println("*** ESP_Now init failed");
    while (true) {
    };
  }
  // role set to COMBO so it can send and receive - not sure this is essential
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  Serial.println("Starting Master");

  WiFi.mode(WIFI_STA); // Station mode for esp-now controller

  Serial.printf("This mac: %s, ", WiFi.macAddress().c_str());
  Serial.printf("target mac: %02x%02x%02x%02x%02x%02x", broadcastMac[0],
                broadcastMac[1], broadcastMac[2], broadcastMac[3],
                broadcastMac[4], broadcastMac[5]);
  Serial.printf(", channel: %i\n", WIFI_CHANNEL);

  esp_now_add_peer(broadcastMac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);

  Serial.println("Setup finished");

  esp_now_register_recv_cb(receiveCallBackFunction);
}
