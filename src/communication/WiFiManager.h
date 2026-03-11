/**
 * WiFiManager - ESP32 WiFi Connection Management
 * Handles WiFi connection and reconnection
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "../include/config.h"

class WiFiManager {
private:
    bool connected;
    unsigned long lastReconnectAttempt;
    unsigned long connectionStartTime;
    
public:
    WiFiManager() : connected(false), 
                   lastReconnectAttempt(0),
                   connectionStartTime(0) {}
    
    /**
     * Initialize and connect to WiFi
     */
    void begin() {
        Serial.println("[WiFi] Initializing WiFi...");
        
        // Set WiFi mode
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(false);
        
        // Start connection
        connect();
    }
    
    /**
     * Connect to WiFi network
     */
    void connect() {
        Serial.printf("[WiFi] Connecting to SSID: %s\n", WIFI_SSID);
        
        connectionStartTime = millis();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        
        // Wait for connection
        int attempts = 0;
        while (!isConnected() && attempts < 30) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (isConnected()) {
            Serial.println("\n[WiFi] Connected!");
            Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("[WiFi] Signal Strength: %d dBm\n", WiFi.RSSI());
            connected = true;
        } else {
            Serial.println("\n[WiFi] Connection failed!");
            connected = false;
        }
    }
    
    /**
     * Check and maintain WiFi connection
     * Should be called regularly in main loop
     */
    void maintain() {
        if (!connected) {
            // Try to reconnect
            unsigned long now = millis();
            if (now - lastReconnectAttempt > 5000) {  // Try every 5 seconds
                lastReconnectAttempt = now;
                Serial.println("[WiFi] Attempting reconnection...");
                connect();
            }
        } else {
            // Check if still connected
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("[WiFi] Connection lost!");
                connected = false;
            }
        }
    }
    
    /**
     * Get connection status
     */
    bool isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }
    
    /**
     * Get current IP address
     */
    String getIPAddress() {
        return WiFi.localIP().toString();
    }
    
    /**
     * Get signal strength (RSSI)
     */
    int getRSSI() {
        return WiFi.RSSI();
    }
    
    /**
     * Get connection uptime
     */
    unsigned long getUptime() {
        if (!connected) return 0;
        return millis() - connectionStartTime;
    }
    
    /**
     * Disconnect from WiFi
     */
    void disconnect() {
        WiFi.disconnect();
        connected = false;
        Serial.println("[WiFi] Disconnected");
    }
    
    /**
     * Get WiFi status as string
     */
    const char* getStatusString() {
        switch (WiFi.status()) {
            case WL_CONNECTED: return "CONNECTED";
            case WL_NO_SSID_AVAIL: return "NO SSID";
            case WL_CONNECT_FAILED: return "FAILED";
            case WL_IDLE_STATUS: return "IDLE";
            case WL_DISCONNECTED: return "DISCONNECTED";
            default: return "UNKNOWN";
        }
    }
};

#endif // WIFI_MANAGER_H

