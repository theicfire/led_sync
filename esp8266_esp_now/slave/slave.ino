// EspnowTwoWaySlave.ino

// a minimal program derived from
//          https://github.com/HarringayMakerSpace/ESP-Now

// This is the program that receives the data and sends a reply. (The Slave)

//=============

#include <ESP8266WiFi.h>
extern "C" {
    #include <espnow.h>
     #include <user_interface.h>
}

// it seems that the mac address needs to be set before setup() is called
//      and the inclusion of user_interface.h facilitates that
//      presumably there is a hidden call to initVariant()

/* Set a private Mac Address
 *  http://serverfault.com/questions/40712/what-range-of-mac-addresses-can-i-safely-use-for-my-virtual-machines
 * Note: the point of setting a specific MAC is so you can replace this Gateway ESP8266 device with a new one
 * and the new gateway will still pick up the remote sensors which are still sending to the old MAC
 */
uint8_t mac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};

//=============

void initVariant() {
  WiFi.mode(WIFI_AP);
  wifi_set_macaddr(SOFTAP_IF, &mac[0]);
}

//==============


#define WIFI_CHANNEL 4

    // keep in sync with slave struct
struct __attribute__((packed)) DataStruct {
    char text[32];
    unsigned int time;
};

DataStruct receivedData;

DataStruct replyData;

//~ uint8_t incomingData[sizeof(receivedData)];


//============

bool on = false;
void setup() {
    Serial.begin(115200); Serial.println();
    Serial.println("Starting EspnowTwoWaySlave.ino");

    Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
    Serial.print("This node STA mac: "); Serial.println(WiFi.macAddress());

    if (esp_now_init()!=0) {
        Serial.println("*** ESP_Now init failed");
        while(true) {};
    }
        // role set to COMBO so it can receive and send - not sure this is essential
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

    esp_now_register_recv_cb(receiveCallBackFunction);


    strcpy(replyData.text, "Goodnight John-Boy");
    Serial.print("Message "); Serial.println(replyData.text);


    Serial.println("End of setup - waiting for messages");
    pinMode(LED_BUILTIN, OUTPUT);
}

//============

void loop() {

}

//============

void receiveCallBackFunction(uint8_t *senderMac, uint8_t *incomingData, uint8_t len) {
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("wwwNewMsg ");
    Serial.print("MacAddr ");
    for (byte n = 0; n < 6; n++) {
        Serial.print (senderMac[n], HEX);
    }
    Serial.print("  MsgLen ");
    Serial.print(len);
    Serial.print("  Name ");
    Serial.print(receivedData.text);
    Serial.print("  Time ");
    Serial.print(receivedData.time);
    Serial.println();

    sendReply(senderMac);
    on = !on;
    if (on) {
      Serial.print("HIGH");
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    } else {
      Serial.print("LOW");
      digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
    }
}

//=============

void sendReply(uint8_t *macAddr) {
        // create a peer with the received mac address
    esp_now_add_peer(macAddr, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);

    replyData.time = millis();
    uint8_t byteArray[sizeof(replyData)];
    memcpy(byteArray, &replyData, sizeof(replyData));

    esp_now_send(NULL, byteArray, sizeof(replyData)); // NULL means send to all peers
    Serial.println("sendReply sent data");

        // data sent so delete the peer
    esp_now_del_peer(macAddr);
}
