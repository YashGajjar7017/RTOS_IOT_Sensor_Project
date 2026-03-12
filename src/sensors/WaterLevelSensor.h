/**
 * WaterLevelSensor - Ultrasonic Water Level Sensor
 * Measures distance to water surface to calculate water level
 * Uses HC-SR04 ultrasonic sensor
 */

#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H

#include <Arduino.h>
#include "../include/config.h"

class WaterLevelSensor {
private:
    int trigPin;
    int echoPin;
    float distance;
    float waterLevel;
    
    // Configuration
    float tankHeight;      // Total height of tank in cm
    float emptyDistance;  // Distance when tank is empty
    float fullDistance;   // Distance when tank is full
    
public:
    WaterLevelSensor() : trigPin(WATER_LEVEL_TRIG_PIN),
                         echoPin(WATER_LEVEL_ECHO_PIN),
                         distance(0),
                         waterLevel(0),
                         tankHeight(100.0),
                         emptyDistance(100.0),
                         fullDistance(10.0) {}
    
    void begin() {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
        Serial.println("[WaterLevel] Ultrasonic water level sensor initialized");
    }
    
    /**
     * Set tank configuration
     */
    void setTankConfig(float height, float empty, float full) {
        tankHeight = height;
        emptyDistance = empty;
        fullDistance = full;
    }
    
    /**
     * Read distance from sensor (in cm)
     */
    float readDistance() {
        // Send trigger pulse
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        
        // Read echo pulse
        long duration = pulseIn(echoPin, HIGH, 30000);  // 30ms timeout
        
        // Calculate distance (speed of sound: 343 m/s)
        // Distance = duration * 0.0343 / 2
        if (duration > 0) {
            distance = duration * 0.0343 / 2.0;
        }
        
        return distance;
    }
    
    /**
     * Read water level as percentage (0-100%)
     */
    float read() {
        readDistance();
        
        // Calculate percentage based on distance
        // Full tank = close distance, Empty tank = far distance
        if (distance >= emptyDistance) {
            waterLevel = 0;
        } else if (distance <= fullDistance) {
            waterLevel = 100;
        } else {
            waterLevel = map(distance, emptyDistance, fullDistance, 0, 100);
        }
        
        waterLevel = constrain(waterLevel, 0, 100);
        return waterLevel;
    }
    
    /**
     * Get water level in liters (requires tank dimensions)
     */
    float getVolumeLiters(float tankCapacityLiters) {
        return (waterLevel / 100.0) * tankCapacityLiters;
    }
    
    /**
     * Get water level description
     */
    const char* getLevel() {
        if (waterLevel < 10) return "EMPTY";
        else if (waterLevel < 25) return "LOW";
        else if (waterLevel < 50) return "MEDIUM";
        else if (waterLevel < 75) return "HIGH";
        else return "FULL";
    }
    
    /**
     * Check if water level is critical (below threshold)
     */
    bool isCritical(float threshold = 20.0) {
        return waterLevel < threshold;
    }
    
    float getDistance() { return distance; }
    float getWaterLevel() { return waterLevel; }
};

#endif // WATER_LEVEL_SENSOR_H

