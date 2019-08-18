// Derives from https://github.com/HarringayMakerSpace/ESP-Now

#include <ESP8266WiFi.h>
extern "C" {
    #include <espnow.h>
}
#define WIFI_CHANNEL 4

// this is the MAC Address of the slave which receives these sensor readings
uint8_t remoteMac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};

struct __attribute__((packed)) DataStruct {
    char text[32];
    unsigned long time;
};

DataStruct sendingData;

unsigned long lastSentMillis = 0;
unsigned long sendIntervalMillis = 1000;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting controller");

    WiFi.mode(WIFI_STA); // Station mode for esp-now controller
    WiFi.disconnect();

    Serial.printf("This mac: %s, ", WiFi.macAddress().c_str());
    Serial.printf("target mac: %02x%02x%02x%02x%02x%02x", remoteMac[0], remoteMac[1], remoteMac[2], remoteMac[3], remoteMac[4], remoteMac[5]);
    Serial.printf(", channel: %i\n", WIFI_CHANNEL);

    if (esp_now_init() != 0) {
        Serial.println("*** ESP_Now init failed");
        while(true) {};
    }

    // role set to COMBO so it can send and receive - not sure this is essential
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

    esp_now_add_peer(remoteMac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);

    Serial.println("Setup finished");
}

void loop() {
    sendData();
}

void sendData() {
    if (millis() - lastSentMillis >= sendIntervalMillis) {
        lastSentMillis += sendIntervalMillis;
        sendingData.time = millis();
        uint8_t byteArray[sizeof(sendingData)];
        memcpy(byteArray, &sendingData, sizeof(sendingData));
        esp_now_send(NULL, byteArray, sizeof(sendingData)); // NULL means send to all peers
        Serial.println("Loop sent data");
    }
}
