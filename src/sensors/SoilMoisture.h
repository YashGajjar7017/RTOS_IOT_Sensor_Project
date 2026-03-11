/**
 * SoilMoisture - Capacitive Soil Moisture Sensor
 * Measures soil moisture content as percentage
 */

#ifndef SOIL_MOISTURE_H
#define SOIL_MOISTURE_H

#include <Arduino.h>
#include "../include/config.h"

class SoilMoisture {
private:
    int sensorPin;
    int dryValue;    // ADC value when sensor is in dry soil
    int wetValue;   // ADC value when sensor is in water
    
public:
    SoilMoisture() : sensorPin(SOIL_MOISTURE_PIN) {
        dryValue = 4095;  // Will be calibrated
        wetValue = 1200;
    }
    
    void begin() {
        pinMode(sensorPin, INPUT);
        Serial.println("[SoilMoisture] Capacitive soil moisture sensor initialized");
    }
    
    /**
     * Read raw ADC value
     */
    int readRaw() {
        // Take multiple readings and average
        int sum = 0;
        for (int i = 0; i < 10; i++) {
            sum += analogRead(sensorPin);
            delay(10);
        }
        return sum / 10;
    }
    
    /**
     * Read soil moisture as percentage (0-100%)
     * 0% = dry, 100% = water
     */
    float read() {
        int rawValue = readRaw();
        
        // Map to percentage
        // dryValue = high ADC (dry), wetValue = low ADC (wet)
        float moisture = map(rawValue, dryValue, wetValue, 0, 100);
        
        // Constrain to valid range
        moisture = constrain(moisture, 0, 100);
        
        return moisture;
    }
    
    /**
     * Calibrate sensor with known conditions
     */
    void calibrate(int dry, int wet) {
        dryValue = dry;
        wetValue = wet;
        Serial.println("[SoilMoisture] Sensor calibrated");
    }
    
    /**
     * Get moisture level description
     */
    const char* getLevel(float moisture) {
        if (moisture < 20) return "DRY";
        else if (moisture < 40) return "LOW";
        else if (moisture < 60) return "OPTIMAL";
        else if (moisture < 80) return "HIGH";
        else return "SATURED";
    }
};

#endif // SOIL_MOISTURE_H

