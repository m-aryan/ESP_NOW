#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Replace with your slave's MAC address
uint8_t slaveAddress[] = {0xa0, 0x85, 0xe3, 0xf1, 0x8d, 0x48}; // <-- UPDATE THIS!

typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message messageToSend;

// Callback for delivery status
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Data sent:");
  display.println(messageToSend.msg);
  display.print("Status: ");
  display.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  display.display();
}

void setup() {
  Serial.begin(115200);

  // OLED Init
  Wire.begin(8, 9); // SDA, SCL pins for ESP32-S3
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED Initialized!");
  display.display();
  delay(2000);

  // Init WiFi in Station Mode
  WiFi.mode(WIFI_STA);

  // Print MAC Address
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("MASTER ESP32");
  display.println("MAC:");
  display.println(WiFi.macAddress());
  display.display();
  delay(2000);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    display.println("ESP-NOW Failed!");
    display.display();
    return;
  }
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, slaveAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    display.println("Peer Add Fail");
    display.display();
    return;
  }

  // Seed random number generator
  randomSeed(analogRead(0));
}

void loop() {
  int randomValue = random(1, 101); // 1 to 100 inclusive
  snprintf(messageToSend.msg, sizeof(messageToSend.msg), "%d", randomValue);
  esp_now_send(slaveAddress, (uint8_t *)&messageToSend, sizeof(messageToSend));
  delay(5000);
}
