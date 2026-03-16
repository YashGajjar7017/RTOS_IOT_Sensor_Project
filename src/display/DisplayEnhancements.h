/**
 * DisplayEnhancements - Additional display screens for web server and database status
 * Shows real-time sensor data in multiple screen formats
 */

#ifndef DISPLAY_ENHANCEMENTS_H
#define DISPLAY_ENHANCEMENTS_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "../include/config.h"

class DisplayEnhancements {
public:
    /**
     * Show web server connection info
     */
    static void showWebServerInfo(Adafruit_SSD1306& display, const String& ipAddress) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Web Server");
        
        display.setTextSize(1);
        display.setCursor(0, 20);
        display.println("Dashboard:");
        display.print("http://");
        display.println(ipAddress);
        
        display.setCursor(0, 36);
        display.println("WebSocket: 81");
        
        display.setCursor(0, 46);
        display.println("API:");
        display.println("/api/sensors/json");
        
        display.display();
    }
    
    /**
     * Show database status
     */
    static void showDatabaseStatus(Adafruit_SSD1306& display, int recordCount, long sizeKB) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Database");
        
        display.setTextSize(1);
        display.setCursor(0, 20);
        display.print("Records: ");
        display.println(recordCount);
        
        display.setCursor(0, 30);
        display.print("Size: ");
        display.print(sizeKB);
        display.println(" KB");
        
        display.setCursor(0, 40);
        display.print("Status: OK");
        
        display.setCursor(0, 50);
        display.print("Logging: Active");
        
        display.display();
    }
    
    /**
     * Show multi-sensor grid view  
     */
    static void showSensorGrid(Adafruit_SSD1306& display, const SensorData& data) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        
        // Row 1: Temp and Humidity
        display.setCursor(0, 0);
        display.print("T:");
        display.print(data.temperature, 1);
        display.print("C ");
        display.print("H:");
        display.print(data.humidity, 0);
        display.println("%");
        
        // Row 2: Pressure and Soil
        display.setCursor(0, 10);
        display.print("P:");
        display.print(data.pressure, 0);
        display.print("h ");
        display.print("S:");
        display.print(data.soilMoisture, 0);
        display.println("%");
        
        // Row 3: Light and Gas
        display.setCursor(0, 20);
        display.print("L:");
        display.print(data.lightLevel, 0);
        display.print("l ");
        display.print("G:");
        display.print(data.gasLevel, 0);
        display.println("p");
        
        // Row 4: Motion and Speed
        display.setCursor(0, 30);
        display.print("M:");
        display.print(data.motionDetected ? "Y" : "N");
        display.print(" ");
        display.print("Spd:");
        display.print(data.speed, 1);
        display.println("km");
        
        // Row 5: Water and UV
        display.setCursor(0, 40);
        display.print("W:");
        display.print(data.waterLevel, 0);
        display.print("% ");
        display.print("UV:");
        display.print(data.uvIndex, 1);
        display.println("");
        
        // Row 6: CPU and RAM
        display.setCursor(0, 50);
        display.print("CPU:");
        display.print(data.cpuTemp, 1);
        display.print("C ");
        display.print("RAM:");
        display.print(data.freeHeap / 1024);
        display.println("K");
        
        display.display();
    }
    
    /**
     * Show loading animation for initialization
     */
    static void showLoadingBar(Adafruit_SSD1306& display, int progress) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Initializing...");
        
        display.setCursor(0, 20);
        display.print("Progress: ");
        display.print(progress);
        display.println("%");
        
        // Draw progress bar
        int barWidth = 110;
        int barX = 8;
        int barY = 35;
        int barHeight = 8;
        
        // Border
        display.drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
        
        // Fill
        int fillWidth = (barWidth - 4) * progress / 100;
        display.fillRect(barX + 2, barY + 2, fillWidth, barHeight - 4, SSD1306_WHITE);
        
        display.display();
    }
    
    /**
     * Show system overview with all critical values
     */
    static void showSystemOverview(Adafruit_SSD1306& display, const SensorData& data, const SystemStatus& status) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        
        // Status dots
        display.setCursor(0, 0);
        display.print(status.wifiConnected ? "W" : "-");
        display.print(status.mqttConnected ? "M" : "-");
        display.print(status.bleConnected ? "B" : "-");
        display.print(status.sdCardMounted ? "S" : "-");
        display.println("");
        
        // Main readings
        display.setTextSize(2);
        display.setCursor(0, 10);
        display.print(data.temperature, 0);
        display.println("C");
        
        display.setCursor(0, 26);
        display.print(data.humidity, 0);
        display.println("%");
        
        display.setTextSize(1);
        display.setCursor(70, 10);
        display.print("P:");
        display.println(data.pressure, 0);
        
        display.setCursor(70, 20);
        display.print("G:");
        display.println(data.gasLevel, 0);
        
        display.setCursor(70, 30);
        display.print("M:");
        display.println(data.motionDetected ? "YES" : "NO");
        
        display.setCursor(70, 42);
        display.print("Uptime:");
        display.print(status.uptime / 60);
        display.println("m");
        
        display.setCursor(70, 52);
        display.print("RAM:");
        display.print(data.freeHeap / 1024);
        display.println("KB");
        
        display.display();
    }
};

#endif // DISPLAY_ENHANCEMENTS_H
