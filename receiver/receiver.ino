#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>

// Structure to receive the data
typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message receivedMessage, messageToSend;

// Callback function to handle received data
void OnDataRecv(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len) {
  // Copy received data into the receivedMessage structure
  memcpy(&receivedMessage, incomingData, sizeof(receivedMessage));
  
  // Print received message on Serial Monitor
  Serial.print("Received message: ");
  Serial.println(receivedMessage.msg);
  
  // Prepare a response to send back to the master
  strcpy(messageToSend.msg, "Hello from Slave");
  esp_now_send(recv_info->src_addr, (uint8_t *)&messageToSend, sizeof(messageToSend)); // Send the response back to the master
}

void setup() {
  Serial.begin(115200);

  // Init WiFi in Station Mode
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  // Register the callback function to handle received data
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Nothing to do in the loop as we're using callbacks to handle communication
}
