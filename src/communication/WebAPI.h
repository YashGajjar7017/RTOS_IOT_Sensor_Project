/**
 * WebAPI - REST API endpoints for data access and export
 * Provides endpoints for database queries, statistics, and CSV export
 */

#ifndef WEB_API_H
#define WEB_API_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../include/config.h"

// Forward declarations
class DatabaseManager;

class WebAPI {
private:
    DatabaseManager* db;
    
public:
    WebAPI(DatabaseManager* database) : db(database) {}
    
    /**
     * Get database statistics as JSON
     */
    String getStatisticsJSON() {
        DynamicJsonDocument doc(512);
        
        if (!db) {
            doc["error"] = "Database not available";
            String response;
            serializeJson(doc, response);
            return response;
        }
        
        doc["recordCount"] = db->getRecordCount();
        doc["databaseSizeKB"] = db->getDatabaseSize() / 1024;
        
        float min_temp, max_temp, avg_temp;
        if (db->getStatistics("temperature", min_temp, max_temp, avg_temp)) {
            JsonObject tempStats = doc.createNestedObject("temperature");
            tempStats["min"] = roundTo(min_temp, 2);
            tempStats["max"] = roundTo(max_temp, 2);
            tempStats["avg"] = roundTo(avg_temp, 2);
        }
        
        float min_humidity, max_humidity, avg_humidity;
        if (db->getStatistics("humidity", min_humidity, max_humidity, avg_humidity)) {
            JsonObject humStats = doc.createNestedObject("humidity");
            humStats["min"] = roundTo(min_humidity, 2);
            humStats["max"] = roundTo(max_humidity, 2);
            humStats["avg"] = roundTo(avg_humidity, 2);
        }
        
        String response;
        serializeJson(doc, response);
        return response;
    }
    
    /**
     * Get recent sensor readings as JSON
     */
    String getRecentReadingsJSON(int limit = 100) {
        if (!db) {
            return "{\"error\":\"Database not available\"}";
        }
        
        String jsonOutput;
        if (db->getRecentReadings(limit, jsonOutput)) {
            return jsonOutput;
        }
        return "{\"error\":\"Failed to retrieve readings\"}";
    }
    
    /**
     * Get export status
     */
    String getExportStatusJSON() {
        DynamicJsonDocument doc(256);
        doc["status"] = "ready";
        doc["format"] = "CSV";
        doc["availableFields"] = 15;
        
        String response;
        serializeJson(doc, response);
        return response;
    }

private:
    /**
     * Round floating point to N decimal places
     */
    float roundTo(float value, int decimals) {
        if (isnan(value) || isinf(value)) return 0;
        float multiplier = pow(10.0, decimals);
        return round(value * multiplier) / multiplier;
    }
};

#endif // WEB_API_H
