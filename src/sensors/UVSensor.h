/**
 * UVSensor - UV (Ultraviolet) Radiation Sensor
 * Measures UV index using UV sensor module (e.g., GYML8511)
 */

#ifndef UV_SENSOR_H
#define UV_SENSOR_H

#include <Arduino.h>
#include "../include/config.h"

class UVSensor {
private:
    int sensorPin;
    int rawValue;
    float uvIndex;
    float voltage;
    
    // Calibration
    float outputVoltage;   // Output at 1 UV index (typically 0.1V)
    float uvResistance;    // Sensor resistance at 1 UV
    
public:
    UVSensor() : sensorPin(UV_SENSOR_PIN),
                 rawValue(0),
                 uvIndex(0),
                 voltage(0),
                 outputVoltage(0.1),
                 uvResistance(250) {}
    
    void begin() {
        pinMode(sensorPin, INPUT);
        Serial.println("[UVSensor] UV radiation sensor initialized");
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
     * Read UV index (0-11+)
     */
    float read() {
        readRaw();
        
        // Convert to voltage (ESP32: 3.3V reference, 12-bit ADC)
        voltage = rawValue * (3.3 / 4095.0);
        
        // Calculate UV index
        // Formula: UV index = (output_voltage - offset) / sensitivity
        // Typical: 0.1V per UV index
        if (voltage > 0) {
            uvIndex = voltage / outputVoltage;
        }
        
        uvIndex = constrain(uvIndex, 0, 15);
        return uvIndex;
    }
    
    /**
     * Get UV exposure risk level
     */
    const char* getRiskLevel() {
        if (uvIndex < 3) return "LOW";
        else if (uvIndex < 6) return "MODERATE";
        else if (uvIndex < 8) return "HIGH";
        else if (uvIndex < 11) return "VERY HIGH";
        else return "EXTREME";
    }
    
    /**
     * Get recommended protection
     */
    const char* getProtection() {
        if (uvIndex < 3) return "No protection required";
        else if (uvIndex < 6) return "Sunscreen recommended";
        else if (uvIndex < 8) return "Wear protective clothing";
        else if (uvIndex < 11) return "Extra protection needed";
        else return "Avoid sun exposure";
    }
    
    /**
     * Get exposure time recommendation (in minutes)
     */
    int getSafeExposureMinutes() {
        if (uvIndex < 3) return 60;
        else if (uvIndex < 5) return 45;
        else if (uvIndex < 7) return 30;
        else if (uvIndex < 9) return 15;
        else if (uvIndex < 11) return 10;
        else return 5;
    }
    
    /**
     * Calibrate sensor
     */
    void calibrate(float outputVolt) {
        outputVoltage = outputVolt;
        Serial.println("[UVSensor] Sensor calibrated");
    }
    
    float getVoltage() { return voltage; }
    float getUVIndex() { return uvIndex; }
};

#endif // UV_SENSOR_H

