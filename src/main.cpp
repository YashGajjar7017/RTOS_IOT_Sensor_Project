/**
 * IoT RTOS Sensor Network - Main Application
 * 
 * Multi-sensor monitoring system using FreeRTOS on ESP32
 * Features: DHT22, Soil Moisture, PIR, Speed, Light, Gas, BMP280 sensors
 * Outputs: OLED Display, WiFi/MQTT, BLE, SD Card Logging
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>

// Include all sensor and system headers
#include "sensors/SensorManager.h"
#include "communication/WiFiManager.h"
#include "communication/MQTTClient.h"
#include "communication/BLEHandler.h"
#include "display/DisplayManager.h"
#include "storage/SDCardManager.h"
#include "tasks/RTOS_Tasks.h"
#include "../include/config.h"

// Global objects
SensorManager sensorManager;
WiFiManager wifiManager;
MQTTClient mqttClient;
BLEHandler bleHandler;
DisplayManager displayManager;
SDCardManager sdCardManager;

// Task handles
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t mqttTaskHandle = NULL;
TaskHandle_t bleTaskHandle = NULL;
TaskHandle_t sdCardTaskHandle = NULL;
TaskHandle_t watchdogTaskHandle = NULL;

// Shared data
SensorData sensorData;
SystemStatus systemStatus;
SemaphoreHandle_t dataMutex;

// Watchdog counter
volatile uint32_t watchdogCounter = 0;

// ==================== Task Implementations ====================

/**
 * Sensor Reading Task - Core 0, High Priority
 * Reads all sensors and updates shared data
 */
void sensorTask(void* parameter) {
    Serial.println("[SensorTask] Started on Core 0");
    
    while (true) {
        // Reset watchdog
        watchdogCounter = 0;
        
        // Take mutex for data access
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            // Read all sensors
            sensorData = sensorManager.readAllSensors();
            sensorData.freeHeap = ESP.getFreeHeap();
            sensorData.timestamp = millis();
            
            // Get CPU temperature
            sensorData.cpuTemp = temperatureRead();
            
            xSemaphoreGive(dataMutex);
            
            // LED blink on successful read
            digitalWrite(LED_SENSOR_PIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(50));
            digitalWrite(LED_SENSOR_PIN, LOW);
        }
        
        vTaskDelay(pdMS_TO_TICKS(DELAY_SENSOR_READ));
    }
}

/**
 * Display Task - Core 0, Medium Priority
 * Updates OLED display with sensor readings
 */
void displayTask(void* parameter) {
    Serial.println("[DisplayTask] Started on Core 0");
    
    displayManager.clear();
    displayManager.showWelcome();
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    while (true) {
        // Take mutex for data access
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
            displayManager.showSensorData(sensorData);
            displayManager.showSystemStatus(systemStatus);
            xSemaphoreGive(dataMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(DELAY_DISPLAY));
    }
}

/**
 * MQTT Task - Core 1, High Priority
 * Publishes sensor data to MQTT broker
 */
void mqttTask(void* parameter) {
    Serial.println("[MQTTTask] Started on Core 1");
    
    // Wait for WiFi connection
    while (!systemStatus.wifiConnected) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // Connect to MQTT
    mqttClient.connect();
    
    while (true) {
        if (systemStatus.wifiConnected && systemStatus.mqttConnected) {
            // Take mutex for data access
            if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                mqttClient.publish(sensorData);
                xSemaphoreGive(dataMutex);
                
                // LED blink on successful publish
                digitalWrite(LED_MQTT_PIN, HIGH);
                vTaskDelay(pdMS_TO_TICKS(50));
                digitalWrite(LED_MQTT_PIN, LOW);
            }
        } else {
            // Reconnect if needed
            if (systemStatus.wifiConnected && !systemStatus.mqttConnected) {
                mqttClient.connect();
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(DELAY_MQTT));
    }
}

/**
 * BLE Task - Core 1, Medium Priority
 * Handles Bluetooth Low Energy connections
 */
void bleTask(void* parameter) {
    Serial.println("[BLETask] Started on Core 1");
    
    bleHandler.start();
    
    while (true) {
        // Update BLE with current sensor data
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
            bleHandler.updateSensorData(sensorData);
            systemStatus.bleConnected = bleHandler.isConnected();
            xSemaphoreGive(dataMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * SD Card Task - Core 1, Low Priority
 * Logs sensor data to SD card
 */
void sdCardTask(void* parameter) {
    Serial.println("[SDCardTask] Started on Core 1");
    
    // Wait for SD card initialization
    while (!systemStatus.sdCardMounted) {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    // Create CSV header if new file
    sdCardManager.createLogFile();
    
    while (true) {
        // Take mutex for data access
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            sdCardManager.logData(sensorData);
            xSemaphoreGive(dataMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(DELAY_SDCARD));
    }
}

/**
 * Watchdog Task - Core 0, Highest Priority
 * Monitors system health and task execution
 */
void watchdogTask(void* parameter) {
    Serial.println("[WatchdogTask] Started on Core 0");
    
    uint32_t lastCounter = 0;
    
    while (true) {
        if (watchdogCounter == lastCounter) {
            // Task may be stuck
            Serial.println("[Watchdog] Warning: Sensor task may be stuck!");
        }
        lastCounter = watchdogCounter;
        
        // Update system status
        systemStatus.wifiConnected = wifiManager.isConnected();
        systemStatus.mqttConnected = mqttClient.isConnected();
        systemStatus.uptime = millis() / 1000;
        
        vTaskDelay(pdMS_TO_TICKS(SYSTEM_STATUS_CHECK));
    }
}

// ==================== Setup & Main ====================

void setup() {
    // Initialize serial
    Serial.begin(115200);
    Serial.println("\n\n========================================");
    Serial.println("IoT RTOS Sensor Network");
    Serial.println("========================================");
    
    // Create mutex for shared data
    dataMutex = xSemaphoreCreateMutex();
    
    // Initialize status LEDs
    pinMode(LED_WIFI_PIN, OUTPUT);
    pinMode(LED_MQTT_PIN, OUTPUT);
    pinMode(LED_SENSOR_PIN, OUTPUT);
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    
    digitalWrite(LED_WIFI_PIN, LOW);
    digitalWrite(LED_MQTT_PIN, LOW);
    digitalWrite(LED_SENSOR_PIN, LOW);
    digitalWrite(LED_BUILTIN_PIN, LOW);
    
    // Initialize sensors
    Serial.println("\n[Setup] Initializing sensors...");
    sensorManager.begin();
    
    // Initialize display
    Serial.println("[Setup] Initializing display...");
    displayManager.begin();
    
    // Initialize SD Card
    Serial.println("[Setup] Initializing SD card...");
    systemStatus.sdCardMounted = sdCardManager.begin();
    
    // Initialize WiFi
    Serial.println("[Setup] Connecting to WiFi...");
    wifiManager.begin();
    
    // Initialize system status
    systemStatus.wifiConnected = false;
    systemStatus.mqttConnected = false;
    systemStatus.bleConnected = false;
    systemStatus.taskCount = 6;
    
    // Create FreeRTOS Tasks
    Serial.println("\n[Setup] Creating RTOS tasks...");
    
    // Sensor Task - Core 0
    xTaskCreatePinnedToCore(
        sensorTask,           // Task function
        "SensorTask",         // Task name
        STACK_SIZE_MEDIUM,    // Stack size
        NULL,                 // Parameters
        PRIORITY_SENSOR_READ, // Priority
        &sensorTaskHandle,    // Task handle
        0                     // Core 0
    );
    
    // Display Task - Core 0
    xTaskCreatePinnedToCore(
        displayTask,
        "DisplayTask",
        STACK_SIZE_MEDIUM,
        NULL,
        PRIORITY_DISPLAY,
        &displayTaskHandle,
        0
    );
    
    // Watchdog Task - Core 0 (highest priority)
    xTaskCreatePinnedToCore(
        watchdogTask,
        "WatchdogTask",
        STACK_SIZE_SMALL,
        NULL,
        PRIORITY_WATCHDOG,
        &watchdogTaskHandle,
        0
    );
    
    // MQTT Task - Core 1
    xTaskCreatePinnedToCore(
        mqttTask,
        "MQTTTask",
        STACK_SIZE_LARGE,
        NULL,
        PRIORITY_MQTT,
        &mqttTaskHandle,
        1
    );
    
    // BLE Task - Core 1
    xTaskCreatePinnedToCore(
        bleTask,
        "BLETask",
        STACK_SIZE_MEDIUM,
        NULL,
        PRIORITY_BLE,
        &bleTaskHandle,
        1
    );
    
    // SD Card Task - Core 1
    xTaskCreatePinnedToCore(
        sdCardTask,
        "SDCardTask",
        STACK_SIZE_MEDIUM,
        NULL,
        PRIORITY_SDCARD,
        &sdCardTaskHandle,
        1
    );
    
    Serial.println("\n[Setup] All tasks created successfully!");
    Serial.println("========================================\n");
    
    // Initialize watchdog counter increment in sensor task
    // (this is done in the sensor task itself)
}

void loop() {
    // Main loop is empty - all work is done in FreeRTOS tasks
    // This allows better real-time performance
    
    // WiFi status LED
    digitalWrite(LED_WIFI_PIN, wifiManager.isConnected() ? HIGH : LOW);
    
    vTaskDelay(pdMS_TO_TICKS(100));
}

