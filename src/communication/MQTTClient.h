/**
 * MQTTClient - MQTT Protocol for Sensor Data Publishing
 * Publishes sensor data to MQTT broker
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "../include/config.h"

class MQTTClient {
private:
    WiFiClient wifiClient;
    PubSubClient mqtt;
    bool connected;
    unsigned long lastPublish;
    unsigned long lastReconnect;
    
    // Callback for incoming messages
    void callback(char* topic, byte* payload, unsigned int length) {
        Serial.printf("[MQTT] Message received on topic: %s\n", topic);
        
        // Null-terminate the payload
        payload[length] = '\0';
        String message = String((char*)payload);
        
        // Handle commands (e.g., reset, configure)
        if (String(topic) == String(MQTT_TOPIC) + "/command") {
            handleCommand(message);
        }
    }
    
    void handleCommand(String command) {
        command.trim();
        command.toLowerCase();
        
        Serial.printf("[MQTT] Handling command: %s\n", command.c_str());
        
        if (command == "reset") {
            ESP.restart();
        }
        // Add more commands as needed
    }
    
public:
    MQTTClient() : mqtt(wifiClient), 
                  connected(false), 
                  lastPublish(0),
                  lastReconnect(0) {}
    
    /**
     * Initialize MQTT client
     */
    void begin() {
        mqtt.setServer(MQTT_SERVER, MQTT_PORT);
        mqtt.setCallback([this](char* topic, byte* payload, unsigned int length) {
            this->callback(topic, payload, length);
        });
        
        Serial.println("[MQTT] Client initialized");
    }
    
    /**
     * Connect to MQTT broker
     */
    bool connect() {
        if (mqtt.connected()) {
            connected = true;
            return true;
        }
        
        Serial.printf("[MQTT] Connecting to broker: %s:%d\n", MQTT_SERVER, MQTT_PORT);
        
        // Generate client ID with random suffix
        String clientId = String(MQTT_CLIENT_ID) + "-" + String(random(0xffff), HEX);
        
        // Connect with optional username/password
        if (strlen(MQTT_USERNAME) > 0 && strlen(MQTT_PASSWORD) > 0) {
            connected = mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
        } else {
            connected = mqtt.connect(clientId.c_str());
        }
        
        if (connected) {
            Serial.println("[MQTT] Connected to broker!");
            
            // Subscribe to command topic
            String commandTopic = String(MQTT_TOPIC) + "/command";
            mqtt.subscribe(commandTopic.c_str());
            Serial.printf("[MQTT] Subscribed to: %s\n", commandTopic.c_str());
        } else {
            Serial.printf("[MQTT] Connection failed, rc=%d\n", mqtt.state());
        }
        
        return connected;
    }
    
    /**
     * Publish sensor data to MQTT broker
     */
    bool publish(SensorData data) {
        if (!mqtt.connected()) {
            if (!connect()) {
                return false;
            }
        }
        
        // Create JSON payload
        String payload = createJSON(data);
        
        // Publish
        bool success = mqtt.publish(MQTT_TOPIC, payload.c_str());
        
        if (success) {
            Serial.printf("[MQTT] Published: %s\n", payload.c_str());
            lastPublish = millis();
        } else {
            Serial.println("[MQTT] Publish failed!");
        }
        
        return success;
    }
    
    /**
     * Create JSON payload from sensor data
     */
    String createJSON(SensorData data) {
        String json = "{";
        json += "\"timestamp\":" + String(data.timestamp) + ",";
        json += "\"temperature\":" + String(data.temperature, 2) + ",";
        json += "\"humidity\":" + String(data.humidity, 2) + ",";
        json += "\"pressure\":" + String(data.pressure, 2) + ",";
        json += "\"soilMoisture\":" + String(data.soilMoisture, 2) + ",";
        json += "\"lightLevel\":" + String(data.lightLevel, 2) + ",";
        json += "\"motionDetected\":" + String(data.motionDetected ? "true" : "false") + ",";
        json += "\"speed\":" + String(data.speed, 2) + ",";
        json += "\"gasLevel\":" + String(data.gasLevel, 2) + ",";
        json += "\"freeHeap\":" + String(data.freeHeap) + ",";
        json += "\"cpuTemp\":" + String(data.cpuTemp, 2);
        json += "}";
        
        return json;
    }
    
    /**
     * Maintain MQTT connection (call in loop)
     */
    void maintain() {
        if (!mqtt.connected()) {
            unsigned long now = millis();
            if (now - lastReconnect > 5000) {  // Reconnect every 5 seconds
                lastReconnect = now;
                connect();
            }
        } else {
            mqtt.loop();  // Process incoming messages
        }
    }
    
    /**
     * Check if connected
     */
    bool isConnected() {
        return mqtt.connected();
    }
    
    /**
     * Get time since last publish
     */
    unsigned long getTimeSinceLastPublish() {
        return millis() - lastPublish;
    }
};

#endif // MQTT_CLIENT_H

