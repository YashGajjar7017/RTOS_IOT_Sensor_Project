/**
 * BLEHandler - Bluetooth Low Energy (BLE) GATT Server
 * Allows BLE clients to read sensor data
 */

#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "../include/config.h"

// BLE Server Callback
class BLEServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("[BLE] Client connected");
    }
    
    void onDisconnect(BLEServer* pServer) {
        Serial.println("[BLE] Client disconnected");
    }
};

// Characteristic Callback for read requests
class SensorDataCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic* pCharacteristic) {
        Serial.println("[BLE] Sensor data requested by client");
    }
};

class BLEHandler {
private:
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pSensorDataCharacteristic;
    BLECharacteristic* pStatusCharacteristic;
    bool connected;
    bool started;
    
    // Current sensor data for BLE
    SensorData currentData;
    
public:
    BLEHandler() : pServer(nullptr), 
                  pService(nullptr),
                  pSensorDataCharacteristic(nullptr),
                  pStatusCharacteristic(nullptr),
                  connected(false),
                  started(false) {}
    
    /**
     * Initialize and start BLE server
     */
    void start() {
        if (started) return;
        
        Serial.println("[BLE] Initializing BLE server...");
        
        // Initialize BLE device
        BLEDevice::init(BLE_DEVICE_NAME);
        
        // Create BLE server
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new BLEServerCallbacks());
        
        // Create BLE service
        pService = pServer->createService(BLE_SERVICE_UUID);
        
        // Create sensor data characteristic (read/notify)
        pSensorDataCharacteristic = pService->createCharacteristic(
            BLE_CHAR_UUID,
            BLECharacteristic::PROPERTY_READ | 
            BLECharacteristic::PROPERTY_NOTIFY
        );
        
        // Add descriptor
        pSensorDataCharacteristic->addDescriptor(new BLE2902());
        pSensorDataCharacteristic->setCallbacks(new SensorDataCallbacks());
        
        // Create status characteristic
        BLEStatusCharacteristic* pStatus = pService->createCharacteristic(
            "beb5483e-36e1-4688-b7f5-ea07361b26a9",
            BLECharacteristic::PROPERTY_READ
        );
        pStatus->addDescriptor(new BLE2902());
        
        // Start service
        pService->start();
        
        // Start advertising
        BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);
        pAdvertising->setMinPreferred(0x12);
        BLEDevice::startAdvertising();
        
        started = true;
        connected = false;
        
        Serial.println("[BLE] BLE server started!");
        Serial.printf("[BLE] Device name: %s\n", BLE_DEVICE_NAME);
    }
    
    /**
     * Update sensor data for BLE clients
     */
    void updateSensorData(SensorData data) {
        currentData = data;
        
        if (!started || !pSensorDataCharacteristic) return;
        
        // Create JSON string
        String json = createJSON(data);
        
        // Set value and notify
        pSensorDataCharacteristic->setValue(json.c_str());
        pSensorDataCharacteristic->notify();
    }
    
    /**
     * Create JSON from sensor data
     */
    String createJSON(SensorData data) {
        String json = "{";
        json += "\"temp\":" + String(data.temperature, 1) + ",";
        json += "\"hum\":" + String(data.humidity, 1) + ",";
        json += "\"pres\":" + String(data.pressure, 1) + ",";
        json += "\"soil\":" + String(data.soilMoisture, 1) + ",";
        json += "\"light\":" + String(data.lightLevel, 1) + ",";
        json += "\"motion\":" + String(data.motionDetected ? "1" : "0") + ",";
        json += "\"speed\":" + String(data.speed, 1) + ",";
        json += "\"gas\":" + String(data.gasLevel, 1);
        json += "}";
        
        return json;
    }
    
    /**
     * Check if a client is connected
     */
    bool isConnected() {
        if (!started || !pServer) return false;
        return pServer->getConnectedCount() > 0;
    }
    
    /**
     * Stop BLE server
     */
    void stop() {
        if (!started) return;
        
        pService->stop();
        BLEDevice::deinit(false);
        started = false;
        
        Serial.println("[BLE] BLE server stopped");
    }
    
    /**
     * Get connection status
     */
    bool isStarted() {
        return started;
    }
};

#endif // BLE_HANDLER_H

