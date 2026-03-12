/**
 * DustSensor - PM2.5 Dust/Particle Sensor
 * Measures particulate matter concentration (PM1.0, PM2.5, PM10)
 * Uses Sharp GP2Y1010AU0F sensor
 */

#ifndef DUST_SENSOR_H
#define DUST_SENSOR_H

#include <Arduino.h>
#include "../include/config.h"

class DustSensor {
private:
    int measurePin;
    int ledPin;
    int rawValue;
    float dustDensity;
    
    // Calibration values
    float voc;        // Voltage at 0 dust (clean air)
    float sensitivity;  // mV per (ug/m3)
    
    // Sampling
    unsigned long sampleStart;
    const unsigned long sampleTime = 280;   // LED pulse time (us)
    const unsigned long deltaTime = 40;     // Wait time (us)
    const unsigned long sleepTime = 9680;   // Total cycle time (us)
    
public:
    DustSensor() : measurePin(DUST_MEASURE_PIN),
                   ledPin(DUST_LED_PIN),
                   rawValue(0),
                   dustDensity(0),
                   voc(0.6),
                   sensitivity(0.5),
                   sampleStart(0) {}
    
    void begin() {
        pinMode(ledPin, OUTPUT);
        pinMode(measurePin, INPUT);
        Serial.println("[DustSensor] Sharp GP2Y1010 dust sensor initialized");
        Serial.println("[DustSensor] Note: Allow 15 seconds for sensor stabilization");
    }
    
    /**
     * Read dust concentration
     * Returns PM2.5 density in μg/m³
     */
    float read() {
        // LED pulse sequence
        digitalWrite(ledPin, LOW);
        delayMicroseconds(sampleTime);
        
        // Read analog value
        rawValue = analogRead(measurePin);
        
        delayMicroseconds(deltaTime);
        digitalWrite(ledPin, HIGH);
        
        // Wait for next cycle
        delayMicroseconds(sleepTime);
        
        // Convert to voltage
        float voltage = rawValue * (3.3 / 4095.0);
        
        // Calculate dust density
        // Formula based on Sharp GP2Y1010AU0F datasheet
        // (V - Voc) / sensitivity
        if (voltage > voc) {
            dustDensity = (voltage - voc) / sensitivity;
        } else {
            dustDensity = 0;
        }
        
        dustDensity = constrain(dustDensity, 0, 500);
        
        return dustDensity;
    }
    
    /**
     * Get PM2.5 level description
     */
    const char* getLevel() {
        if (dustDensity < 12) return "GOOD";
        else if (dustDensity < 35) return "MODERATE";
        else if (dustDensity < 55) return "UNHEALTHY_SENSITIVE";
        else if (dustDensity < 150) return "UNHEALTHY";
        else if (dustDensity < 250) return "VERY_UNHEALTHY";
        else return "HAZARDOUS";
    }
    
    /**
     * Get health recommendation
     */
    const char* getRecommendation() {
        if (dustDensity < 12) return "Air quality is satisfactory";
        else if (dustDensity < 35) return "Acceptable for most";
        else if (dustDensity < 55) return "Sensitive groups limit exposure";
        else if (dustDensity < 150) return "Everyone should reduce outdoor activity";
        else if (dustDensity < 250) return "Avoid outdoor activities";
        else return "Emergency conditions";
    }
    
    /**
     * Calculate Air Quality Index (AQI) for PM2.5
     */
    int getAQI() {
        // Simplified AQI calculation for PM2.5
        // Based on EPA standard
        if (dustDensity <= 12) return map(dustDensity, 0, 12, 0, 50);
        else if (dustDensity <= 35) return map(dustDensity, 12, 35, 51, 100);
        else if (dustDensity <= 55) return map(dustDensity, 35, 55, 101, 150);
        else if (dustDensity <= 150) return map(dustDensity, 55, 150, 151, 200);
        else if (dustDensity <= 250) return map(dustDensity, 150, 250, 201, 300);
        else return 300 + (int)((dustDensity - 250) / 50) * 100;
    }
    
    /**
     * Check if sensor needs cleaning
     */
    bool needsCleaning() {
        // High baseline voltage indicates dirty sensor
        return rawValue > 500;
    }
    
    /**
     * Calibrate sensor
     */
    void calibrate(float voltageOffset, float sens) {
        voc = voltageOffset;
        sensitivity = sens;
        Serial.println("[DustSensor] Sensor calibrated");
    }
    
    float getDustDensity() { return dustDensity; }
    int getRawValue() { return rawValue; }
};

#endif // DUST_SENSOR_H

