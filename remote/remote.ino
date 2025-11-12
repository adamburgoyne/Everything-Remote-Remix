#include <esp_now.h>
#include "esp_wifi.h"
#include <WiFi.h>

// ========= CONFIG =========

// Hub A MAC address
uint8_t hubAddress[] = {0xB1, 0xE1, 0x2D, 0x4F, 0x2D, 0x6D};
#define ESPNOW_CHANNEL 6

// Button configuration
struct Button {
  gpio_num_t pin;
  const char* name;
};

Button buttons[] = {
  {GPIO_NUM_0,  "power"}, //wake pin
  {GPIO_NUM_2,  "back"},
  {GPIO_NUM_4,  "home"},
  {GPIO_NUM_5,  "play"},
  {GPIO_NUM_18, "up"},
  {GPIO_NUM_19, "left"},
  {GPIO_NUM_25, "down"},
  {GPIO_NUM_23, "right"},
  {GPIO_NUM_22, "centre"},
  {GPIO_NUM_12, "volume_up"},
  {GPIO_NUM_15, "volume_down"},
  {GPIO_NUM_13, "menu"},
  {GPIO_NUM_16, "settings"},
  {GPIO_NUM_14, "channel_up"},
  {GPIO_NUM_17, "channel_down"},
  {GPIO_NUM_34, "shortcut_1"},
  {GPIO_NUM_35, "shortcut_2"},
  {GPIO_NUM_32, "shortcut_3"},
  {GPIO_NUM_33, "shortcut_4"},
  {GPIO_NUM_26, "brightness_down"},
  {GPIO_NUM_27, "brightness_up"},
};

const int NUM_BUTTONS = sizeof(buttons) / sizeof(buttons[0]);

const unsigned long LONG_PRESS_MS = 700;
const unsigned long IDLE_TIMEOUT_MS = 20000; // 20 seconds idle = sleep

RTC_DATA_ATTR int lastButtonIndex = -1;
bool sendDone = false;
unsigned long lastActivity = 0;

// ==========================

void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
  sendDone = true;
}

void sendButton(const char* name, const char* type) {
  char msg[80];
  snprintf(msg, sizeof(msg),
           "{\"button\":\"%s\",\"type\":\"%s\"}", name, type);
  esp_now_send(hubAddress, (uint8_t*)msg, strlen(msg));
  Serial.printf("Sent: %s\n", msg);
  lastActivity = millis();  // reset idle timer on send
}

void setup() {
  Serial.begin(115200);
  delay(20);
  Serial.println("=== Remote (ESP32, Deep Sleep) ===");

  // Setup button pins
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (buttons[i].pin >= 34 && buttons[i].pin <= 39) {
      pinMode(buttons[i].pin, INPUT);  // input-only, must have external pull-up
    } else {
      pinMode(buttons[i].pin, INPUT_PULLUP);
    }
  }

  // Init Wi-Fi + ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);  // FIXED CHANNEL

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed, sleeping.");
    esp_deep_sleep_start();
  }

  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, hubAddress, 6);
  peer.channel = ESPNOW_CHANNEL;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  lastActivity = millis();  // start idle timer

  Serial.println("Monitoring buttons...");

  // Main loop to handle multiple presses
  while (millis() - lastActivity < IDLE_TIMEOUT_MS) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
      if (digitalRead(buttons[i].pin) == LOW) {
        delay(20); // debounce
        if (digitalRead(buttons[i].pin) == LOW) {
          unsigned long pressedAt = millis();
          while (digitalRead(buttons[i].pin) == LOW && millis() - pressedAt < 3000) {
            delay(10);
          }
          unsigned long heldMs = millis() - pressedAt;
          const char* pressType = (heldMs >= LONG_PRESS_MS) ? "long" : "single";
          sendButton(buttons[i].name, pressType);
          delay(50);
        }
      }
    }
    delay(10);
  }

  Serial.println("Idle timeout reached. Sleeping...");
  // EXT0 wake on vibration
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // wake when pulled LOW
  esp_deep_sleep_start();
}

void loop() {
  // never reached
}