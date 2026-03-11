/**
 * GasSensor - MQ-135 Air Quality / Gas Sensor
 * Measures various gases: CO, CO2, Alcohol, Benzene, Smoke, NH3
 */

#ifndef GAS_SENSOR_H
#define GAS_SENSOR_H

#include <Arduino.h>
#include "../include/config.h"

class GasSensor {
private:
    int sensorPin;
    int rawValue;
    float ppmValue;
    bool warmingUp;
    unsigned long warmupStart;
    
    // Calibration
    float r0;  // Sensor resistance in clean air
    
public:
    GasSensor() : sensorPin(GAS_SENSOR_PIN),
                 rawValue(0),
                 ppmValue(0),
                 warmingUp(true),
                 warmupStart(0) {
        r0 = GAS_R0_CALIBRATION;
    }
    
    void begin() {
        pinMode(sensorPin, INPUT);
        warmupStart = millis();
        Serial.println("[GasSensor] MQ-135 gas sensor initializing...");
        Serial.println("[GasSensor] Please wait 20 seconds for sensor warmup...");
    }
    
    /**
     * Read raw ADC value
     */
    int readRaw() {
        int sum = 0;
        for (int i = 0; i < 10; i++) {
            sum += analogRead(sensorPin);
            delay(10);
        }
        rawValue = sum / 10;
        return rawValue;
    }
    
    /**
     * Read gas level as percentage (0-100%)
     * Based on CO2 equivalent
     */
    float read() {
        // Check warmup
        if (warmingUp) {
            if (millis() - warmupStart > GAS_WARMUP_TIME) {
                warmingUp = false;
                Serial.println("[GasSensor] Sensor warmup complete!");
            }
            return 0;
        }
        
        readRaw();
        
        // Calculate ppm (simplified - actual calibration needed)
        // MQ-135: higher voltage = lower resistance = more gas
        float voltage = rawValue * (3.3 / 4095.0);
        float rs = (3.3 - voltage) / voltage * 10;  // Sensor resistance
        
        // Ratio to calculate ppm (simplified curve)
        float ratio = rs / r0;
        
        // Approximate CO2 ppm (350-10000 ppm typical)
        // This is a simplified formula - real calibration needed
        if (ratio > 0) {
            ppmValue = 116.6020682 * pow(ratio, -2.769034857);
        }
        
        // Convert to percentage (10000 ppm = 100%)
        float percentage = (ppmValue / 10000.0) * 100.0;
        percentage = constrain(percentage, 0, 100);
        
        return percentage;
    }
    
    /**
     * Get specific gas concentrations (approximate)
     */
    float getCO2() { return ppmValue; }
    
    /**
     * Get air quality level
     */
    const char* getQuality() {
        if (ppmValue < 400) return "EXCELLENT";
        else if (ppmValue < 600) return "GOOD";
        else if (ppmValue < 1000) return "FAIR";
        else if (ppmValue < 2000) return "POOR";
        else return "HAZARDOUS";
    }
    
    /**
     * Calibrate sensor in fresh air
     */
    void calibrate(float r0Value) {
        r0 = r0Value;
        Serial.println("[GasSensor] Sensor calibrated");
    }
    
    /**
     * Check if sensor is still warming up
     */
    bool isWarmingUp() {
        return warmingUp;
    }
    
    /**
     * Get remaining warmup time
     */
    unsigned long getWarmupRemaining() {
        if (!warmingUp) return 0;
        long remaining = GAS_WARMUP_TIME - (millis() - warmupStart);
        return remaining > 0 ? remaining : 0;
    }
};

#endif // GAS_SENSOR_H

