/*
  ListenOnlyExample.ino - Example for monitoring CAN bus traffic with WaveshareCAN library
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
bool rxRtr;

// Stats
unsigned long messagesReceived = 0;
unsigned long lastStatsTime = 0;
const unsigned long statsInterval = 5000; // Print stats every 5 seconds

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("WaveshareCAN Listen-Only Example");
  
  // Initialize IO expander (required for Waveshare board)
  if (!can.initIOExpander()) {
    Serial.println("Failed to initialize IO expander");
    while (1); // Stop if initialization fails
  }
  
  // Set to listen-only mode before beginning
  can.setListenOnly(true);
  
  // Begin CAN bus communication at 500 kbps
  if (!can.begin(CAN_250KBPS)) {
    Serial.println("Failed to initialize CAN bus");
    while (1); // Stop if initialization fails
  }
  
  Serial.println("CAN bus initialized in listen-only mode");
  Serial.println("Waiting for CAN traffic...");
}

void loop() {
  // Check for received CAN messages
  if (can.available()) {
    // Read the next CAN message
    if (can.receiveMessage(&rxId, &rxExtended, rxData, &rxLength, &rxRtr) > 0) {
      messagesReceived++;
      
      // Print received message
      Serial.print("ID: 0x");
      if (rxExtended) {
        Serial.print(rxId, HEX);
        Serial.print(" (Extended) ");
      } else {
        Serial.print(rxId, HEX);
        Serial.print(" (Standard) ");
      }
      
      if (rxRtr) {
        Serial.print("RTR");
      } else {
        Serial.print("Data[");
        Serial.print(rxLength);
        Serial.print("]: ");
        
        for (int i = 0; i < rxLength; i++) {
          if (rxData[i] < 0x10) {
            Serial.print("0");  // Leading zero for proper formatting
          }
          Serial.print(rxData[i], HEX);
          Serial.print(" ");
        }
      }
      Serial.println();
    }
  }
  
  // Print stats every 5 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - lastStatsTime >= statsInterval) {
    lastStatsTime = currentMillis;
    
    // Get CAN bus status
    twai_status_info_t status;
    if (can.getStatus(&status)) {
      Serial.println("\n----- CAN Bus Statistics -----");
      Serial.print("Messages received: ");
      Serial.println(messagesReceived);
      Serial.print("RX buffer: ");
      Serial.print(status.msgs_to_rx);
      Serial.print(" msgs, missed: ");
      Serial.print(status.rx_missed_count);
      Serial.print(", overrun: ");
      Serial.println(status.rx_overrun_count);
      Serial.print("Bus state: ");
      
      switch (status.state) {
        case TWAI_STATE_STOPPED:
          Serial.println("STOPPED");
          break;
        case TWAI_STATE_RUNNING:
          Serial.println("RUNNING");
          break;
        case TWAI_STATE_BUS_OFF:
          Serial.println("BUS OFF");
          break;
        case TWAI_STATE_RECOVERING:
          Serial.println("RECOVERING");
          break;
        default:
          Serial.println("UNKNOWN");
      }
      
      Serial.print("TX error counter: ");
      Serial.print(status.tx_error_counter);
      Serial.print(", RX error counter: ");
      Serial.print(status.rx_error_counter);
      Serial.print(", bus error count: ");
      Serial.println(status.bus_error_count);
      Serial.println("-----------------------------\n");
    }
  }
  
  // Process any CAN bus alerts
  can.processAlerts();
}
