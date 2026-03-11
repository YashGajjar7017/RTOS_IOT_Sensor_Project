/**
 * LightSensor - LDR (Light Dependent Resistor) / Photoresistor
 * Measures ambient light intensity
 */

#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>
#include "../include/config.h"

class LightSensor {
private:
    int sensorPin;
    int rawValue;
    float luxValue;
    
    // Calibration values
    int minLight;   // ADC value in complete darkness
    int maxLight;   // ADC value at max brightness
    
public:
    LightSensor() : sensorPin(LIGHT_SENSOR_PIN), 
                   rawValue(0), 
                   luxValue(0) {
        minLight = 4095;   // Dark
        maxLight = 0;      // Bright
    }
    
    void begin() {
        pinMode(sensorPin, INPUT);
        Serial.println("[LightSensor] LDR light sensor initialized");
    }
    
    /**
     * Read raw ADC value (0-4095)
     */
    int readRaw() {
        // Take multiple readings for stability
        int sum = 0;
        for (int i = 0; i < 10; i++) {
            sum += analogRead(sensorPin);
            delay(5);
        }
        rawValue = sum / 10;
        return rawValue;
    }
    
    /**
     * Read light level as percentage (0-100%)
     */
    float read() {
        readRaw();
        
        // Convert to percentage (inverted - higher ADC = darker)
        float light = map(rawValue, minLight, maxLight, 0, 100);
        light = constrain(light, 0, 100);
        
        luxValue = lightToLux(light);
        
        return light;
    }
    
    /**
     * Convert percentage to approximate lux
     */
    float lightToLux(float percentage) {
        // Approximate conversion (not exact, varies by LDR)
        // Full sunlight: ~100,000 lux
        // Indoor: ~500 lux
        // Dark: < 1 lux
        if (percentage <= 0) return 0;
        return pow(10, percentage / 20.0);  // Logarithmic scale
    }
    
    /**
     * Get light level description
     */
    const char* getLevel(float percentage) {
        if (percentage < 10) return "DARK";
        else if (percentage < 30) return "DIM";
        else if (percentage < 50) return "NORMAL";
        else if (percentage < 70) return "BRIGHT";
        else return "VERY BRIGHT";
    }
    
    /**
     * Calibrate sensor
     */
    void calibrate(int min, int max) {
        minLight = min;
        maxLight = max;
        Serial.println("[LightSensor] Sensor calibrated");
    }
    
    /**
     * Auto-calibrate based on current readings
     */
    void autoCalibrate() {
        int current = readRaw();
        
        // Update min/max with slow adaptation
        if (current < minLight) minLight = current;
        if (current > maxLight) maxLight = current;
    }
    
    float getLux() { return luxValue; }
};

#endif // LIGHT_SENSOR_H

