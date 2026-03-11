/**
 * SpeedSensor - Implementation
 */

#include "SpeedSensor.h"

// Static instance initialization
SpeedSensor* SpeedSensor::_instance = nullptr;

// Constructor
SpeedSensor::SpeedSensor() : sensorPin(SPEED_SENSOR_PIN),
               ledPin(SPEED_LED_PIN),
               pulseCount(0),
               currentSpeed(0),
               lastPulseTime(0) {
    _instance = this;
}

void SpeedSensor::begin() {
    pinMode(sensorPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    
    // Attach interrupt
    attachInterrupt(digitalPinToInterrupt(sensorPin), pulseISR, FALLING);
    
    Serial.println("[SpeedSensor] Speed sensor initialized");
}

/**
 * Get current speed in km/h
 */
float SpeedSensor::getSpeed() {
    // Update speed calculation every second
    if (millis() - lastSpeedUpdate >= speedUpdateInterval) {
        calculateSpeed();
        lastSpeedUpdate = millis();
    }
    
    return currentSpeed;
}

/**
 * Calculate speed from pulse count
 */
void SpeedSensor::calculateSpeed() {
    // Disable interrupts temporarily
    noInterrupts();
    int pulses = pulseCount;
    pulseCount = 0;
    interrupts();
    
    // Calculate time since last update
    unsigned long timeDiff = millis() - lastSpeedUpdate;
    if (timeDiff == 0) timeDiff = 1;
    
    // Check for timeout (no pulses = stopped)
    if (millis() - lastPulseTime > SPEED_TIMEOUT) {
        currentSpeed = 0;
        return;
    }
    
    // Calculate RPM
    // pulses * (60000 / timeDiff) = pulses per minute
    float rpm = (pulses * 60000.0) / timeDiff;
    
    // Convert RPM to km/h
    // speed = RPM * circumference * 60 / 1000
    currentSpeed = (rpm * SPEED_WHEEL_CIRCUMFERENCE * 60) / 1000;
    
    // Constrain to reasonable values
    currentSpeed = constrain(currentSpeed, 0, 100);
}

/**
 * Get total pulse count
 */
int SpeedSensor::getPulseCount() {
    return pulseCount;
}

/**
 * Reset pulse counter
 */
void SpeedSensor::reset() {
    noInterrupts();
    pulseCount = 0;
    interrupts();
    currentSpeed = 0;
}

/**
 * Handle pulse interrupt
 */
void SpeedSensor::handlePulse() {
    pulseCount++;
    lastPulseTime = millis();
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(50);
    digitalWrite(ledPin, LOW);
}

// ISR Handler
void IRAM_ATTR pulseISR() {
    if (SpeedSensor::_instance) {
        SpeedSensor::_instance->handlePulse();
    }
}

