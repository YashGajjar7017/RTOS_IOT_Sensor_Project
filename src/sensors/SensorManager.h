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
#include "WaterLevelSensor.h"
#include "UVSensor.h"
#include "RainSensor.h"
#include "DustSensor.h"

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
    WaterLevelSensor waterLevelSensor;
    UVSensor uvSensor;
    RainSensor rainSensor;
    DustSensor dustSensor;
    
    // Last sensor readings for smoothing
    float lastTemp = 0;
    float lastHumidity = 0;
    float lastPressure = 0;
    float lastSoilMoisture = 0;
    float lastLightLevel = 0;
    float lastGasLevel = 0;
    float lastWaterLevel = 0;
    float lastUVIndex = 0;
    float lastRainIntensity = 0;
    float lastDustDensity = 0;
    
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
        waterLevelSensor.begin();
        uvSensor.begin();
        rainSensor.begin();
        dustSensor.begin();
        
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
        
        // Read Water Level (Ultrasonic)
        data.waterLevel = smoothValue(waterLevelSensor.read(), lastWaterLevel);
        lastWaterLevel = data.waterLevel;
        
        // Read UV Index
        data.uvIndex = smoothValue(uvSensor.read(), lastUVIndex);
        lastUVIndex = data.uvIndex;
        
        // Read Rain Intensity
        data.rainIntensity = smoothValue(rainSensor.read(), lastRainIntensity);
        lastRainIntensity = data.rainIntensity;
        
        // Read Dust Density (PM2.5)
        data.dustDensity = smoothValue(dustSensor.read(), lastDustDensity);
        lastDustDensity = data.dustDensity;
        
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
    float getWaterLevel() { return waterLevelSensor.getWaterLevel(); }
    float getUVIndex() { return uvSensor.getUVIndex(); }
    float getRainIntensity() { return rainSensor.getRainIntensity(); }
    float getDustDensity() { return dustSensor.getDustDensity(); }
    
    // Convenience methods for additional sensor info
    bool isRaining() { return rainSensor.isRainingNow(); }
    bool isWaterLevelCritical(float threshold = 20.0) { 
        return waterLevelSensor.isCritical(threshold); 
    }
    int getDustAQI() { return dustSensor.getAQI(); }
};

#endif // SENSOR_MANAGER_H

