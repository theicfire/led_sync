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
#define OUTPUT_GPIO_PIN 2

const bool IS_COORDINATOR = false; // True for only one device

uint8_t const broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF}; // NULL means send to all peers

enum States_t {
  SLEEP_LISTEN = 1,
  DOOR_DASH_WAITING = 2,
  DOOR_DASH_WINNER = 3,
  DOOR_DASH_LOSER = 4,
  DOOR_DASH_COOL_DOWN = 5
} States;
States_t globalState = SLEEP_LISTEN;

const unsigned long SLEEP_DURATION__us = 1e6;
const unsigned long LISTEN_TIME__ms = 50;
const unsigned long DOOR_DASH_REBROADCAST_INTERVAL__ms = 20;
const unsigned long DOOR_DASH_WAITING_FLASH_FREQUENCY__ms = 500;
const unsigned long DOOR_DASH_WINNER_FLASH_FREQUENCY__ms = 120;
const unsigned long FLASH_DURATION__ms = 5e3;
const unsigned long COOL_DOWN__ms = 15e3;

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

unsigned long wakeTime = 0;
unsigned long messageReceivedAt = 0;
uint8_t winnerMac[6] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

void setup() {
  Serial.begin(115200);
  waitForSerial();
  Serial.println("Hello world!");
  Radio_Init();
  if (IS_COORDINATOR) {
    setupCoordinator();
  } else {
    setupButton();
  }
}

void setupButton() {
  wakeTime = millis();

  // Wait for a message to have been received
  while (millis() - wakeTime < LISTEN_TIME__ms) {
  }

  if (globalState == SLEEP_LISTEN) {
    Serial.println("Back to sleep");
    ESP.deepSleep(SLEEP_DURATION__us);
  }

  unsigned long lastBroadcast = 0;
  while (true) {
    if (globalState == DOOR_DASH_WAITING) {
      // Slowly flash LED
      if ((millis() - messageReceivedAt) % 1000 <
          DOOR_DASH_WAITING_FLASH_FREQUENCY__ms) {
        gpio_set_level(OUTPUT_GPIO_PIN, 1);
      } else {
        gpio_set_level(OUTPUT_GPIO_PIN, 0);
      }
      // Rebroadcast button pressed every 20ms
      if (millis() - lastBroadcast > DOOR_DASH_REBROADCAST_INTERVAL__ms) {
        sendButtonPressed();
        lastBroadcast = millis();
      }
      // If more than 20 seconds have passed, then go to sleep
      if (millis() - messageReceivedAt > FLASH_DURATION__ms + COOL_DOWN__ms) {
        ESP.deepSleep(SLEEP_DURATION__us);
      }
    } else if (globalState == DOOR_DASH_WINNER) {
      // Broadcast repeatedly who the winner is
      if (millis() - lastBroadcast > DOOR_DASH_REBROADCAST_INTERVAL__ms) {
        sendWinner(winnerMac);
        lastBroadcast = millis();
      }
      // Flash LED fast
      if ((millis() - messageReceivedAt) % 200 <
          DOOR_DASH_WINNER_FLASH_FREQUENCY__ms) {
        gpio_set_level(OUTPUT_GPIO_PIN, 1);
      } else {
        gpio_set_level(OUTPUT_GPIO_PIN, 0);
      }
      // If more than 5 seconds have passed, go to cool down state
      if (millis() - messageReceivedAt > FLASH_DURATION__ms) {
        globalState = DOOR_DASH_COOL_DOWN;
      }
    } else if (globalState == DOOR_DASH_LOSER) {
      // Broadcast repeatedly who the winner is
      if (millis() - lastBroadcast > DOOR_DASH_REBROADCAST_INTERVAL__ms) {
        sendWinner(winnerMac);
        lastBroadcast = millis();
      }
      // If more than 5 seconds have passed, go to cool down state
      if (millis() - messageReceivedAt > FLASH_DURATION__ms) {
        globalState = DOOR_DASH_COOL_DOWN;
      }
    } else { // DOOR_DASH_COOL_DOWN
      // Sleep after cool down period is over
      if (millis() - messageReceivedAt > FLASH_DURATION__ms + COOL_DOWN__ms) {
        ESP.deepSleep(SLEEP_DURATION__us);
      }
    }
  }
}

void setupCoordinator() {
  // Coordinator pseudo-code
  // Whenever a button press comes in, mark the time, and send M2 to all other
  // devices with the mac address of the winner. all messages coming in after
  // the first one are ignored for the next TBD time (maybe the same length of
  // time as the door dash cool down period or a bit longer?).
}

void loop() { Serial.println("ERROR, this should never run"); }

struct __attribute__((packed)) DataStruct {
  // Device sending to master that the device's button was pressed
  uint8_t button_pressed_mac[6]; // M1

  // Master sends a message to everyone about who the winner
  uint8_t winner_mac[6]; // M2
};

// // Scenario: my button is pressed
// // Do I update my own state, or do I wait to hear from the master?
// FAR                  CLOSE                MASTER
//                     Pressed
//                     broadcast
// update state to M1  Update state to M1
// // Master never has to tell others that a button has been pressed

// A                    B                    C
// M1 + broadcast
//                    receives M1
//                    updates state to M1
//                    rebroadcasts M1
// Ignores M1                                Receives M1, updates state,
// rebroadcasts

bool hasWinnerMsg(DataStruct data) {
  for (int i = 0; i < 6; i++) {
    if (data.winner_mac[i] != 0) {
      return true;
    }
  }
  return false;
}

void sendButtonPressed() {
  DataStruct sendingData = {};
  sendingData.button_pressed_mac = WiFi.macAddress();
  esp_now_send(broadcastMac, (uint8_t *)sendingData,
               sizeof(sendingData)); // NULL means send to all peers
}

void sendWinner(uint8_t[6] winner) {
  DataStruct sendingData = {};
  sendingData.winner_mac = winner;
  esp_now_send(broadcastMac, (uint8_t *)sendingData,
               sizeof(sendingData)); // NULL means send to all peers
}

// Only master sends this
void sendWinnerSelected(uint8_t[] winnerMac) {
  DataStruct sendingData = {};
  sendingData.winner_mac = winnerMac;
  esp_now_send(broadcastMac, (uint8_t *)sendingData, sizeof(sendingData));
}

void rebroadcast(DataStruct *data) {
  esp_now_send(broadcastMac, (uint8_t *)data, sizeof(data));
}

void receiveCallBackFunction(uint8_t *senderMac, uint8_t *incomingData,
                             uint8_t len) {

  DataStruct receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  // Handle state changes, and rebroadcasting
  if (hasWinnerMsg(receivedData)) { // M2 message
    if (globalState == SLEEP_LISTEN || globalState == DOOR_DASH_WAITING) {
      winnerMac = receivedData.winner_mac;
      if isMacAddressSelf (receivedData.winner_mac) {
        globalState = DOOR_DASH_WINNER;
      } else {
        globalState = DOOR_DASH_LOSER;
      }
      rebroadcast(&receivedData);
    }
  } else { // M1 message
    if (globalState == SLEEP_LISTEN) {
      globalState = DOOR_DASH_WAITING;
      rebroadcast(&receivedData);
    }
  }
  messageReceivedAt = millis();
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
