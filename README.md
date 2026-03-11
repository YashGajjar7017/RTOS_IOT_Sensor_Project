# IoT RTOS Sensor Network

Multi-sensor monitoring system using ESP32 with FreeRTOS. Reads data from various sensors and outputs to OLED display, WiFi/MQTT, Bluetooth BLE, and SD card logging.

## Features

### Sensors
- **DHT22**: Temperature & Humidity
- **Soil Moisture**: Capacitive soil moisture sensor
- **PIR Motion**: Motion detection (HC-SR501)
- **Speed Sensor**: Hall effect / IR speed measurement
- **Light Sensor**: LDR/Photoresistor
- **Gas Sensor**: MQ-135 air quality (CO2 equivalent)
- **BMP280**: Barometric pressure

### Communication
- **WiFi**: Connect to local network
- **MQTT**: Publish sensor data to broker (HiveMQ, etc.)
- **BLE**: Bluetooth Low Energy GATT server

### Outputs
- **OLED Display**: SSD1306 128x64 OLED
- **SD Card**: CSV data logging

### RTOS Tasks
- SensorReadTask (Core 0) - Reads all sensors every 2s
- DisplayTask (Core 0) - Updates OLED every 1s
- MQTTTask (Core 1) - Publishes to MQTT every 10s
- BLETask (Core 1) - Handles BLE connections
- SDCardTask (Core 1) - Logs to SD every 30s
- WatchdogTask (Core 0) - System monitoring

## Hardware Requirements

### ESP32 Board
- ESP32 DevKit V1 or similar

### Sensors (7 sensors)
| Sensor | Pin | Notes |
|--------|-----|-------|
| DHT22 | GPIO4 | Temperature & Humidity |
| Soil Moisture | GPIO34 | Analog |
| PIR Motion | GPIO14 | Digital |
| Speed Sensor | GPIO33 | Digital (Interrupt) |
| Light Sensor | GPIO35 | Analog |
| Gas Sensor | GPIO32 | Analog |
| BMP280 | I2C | SDA=21, SCL=22 |

### Outputs
| Device | Pin | Notes |
|--------|-----|-------|
| OLED Display | I2C | 0x3C, SDA=21, SCL=22 |
| SD Card | SPI | CS=5, MOSI=23, MISO=19, SCK=18 |
| Status LEDs | 2, 15, 16 | WiFi, MQTT, Sensor |

## Wiring Diagram

```
ESP32 Pin Connections:
======================
GPIO 4  → DHT22 DATA
GPIO 14 → PIR MOTION OUT
GPIO 18 → SPEED SENSOR (LED)
GPIO 21 → OLED SDA
GPIO 22 → OLED SCL
GPIO 23 → SD Card MOSI
GPIO 32 → MQ-135 AO
GPIO 33 → SPEED SENSOR OUT (Interrupt)
GPIO 34 → SOIL MOISTURE AO
GPIO 35 → LIGHT SENSOR (LDR)
GPIO  2 → LED (WiFi Status)
GPIO 15 → LED (MQTT Status)
GPIO 16 → LED (Sensor Status)
GPIO  5 → SD Card CS
GPIO 19 → SD Card MISO
GPIO 18 → SD Card SCK
```

## Software Setup

### 1. Install PlatformIO
```bash
# Using pip
pip install platformio

# Or use VSCode PlatformIO Extension
```

### 2. Configure WiFi & MQTT
Edit `include/config.h`:
```cpp
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"

#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
```

### 3. Build and Upload
```bash
# Build
pio run

# Upload
pio run --target upload

# Monitor
pio device monitor
```

## Data Format

### MQTT JSON Payload
```json
{
  "timestamp": 1234567890,
  "temperature": 25.3,
  "humidity": 65.2,
  "pressure": 1013.25,
  "soilMoisture": 45.0,
  "lightLevel": 72.5,
  "motionDetected": false,
  "speed": 0.0,
  "gasLevel": 12.3,
  "freeHeap": 23456,
  "cpuTemp": 45.2
}
```

### SD Card CSV Format
```
timestamp,temperature,humidity,pressure,soil_moisture,light_level,motion,speed,gas_level,free_heap,cpu_temp
1234567890,25.3,65.2,1013.25,45.0,72.5,0,0.0,12.3,23456,45.2
```

## Task Priorities

| Task | Priority | Core | Stack Size |
|------|----------|------|------------|
| WatchdogTask | 4 (highest) | 0 | 2KB |
| SensorReadTask | 3 | 0 | 4KB |
| MQTTTask | 3 | 1 | 8KB |
| DisplayTask | 2 | 0 | 4KB |
| BLETask | 2 | 1 | 4KB |
| SDCardTask | 1 (lowest) | 1 | 4KB |

## Testing MQTT

Use an MQTT client to subscribe to the topic:
```
Topic: iot/sensors/data
Broker: broker.hivemq.com:1883
```

## BLE Communication

Connect via nRF Connect or similar BLE app:
- Device Name: IoT RTOS Sensor
- Service UUID: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
- Characteristic UUID: beb5483e-36e1-4688-b7f5-ea07361b26a8

## Troubleshooting

### No sensor readings
- Check wiring connections
- Verify pin numbers in config.h

### WiFi not connecting
- Check SSID and password
- Ensure 2.4GHz network (not 5GHz)

### MQTT not publishing
- Check WiFi connection first
- Verify broker address and port

### SD card not working
- Verify SPI pin connections
- Check if card is formatted FAT32

### Display not showing
- Check I2C connections (SDA, SCL)
- Verify I2C address (0x3C default)

## License

MIT License

