#ifndef CONFIG_H
#define CONFIG_H

// ==================== WiFi Configuration ====================
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define WIFI_TIMEOUT_MS 30000

// ==================== MQTT Configuration ====================
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "ESP32_RTOS_Sensor_001"
#define MQTT_TOPIC "iot/sensors/data"
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

// ==================== BLE Configuration ====================
#define BLE_DEVICE_NAME "IoT RTOS Sensor"
#define BLE_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ==================== Pin Definitions ====================
// Analog Sensors
#define SOIL_MOISTURE_PIN 34
#define LIGHT_SENSOR_PIN 35
#define GAS_SENSOR_PIN 32
#define SPEED_SENSOR_PIN 33

// Digital Sensors
#define DHT22_PIN 4
#define PIR_MOTION_PIN 14
#define SPEED_LED_PIN 18

// I2C Pins (Display & BMP280)
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22

// Status LEDs
#define LED_WIFI_PIN 2
#define LED_MQTT_PIN 15
#define LED_SENSOR_PIN 16
#define LED_BUILTIN_PIN 2

// SD Card Pins
#define SD_CS_PIN 5
#define SD_MOSI_PIN 23
#define SD_MISO_PIN 19
#define SD_SCK_PIN 18

// ==================== RTOS Task Configuration ====================
// Task Priorities (0-24, higher = more important)
#define PRIORITY_SENSOR_READ 3
#define PRIORITY_DISPLAY 2
#define PRIORITY_MQTT 3
#define PRIORITY_BLE 2
#define PRIORITY_SDCARD 1
#define PRIORITY_WATCHDOG 4

// Task Stack Sizes (bytes)
#define STACK_SIZE_SMALL 2048
#define STACK_SIZE_MEDIUM 4096
#define STACK_SIZE_LARGE 8192

// Task Delays (milliseconds)
#define DELAY_SENSOR_READ 2000
#define DELAY_DISPLAY 1000
#define DELAY_MQTT 10000
#define DELAY_SDCARD 30000

// ==================== Sensor Configuration ====================
// DHT22
#define DHT_TYPE DHT22
#define DHT_READING_INTERVAL 2000

// PIR Motion
#define PIR_WARMUP_TIME 60000  // 60 seconds warmup
#define PIR_DEBOUNCE_TIME 5000 // 5 seconds between detections

// Speed Sensor
#define SPEED_PULSES_PER_REV 1
#define SPEED_WHEEL_CIRCUMFERENCE 2.07  // meters (700x25c tire)
#define SPEED_TIMEOUT 5000  // ms without pulse = stopped

// Gas Sensor
#define GAS_WARMUP_TIME 20000  // 20 seconds warmup
#define GAS_R0_CALIBRATION 10  // R0 value for MQ-135

// ==================== Display Configuration ====================
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET_PIN -1
#define OLED_I2C_ADDRESS 0x3C

// BMP280 I2C Address
#define BMP280_ADDRESS_ALT 0x76
#define BMP280_ADDRESS_PRIMARY 0x77

// ==================== System Configuration ====================
#define WATCHDOG_TIMEOUT 30000  // 30 seconds
#define SYSTEM_STATUS_CHECK 5000

// ==================== Data Structures ====================
struct SensorData {
    // Environmental
    float temperature;
    float humidity;
    float pressure;
    
    // Soil & Light
    float soilMoisture;
    float lightLevel;
    
    // Motion & Speed
    bool motionDetected;
    float speed;  // km/h
    
    // Air Quality
    float gasLevel;
    
    // System
    unsigned long timestamp;
    uint32_t freeHeap;
    float cpuTemp;
};

struct SystemStatus {
    bool wifiConnected;
    bool mqttConnected;
    bool bleConnected;
    bool sdCardMounted;
    uint32_t uptime;
    uint8_t taskCount;
};

#endif // CONFIG_H

