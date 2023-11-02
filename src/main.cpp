#include <Arduino.h>
#include <FastLED.h>
#include <stdio.h>
#include <string.h>

// Derives from https://github.com/HarringayMakerSpace/ESP-Now
#include <ESP8266WiFi.h>

#include <ios>
extern "C" {
#include <espnow.h>
#include <user_interface.h>
}

const int WIFI_CHANNEL = 4;
const int BUTTON_INPUT = D1;
const int BUTTON_LED = D2;

const bool IS_COORDINATOR = false; // True for only one device

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF,
                          0xFF, 0xFF, 0xFF}; // NULL means send to all peers

enum States_t {
  SLEEP_LISTEN = 1,
  DOOR_DASH_WAITING = 2,
  DOOR_DASH_WINNER = 3,
  DOOR_DASH_LOSER = 4,
  DOOR_DASH_COOL_DOWN_WINNER = 5,
  DOOR_DASH_COOL_DOWN_LOSER = 6,
  DOOR_DASH_COOL_DOWN_UNKNOWN = 7,
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

unsigned long wakeTime = 0;
unsigned long doorDashStartedAt = 0;
bool hasDeclaredWinner = false;
uint8_t winnerMac[6] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

struct __attribute__((packed)) DataStruct {
  // Device sending to master that the device's button was pressed
  uint8_t button_pressed_mac[6]; // PRESSED_MSG

  // Master sends a message to everyone about who the winner
  uint8_t winner_mac[6]; // WINNER_MSG
};

/* While the capacitor is charged, the button will not be able to reset the
 * ESP*/
void keepCapacitorCharged() {
  pinMode(BUTTON_INPUT, OUTPUT);
  digitalWrite(BUTTON_INPUT, HIGH); // Prevent button from resetting
}

/* Before going to sleep, the capacitor needs to discharge so that we don't
 * prevent the button from waking the ESP back up.*/
void goToSleep() {
  pinMode(BUTTON_INPUT, OUTPUT);
  digitalWrite(BUTTON_INPUT, LOW); // Discharge capacitor
  delay(5);

  Serial.println("Going to sleep");
  delay(10); // TODO remove I think
  ESP.deepSleep(SLEEP_DURATION__us);
}

bool isWinnerMsg(DataStruct *data) {
  for (int i = 0; i < 6; i++) {
    if (data->winner_mac[i] != 0) {
      return true;
    }
  }
  return false;
}

void waitForSerial() {
  while (!Serial) {
    delay(1);
  }
}

void callWatchdog() { yield(); }

void setupSerial() {
  Serial.begin(115200);
  waitForSerial();
  Serial.println("Hello world!");
}

void transitionState(States_t newState) {
  globalState = newState;

  switch (newState) {
  case SLEEP_LISTEN:
    Serial.println("Transitioned to SLEEP_LISTEN");
    break;
  case DOOR_DASH_WAITING:
    Serial.println("Transitioned to DOOR_DASH_WAITING");
    break;
  case DOOR_DASH_WINNER:
    Serial.println("Transitioned to DOOR_DASH_WINNER");
    break;
  case DOOR_DASH_LOSER:
    Serial.println("Transitioned to DOOR_DASH_LOSER");
    break;
  case DOOR_DASH_COOL_DOWN_WINNER:
    Serial.println("Transitioned to DOOR_DASH_COOL_DOWN_WINNER");
    break;
  case DOOR_DASH_COOL_DOWN_LOSER:
    Serial.println("Transitioned to DOOR_DASH_COOL_DOWN_LOSER");
    break;
  case DOOR_DASH_COOL_DOWN_UNKNOWN:
    Serial.println("Transitioned to DOOR_DASH_COOL_DOWN_UNKNOWN");
    break;
  default:
    Serial.println("Transitioned to ERROR, unknown state");
    break;
  }
}

void printMac(uint8_t *macaddr) {
  for (byte n = 0; n < 6; n++) {
    Serial.print(macaddr[n], HEX);
  }
}

void setMacAddress(uint8_t *mac) { WiFi.macAddress(mac); }

void sendButtonPressed() {
  DataStruct sendingData = {};
  setMacAddress(
      (uint8_t *)&sendingData.button_pressed_mac); // TODO make this nicer
  esp_now_send(broadcastMac, (uint8_t *)&sendingData,
               sizeof(sendingData)); // NULL means send to all peers
}

void sendWinner(uint8_t *winner) {
  DataStruct sendingData = {};
  // TODO make it cleaner instead of this 6. Make it a struct instead?
  memcpy((uint8_t *)sendingData.winner_mac, winner, 6);
  esp_now_send(broadcastMac, (uint8_t *)&sendingData,
               sizeof(sendingData)); // NULL means send to all peers
}

void rebroadcast(uint8_t *data, uint8_t len) {
  esp_now_send(broadcastMac, data, len);
}

bool isMacAddressSelf(uint8_t *mac) {
  uint8_t selfMacAddress[6] = {};
  setMacAddress((uint8_t *)selfMacAddress);
  for (int i = 0; i < 6; i++) {
    if (mac[i] != selfMacAddress[i]) {
      return false;
    }
  }
  return true;
}

void coordinatorCallBackFunction(uint8_t *senderMac, uint8_t *incomingData,
                                 uint8_t len) {
  DataStruct *data = (DataStruct *)incomingData;
  if (!hasDeclaredWinner) {
    Serial.print("Declare winner: ");
    printMac(data->button_pressed_mac);
    Serial.println();
    memcpy((uint8_t *)winnerMac, data->button_pressed_mac, 6);

    doorDashStartedAt = millis();
    hasDeclaredWinner = true;
  }

  if (hasDeclaredWinner) {
    sendWinner((uint8_t *)winnerMac);
  }
}

void buttonCallBackFunction(uint8_t *senderMac, uint8_t *incomingData,
                            uint8_t len) {

  DataStruct *data = (DataStruct *)incomingData;
  // Handle state changes, and rebroadcasting
  if (isWinnerMsg(data)) { // WINNER_MSG
    if (globalState == SLEEP_LISTEN || globalState == DOOR_DASH_WAITING) {
      memcpy(winnerMac, data->winner_mac, 6);
      if (isMacAddressSelf(data->winner_mac)) {
        transitionState(DOOR_DASH_WINNER);
      } else {
        transitionState(DOOR_DASH_LOSER);
      }

      rebroadcast(incomingData, len);
    }
  } else { // PRESSED_MSG
    if (globalState == SLEEP_LISTEN) {
      transitionState(DOOR_DASH_WAITING);

      rebroadcast(incomingData, len);
    }
  }
  if (doorDashStartedAt == 0) {
    // Gets reset after the button goes to sleep
    doorDashStartedAt = millis();
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

  WiFi.mode(WIFI_STA); // Station mode for esp-now controller

  Serial.printf("This mac: %s, ", WiFi.macAddress().c_str());
  Serial.printf("target mac: %02x%02x%02x%02x%02x%02x", broadcastMac[0],
                broadcastMac[1], broadcastMac[2], broadcastMac[3],
                broadcastMac[4], broadcastMac[5]);
  Serial.printf(", channel: %i\n", WIFI_CHANNEL);

  esp_now_add_peer(broadcastMac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);

  // TODO consider bringing back receiveCallBackFunction so that IS_COORDINATOR
  // is not checked twice
  if (IS_COORDINATOR) {
    Serial.println("Setup finished for coordinator");
    esp_now_register_recv_cb(coordinatorCallBackFunction);
  } else {
    Serial.println("Setup finished for button");
    esp_now_register_recv_cb(buttonCallBackFunction);
  }
}

void ledWinner() {
  // Flash LED fast
  if ((millis() - doorDashStartedAt) %
          (DOOR_DASH_WINNER_FLASH_FREQUENCY__ms * 2) <
      DOOR_DASH_WINNER_FLASH_FREQUENCY__ms) {
    digitalWrite(BUTTON_LED, HIGH);
  } else {
    digitalWrite(BUTTON_LED, LOW);
  }
}

void ledLoser() { digitalWrite(BUTTON_LED, HIGH); }

void setupButton() {
  pinMode(BUTTON_INPUT, INPUT);
  bool btnPressed = digitalRead(BUTTON_INPUT);
  setupSerial();
  Radio_Init();
  wakeTime = millis();

  pinMode(BUTTON_LED, OUTPUT);

  if (!btnPressed) {
    // Wait for a message to have been received. Warning: while(true) loop won't
    // work here because we won't receive an ESP_NOW message callback. It's
    // unclear why -- at startup, it seems we need to wait some time with delay
    // for the setup to happen. Maybe there's some async setup that gets stuck
    // if we have a while(true) loop here. After this line, while (true) loops
    // are fine.
    delay(50);

    if (globalState == SLEEP_LISTEN) {
      Serial.println("Back to sleep");
      goToSleep();
    }
  }
  // At this point, one of two things has happened: the button was pressed, or
  // we received a message

  if (btnPressed) {
    Serial.println("Button pressed");
    transitionState(DOOR_DASH_WAITING);
    doorDashStartedAt = millis();
  }

  keepCapacitorCharged(); // Prevent button from resetting mid-doordash

  unsigned long lastBroadcast = 0;
  while (true) {
    callWatchdog();
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
      // If more than FLASH_DURATION__ms has passed, cool down
      if (millis() - doorDashStartedAt >
          FLASH_DURATION__ms) { // Should theoretically never happen as long as
                                // the coordinator does its job.
        Serial.println("ERROR, never received a WINNER_MSG before cooldown");
        globalState = DOOR_DASH_COOL_DOWN_UNKNOWN;
      }
    } else if (globalState == DOOR_DASH_WINNER) {
      // Broadcast repeatedly who the winner is
      if (millis() - lastBroadcast > DOOR_DASH_REBROADCAST_INTERVAL__ms) {
        sendWinner((uint8_t *)winnerMac);
        lastBroadcast = millis();
      }
      ledWinner();
      // If more than 5 seconds have passed, go to cool down state
      if (millis() - doorDashStartedAt > FLASH_DURATION__ms) {
        transitionState(DOOR_DASH_COOL_DOWN_WINNER);
      }
    } else if (globalState == DOOR_DASH_LOSER) {
      // Broadcast repeatedly who the winner is
      if (millis() - lastBroadcast > DOOR_DASH_REBROADCAST_INTERVAL__ms) {
        sendWinner((uint8_t *)winnerMac);
        lastBroadcast = millis();
      }
      ledLoser();
      // If more than 5 seconds have passed, go to cool down state
      if (millis() - doorDashStartedAt > FLASH_DURATION__ms) {
        transitionState(DOOR_DASH_COOL_DOWN_LOSER);
      }
    } else if (globalState == DOOR_DASH_COOL_DOWN_WINNER) {
      ledWinner();
      // Sleep after cool down period is over
      if (millis() - doorDashStartedAt > FLASH_DURATION__ms + COOL_DOWN__ms) {
        goToSleep();
      }
    } else if (globalState == DOOR_DASH_COOL_DOWN_LOSER) {
      ledLoser();
      // Sleep after cool down period is over
      if (millis() - doorDashStartedAt > FLASH_DURATION__ms + COOL_DOWN__ms) {
        goToSleep();
      }
    } else { // DOOR_DASH_COOL_DOWN_UNKNOWN
      // Sleep after cool down period is over
      if (millis() - doorDashStartedAt > FLASH_DURATION__ms + COOL_DOWN__ms) {
        goToSleep();
      }
    }
  }
}

void setupCoordinator() {
  setupSerial();
  Radio_Init();

  while (true) {
    callWatchdog();
    if (hasDeclaredWinner &&
        millis() - doorDashStartedAt > DOOR_DASH_COORDINATION_DURATION__ms) {
      Serial.println("Resetting after doordash");
      doorDashStartedAt = 0;
      hasDeclaredWinner = false;
    }
  }
}

void loop() { Serial.println("ERROR, this should never run"); }

void setup() {
  if (IS_COORDINATOR) {
    setupCoordinator();
  } else {
    setupButton();
  }
}