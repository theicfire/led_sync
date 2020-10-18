// Derives from https://github.com/HarringayMakerSpace/ESP-Now
#include <Arduino.h>
#include "time.h"
#include <ESP8266WiFi.h>
#include "friend.h"
extern "C" {
#include <espnow.h>
#include <user_interface.h>
}
#define WIFI_CHANNEL 4

// Id 0 means don't ignore anyone
#define FRIEND_ID 0 // everyone is a cool friend

// it seems that the mac address needs to be set before setup() is called
//      and the inclusion of user_interface.h facilitates that
//      presumably there is a hidden call to initVariant()

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct __attribute__((packed)) DataStruct {
  unsigned int version;
  uint16_t seq_no;
  uint16_t swing_mag;
  char text[31];
};
DataStruct receivedData;
DataStruct sendingData;

unsigned long lastSentMillis = 0;
unsigned long sendIntervalMillis = 3000;
bool friendNewlyFound = false;
bool quickRespond = false;
unsigned long friendTimeout = 5000;
unsigned long lastSeenFriendTime = 0;
uint16_t seq_no = 0;

void initVariant() {
}

void sendData(uint16_t swing_mag) {
  sendingData.version = 1;
  sendingData.swing_mag = swing_mag;
  sendingData.seq_no = seq_no;
  uint8_t byteArray[sizeof(sendingData)];
  memcpy(byteArray, &sendingData, sizeof(sendingData));
  esp_now_send(broadcastMac, byteArray, sizeof(sendingData)); // NULL means send to all peers
  //Serial.println("Loop sent data");
  seq_no += 1;
}

void receiveCallBackFunction(uint8_t *senderMac, uint8_t *incomingData, uint8_t len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  //Serial.print("seqno: ");
  //Serial.println(receivedData.seq_no);
}

uint16_t Radio_GetRecentMag() {
  if (receivedData.swing_mag != 0) {
    uint16_t ret = receivedData.swing_mag;
    receivedData.swing_mag = 0;
    return ret;
  }
  return 0;
}

void Radio_Update(uint16_t swing_mag) {
  sendData(swing_mag);
  //if (friendNewlyFound) {
    //Serial.println("Found a friend!");
    //friendNewlyFound = false;
  //}
  //if (quickRespond) {
    //sendData();
    //quickRespond = false;
  //} else if (millis() > sendIntervalMillis && millis() - lastSentMillis >= sendIntervalMillis) {
    //lastSentMillis = millis();
    //sendData();
  //}
  //if (friendExists && lastSeenFriendTime > 0 && millis() - lastSeenFriendTime > friendTimeout) {
    //Serial.println("Friend timeout!");
    //friendExists = false;
  //}
  // TODO delay to save battery.. I think?
}

void Radio_Init() {
  Serial.begin(115200);
  if (esp_now_init()!=0) {
    Serial.println("*** ESP_Now init failed");
    while(true) {};
  }
  // role set to COMBO so it can send and receive - not sure this is essential
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  Serial.println("Starting ESPNow");

  WiFi.mode(WIFI_STA); // Station mode for esp-now controller

  Serial.printf("This mac: %s, ", WiFi.macAddress().c_str());
  Serial.printf("target mac: %02x%02x%02x%02x%02x%02x", broadcastMac[0], broadcastMac[1], broadcastMac[2], broadcastMac[3], broadcastMac[4], broadcastMac[5]);
  Serial.printf(", channel: %i\n", WIFI_CHANNEL);

  esp_now_add_peer(broadcastMac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);

  Serial.println("Setup finished");

  esp_now_register_recv_cb(receiveCallBackFunction);
}
