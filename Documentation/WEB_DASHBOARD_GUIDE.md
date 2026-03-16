# Real-Time Web Dashboard & Database Features

## Overview

This document describes the new real-time web dashboard and SQLite database features added to the IoT RTOS Sensor Network.

## Features Implemented

### 🌐 Real-Time Web Dashboard

**Location:** `src/communication/WebServer.h`

#### Key Features:
- **Modern, Responsive Web UI** - Beautiful gradient-based design with smooth animations
- **Live Sensor Data Display** - 12 sensor cards showing real-time measurements
- **WebSocket Support** - Real-time data updates at 1-second intervals
- **System Monitoring** - CPU temperature, free RAM, uptime tracking
- **Visual Indicators** - Progress bars, status dots, color-coded alerts
- **Mobile Responsive** - Fully responsive design for all screen sizes

#### Sensor Display Cards:
1. **Temperature** - °C with visual bar
2. **Humidity** - Percentage with visual bar
3. **Pressure** - hPa measurement
4. **Soil Moisture** - Percentage with visual bar
5. **Light Level** - Lux measurement with bar
6. **Air Quality (Gas)** - PPM with progress bar
7. **Motion Detection** - Real-time detection status
8. **Speed** - km/h with progress bar
9. **Water Level** - cm with visual bar
10. **UV Index** - Current UV level
11. **Rain Intensity** - mm with progress bar
12. **Dust Density** - µg/m³ with progress bar

#### System Information Panel:
- CPU Temperature monitoring
- Free RAM display (updated in real-time)
- Last update timestamp
- System uptime counter

#### Access Points:
```
HTTP Server: http://<ESP32_IP>:80
  - GET  /                    - Serve dashboard HTML
  - GET  /api/sensors         - Plain text sensor data
  - GET  /api/sensors/json    - JSON sensor data
  - GET  /api/system          - System status JSON

WebSocket Server: ws://<ESP32_IP>:81
  - Real-time sensor data streaming
  - Automatic reconnection
  - ~2KB per update message
```

### 💾 SQLite Database

**Location:** `src/storage/DatabaseManager.h`

#### Key Features:
- **Persistent Data Storage** - SQLite database on SD card
- **Automatic Logging** - Logs all 15 sensor readings every 30 seconds
- **Historical Data** - Query and analyze trends over time
- **Statistics** - Min/max/average values for sensors
- **CSV Export** - Export database to CSV format
- **Indexed Queries** - Fast timestamp-based lookups

#### Database Schema:

**sensor_readings table:**
```
id (PRIMARY KEY)
timestamp (DATETIME, INDEXED)
temperature (REAL)
humidity (REAL)
pressure (REAL)
soilMoisture (REAL)
lightLevel (REAL)
gasLevel (REAL)
motionDetected (INTEGER)
speed (REAL)
waterLevel (REAL)
uvIndex (REAL)
rainIntensity (REAL)
dustDensity (REAL)
cpuTemp (REAL)
freeHeap (INTEGER)
```

**system_events table:**
```
id (PRIMARY KEY)
timestamp (DATETIME)
eventType (TEXT)
description (TEXT)
severity (TEXT)
```

#### Functions:

```cpp
// Initialize database
bool begin();

// Log sensor data (called every 30s)
bool logSensorData(const SensorData& data);

// Retrieve data
bool getRecentReadings(int limit, String& jsonOutput);
bool getStatistics(const char* sensorName, float& min, float& max, float& avg);

// Export functionality
bool exportToCSV(const char* filename);
long getDatabaseSize();
int getRecordCount();

// Cleanup
void end();
```

### 📊 REST API & WebAPI

**Location:** `src/communication/WebAPI.h`

#### Available Endpoints:

```
GET /api/sensors/json
Response: Current sensor readings as JSON
Example: {"temperature": 25.5, "humidity": 60, ...}

GET /api/system
Response: System status and resources
Example: {"uptime": 3600, "freeRAM": 65536, ...}

GET /api/stats
Response: Database statistics
Example: {"recordCount": 100, "databaseSizeKB": 256, ...}

GET /api/readings?limit=50
Response: Recent sensor readings
Example: [{"timestamp": 1234567890, "temperature": 25.5, ...}, ...]
```

### 🎨 Enhanced OLED Display

**Location:** `src/display/DisplayEnhancements.h`

Additional display modes for the OLED screen:

1. **Web Server Info Screen** - Shows dashboard URL and API endpoints
2. **Database Status Screen** - Displays record count and database size
3. **Sensor Grid View** - Compact view of all sensor values
4. **System Overview** - Critical values in one view
5. **Loading Bar** - Initialization progress indicator

## Hardware Requirements

### Web Server:
- ESP32 with WiFi
- RAM: ~100KB for web server buffers
- Network connection to host WiFi network

### Database:
- SD card with at least 10MB free space
- SD card pins configured in config.h
- Power supply for continuous operation

## Software Requirements

### New Libraries Added to platformio.ini:

```ini
me-no-dev/ESP Async WebServer@^1.4.1
links2004/WebSockets@^2.4.1
ArduinoJson@^6.21.3
siara.cc/esp32_arduino_sqlite3_lib@^1.3.0
```

## Configuration

### WiFi Setup
Edit `include/config.h`:
```cpp
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"
```

### Web Server Ports
```cpp
#define WEB_SERVER_PORT 80
#define WEBSOCKET_PORT 81
#define WEBSOCKET_UPDATE_INTERVAL 1000  // ms
```

## RTOS Task Architecture

### New Tasks Added:

1. **webServerTask** - Core 1, Medium Priority
   - Initializes HTTP server and WebSocket
   - Waits for WiFi connection
   - Handles WebSocket updates
   - Stack: STACK_SIZE_LARGE

2. **dbLogTask** - Core 1, Low Priority
   - Initializes SQLite database
   - Logs sensor data every 30 seconds
   - Monitors database statistics
   - Stack: STACK_SIZE_LARGE

## Data Flow

```
┌─────────────────┐
│ Sensors         │
└────────┬────────┘
         │ (2s)
    ┌────▼─────────────┐
    │ Sensor Task      │
    │ (Core 0)         │
    └────┬───┬───┬─────┘
         │   │   │
    ┌────▼─┐ │   │
    │OLED  │ │   │
    │Disp  │ │   │
    └──────┘ │   │
             │   │
    ┌────────▼┐  │
    │   DB    │  │
    │  Log    │  │
    │(30s)    │  │
    └────────┘   │
                 │
    ┌────────────▼─────────┐
    │ SharedData (Mutex)   │
    ├──────────────────────┤
    │ • Temperature        │
    │ • Humidity           │
    │ • All 12+ sensors    │
    │ • System status      │
    └────────────┬─────────┘
                 │
    ┌────────────▼──────────┐
    │ Web Server (Core 1)   │
    ├──────────────────────┤
    │ HTTP /api/sensors    │
    │ HTTP /api/system     │
    │ WebSocket updates    │
    └──────────────────────┘
```

## Performance Specifications

### Memory Usage:
- Web Server: ~100KB for buffers
- Database: ~50KB for driver
- Total overhead: ~150KB

### Network Bandwidth:
- WebSocket update: ~2KB per second
- HTTP API request: ~1-2KB per request
- Database: ~500 bytes per record

### Storage:
- Each database record: ~500 bytes
- 30-second logging interval = ~100KB/hour
- 1000 hour history = ~100MB (requires SD card)

## Usage Examples

### Access the Dashboard:
```
1. Power on ESP32
2. Connect to WiFi network
3. Open browser: http://<ESP32_IP>
4. View real-time sensor data in modern UI
```

### Query Sensor Data via API:
```bash
# Get current sensor readings
curl http://<ESP32_IP>/api/sensors/json

# Get system information
curl http://<ESP32_IP>/api/system

# Get database statistics
curl http://<ESP32_IP>/api/stats
```

### JavaScript WebSocket Connection:
```javascript
const ws = new WebSocket('ws://<ESP32_IP>:81/');

ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log('Temperature:', data.temperature);
    console.log('Humidity:', data.humidity);
    // Update UI with real-time data
};
```

### Export Database:
```cpp
// In dbLogTask, you can call:
databaseManager.exportToCSV("/sd/sensor_export.csv");
```

## Troubleshooting

### Web Dashboard Not Loading:
1. Verify WiFi connection: Check LED indicators
2. Get ESP32 IP address from serial monitor
3. Ensure port 80 is not blocked by firewall
4. Check that WebServer.h is included in main.cpp

### Database Not Logging:
1. Verify SD card is inserted and mounted
2. Check SD card has free space (>10MB)
3. Verify SD card pins in config.h
4. Monitor serial output for database errors

### WebSocket Connection Fails:
1. Verify both port 80 and 81 are accessible
2. Check WiFi signal strength
3. Monitor browser console for connection errors
4. Ensure WebSocket server is properly initialized

### High Memory Usage:
1. Reduce WebSocket update frequency
2. Limit number of concurrent WebSocket clients
3. Reduce database query limits
4. Monitor with /api/system endpoint

## Advanced Features (Extensible)

The system can be extended with:
- **Data Export API** - Download historical data as CSV/JSON
- **Alerting** - Trigger alerts when thresholds exceeded
- **Graphing** - Chart historical data with Chart.js
- **Mobile App** - Native app consuming WebSocket/API
- **Cloud Integration** - Send data to cloud database
- **Authentication** - Secure API access with tokens

## Support & Maintenance

### Regular Maintenance:
- Monitor database size monthly
- Export and archive old data quarterly
- Clear log files when storage exceeds limits
- Update WiFi credentials as needed

### Monitoring Commands:
```cpp
// Check database health
int recordCount = databaseManager.getRecordCount();
long sizeKB = databaseManager.getDatabaseSize() / 1024;

// Get system status
String statsJSON = webAPI.getStatisticsJSON();

// Export for backup
databaseManager.exportToCSV("/sd/backup.csv");
```

## Conclusion

The IoT RTOS Sensor Network now includes:
✅ Modern, real-time web dashboard
✅ Professional SQLite database for historical data
✅ REST API for programmatic access
✅ WebSocket for live data streaming
✅ Mobile-responsive design
✅ Multiple display views on OLED
✅ Comprehensive data logging and export

All with proper RTOS task management and thread-safe data sharing using mutexes!
