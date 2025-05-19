/*
  BasicSendReceive.ino - Example for sending and receiving CAN messages with WaveshareCAN library
  Created by Claude, April 30, 2025
*/

#include "WaveshareCAN.h"

// Create CAN bus instance
WaveshareCAN can;

// Buffer for received data
uint8_t rxData[8];
uint32_t rxId;
uint8_t rxLength;
bool rxExtended;

// Sample data to send
uint8_t txData[8] = {0, 1, 2, 3, 4, 5, 6, 7};
uint32_t txId = 0x18FEF1;

// Timers
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 1000; // Send every 1 second

// Alert callback function
void onCANAlert(uint32_t alerts) {
  if (alerts & TWAI_ALERT_BUS_ERROR) {
    Serial.println("Custom handler: CAN bus error detected!");
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("WaveshareCAN Basic Send/Receive Example");
  
  // Initialize IO expander (required for Waveshare board)
  if (!can.initIOExpander()) {
    Serial.println("Failed to initialize IO expander");
    while (1); // Stop if initialization fails
  }
  
  // Set alert callback (optional)
  can.onAlert(onCANAlert);
  
  // Begin CAN bus communication at 250 kbps
  if (!can.begin(CAN_250KBPS)) {
    Serial.println("Failed to initialize CAN bus");
    while (1); // Stop if initialization fails
  }
  
  Serial.println("CAN bus initialized successfully");
  
  // Optional: Set filter to only receive messages with ID 0x123
  // can.filter(0x123, 0x7FF); // To accept only 0x123
  // Use can.filter(0, 0); to accept all messages (default)
}

void loop() {
  // Check for received CAN messages
  if (can.available()) {
    // Read the next CAN message
    if (can.receiveMessage(&rxId, &rxExtended, rxData, &rxLength) > 0) {
      // Print received message
      Serial.print("Received ");
      if (rxExtended) {
        Serial.print("extended ");
      } else {
        Serial.print("standard ");
      }
      Serial.print("frame from ID: 0x");
      Serial.print(rxId, HEX);
      
      if (rxLength > 0) {
        Serial.print(" Data: ");
        for (int i = 0; i < rxLength; i++) {
          Serial.print(rxData[i], HEX);
          Serial.print(" ");
        }
      }
      Serial.println();
    }
  }
  
  // Send a message every second
  unsigned long currentMillis = millis();
  if (currentMillis - lastSendTime >= sendInterval) {
    lastSendTime = currentMillis;
    
    // Increment the first byte as a counter
    txData[0]++;
    
    // Send the message
    if (can.sendMessage(txId, true, txData, 8)) {
      Serial.print("Sent message with ID: 0x");
      Serial.print(txId, HEX);
      Serial.print(" Data: ");
      for (int i = 0; i < 8; i++) {
        Serial.print(txData[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.println("Failed to send message");
    }
  }
  
  // Process any CAN bus alerts
  can.processAlerts();
}
