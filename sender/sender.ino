#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Receiver MAC
uint8_t slaveAddress[] = { 0xa0, 0x85, 0xe3, 0xf1, 0x8d, 0x48 };

typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message messageToSend;

volatile bool dataSent = false;
volatile esp_now_send_status_t lastStatus;
unsigned long startTime, endTime;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  lastStatus = status;
  endTime = millis();
  dataSent = true;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED failed");
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED Initialized!");
  display.display();
  delay(1000);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, slaveAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  randomSeed(analogRead(0));
}

void loop() {
  int randomValue = random(1, 101);
  snprintf(messageToSend.msg, sizeof(messageToSend.msg), "%d", randomValue);

  // Screen 1: MAC address
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Sending to:");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           slaveAddress[0], slaveAddress[1], slaveAddress[2],
           slaveAddress[3], slaveAddress[4], slaveAddress[5]);
  display.println(macStr);
  display.display();
  delay(1500);

  dataSent = false;
  startTime = millis();
  esp_now_send(slaveAddress, (uint8_t *)&messageToSend, sizeof(messageToSend));

  while (!dataSent) delay(1);

  float durationSec = (endTime - startTime) / 1000.0;
  float speed = sizeof(messageToSend) / durationSec;

  // Screen 2: Data, Status, Speed
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Data sent: ");
  display.println(messageToSend.msg);
  display.print("Status: ");
  display.println(lastStatus == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  display.print("TS: ");
  display.print(speed, 2);
  display.println(" B/s");
  display.display();
  delay(2500);
}
