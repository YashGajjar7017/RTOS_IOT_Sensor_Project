# Quick Start: Web Dashboard & Database

## 🚀 Getting Started (5 Minutes)

### 1. Installation
All required libraries have been added to `platformio.ini`. Just build and upload:
```bash
pio run -t upload
```

### 2. Configure WiFi
Edit `include/config.h`:
```cpp
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"
```

### 3. Power On & Connect
- Upload code to ESP32
- Monitor serial output (115200 baud)
- Watch for "System Online" message
- Get the IP address from serial output

### 4. Access Dashboard
Open browser and navigate to:
```
http://<ESP32_IP>
```

Example: `http://192.168.1.100`

## 📊 What You'll See

### Dashboard Features:
- **12 Sensor Cards** - Real-time temperature, humidity, pressure, etc.
- **System Info Panel** - CPU temp, RAM usage, uptime
- **Live Updates** - WebSocket data streaming every 1 second
- **Status Indicators** - Green pulse dot shows system is online
- **Progress Bars** - Visual representation of sensor values
- **Mobile Responsive** - Works on phones, tablets, desktops

### Data Recorded:
- Every 30 seconds: All sensor values saved to database
- Stored on SD card as SQLite database
- Access via REST API at `/api/sensors/json`

## 🔧 Configuration

### Change Web Server Port:
```cpp
// In include/config.h
#define WEB_SERVER_PORT 80      // Change from 80 to any port
#define WEBSOCKET_PORT 81       // WebSocket port
```

### Change Database Logging Interval:
```cpp
// In src/storage/DatabaseManager.h
const unsigned long logInterval = 30000;  // milliseconds (change from 30s)
```

### Change WebSocket Update Rate:
```cpp
// In src/communication/WebServer.h
const unsigned long wsUpdateInterval = 1000;  // milliseconds (change from 1s)
```

## 📱 API Endpoints

### Get Current Sensor Data:
```bash
curl http://<ESP32_IP>/api/sensors/json
```

Response:
```json
{
  "temperature": 25.5,
  "humidity": 60.2,
  "pressure": 1013.25,
  "soilMoisture": 45.0,
  "lightLevel": 500,
  "gasLevel": 150,
  "motionDetected": 0,
  "speed": 0.0,
  "waterLevel": 75.0,
  "uvIndex": 3.5,
  "rainIntensity": 0.0,
  "dustDensity": 25.5,
  "cpuTemp": 45.2,
  "freeHeap": 69840,
  "timestamp": 1234567890
}
```

### Get System Status:
```bash
curl http://<ESP32_IP>/api/system
```

## 🗄️ Database Operations

### Check Database from Serial Monitor:
```
[Database] Records: 180, Size: 256 KB
```

### Export Database to CSV:
Add to your code:
```cpp
databaseManager.exportToCSV("/sd/sensor_export.csv");
```

Then download from SD card.

## 🔌 Hardware Checklist

- [ ] ESP32 board connected and powered
- [ ] WiFi antenna (if external)
- [ ] SD card inserted (required for database)
- [ ] Sensors wired according to config.h pins
- [ ] OLED display connected (SDA=21, SCL=22)

## ⚡ Troubleshooting

### Dashboard Won't Load
1. Check WiFi connection (green LED)
2. Verify IP address in serial monitor
3. Try accessing http://192.168.1.1 to find ESP32
4. Restart ESP32 and try again

### WebSocket Not Updating
1. Check browser console (F12)
2. Verify WebSocket port 81 is open
3. Check WiFi signal strength
4. Restart browser and reconnect

### Database Not Logging
1. Check SD card is present
2. Verify SD card has free space
3. Monitor serial for "Database initialized" message
4. Check database size at /api/system

### Can't Find ESP32 IP
Get it from serial monitor:
```
[setup] Your ESP32 IP address is: 192.168.1.100
```

Or use:
```bash
arp-scan -l  # On Linux/Mac
arp -a       # On Windows
```

## 📖 More Information

- Full documentation: See `WEB_DASHBOARD_GUIDE.md`
- API Reference: See `WEB_DASHBOARD_GUIDE.md` - REST API section
- Database queries: See `WEB_DASHBOARD_GUIDE.md` - SQLite Database section
- RTOS Tasks: See `README.md` - Task Architecture section

## 🎯 Next Steps

1. **Monitor Sensors** - Watch real-time data on dashboard
2. **Export Data** - Download historical data from database
3. **Build Mobile App** - Use the REST API to build a mobile app
4. **Set Thresholds** - Modify code to trigger alerts on sensor values
5. **Cloud Sync** - Send data to cloud using MQTT or custom API

## 💡 Tips & Tricks

### Dashboard Tips:
- Customize gradients by editing the CSS color values
- Add custom sensor cards by modifying the HTML
- Create custom alerts by modifying JavaScript
- Host locally or on cloud server

### Database Tips:
- Regularly export CSV files for long-term storage
- Monitor database size to avoid SD card overflow
- Create backups monthly
- Query historical data for trend analysis

### Performance Tips:
- Reduce WebSocket update frequency for slower networks
- Limit number of WebSocket clients
- Archive old database records monthly
- Use SD card with high write speed

## 📞 Support

For issues with:
- **Web Server**: Check WiFi connection and firewall settings
- **Database**: Check SD card space and format
- **Sensors**: Verify wiring against config.h pins
- **FreeRTOS**: Monitor serial output for task errors

Check serial monitor output for detailed error messages and status updates!

---

**Happy Monitoring! 🎉**
