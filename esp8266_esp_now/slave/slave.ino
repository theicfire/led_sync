// Derives from https://github.com/HarringayMakerSpace/ESP-Now

#include <ESP8266WiFi.h>
extern "C" {
    #include <espnow.h>
     #include <user_interface.h>
}
#define WIFI_CHANNEL 4

// it seems that the mac address needs to be set before setup() is called
//      and the inclusion of user_interface.h facilitates that
//      presumably there is a hidden call to initVariant()

// http://serverfault.com/questions/40712/what-range-of-mac-addresses-can-i-safely-use-for-my-virtual-machines
uint8_t slaveMac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
void initVariant() {
  if (isSlave) {
    WiFi.mode(WIFI_AP);
    wifi_set_macaddr(SOFTAP_IF, &slaveMac[0]);
  }
}

struct __attribute__((packed)) DataStruct {
    char text[32];
    unsigned int time;
};
DataStruct receivedData;
DataStruct sendingData;

unsigned long lastSentMillis = 0;
unsigned long sendIntervalMillis = 1000;
bool isMaster = true;

void setup() {
    Serial.begin(115200);
    if (esp_now_init()!=0) {
        Serial.println("*** ESP_Now init failed");
        while(true) {};
    }
    // role set to COMBO so it can send and receive - not sure this is essential
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

    if (isMaster) {
      master_setup();
    } else {
      slave_setup();
    }
}

void slave_setup() {
    Serial.println("Starting Slave");

    Serial.print("This AP mac: "); Serial.println(WiFi.softAPmacAddress());
    Serial.print("This STA mac: "); Serial.println(WiFi.macAddress());

    esp_now_register_recv_cb(receiveCallBackFunction);
    Serial.println("End of setup - waiting for messages");
}

void master_setup() {
    Serial.println("Starting Master");

    WiFi.mode(WIFI_STA); // Station mode for esp-now controller
    WiFi.disconnect();

    Serial.printf("This mac: %s, ", WiFi.macAddress().c_str());
    Serial.printf("target mac: %02x%02x%02x%02x%02x%02x", slaveMac[0], slaveMac[1], slaveMac[2], slaveMac[3], slaveMac[4], slaveMac[5]);
    Serial.printf(", channel: %i\n", WIFI_CHANNEL);

    esp_now_add_peer(slaveMac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);

    Serial.println("Setup finished");
}

void loop() {
  if (isMaster) {
    sendData();
  }
}

void sendData() {
    if (Time_GetTime() - lastSentMillis >= sendIntervalMillis) {
        lastSentMillis += sendIntervalMillis;
        sendingData.time = Time_GetTime();
        uint8_t byteArray[sizeof(sendingData)];
        memcpy(byteArray, &sendingData, sizeof(sendingData));
        esp_now_send(NULL, byteArray, sizeof(sendingData)); // NULL means send to all peers
        Serial.println("Loop sent data");
    }
}

void receiveCallBackFunction(uint8_t *senderMac, uint8_t *incomingData, uint8_t len) {
    Time_SetTime(receivedData.time);
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("NewMsg ");
    Serial.print("MacAddr ");
    for (byte n = 0; n < 6; n++) {
        Serial.print(senderMac[n], HEX);
    }
    Serial.print("  MsgLen ");
    Serial.print(len);
    Serial.print("  Text ");
    Serial.print(receivedData.text);
    Serial.print("  Time ");
    Serial.print(receivedData.time);
    Serial.println();

}
