/**
 * TempHumidity - DHT22 Temperature & Humidity Sensor
 * Measures ambient temperature and relative humidity
 */

#ifndef TEMP_HUMIDITY_H
#define TEMP_HUMIDITY_H

#include <Arduino.h>
#include <DHT.h>
#include "../include/config.h"

struct TempHumidityData {
    float temperature;
    float humidity;
    bool valid;
};

class TempHumidity {
private:
    DHT dht;
    unsigned long lastReadTime = 0;
    const unsigned long readInterval = DHT_READING_INTERVAL;
    
public:
    TempHumidity() : dht(DHT22_PIN, DHT_TYPE) {}
    
    void begin() {
        dht.begin();
        Serial.println("[TempHumidity] DHT22 sensor initialized");
    }
    
    TempHumidityData read() {
        TempHumidityData data;
        data.valid = false;
        
        // Check if enough time has passed
        if (millis() - lastReadTime < readInterval) {
            return data;
        }
        
        // Read values
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        
        // Check if reads are valid
        if (!isnan(h) && !isnan(t)) {
            data.humidity = h;
            data.temperature = t;
            data.valid = true;
            lastReadTime = millis();
        } else {
            Serial.println("[TempHumidity] Failed to read from DHT sensor!");
        }
        
        return data;
    }
    
    float getTemperature() {
        TempHumidityData data = read();
        return data.valid ? data.temperature : NAN;
    }
    
    float getHumidity() {
        TempHumidityData data = read();
        return data.valid ? data.humidity : NAN;
    }
};

#endif // TEMP_HUMIDITY_H

