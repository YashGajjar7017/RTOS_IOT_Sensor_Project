/**
 * SpeedSensor - Hall Effect / IR Speed Sensor
 * Measures rotational speed and converts to velocity
 */

#ifndef SPEED_SENSOR_H
#define SPEED_SENSOR_H

#include <Arduino.h>
#include "../include/config.h"

// Forward declaration for ISR
class SpeedSensor;

class SpeedSensor {
private:
    int sensorPin;
    int ledPin;
    
    // Pulse counting
    volatile int pulseCount;
    unsigned long lastPulseTime;
    unsigned long pulseInterval;
    
    // Speed calculation
    float currentSpeed;  // km/h
    unsigned long lastSpeedUpdate;
    const unsigned long speedUpdateInterval = 1000;  // 1 second
    
    // Static instance for ISR access
    static SpeedSensor* _instance;
    
public:
    SpeedSensor();
    
    void begin();
    
    /**
     * Get current speed in km/h
     */
    float getSpeed();
    
    /**
     * Calculate speed from pulse count
     */
    void calculateSpeed();
    
    /**
     * Get total pulse count
     */
    int getPulseCount();
    
    /**
     * Reset pulse counter
     */
    void reset();
    
    /**
     * Handle pulse interrupt
     */
    void handlePulse();
    
    // ISR Handler
    friend void IRAM_ATTR pulseISR();
};

// ISR Handler
void IRAM_ATTR pulseISR();

#endif // SPEED_SENSOR_H

