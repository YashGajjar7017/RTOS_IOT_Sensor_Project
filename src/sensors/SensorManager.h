/**
 * SensorManager - Central hub for all sensor operations
 * Manages all sensors and provides unified data access
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "../include/config.h"

// Individual sensor headers
#include "TempHumidity.h"
#include "SoilMoisture.h"
#include "MotionSensor.h"
#include "SpeedSensor.h"  // This includes both .h and links .cpp
#include "LightSensor.h"
#include "GasSensor.h"
#include "PressureSensor.h"

class SensorManager {
private:
    // Sensor objects
    TempHumidity tempHumidity;
    SoilMoisture soilMoisture;
    MotionSensor motionSensor;
    SpeedSensor speedSensor;
    LightSensor lightSensor;
    GasSensor gasSensor;
    PressureSensor pressureSensor;
    
    // Last sensor readings for smoothing
    float lastTemp = 0;
    float lastHumidity = 0;
    float lastPressure = 0;
    float lastSoilMoisture = 0;
    float lastLightLevel = 0;
    float lastGasLevel = 0;
    
    // Smoothing factor (0.0-1.0, higher = more responsive)
    const float smoothingFactor = 0.7;
    
public:
    SensorManager() {}
    
    /**
     * Initialize all sensors
     */
    void begin() {
        Serial.println("[SensorManager] Initializing all sensors...");
        
        // Initialize each sensor
        tempHumidity.begin();
        soilMoisture.begin();
        motionSensor.begin();
        speedSensor.begin();
        lightSensor.begin();
        gasSensor.begin();
        pressureSensor.begin();
        
        // Initial readings to initialize values
        delay(100);
        SensorData initial = readAllSensors();
        
        Serial.println("[SensorManager] All sensors initialized!");
    }
    
    /**
     * Read all sensors and return combined data
     */
    SensorData readAllSensors() {
        SensorData data;
        
        // Read Temperature & Humidity (DHT22)
        TempHumidityData thData = tempHumidity.read();
        data.temperature = smoothValue(thData.temperature, lastTemp);
        lastTemp = data.temperature;
        
        data.humidity = smoothValue(thData.humidity, lastHumidity);
        lastHumidity = data.humidity;
        
        // Read Pressure (BMP280)
        data.pressure = smoothValue(pressureSensor.read(), lastPressure);
        lastPressure = data.pressure;
        
        // Read Soil Moisture
        data.soilMoisture = smoothValue(soilMoisture.read(), lastSoilMoisture);
        lastSoilMoisture = data.soilMoisture;
        
        // Read Light Level
        data.lightLevel = smoothValue(lightSensor.read(), lastLightLevel);
        lastLightLevel = data.lightLevel;
        
        // Read Motion
        data.motionDetected = motionSensor.isDetected();
        
        // Read Speed
        data.speed = speedSensor.getSpeed();
        
        // Read Gas Level
        data.gasLevel = smoothValue(gasSensor.read(), lastGasLevel);
        lastGasLevel = data.gasLevel;
        
        return data;
    }
    
    /**
     * Apply exponential smoothing to sensor readings
     */
    float smoothValue(float newValue, float lastValue) {
        if (isnan(newValue) || newValue < 0) {
            return lastValue;  // Return last valid value
        }
        return (smoothingFactor * newValue) + ((1 - smoothingFactor) * lastValue);
    }
    
    /**
     * Get individual sensor readings
     */
    float getTemperature() { return lastTemp; }
    float getHumidity() { return lastHumidity; }
    float getPressure() { return lastPressure; }
    float getSoilMoisture() { return lastSoilMoisture; }
    float getLightLevel() { return lastLightLevel; }
    float getGasLevel() { return lastGasLevel; }
    bool getMotionDetected() { return motionSensor.isDetected(); }
    float getSpeed() { return speedSensor.getSpeed(); }
};

#endif // SENSOR_MANAGER_H

