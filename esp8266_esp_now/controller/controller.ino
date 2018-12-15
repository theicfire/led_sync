// EspnowTwoWayController.ino

// a minimal program derived from
//          https://github.com/HarringayMakerSpace/ESP-Now

// This is the program that sends the data and receives the reply. (The Controller)

//=============

#include <ESP8266WiFi.h>
extern "C" {
    #include <espnow.h>
}

    // this is the MAC Address of the slave which receives these sensor readings
uint8_t remoteMac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};

#define WIFI_CHANNEL 4

    // must match slave struct
struct __attribute__((packed)) DataStruct {
    char text[32];
    unsigned long time;
};

DataStruct sendingData;

DataStruct receivedData;
    // receivedData could use a completely different struct as long as it matches
    //   the reply that is sent by the slave

unsigned long lastSentMillis;
unsigned long sendIntervalMillis = 1000;
unsigned long sentMicros;
unsigned long ackMicros;
unsigned long replyMicros;

unsigned long lastBlinkMillis;
unsigned long fastBlinkMillis = 200;
unsigned long slowBlinkMillis = 700;
unsigned long blinkIntervalMillis = slowBlinkMillis;

byte ledPin = 14;

//==============

void setup() {
    Serial.begin(115200); Serial.println();
    Serial.println("Starting EspnowTwoWayController.ino");

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


    esp_now_register_send_cb(sendCallBackFunction);
    esp_now_register_recv_cb(receiveCallBackFunction);

    strcpy(sendingData.text, "Hello World");
    Serial.print("Message "); Serial.println(sendingData.text);

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    Serial.println("Setup finished");

}

//==============

void loop() {
    sendData();
    blinkLed();
}

//==============

void sendData() {
    if (millis() - lastSentMillis >= sendIntervalMillis) {
        lastSentMillis += sendIntervalMillis;
        sendingData.time = millis();
        uint8_t byteArray[sizeof(sendingData)];
        memcpy(byteArray, &sendingData, sizeof(sendingData));
        sentMicros = micros();
        esp_now_send(NULL, byteArray, sizeof(sendingData)); // NULL means send to all peers
        Serial.println("Loop sent data");
    }
}

//==============

void sendCallBackFunction(uint8_t* mac, uint8_t sendStatus) {
    ackMicros = micros();
    Serial.print("Trip micros "); Serial.println(ackMicros - sentMicros);
    Serial.printf("Send status = %i", sendStatus);
    Serial.println();
    if (sendStatus == 0) {
        blinkIntervalMillis = fastBlinkMillis;
    }
    else {
        blinkIntervalMillis = slowBlinkMillis;
    }

}

//================

void receiveCallBackFunction(uint8_t *senderMac, uint8_t *incomingData, uint8_t len) {
    replyMicros = micros();
    Serial.print("Reply Trip micros "); Serial.println(replyMicros - sentMicros);
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("NewReply ");
    Serial.print("MacAddr ");
    for (byte n = 0; n < 6; n++) {
        Serial.print (senderMac[n], HEX);
    }
    Serial.print("  MsgLen ");
    Serial.print(len);
    Serial.print("  Text ");
    Serial.print(receivedData.text);
    Serial.print("  Time ");
    Serial.print(receivedData.time);
    Serial.println();
    Serial.println();
}

//================

void blinkLed() {
    if (millis() - lastBlinkMillis >= blinkIntervalMillis) {
        lastBlinkMillis += blinkIntervalMillis;
        digitalWrite(ledPin, ! digitalRead(ledPin));
    }
}
