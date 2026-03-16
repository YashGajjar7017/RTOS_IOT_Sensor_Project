# IoT RTOS Sensor Network Project Plan

## Project Overview
- **Project Name**: IoT RTOS Sensor Network
- **Hardware**: ESP32 (Dual-core, WiFi, Bluetooth, FreeRTOS native)
- **Objective**: Multi-sensor monitoring system with RTOS task management

## Hardware Components
- ESP32 DevKit V1
- DHT22 (Temperature & Humidity)
- Capacitive Soil Moisture Sensor
- PIR Motion Sensor (HC-SR501)
- Speed Sensor (Hall Effect/IR)
- Light Sensor (LDR/Photoresistor)
- Gas Sensor (MQ-135)
- BMP280 (Pressure/Temperature)
- OLED Display (SSD1306 0.96")
- SD Card Module
- LEDs for status indication

## Project Structure
```
IoT_RTOS_Sensor_Network/
├── src/
│   ├── main.cpp              # Main application entry
│   ├── sensors/
│   │   ├── SensorManager.h   # Sensor abstraction layer
│   │   ├── TempHumidity.h    # DHT22 sensor
│   │   ├── SoilMoisture.h    # Soil moisture sensor
│   │   ├── MotionSensor.h    # PIR motion detection
│   │   ├── SpeedSensor.h     # Speed measurement
│   │   ├── LightSensor.h     # LDR sensor
│   │   ├── GasSensor.h       # MQ-135 gas sensor
│   │   └── PressureSensor.h  # BMP280 sensor
│   ├── communication/
│   │   ├── WiFiManager.h     # WiFi connection
│   │   ├── MQTTClient.h      # MQTT publishing
│   │   └── BLEHandler.h      # Bluetooth Low Energy
│   ├── display/
│   │   └── DisplayManager.h  # OLED display control
│   ├── storage/
│   │   └── SDCardManager.h   # SD card logging
│   └── tasks/
│       └── RTOS_Tasks.h      # FreeRTOS task definitions
├── include/
│   └── config.h              # Configuration constants
├── lib/                      # Libraries (if needed)
├── platformio.ini           # PlatformIO configuration
└── README.md
```

## RTOS Task Architecture
1. **SensorReadTask** - Reads all sensors every 2 seconds
2. **DisplayTask** - Updates OLED display every 1 second
3. **MQTTTask** - Publishes data to broker every 10 seconds
4. **BLETask** - Handles BLE notifications
5. **SDCardTask** - Logs data to SD card every 30 seconds
6. **WatchdogTask** - Monitors system health

## FreeRTOS Configuration
- Task Priority: 1-5 (5 highest)
- Stack Size: 2048-4096 bytes
- Core Assignment: Core 0 for sensors, Core 1 for communication

## Features
1. Real-time sensor data acquisition
2. OLED display of all sensor readings
3. WiFi + MQTT cloud publishing
4. Bluetooth BLE GATT server
5. SD card data logging
6. Status LED indicators
7. Task monitoring with watchdog

