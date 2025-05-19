/*
 * EnhancedCANBus - Basic Send and Receive Example
 * 
 * This example demonstrates the basic functionality of the EnhancedCANBus library
 * by sending and receiving CAN messages.
 * 
 * Hardware:
 * - ESP32 development board
 * - CAN transceiver (SN65HVD230, MCP2551, TJA1050, etc.)
 * 
 * Connections:
 * - GPIO 19: CAN RX
 * - GPIO 20: CAN TX
 */

#include <Arduino.h>
#include <EnhancedCANBus.h>

// Pin definitions
#define CAN_RX_PIN 19
#define CAN_TX_PIN 20

// Create CAN bus instance
EnhancedCANBus canBus;

// Variables for periodic transmission
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 1000;  // Send every 1 second
uint8_t counter = 0;  // Message counter

// Buffer for received message data
uint8_t rxBuffer[8];

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("\nEnhancedCANBus - Basic Send and Receive Example");
  
  // Initialize CAN bus at 500 kbps
  int result = canBus.begin(CAN_TX_PIN, CAN_RX_PIN, CANBitRate::KBPS_500, CANMode::NORMAL);
  
  if (result == CAN_OK) {
    Serial.println("CAN bus initialized successfully at 500 kbps");
  } else {
    Serial.print("Failed to initialize CAN bus! Error code: ");
    Serial.println(result);
    
    // Halt on error
    while (1) {
      delay(1000);
    }
  }
}

void loop() {
  // Process CAN events
  canBus.poll(5);  // 5ms timeout for event processing
  
  // Send a message periodically
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= sendInterval) {
    lastSendTime = currentTime;
    
    // Prepare data to send (counter value and some random data)
    uint8_t txData[8] = {
      counter,                  // Counter byte (incrementing)
      (uint8_t)(counter * 2),   // Doubled counter value
      (uint8_t)(random(0, 255)),// Random value
      0xAA,                     // Fixed value
      0x55,                     // Fixed value
      (uint8_t)(currentTime & 0xFF),       // Timestamp (lowest byte)
      (uint8_t)((currentTime >> 8) & 0xFF),// Timestamp (second byte)
      (uint8_t)((currentTime >> 16) & 0xFF)// Timestamp (third byte)
    };
    
    // Send with ID 0x123
    int sendResult = canBus.sendMessage(0x123, txData, 8);
    
    if (sendResult == CAN_OK) {
      Serial.print("Message sent with counter value: ");
      Serial.println(counter);
    } else {
      Serial.print("Failed to send message. Error code: ");
      Serial.println(sendResult);
    }
    
    // Increment counter for next message
    counter++;
  }
  
  // Check for received messages
  if (canBus.available() > 0) {
    uint32_t rxId;
    uint8_t rxLength;
    CANFrameType rxFrameType;
    bool rxRtr;
    
    // Read the message
    if (canBus.readMessage(rxId, rxBuffer, rxLength, rxFrameType, rxRtr)) {
      // Print message details
      Serial.print("Message received - ID: 0x");
      Serial.print(rxId, HEX);
      
      if (rxFrameType == CANFrameType::EXTENDED) {
        Serial.print(" (Extended)");
      } else {
        Serial.print(" (Standard)");
      }
      
      if (rxRtr) {
        Serial.println(" - RTR Frame");
      } else {
        Serial.print(" - Data: ");
        
        // Print data bytes in hexadecimal
        for (int i = 0; i < rxLength; i++) {
          // Add leading zero for values less than 0x10
          if (rxBuffer[i] < 0x10) {
            Serial.print("0");
          }
          Serial.print(rxBuffer[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
      }
    }
  }
  
  // Check bus status periodically
  static unsigned long lastStatusCheck = 0;
  if (currentTime - lastStatusCheck >= 5000) {  // Every 5 seconds
    lastStatusCheck = currentTime;
    
    // Get current bus status
    twai_status_info_t status = canBus.getStatus();
    
    Serial.println("\n--- CAN Bus Status ---");
    Serial.print("State: ");
    switch (status.state) {
      case TWAI_STATE_STOPPED:
        Serial.println("STOPPED");
        break;
      case TWAI_STATE_RUNNING:
        Serial.println("RUNNING");
        break;
      case TWAI_STATE_BUS_OFF:
        Serial.println("BUS OFF");
        // Try to recover if bus-off condition detected
        Serial.println("Attempting recovery...");
        canBus.recover();
        break;
      case TWAI_STATE_RECOVERING:
        Serial.println("RECOVERING");
        break;
      default:
        Serial.println("UNKNOWN");
    }
    
    // Error counters
    Serial.print("TX Error Counter: ");
    Serial.println(status.tx_error_counter);
    Serial.print("RX Error Counter: ");
    Serial.println(status.rx_error_counter);
    
    // Queue status
    Serial.print("Messages in TX Queue: ");
    Serial.println(status.msgs_to_tx);
    Serial.print("Messages in RX Queue: ");
    Serial.println(status.msgs_to_rx);
    
    Serial.println("----------------------");
  }
  
  // Small delay to prevent CPU hogging
  delay(1);
}
