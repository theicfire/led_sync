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

const int WIFI_CHANNEL = 4;
const int BUTTON_INPUT = D1;
const int BUTTON_LED = D2;

const bool IS_COORDINATOR = true; // True for only one device

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
const unsigned long DOOR_DASH_COORDINATION_DURATION__ms = 17e3;
const unsigned long FLASH_DURATION__ms = 5e3;
const unsigned long COOL_DOWN__ms = 15e3;

/* While the capacitor is charged, the button will not be able to reset the
 * ESP*/
void keepCapacitorCharged() {
  pinMode(BUTTON_INPUT, OUTPUT);
  digitalWrite(BUTTON_INPUT, HIGH); // Prevent button from resetting
}

/* Before going to sleep, the capacitor needs to discharge so that we don't
 * prevent the button from waking the ESP back up.*/
void dischargeCapacitor() {
  pinMode(BUTTON_INPUT, OUTPUT);
  digitalWrite(BUTTON_INPUT, LOW); // Discharge capacitor
  delay(5);
}

void setupSerial() {
  Serial.begin(115200);
  waitForSerial();
  Serial.println("Hello world!");
}

void transitionState(States_t newState) {
  globalState = newState;
  Serial.printf("Transitioning to state %i\n", newState);
}

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
unsigned long doorDashStartedAt = 0;
bool hasDeclaredWinner = false;
uint8_t winnerMac[6] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

void setup() {
  if (IS_COORDINATOR) {
    setupCoordinator();
  } else {
    setupButton();
  }
}

void setupButton() {
  pinMode(BUTTON_INPUT, INPUT);
  bool btnPressed = digitalRead(BUTTON_INPUT);
  setupSerial();
  Radio_Init(buttonCallBackFunction);
  wakeTime = millis();

  if (!btnPressed) {
    // Wait for a message to have been received
    while (millis() - wakeTime < LISTEN_TIME__ms) {
    }

    if (globalState == SLEEP_LISTEN) {
      Serial.println("Back to sleep");
      ESP.deepSleep(SLEEP_DURATION__us);
    }
  }
  // At this point, one of two things has happened: the button was pressed, or
  // we received a message

  if (btnPressed) {
    transitionState(DOOR_DASH_WAITING);
    doorDashStartedAt = millis();
  }

  keepCapacitorCharged(); // Prevent button from resetting mid-doordash

  unsigned long lastBroadcast = 0;
  while (true) {
    if (globalState == DOOR_DASH_WAITING) {
      // Slowly flash LED
      if ((millis() - doorDashStartedAt) %
              (DOOR_DASH_WAITING_FLASH_FREQUENCY__ms * 2) <
          DOOR_DASH_WAITING_FLASH_FREQUENCY__ms) {
        digitalWrite(BUTTON_LED, HIGH);
      } else {
        digitalWrite(BUTTON_LED, LOW);
      }
      // Rebroadcast button pressed every 20ms
      if (millis() - lastBroadcast > DOOR_DASH_REBROADCAST_INTERVAL__ms) {
        sendButtonPressed();
        lastBroadcast = millis();
      }
      // If more than 20 seconds have passed, then go to sleep
      if (millis() - doorDashStartedAt > FLASH_DURATION__ms + COOL_DOWN__ms) {
        ESP.deepSleep(SLEEP_DURATION__us);
      }
    } else if (globalState == DOOR_DASH_WINNER) {
      // Broadcast repeatedly who the winner is
      if (millis() - lastBroadcast > DOOR_DASH_REBROADCAST_INTERVAL__ms) {
        sendWinner(winnerMac);
        lastBroadcast = millis();
      }
      // Flash LED fast
      if ((millis() - doorDashStartedAt) %
              (DOOR_DASH_WINNER_FLASH_FREQUENCY__ms * 2) <
          DOOR_DASH_WINNER_FLASH_FREQUENCY__ms) {
        digitalWrite(BUTTON_LED, HIGH);
      } else {
        digitalWrite(BUTTON_LED, LOW);
      }
      // If more than 5 seconds have passed, go to cool down state
      if (millis() - doorDashStartedAt > FLASH_DURATION__ms) {
        transitionState(DOOR_DASH_COOL_DOWN);
      }
    } else if (globalState == DOOR_DASH_LOSER) {
      // Broadcast repeatedly who the winner is
      if (millis() - lastBroadcast > DOOR_DASH_REBROADCAST_INTERVAL__ms) {
        sendWinner(winnerMac);
        lastBroadcast = millis();
      }
      digitalWrite(BUTTON_LED, HIGH);
      // If more than 5 seconds have passed, go to cool down state
      if (millis() - doorDashStartedAt > FLASH_DURATION__ms) {
        transitionState(DOOR_DASH_COOL_DOWN);
      }
    } else { // DOOR_DASH_COOL_DOWN
      // Sleep after cool down period is over
      if (millis() - doorDashStartedAt > FLASH_DURATION__ms + COOL_DOWN__ms) {
        dischargeCapacitor();
        ESP.deepSleep(SLEEP_DURATION__us);
      }
    }
  }
}

void setupCoordinator() {
  setupSerial();
  Radio_Init(coordinatorCallBackFunction);

  while (true) {
    if (hasDeclaredWinner &&
        millis() - doorDashStartedAt > DOOR_DASH_COORDINATION_DURATION__ms) {
      Serial.println("Resetting after doordash");
      doorDashStartedAt = 0;
      hasDeclaredWinner = false;
    }
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

void coordinatorCallBackFunction(uint8_t *senderMac, uint8_t *incomingData,
                                 uint8_t len) {
  rebroadcast(&incomingData); // Could be optimized to only rebroadcast M2s

  if (!hasDeclaredWinner) {
    Serial.print("Declare winner: ");
    printMac(incomingData.button_pressed_mac);
    winnerMac = incomingData.button_pressed_mac;
    doorDashStartedAt = millis();
    hasDeclaredWinner = true;
  }
}

void buttonCallBackFunction(uint8_t *senderMac, uint8_t *incomingData,
                            uint8_t len) {

  // Handle state changes, and rebroadcasting
  if (hasWinnerMsg(incomingData)) { // M2 message
    if (globalState == SLEEP_LISTEN || globalState == DOOR_DASH_WAITING) {
      winnerMac = incomingData.winner_mac;
      if isMacAddressSelf (incomingData.winner_mac) {
        transitionState(DOOR_DASH_WINNER);
      } else {
        transitionState(DOOR_DASH_LOSER);
      }
      rebroadcast(&incomingData);
    }
  } else { // M1 message
    if (globalState == SLEEP_LISTEN) {
      transitionState(DOOR_DASH_WAITING);
      rebroadcast(&incomingData);
    }
  }
  if (doorDashStartedAt == 0) {
    // Gets reset after the button goes to sleep
    doorDashStartedAt = millis();
  }
}

void printMac(uint8_t *macaddr) {
  for (byte n = 0; n < 6; n++) {
    Serial.print(macaddr[n], HEX);
  }
}

void Radio_Init(std::function<void(uint8_t *, uint8_t *, uint8_t)>
                    receiveCallBackFunction) {
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
