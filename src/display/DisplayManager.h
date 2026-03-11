/**
 * DisplayManager - OLED Display Control
 * Manages SSD1306 OLED display for sensor data visualization
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../include/config.h"

class DisplayManager {
private:
    Adafruit_SSD1306 display;
    bool initialized;
    int displayPage;
    unsigned long lastPageChange;
    const int pageTimeout = 8000;  // Switch page every 8 seconds
    
public:
    DisplayManager() : display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET_PIN),
                      initialized(false),
                      displayPage(0),
                      lastPageChange(0) {}
    
    /**
     * Initialize OLED display
     */
    void begin() {
        // Initialize I2C
        Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
        
        // Initialize display
        if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
            Serial.println("[Display] SSD1306 allocation failed!");
            initialized = false;
            return;
        }
        
        // Configure display
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        
        initialized = true;
        Serial.println("[Display] OLED display initialized");
    }
    
    /**
     * Show welcome screen
     */
    void showWelcome() {
        if (!initialized) return;
        
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(20, 10);
        display.println("IoT RTOS");
        display.setCursor(25, 35);
        display.println("Sensors");
        display.display();
    }
    
    /**
     * Clear display
     */
    void clear() {
        if (!initialized) return;
        display.clearDisplay();
    }
    
    /**
     * Show sensor data on display
     */
    void showSensorData(SensorData data) {
        if (!initialized) return;
        
        // Auto-switch pages
        if (millis() - lastPageChange > pageTimeout) {
            displayPage = (displayPage + 1) % 3;
            lastPageChange = millis();
        }
        
        display.clearDisplay();
        display.setTextSize(1);
        
        switch (displayPage) {
            case 0:
                showPage1(data);
                break;
            case 1:
                showPage2(data);
                break;
            case 2:
                showPage3(data);
                break;
        }
        
        display.display();
    }
    
    // Page 1: Temperature, Humidity, Pressure
    void showPage1(SensorData data) {
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("=== ENVIRONMENT ===");
        
        display.setTextSize(2);
        display.printf("%.1f%cC\n", data.temperature, 167);  // Degree symbol
        
        display.setTextSize(1);
        display.printf("Humidity: %.1f%%\n", data.humidity);
        
        display.printf("Pressure: %.1f hPa", data.pressure);
    }
    
    // Page 2: Soil, Light, Gas
    void showPage2(SensorData data) {
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("=== SENSORS 2 ===");
        
        display.setTextSize(2);
        display.printf("Soil: %.0f%%\n", data.soilMoisture);
        
        display.setTextSize(1);
        display.printf("Light: %.0f%%\n", data.lightLevel);
        
        display.printf("Gas: %.0f%%", data.gasLevel);
    }
    
    // Page 3: Motion, Speed, System
    void showPage3(SensorData data) {
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("=== MOTION/SPEED ===");
        
        display.setTextSize(2);
        display.printf("Speed: %.1f\n", data.speed);
        display.setTextSize(1);
        display.println("km/h");
        
        display.setTextSize(1);
        display.printf("Motion: %s", data.motionDetected ? "YES" : "NO");
    }
    
    /**
     * Show system status
     */
    void showSystemStatus(SystemStatus status) {
        if (!initialized) return;
        
        // Show status bar at bottom
        display.setTextSize(1);
        display.setCursor(0, 56);
        
        // WiFi status
        display.print(status.wifiConnected ? "W" : "w");
        // MQTT status  
        display.print(status.mqttConnected ? "M" : "m");
        // BLE status
        display.print(status.bleConnected ? "B" : "b");
        // SD Card status
        display.print(status.sdCardMounted ? "S" : "s");
        
        // Uptime
        display.printf(" %lus", status.uptime);
    }
    
    /**
     * Show custom message
     */
    void showMessage(const char* line1, const char* line2 = "", const char* line3 = "") {
        if (!initialized) return;
        
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println(line1);
        
        if (strlen(line2) > 0) {
            display.setCursor(0, 20);
            display.println(line2);
        }
        
        if (strlen(line3) > 0) {
            display.setCursor(0, 40);
            display.println(line3);
        }
        
        display.display();
    }
    
    /**
     * Check if display is initialized
     */
    bool isReady() {
        return initialized;
    }
};

#endif // DISPLAY_MANAGER_H

