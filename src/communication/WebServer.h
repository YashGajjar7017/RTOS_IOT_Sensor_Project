/**
 * WebServer - Async Web Server with Real-time WebSocket
 * Serves modern dashboard UI and REST API for sensor data
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "../include/config.h"

// Forward declarations
struct SensorData;

class WebServer {
private:
    AsyncWebServer* server;
    WebSocketsServer webSocket;
    bool initialized;
    uint32_t webSocketConnectionId = 255;  // WebSocket client ID
    unsigned long lastWebSocketUpdate = 0;
    const unsigned long wsUpdateInterval = 1000;  // Send data every 1 second
    
    // Callback function pointer for sensor data
    std::function<SensorData()> getSensorDataCallback;
    
public:
    WebServer() : server(nullptr), webSocket(81), initialized(false) {}
    
    /**
     * Initialize web server and WebSocket
     */
    void begin(std::function<SensorData()> callback) {
        if (!WiFi.isConnected()) {
            Serial.println("[WebServer] WiFi not connected, skipping initialization");
            return;
        }
        
        getSensorDataCallback = callback;
        
        server = new AsyncWebServer(80);
        
        // Setup CORS headers
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
        
        // Serve the dashboard HTML
        server->on("/", HTTP_GET, std::bind(&WebServer::handleRoot, this, std::placeholders::_1));
        
        // REST API endpoints
        server->on("/api/sensors", HTTP_GET, std::bind(&WebServer::handleSensorData, this, std::placeholders::_1));
        server->on("/api/sensors/json", HTTP_GET, std::bind(&WebServer::handleSensorJSON, this, std::placeholders::_1));
        server->on("/api/system", HTTP_GET, std::bind(&WebServer::handleSystemStatus, this, std::placeholders::_1));
        
        // WebSocket
        webSocket.begin();
        webSocket.onEvent(std::bind(&WebServer::webSocketEvent, this, std::placeholders::_1, 
                                   std::placeholders::_2, std::placeholders::_3, 
                                   std::placeholders::_4, std::placeholders::_5));
        
        server->begin();
        initialized = true;
        
        Serial.println("[WebServer] Web server started at http://" + WiFi.localIP().toString());
        Serial.println("[WebServer] Dashboard available at http://" + WiFi.localIP().toString());
        Serial.println("[WebServer] WebSocket port: 81");
    }
    
    /**
     * Handle WebSocket connections and send periodic updates
     */
    void update() {
        if (!initialized) return;
        
        webSocket.loop();
        
        // Send sensor data via WebSocket every second
        if (millis() - lastWebSocketUpdate >= wsUpdateInterval) {
            if (webSocket.connectedClients() > 0 && getSensorDataCallback) {
                SensorData data = getSensorDataCallback();
                sendSensorDataViaWebSocket(data);
            }
            lastWebSocketUpdate = millis();
        }
    }
    
    /**
     * Send sensor data to all connected WebSocket clients
     */
    void sendSensorDataViaWebSocket(const SensorData& data) {
        DynamicJsonDocument doc(1024);
        
        doc["temperature"] = data.temperature;
        doc["humidity"] = data.humidity;
        doc["pressure"] = data.pressure;
        doc["soilMoisture"] = data.soilMoisture;
        doc["lightLevel"] = data.lightLevel;
        doc["gasLevel"] = data.gasLevel;
        doc["motionDetected"] = data.motionDetected;
        doc["speed"] = data.speed;
        doc["waterLevel"] = data.waterLevel;
        doc["uvIndex"] = data.uvIndex;
        doc["rainIntensity"] = data.rainIntensity;
        doc["dustDensity"] = data.dustDensity;
        doc["cpuTemp"] = data.cpuTemp;
        doc["freeHeap"] = data.freeHeap;
        doc["timestamp"] = data.timestamp;
        
        String json;
        serializeJson(doc, json);
        
        webSocket.broadcastTXT(json);
    }
    
private:
    /**
     * Handle root request - serve dashboard HTML
     */
    void handleRoot(AsyncWebServerRequest* request) {
        String html = getDashboardHTML();
        request->send(200, "text/html; charset=utf-8", html);
    }
    
    /**
     * Handle REST API - sensor data as JSON
     */
    void handleSensorJSON(AsyncWebServerRequest* request) {
        if (!getSensorDataCallback) {
            request->send(500, "application/json", "{\"error\":\"Sensor data unavailable\"}");
            return;
        }
        
        SensorData data = getSensorDataCallback();
        DynamicJsonDocument doc(1024);
        
        doc["temperature"] = roundTo(data.temperature, 2);
        doc["humidity"] = roundTo(data.humidity, 2);
        doc["pressure"] = roundTo(data.pressure, 2);
        doc["soilMoisture"] = roundTo(data.soilMoisture, 2);
        doc["lightLevel"] = roundTo(data.lightLevel, 2);
        doc["gasLevel"] = roundTo(data.gasLevel, 2);
        doc["motionDetected"] = data.motionDetected;
        doc["speed"] = roundTo(data.speed, 2);
        doc["waterLevel"] = roundTo(data.waterLevel, 2);
        doc["uvIndex"] = roundTo(data.uvIndex, 2);
        doc["rainIntensity"] = roundTo(data.rainIntensity, 2);
        doc["dustDensity"] = roundTo(data.dustDensity, 2);
        doc["cpuTemp"] = roundTo(data.cpuTemp, 2);
        doc["freeHeap"] = data.freeHeap;
        doc["timestamp"] = data.timestamp;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    }
    
    /**
     * Handle sensor data text response
     */
    void handleSensorData(AsyncWebServerRequest* request) {
        String response = "Sensor Data\n";
        request->send(200, "text/plain", response);
    }
    
    /**
     * Handle system status
     */
    void handleSystemStatus(AsyncWebServerRequest* request) {
        DynamicJsonDocument doc(512);
        doc["uptime"] = millis();
        doc["freeRAM"] = ESP.getFreeHeap();
        doc["totalRAM"] = ESP.getHeapSize();
        doc["cpuFreq"] = ESP.getCpuFreqMHz();
        doc["chipID"] = ESP.getEfuseMac();
        doc["firmwareVersion"] = "1.0.0";
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    }
    
    /**
     * WebSocket event handler
     */
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        switch(type) {
            case WStype_CONNECTED:
                Serial.printf("[WebSocket] Client %u connected\n", num);
                webSocketConnectionId = num;
                break;
            case WStype_DISCONNECTED:
                Serial.printf("[WebSocket] Client %u disconnected\n", num);
                webSocketConnectionId = 255;
                break;
            case WStype_TEXT:
                handleWebSocketMessage(num, payload, length);
                break;
        }
    }
    
    /**
     * Handle WebSocket message
     */
    void handleWebSocketMessage(uint8_t clientNum, uint8_t* payload, size_t length) {
        Serial.printf("[WebSocket] Message from client %u: %s\n", clientNum, (char*)payload);
    }
    
    /**
     * Round floating point to N decimal places
     */
    float roundTo(float value, int decimals) {
        float multiplier = pow(10.0, decimals);
        return round(value * multiplier) / multiplier;
    }
    
    /**
     * Get the dashboard HTML/CSS/JavaScript
     */
    String getDashboardHTML() {
        return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IoT Sensor Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
            color: #333;
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
        }
        
        header {
            text-align: center;
            color: white;
            margin-bottom: 30px;
        }
        
        header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        
        .status-bar {
            display: flex;
            justify-content: center;
            gap: 30px;
            margin-bottom: 30px;
            color: white;
            font-weight: bold;
        }
        
        .status-item {
            display: flex;
            align-items: center;
            gap: 10px;
            background: rgba(255,255,255,0.2);
            padding: 10px 20px;
            border-radius: 50px;
            backdrop-filter: blur(10px);
        }
        
        .status-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background: #4ade80;
            animation: pulse 2s infinite;
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .card {
            background: white;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.15);
            transition: transform 0.3s ease, box-shadow 0.3s ease;
            position: relative;
            overflow: hidden;
        }
        
        .card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 4px;
            background: linear-gradient(90deg, #667eea, #764ba2);
        }
        
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 15px 50px rgba(0,0,0,0.25);
        }
        
        .sensor-title {
            font-size: 0.9em;
            color: #666;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 10px;
            font-weight: 600;
        }
        
        .sensor-value {
            font-size: 2.5em;
            font-weight: bold;
            color: #667eea;
            margin: 15px 0;
        }
        
        .sensor-unit {
            font-size: 0.8em;
            color: #999;
            margin-left: 5px;
        }
        
        .sensor-bar {
            background: #f0f0f0;
            height: 6px;
            border-radius: 3px;
            margin-top: 15px;
            overflow: hidden;
        }
        
        .sensor-bar-fill {
            height: 100%;
            background: linear-gradient(90deg, #4ade80, #667eea);
            transition: width 0.3s ease;
            border-radius: 3px;
        }
        
        .icon {
            width: 50px;
            height: 50px;
            background: linear-gradient(135deg, #667eea, #764ba2);
            border-radius: 10px;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-size: 1.5em;
            margin-bottom: 10px;
        }
        
        .motion-detected {
            color: #ef4444;
            font-weight: bold;
        }
        
        .motion-clear {
            color: #4ade80;
        }
        
        .system-info {
            background: white;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.15);
        }
        
        .system-info h3 {
            color: #667eea;
            margin-bottom: 15px;
        }
        
        .system-stat {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid #eee;
        }
        
        .system-stat:last-child {
            border-bottom: none;
        }
        
        .system-label {
            font-weight: 600;
            color: #666;
        }
        
        .system-value {
            color: #667eea;
            font-weight: bold;
        }
        
        .legend {
            display: flex;
            justify-content: center;
            gap: 30px;
            margin-top: 20px;
            color: white;
            font-size: 0.9em;
            flex-wrap: wrap;
        }
        
        .legend-item {
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .legend-box {
            width: 20px;
            height: 20px;
            border-radius: 3px;
        }
        
        .legend-critical { background: #ef4444; }
        .legend-warning { background: #f97316; }
        .legend-normal { background: #4ade80; }
        
        @media (max-width: 768px) {
            header h1 { font-size: 1.8em; }
            .grid { grid-template-columns: 1fr; }
            .status-bar { flex-direction: column; gap: 10px; }
            .sensor-value { font-size: 1.8em; }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🌍 IoT Sensor Hub</h1>
            <p>Real-time Multi-Sensor Monitoring System</p>
        </header>
        
        <div class="status-bar">
            <div class="status-item">
                <div class="status-dot"></div>
                <span>System Online</span>
            </div>
            <div class="status-item">
                <span>⏱️ Uptime: <span id="uptime">0s</span></span>
            </div>
            <div class="status-item">
                <span>🕐 <span id="currentTime">--:--:--</span></span>
            </div>
        </div>
        
        <div class="grid">
            <!-- Temperature & Humidity -->
            <div class="card">
                <div class="icon">🌡️</div>
                <div class="sensor-title">Temperature</div>
                <div class="sensor-value"><span id="temp">--</span><span class="sensor-unit">°C</span></div>
                <div class="sensor-bar">
                    <div class="sensor-bar-fill" id="tempBar" style="width: 50%"></div>
                </div>
                <small id="tempStatus" style="color: #666;">-</small>
            </div>
            
            <div class="card">
                <div class="icon">💧</div>
                <div class="sensor-title">Humidity</div>
                <div class="sensor-value"><span id="humidity">--</span><span class="sensor-unit">%</span></div>
                <div class="sensor-bar">
                    <div class="sensor-bar-fill" id="humidityBar" style="width: 50%"></div>
                </div>
            </div>
            
            <!-- Pressure -->
            <div class="card">
                <div class="icon">🔽</div>
                <div class="sensor-title">Pressure</div>
                <div class="sensor-value"><span id="pressure">--</span><span class="sensor-unit">hPa</span></div>
            </div>
            
            <!-- Soil Moisture -->
            <div class="card">
                <div class="icon">🌱</div>
                <div class="sensor-title">Soil Moisture</div>
                <div class="sensor-value"><span id="soilMoisture">--</span><span class="sensor-unit">%</span></div>
                <div class="sensor-bar">
                    <div class="sensor-bar-fill" id="soilBar" style="width: 50%"></div>
                </div>
            </div>
            
            <!-- Light Level -->
            <div class="card">
                <div class="icon">💡</div>
                <div class="sensor-title">Light Level</div>
                <div class="sensor-value"><span id="lightLevel">--</span><span class="sensor-unit">lux</span></div>
                <div class="sensor-bar">
                    <div class="sensor-bar-fill" id="lightBar" style="width: 50%"></div>
                </div>
            </div>
            
            <!-- Gas Level -->
            <div class="card">
                <div class="icon">⚠️</div>
                <div class="sensor-title">Air Quality (Gas)</div>
                <div class="sensor-value"><span id="gasLevel">--</span><span class="sensor-unit">ppm</span></div>
                <div class="sensor-bar">
                    <div class="sensor-bar-fill" id="gasBar" style="width: 50%"></div>
                </div>
            </div>
            
            <!-- Motion -->
            <div class="card">
                <div class="icon">🚨</div>
                <div class="sensor-title">Motion Detection</div>
                <div class="sensor-value" id="motion" style="font-size:1.5em;">❌ NO MOTION</div>
            </div>
            
            <!-- Speed -->
            <div class="card">
                <div class="icon">🏃</div>
                <div class="sensor-title">Speed</div>
                <div class="sensor-value"><span id="speed">--</span><span class="sensor-unit">km/h</span></div>
                <div class="sensor-bar">
                    <div class="sensor-bar-fill" id="speedBar" style="width: 0%"></div>
                </div>
            </div>
            
            <!-- Water Level -->
            <div class="card">
                <div class="icon">💦</div>
                <div class="sensor-title">Water Level</div>
                <div class="sensor-value"><span id="waterLevel">--</span><span class="sensor-unit">cm</span></div>
                <div class="sensor-bar">
                    <div class="sensor-bar-fill" id="waterBar" style="width: 50%"></div>
                </div>
            </div>
            
            <!-- UV Index -->
            <div class="card">
                <div class="icon">☀️</div>
                <div class="sensor-title">UV Index</div>
                <div class="sensor-value"><span id="uvIndex">--</span></div>
            </div>
            
            <!-- Rain -->
            <div class="card">
                <div class="icon">🌧️</div>
                <div class="sensor-title">Rain Intensity</div>
                <div class="sensor-value"><span id="rainIntensity">--</span><span class="sensor-unit">mm</span></div>
                <div class="sensor-bar">
                    <div class="sensor-bar-fill" id="rainBar" style="width: 0%"></div>
                </div>
            </div>
            
            <!-- Dust -->
            <div class="card">
                <div class="icon">💨</div>
                <div class="sensor-title">Dust Density</div>
                <div class="sensor-value"><span id="dustDensity">--</span><span class="sensor-unit">µg/m³</span></div>
                <div class="sensor-bar">
                    <div class="sensor-bar-fill" id="dustBar" style="width: 50%"></div>
                </div>
            </div>
        </div>
        
        <!-- System Information -->
        <div class="system-info">
            <h3>📊 System Information</h3>
            <div class="system-stat">
                <span class="system-label">CPU Temperature:</span>
                <span class="system-value"><span id="cpuTemp">--</span>°C</span>
            </div>
            <div class="system-stat">
                <span class="system-label">Free RAM:</span>
                <span class="system-value"><span id="freeHeap">--</span> KB</span>
            </div>
            <div class="system-stat">
                <span class="system-label">Last Update:</span>
                <span class="system-value"><span id="lastUpdate">--</span></span>
            </div>
        </div>
        
        <div class="legend">
            <div class="legend-item">
                <div class="legend-box legend-critical"></div>
                <span>Critical</span>
            </div>
            <div class="legend-item">
                <div class="legend-box legend-warning"></div>
                <span>Warning</span>
            </div>
            <div class="legend-item">
                <div class="legend-box legend-normal"></div>
                <span>Normal</span>
            </div>
        </div>
    </div>
    
    <script>
        let ws = null;
        let startTime = Date.now();
        
        function connectWebSocket() {
            const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            ws = new WebSocket(protocol + '//' + window.location.host + ':81/');
            
            ws.onopen = function() {
                console.log('WebSocket connected');
            };
            
            ws.onmessage = function(event) {
                const data = JSON.parse(event.data);
                updateDashboard(data);
            };
            
            ws.onerror = function(error) {
                console.log('WebSocket error: ' + error);
            };
            
            ws.onclose = function() {
                console.log('WebSocket disconnected. Reconnecting in 3s...');
                setTimeout(connectWebSocket, 3000);
            };
        }
        
        function updateDashboard(data) {
            // Temperature & Humidity
            document.getElementById('temp').textContent = formatNumber(data.temperature);
            document.getElementById('humidity').textContent = formatNumber(data.humidity);
            document.getElementById('pressure').textContent = formatNumber(data.pressure);
            document.getElementById('soilMoisture').textContent = formatNumber(data.soilMoisture);
            document.getElementById('lightLevel').textContent = formatNumber(data.lightLevel);
            document.getElementById('gasLevel').textContent = formatNumber(data.gasLevel);
            document.getElementById('speed').textContent = formatNumber(data.speed);
            document.getElementById('waterLevel').textContent = formatNumber(data.waterLevel);
            document.getElementById('uvIndex').textContent = formatNumber(data.uvIndex);
            document.getElementById('rainIntensity').textContent = formatNumber(data.rainIntensity);
            document.getElementById('dustDensity').textContent = formatNumber(data.dustDensity);
            document.getElementById('cpuTemp').textContent = formatNumber(data.cpuTemp);
            document.getElementById('freeHeap').textContent = (data.freeHeap / 1024).toFixed(0);
            
            // Motion Detection
            const motionEl = document.getElementById('motion');
            if (data.motionDetected) {
                motionEl.textContent = '✅ MOTION DETECTED';
                motionEl.classList.add('motion-detected');
                motionEl.classList.remove('motion-clear');
            } else {
                motionEl.textContent = '❌ NO MOTION';
                motionEl.classList.add('motion-clear');
                motionEl.classList.remove('motion-detected');
            }
            
            // Progress bars
            updateBar('tempBar', data.temperature, 0, 50);
            updateBar('humidityBar', data.humidity, 0, 100);
            updateBar('soilBar', data.soilMoisture, 0, 100);
            updateBar('lightBar', data.lightLevel, 0, 100000);
            updateBar('gasBar', data.gasLevel, 0, 1000);
            updateBar('speedBar', data.speed, 0, 100);
            updateBar('waterBar', data.waterLevel, 0, 100);
            updateBar('rainBar', data.rainIntensity, 0, 100);
            updateBar('dustBar', data.dustDensity, 0, 500);
            
            // Last update timestamp
            document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString();
        }
        
        function formatNumber(value) {
            if (typeof value !== 'number' || isNaN(value)) return '--';
            return value.toFixed(2);
        }
        
        function updateBar(elementId, value, min, max) {
            const el = document.getElementById(elementId);
            const percentage = Math.min(100, Math.max(0, ((value - min) / (max - min)) * 100));
            el.style.width = percentage + '%';
        }
        
        function updateClock() {
            document.getElementById('currentTime').textContent = new Date().toLocaleTimeString();
            
            const elapsed = Math.floor((Date.now() - startTime) / 1000);
            let uptime = '';
            const hours = Math.floor(elapsed / 3600);
            const minutes = Math.floor((elapsed % 3600) / 60);
            const seconds = elapsed % 60;
            
            if (hours > 0) uptime = hours + 'h ' + minutes + 'm';
            else if (minutes > 0) uptime = minutes + 'm ' + seconds + 's';
            else uptime = seconds + 's';
            
            document.getElementById('uptime').textContent = uptime;
        }
        
        // Initialize
        connectWebSocket();
        updateClock();
        setInterval(updateClock, 1000);
    </script>
</body>
</html>
)rawliteral";
    }
};

#endif // WEB_SERVER_H
