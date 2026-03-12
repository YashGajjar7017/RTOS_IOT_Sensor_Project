/**
 * RainSensor - Rain Drop Detection Sensor
 * Detects rainfall using conductive rain sensor module
 */

#ifndef RAIN_SENSOR_H
#define RAIN_SENSOR_H

#include <Arduino.h>
#include "../include/config.h"

class RainSensor {
private:
    int sensorPin;
    int rawValue;
    float rainIntensity;
    bool isRaining;
    
    // Calibration
    int dryValue;     // ADC value when dry
    int wetValue;    // ADC value when fully submerged
    
    // Debounce
    unsigned long lastChangeTime;
    bool lastRainState;
    const unsigned long debounceTime = 1000;  // 1 second
    
public:
    RainSensor() : sensorPin(RAIN_SENSOR_PIN),
                   rawValue(0),
                   rainIntensity(0),
                   isRaining(false),
                   dryValue(4095),
                   wetValue(0),
                   lastChangeTime(0),
                   lastRainState(false) {}
    
    void begin() {
        pinMode(sensorPin, INPUT);
        Serial.println("[RainSensor] Rain drop sensor initialized");
    }
    
    /**
     * Read raw ADC value
     */
    int readRaw() {
        int sum = 0;
        for (int i = 0; i < 10; i++) {
            sum += analogRead(sensorPin);
            delay(5);
        }
        rawValue = sum / 10;
        return rawValue;
    }
    
    /**
     * Read rain intensity as percentage (0-100%)
     * 0% = No rain, 100% = Heavy rain
     */
    float read() {
        readRaw();
        
        // Convert to percentage (lower ADC = more rain)
        // Invert the values: wet = low ADC, dry = high ADC
        rainIntensity = map(rawValue, dryValue, wetValue, 0, 100);
        rainIntensity = constrain(rainIntensity, 0, 100);
        
        // Determine if raining with debounce
        bool currentRain = rainIntensity > 10;  // 10% threshold
        
        if (currentRain != lastRainState) {
            if (millis() - lastChangeTime > debounceTime) {
                isRaining = currentRain;
                lastRainState = currentRain;
                lastChangeTime = millis();
            }
        }
        
        return rainIntensity;
    }
    
    /**
     * Check if it's raining
     */
    bool isRainingNow() {
        return isRaining;
    }
    
    /**
     * Get rainfall severity
     */
    const char* getSeverity() {
        if (rainIntensity < 10) return "NONE";
        else if (rainIntensity < 30) return "LIGHT";
        else if (rainIntensity < 60) return "MODERATE";
        else return "HEAVY";
    }
    
    /**
     * Calibrate sensor
     */
    void calibrate(int dry, int wet) {
        dryValue = dry;
        wetValue = wet;
        Serial.println("[RainSensor] Sensor calibrated");
    }
    
    /**
     * Auto-calibrate based on environment
     */
    void autoCalibrate() {
        int current = readRaw();
        
        // Slowly adapt to environmental conditions
        if (current < dryValue && current > wetValue) {
            // Could implement dynamic calibration here
        }
    }
    
    float getRainIntensity() { return rainIntensity; }
};

#endif // RAIN_SENSOR_H

