/**
 * MotionSensor - PIR (Passive Infrared) Motion Detection
 * Detects human/movement within sensor range
 */

#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <Arduino.h>
#include "../include/config.h"

class MotionSensor {
private:
    int sensorPin;
    bool lastState;
    bool currentState;
    unsigned long lastMotionTime;
    unsigned long lastDebounceTime;
    bool warmingUp;
    unsigned long warmupStart;
    
public:
    MotionSensor() : sensorPin(PIR_MOTION_PIN), 
                    lastState(LOW), 
                    currentState(LOW),
                    warmingUp(true),
                    warmupStart(0) {}
    
    void begin() {
        pinMode(sensorPin, INPUT);
        warmupStart = millis();
        Serial.println("[MotionSensor] PIR motion sensor initializing...");
        Serial.println("[MotionSensor] Please wait 60 seconds for sensor warmup...");
    }
    
    /**
     * Check if motion is detected
     * Includes debouncing to prevent false triggers
     */
    bool isDetected() {
        // Check warmup period
        if (warmingUp) {
            if (millis() - warmupStart > PIR_WARMUP_TIME) {
                warmingUp = false;
                Serial.println("[MotionSensor] Sensor warmup complete!");
            }
            return false;
        }
        
        // Read sensor
        bool reading = digitalRead(sensorPin);
        
        // Debounce check
        if (reading != lastState) {
            lastDebounceTime = millis();
        }
        
        if ((millis() - lastDebounceTime) > PIR_DEBOUNCE_TIME) {
            if (reading != currentState) {
                currentState = reading;
                
                if (currentState == HIGH) {
                    lastMotionTime = millis();
                    Serial.println("[MotionSensor] Motion detected!");
                }
            }
        }
        
        lastState = reading;
        
        // Motion stays active for debounce time after detection
        return currentState == HIGH || 
               (millis() - lastMotionTime) < PIR_DEBOUNCE_TIME;
    }
    
    /**
     * Get time since last motion detected (milliseconds)
     */
    unsigned long getTimeSinceLastMotion() {
        return millis() - lastMotionTime;
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
        long remaining = PIR_WARMUP_TIME - (millis() - warmupStart);
        return remaining > 0 ? remaining : 0;
    }
};

#endif // MOTION_SENSOR_H

