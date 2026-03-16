/**
 * DatabaseManager - SQLite Database for Sensor Data
 * Stores sensor readings in SQLite database on SD card
 */

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <Arduino.h>
#include <sqlite3.h>
#include <FS.h>
#include <SD.h>
#include "../include/config.h"

// Forward declaration
struct SensorData;

class DatabaseManager {
private:
    sqlite3* db = NULL;
    bool initialized = false;
    const char* dbPath = "/sd/sensors.db";
    int lastErrorCode = SQLITE_OK;
    char lastErrorMsg[256] = {0};
    unsigned long lastLogTime = 0;
    const unsigned long logInterval = 30000;  // Log every 30 seconds
    
public:
    DatabaseManager() {}
    
    /**
     * Initialize database and create tables
     */
    bool begin() {
        // Check if SD card is available
        if (!SD.begin(SD_CS_PIN)) {
            Serial.println("[Database] SD card initialization failed!");
            return false;
        }
        
        // Open or create database
        lastErrorCode = sqlite3_open(dbPath, &db);
        
        if (lastErrorCode != SQLITE_OK) {
            snprintf(lastErrorMsg, sizeof(lastErrorMsg), 
                    "Cannot open database: %s", sqlite3_errmsg(db));
            Serial.println(lastErrorMsg);
            return false;
        }
        
        Serial.println("[Database] SQLite3 database opened successfully");
        
        // Create tables
        if (!createTables()) {
            Serial.println("[Database] Failed to create tables");
            return false;
        }
        
        initialized = true;
        Serial.println("[Database] Database initialized and ready");
        return true;
    }
    
    /**
     * Log sensor data to database
     */
    bool logSensorData(const SensorData& data) {
        if (!initialized) {
            Serial.println("[Database] Database not initialized");
            return false;
        }
        
        // Check if enough time has passed since last log
        if (millis() - lastLogTime < logInterval) {
            return true; // Skip logging, not enough time passed
        }
        
        sqlite3_stmt* stmt = NULL;
        const char* sql = "INSERT INTO sensor_readings ("
            "timestamp, temperature, humidity, pressure, soilMoisture, lightLevel, "
            "gasLevel, motionDetected, speed, waterLevel, uvIndex, rainIntensity, dustDensity, "
            "cpuTemp, freeHeap) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        
        lastErrorCode = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (lastErrorCode != SQLITE_OK) {
            snprintf(lastErrorMsg, sizeof(lastErrorMsg), 
                    "Prepare failed: %s", sqlite3_errmsg(db));
            Serial.println(lastErrorMsg);
            return false;
        }
        
        // Bind parameters
        sqlite3_bind_int64(stmt, 1, data.timestamp);
        sqlite3_bind_double(stmt, 2, data.temperature);
        sqlite3_bind_double(stmt, 3, data.humidity);
        sqlite3_bind_double(stmt, 4, data.pressure);
        sqlite3_bind_double(stmt, 5, data.soilMoisture);
        sqlite3_bind_double(stmt, 6, data.lightLevel);
        sqlite3_bind_double(stmt, 7, data.gasLevel);
        sqlite3_bind_int(stmt, 8, data.motionDetected ? 1 : 0);
        sqlite3_bind_double(stmt, 9, data.speed);
        sqlite3_bind_double(stmt, 10, data.waterLevel);
        sqlite3_bind_double(stmt, 11, data.uvIndex);
        sqlite3_bind_double(stmt, 12, data.rainIntensity);
        sqlite3_bind_double(stmt, 13, data.dustDensity);
        sqlite3_bind_double(stmt, 14, data.cpuTemp);
        sqlite3_bind_int(stmt, 15, data.freeHeap);
        
        // Execute
        lastErrorCode = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (lastErrorCode != SQLITE_DONE) {
            snprintf(lastErrorMsg, sizeof(lastErrorMsg), 
                    "Insert failed: %s", sqlite3_errmsg(db));
            Serial.println(lastErrorMsg);
            return false;
        }
        
        lastLogTime = millis();
        Serial.println("[Database] Sensor data logged successfully");
        return true;
    }
    
    /**
     * Get recent sensor readings (last N records)
     */
    bool getRecentReadings(int limit, String& jsonOutput) {
        if (!initialized) return false;
        
        sqlite3_stmt* stmt = NULL;
        String sql = "SELECT * FROM sensor_readings ORDER BY timestamp DESC LIMIT " + String(limit);
        
        lastErrorCode = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        if (lastErrorCode != SQLITE_OK) {
            Serial.println("[Database] Query prepare failed");
            return false;
        }
        
        jsonOutput = "[";
        int count = 0;
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (count > 0) jsonOutput += ",";
            
            jsonOutput += "{";
            jsonOutput += "\"timestamp\":" + String((long long)sqlite3_column_int64(stmt, 1)) + ",";
            jsonOutput += "\"temperature\":" + String(sqlite3_column_double(stmt, 2)) + ",";
            jsonOutput += "\"humidity\":" + String(sqlite3_column_double(stmt, 3)) + ",";
            jsonOutput += "\"pressure\":" + String(sqlite3_column_double(stmt, 4)) + ",";
            jsonOutput += "\"soilMoisture\":" + String(sqlite3_column_double(stmt, 5)) + ",";
            jsonOutput += "\"lightLevel\":" + String(sqlite3_column_double(stmt, 6)) + ",";
            jsonOutput += "\"gasLevel\":" + String(sqlite3_column_double(stmt, 7)) + ",";
            jsonOutput += "\"motionDetected\":" + String(sqlite3_column_int(stmt, 8)) + ",";
            jsonOutput += "\"speed\":" + String(sqlite3_column_double(stmt, 9)) + ",";
            jsonOutput += "\"waterLevel\":" + String(sqlite3_column_double(stmt, 10)) + ",";
            jsonOutput += "\"uvIndex\":" + String(sqlite3_column_double(stmt, 11)) + ",";
            jsonOutput += "\"rainIntensity\":" + String(sqlite3_column_double(stmt, 12)) + ",";
            jsonOutput += "\"dustDensity\":" + String(sqlite3_column_double(stmt, 13)) + ",";
            jsonOutput += "\"cpuTemp\":" + String(sqlite3_column_double(stmt, 14)) + ",";
            jsonOutput += "\"freeHeap\":" + String(sqlite3_column_int(stmt, 15));
            jsonOutput += "}";
            
            count++;
        }
        
        jsonOutput += "]";
        sqlite3_finalize(stmt);
        
        Serial.print("[Database] Retrieved ");
        Serial.print(count);
        Serial.println(" records from database");
        return true;
    }
    
    /**
     * Get statistics for a given sensor
     */
    bool getStatistics(const char* sensorName, float& minValue, float& maxValue, float& avgValue) {
        if (!initialized) return false;
        
        sqlite3_stmt* stmt = NULL;
        String col;
        
        // Map sensor names to column names
        if (strcmp(sensorName, "temperature") == 0) col = "temperature";
        else if (strcmp(sensorName, "humidity") == 0) col = "humidity";
        else if (strcmp(sensorName, "pressure") == 0) col = "pressure";
        else {
            Serial.println("[Database] Unknown sensor name");
            return false;
        }
        
        String sql = "SELECT MIN(" + col + "), MAX(" + col + "), AVG(" + col + ") FROM sensor_readings";
        
        lastErrorCode = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        if (lastErrorCode != SQLITE_OK) {
            Serial.println("[Database] Stats query prepare failed");
            return false;
        }
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            minValue = sqlite3_column_double(stmt, 0);
            maxValue = sqlite3_column_double(stmt, 1);
            avgValue = sqlite3_column_double(stmt, 2);
            sqlite3_finalize(stmt);
            return true;
        }
        
        sqlite3_finalize(stmt);
        return false;
    }
    
    /**
     * Get database size in bytes
     */
    long getDatabaseSize() {
        if (!SD.exists(dbPath)) return 0;
        return SD.open(dbPath)->size();
    }
    
    /**
     * Get record count
     */
    int getRecordCount() {
        if (!initialized) return 0;
        
        sqlite3_stmt* stmt = NULL;
        const char* sql = "SELECT COUNT(*) FROM sensor_readings";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
            return 0;
        }
        
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return count;
    }
    
    /**
     * Export data to CSV file
     */
    bool exportToCSV(const char* filename) {
        if (!initialized) return false;
        
        sqlite3_stmt* stmt = NULL;
        const char* sql = "SELECT * FROM sensor_readings ORDER BY timestamp DESC";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
            Serial.println("[Database] Export query prepare failed");
            return false;
        }
        
        File csvFile = SD.open(filename, FILE_WRITE);
        if (!csvFile) {
            Serial.println("[Database] Cannot create CSV file");
            sqlite3_finalize(stmt);
            return false;
        }
        
        // Write header
        csvFile.println("Timestamp,Temperature,Humidity,Pressure,SoilMoisture,LightLevel,"
                       "GasLevel,MotionDetected,Speed,WaterLevel,UVIndex,RainIntensity,"
                       "DustDensity,CPUTemp,FreeHeap");
        
        // Write data
        int count = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            String line = String((long long)sqlite3_column_int64(stmt, 1)) + ",";
            line += String(sqlite3_column_double(stmt, 2)) + ",";
            line += String(sqlite3_column_double(stmt, 3)) + ",";
            line += String(sqlite3_column_double(stmt, 4)) + ",";
            line += String(sqlite3_column_double(stmt, 5)) + ",";
            line += String(sqlite3_column_double(stmt, 6)) + ",";
            line += String(sqlite3_column_double(stmt, 7)) + ",";
            line += String(sqlite3_column_int(stmt, 8)) + ",";
            line += String(sqlite3_column_double(stmt, 9)) + ",";
            line += String(sqlite3_column_double(stmt, 10)) + ",";
            line += String(sqlite3_column_double(stmt, 11)) + ",";
            line += String(sqlite3_column_double(stmt, 12)) + ",";
            line += String(sqlite3_column_double(stmt, 13)) + ",";
            line += String(sqlite3_column_double(stmt, 14)) + ",";
            line += String(sqlite3_column_int(stmt, 15));
            
            csvFile.println(line);
            count++;
        }
        
        csvFile.close();
        sqlite3_finalize(stmt);
        
        Serial.print("[Database] Exported ");
        Serial.print(count);
        Serial.print(" records to ");
        Serial.println(filename);
        
        return true;
    }
    
    /**
     * Close database
     */
    void end() {
        if (db) {
            sqlite3_close(db);
            db = NULL;
            initialized = false;
            Serial.println("[Database] Database closed");
        }
    }
    
    /**
     * Get last error message
     */
    const char* getLastError() {
        return lastErrorMsg;
    }

private:
    /**
     * Create database tables
     */
    bool createTables() {
        const char* createTableSQL = R"(
            CREATE TABLE IF NOT EXISTS sensor_readings (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp INTEGER NOT NULL,
                temperature REAL,
                humidity REAL,
                pressure REAL,
                soilMoisture REAL,
                lightLevel REAL,
                gasLevel REAL,
                motionDetected INTEGER,
                speed REAL,
                waterLevel REAL,
                uvIndex REAL,
                rainIntensity REAL,
                dustDensity REAL,
                cpuTemp REAL,
                freeHeap INTEGER
            );
            
            CREATE TABLE IF NOT EXISTS system_events (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp INTEGER NOT NULL,
                eventType TEXT,
                description TEXT,
                severity TEXT
            );
            
            CREATE INDEX IF NOT EXISTS idx_readings_timestamp 
            ON sensor_readings(timestamp DESC);
        )";
        
        char* errMsg = 0;
        int rc = sqlite3_exec(db, createTableSQL, NULL, 0, &errMsg);
        
        if (rc != SQLITE_OK) {
            snprintf(lastErrorMsg, sizeof(lastErrorMsg), 
                    "SQL error: %s", errMsg);
            sqlite3_free(errMsg);
            return false;
        }
        
        return true;
    }
};

#endif // DATABASE_MANAGER_H
