/**
 * SDCardManager - SD Card Data Logging
 * Logs sensor data to SD card in CSV format
 */

#ifndef SD_CARD_MANAGER_H
#define SD_CARD_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "../include/config.h"

class SDCardManager {
private:
    bool initialized;
    bool mounted;
    File dataFile;
    String currentFileName;
    unsigned long lastLogTime;
    int logCount;
    
public:
    SDCardManager() : initialized(false),
                     mounted(false),
                     lastLogTime(0),
                     logCount(0) {}
    
    /**
     * Initialize SD card
     */
    bool begin() {
        Serial.println("[SDCard] Initializing SD card...");
        
        // Initialize SPI
        SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
        
        // Initialize SD card
        if (!SD.begin(SD_CS_PIN, SPI, 4000000)) {
            Serial.println("[SDCard] Card mount failed!");
            mounted = false;
            return false;
        }
        
        // Check card type
        uint8_t cardType = SD.cardType();
        if (cardType == CARD_NONE) {
            Serial.println("[SDCard] No SD card attached!");
            mounted = false;
            return false;
        }
        
        Serial.printf("[SDCard] Card type: %s\n", getCardTypeString(cardType));
        
        // Get card info
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("[SDCard] Card size: %llu MB\n", cardSize);
        
        mounted = true;
        initialized = true;
        
        // List files
        listFiles();
        
        Serial.println("[SDCard] SD card initialized successfully!");
        return true;
    }
    
    /**
     * Create log file with header
     */
    void createLogFile() {
        if (!mounted) return;
        
        // Generate filename with date
        String filename = "/sensor_log_";
        filename += String(millis() / 1000);  // Simple timestamp
        filename += ".csv";
        
        currentFileName = filename;
        
        // Check if file exists
        bool exists = SD.exists(filename.c_str());
        
        // Open file for append
        dataFile = SD.open(filename.c_str(), FILE_APPEND);
        
        if (!dataFile) {
            Serial.printf("[SDCard] Failed to open file: %s\n", filename.c_str());
            return;
        }
        
        // Write header if new file
        if (!exists || dataFile.size() == 0) {
            dataFile.println("timestamp,temperature,humidity,pressure,soil_moisture,light_level,motion,speed,gas_level,free_heap,cpu_temp");
            Serial.printf("[SDCard] Created new log file: %s\n", filename.c_str());
        }
        
        dataFile.close();
    }
    
    /**
     * Log sensor data to SD card
     */
    bool logData(SensorData data) {
        if (!mounted) return false;
        
        // Open file for append
        dataFile = SD.open(currentFileName.c_str(), FILE_APPEND);
        
        if (!dataFile) {
            Serial.println("[SDCard] Failed to open file for logging!");
            return false;
        }
        
        // Create CSV line
        String line = String(data.timestamp) + ",";
        line += String(data.temperature, 2) + ",";
        line += String(data.humidity, 2) + ",";
        line += String(data.pressure, 2) + ",";
        line += String(data.soilMoisture, 2) + ",";
        line += String(data.lightLevel, 2) + ",";
        line += String(data.motionDetected ? "1" : "0") + ",";
        line += String(data.speed, 2) + ",";
        line += String(data.gasLevel, 2) + ",";
        line += String(data.freeHeap) + ",";
        line += String(data.cpuTemp, 2);
        
        // Write to file
        dataFile.println(line);
        dataFile.close();
        
        logCount++;
        lastLogTime = millis();
        
        Serial.printf("[SDCard] Logged data (#%d): %s\n", logCount, line.c_str());
        
        // Check file size periodically
        checkFileSize();
        
        return true;
    }
    
    /**
     * Check and manage file size
     */
    void checkFileSize() {
        if (!mounted || currentFileName.length() == 0) return;
        
        dataFile = SD.open(currentFileName.c_str());
        if (dataFile) {
            size_t fileSize = dataFile.size();
            dataFile.close();
            
            // If file > 10MB, create new file
            if (fileSize > 10 * 1024 * 1024) {
                Serial.println("[SDCard] File too large, creating new one...");
                createLogFile();
            }
        }
    }
    
    /**
     * List all files on SD card
     */
    void listFiles() {
        if (!mounted) return;
        
        Serial.println("[SDCard] Files on SD card:");
        
        File root = SD.open("/");
        File file = root.openNextFile();
        
        while (file) {
            if (file.isDirectory()) {
                Serial.printf("[SDCard] DIR: %s\n", file.name());
            } else {
                Serial.printf("[SDCard] FILE: %s (%lu bytes)\n", file.name(), file.size());
            }
            file = root.openNextFile();
        }
        
        root.close();
    }
    
    /**
     * Read last N lines from log file
     */
    void readLastLines(int numLines) {
        if (!mounted) return;
        
        dataFile = SD.open(currentFileName.c_str());
        if (!dataFile) return;
        
        // Go to end of file
        if (dataFile.seek(dataFile.size() - 1)) {
            // Read backwards to find lines
            String lines[10];
            int lineCount = 0;
            
            // Simple implementation - just read last portion
            char buffer[256];
            size_t pos = dataFile.size() - 256;
            if (pos < 0) pos = 0;
            
            dataFile.seek(pos);
            
            while (dataFile.available()) {
                String line = dataFile.readStringUntil('\n');
                if (line.length() > 0) {
                    Serial.printf("[SDCard] %s\n", line.c_str());
                }
            }
        }
        
        dataFile.close();
    }
    
    /**
     * Delete old log files
     */
    void cleanupOldFiles(int maxFiles) {
        if (!mounted) return;
        
        File root = SD.open("/");
        File file = root.openNextFile();
        
        // Collect files
        struct FileInfo {
            String name;
            unsigned long size;
            unsigned long created;
        };
        
        // Simple cleanup - just keep last file
        // More sophisticated implementation would track all files
        
        root.close();
    }
    
    /**
     * Check if SD card is mounted
     */
    bool isMounted() {
        return mounted;
    }
    
    /**
     * Get current file name
     */
    String getCurrentFileName() {
        return currentFileName;
    }
    
    /**
     * Get card type string
     */
    const char* getCardTypeString(uint8_t cardType) {
        switch (cardType) {
            case CARD_MMC: return "MMC";
            case CARD_SD: return "SDSC";
            case CARD_SDHC: return "SDHC";
            default: return "UNKNOWN";
        }
    }
};

#endif // SD_CARD_MANAGER_H

