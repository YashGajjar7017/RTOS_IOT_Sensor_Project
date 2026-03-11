/**
 * PressureSensor - BMP280 Barometric Pressure Sensor
 * Measures atmospheric pressure and temperature
 */

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "../include/config.h"

class PressureSensor {
private:
    Adafruit_BMP280 bmp;
    bool initialized;
    float lastPressure;
    float lastTemperature;
    
public:
    PressureSensor() : initialized(false), 
                       lastPressure(0), 
                       lastTemperature(0) {}
    
    void begin() {
        // Initialize with I2C - try default address first
        if (!bmp.begin(BMP280_ADDRESS_PRIMARY)) {
            // Try alternate address
            if (!bmp.begin(BMP280_ADDRESS_ALT)) {
                Serial.println("[PressureSensor] Could not find BMP280 sensor!");
                initialized = false;
                return;
            }
        }
        
        // Configure sensor
        bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                       Adafruit_BMP280::SAMPLING_X2,
                       Adafruit_BMP280::SAMPLING_X16,
                       Adafruit_BMP280::FILTER_X16,
                       Adafruit_BMP280::STANDBY_MS_500);
        
        initialized = true;
        Serial.println("[PressureSensor] BMP280 pressure sensor initialized");
    }
    
    /**
     * Read pressure in hPa (hectopascals)
     */
    float read() {
        if (!initialized) return lastPressure;
        
        lastPressure = bmp.readPressure() / 100.0F;  // Convert Pa to hPa
        return lastPressure;
    }
    
    /**
     * Read temperature from BMP280
     */
    float readTemperature() {
        if (!initialized) return lastTemperature;
        
        lastTemperature = bmp.readTemperature();
        return lastTemperature;
    }
    
    /**
     * Calculate altitude estimate (meters)
     * Uses standard sea level pressure
     */
    float readAltitude() {
        if (!initialized) return 0;
        
        // Default sea level pressure: 1013.25 hPa
        return bmp.readAltitude(1013.25);
    }
    
    /**
     * Calculate altitude with custom sea level pressure
     */
    float readAltitude(float seaLevelPressure) {
        if (!initialized) return 0;
        
        return bmp.readAltitude(seaLevelPressure);
    }
    
    /**
     * Check if sensor is working
     */
    bool isReady() {
        return initialized;
    }
    
    /**
     * Get pressure trend
     */
    const char* getTrend() {
        // Simple trend based on current reading
        // Would need historical data for proper trend
        if (lastPressure < 1000) return "LOW";
        else if (lastPressure < 1015) return "NORMAL";
        else if (lastPressure < 1030) return "HIGH";
        else return "VERY HIGH";
    }
};

#endif // PRESSURE_SENSOR_H

