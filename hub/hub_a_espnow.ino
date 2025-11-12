#include <ESP8266WiFi.h>
#include <espnow.h>

// ====== CONFIG ======

// Wi-Fi channel must match your remote (from its #define ESPNOW_CHANNEL)
#define ESPNOW_CHANNEL 6

// UART config to talk to Hub B
#define HUB_B_SERIAL Serial
#define HUB_B_BAUD   115200

// =====================

// Called when ESP-NOW data is received
void onReceive(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  String msg = String((char*)incomingData).substring(0, len);
  Serial.printf("From %s => %s\n", macStr, msg.c_str());

  // Forward JSON directly to Hub B
  HUB_B_SERIAL.println(msg);
}

void setup() {
  Serial.begin(115200);
  HUB_B_SERIAL.begin(HUB_B_BAUD);

  // Print the Wi-Fi MAC address so you can paste it into your remote sketch
  Serial.print("Hub A MAC address: ");
  Serial.println(WiFi.macAddress());

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed!");
    ESP.restart();
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onReceive);

  // Channel lock
  wifi_promiscuous_enable(true);
  wifi_set_channel(ESPNOW_CHANNEL);
  wifi_promiscuous_enable(false);

  Serial.printf("Hub A ready on channel %d\n", ESPNOW_CHANNEL);
}

void loop() {
  // Nothing here — ESP-NOW callback handles all messages
}