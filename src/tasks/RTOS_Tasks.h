/**
 * RTOS_Tasks - FreeRTOS Task Definitions and Utilities
 * Contains task configurations and helper functions
 */

#ifndef RTOS_TASKS_H
#define RTOS_TASKS_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "../include/config.h"

// Task handles (extern declarations)
extern TaskHandle_t sensorTaskHandle;
extern TaskHandle_t displayTaskHandle;
extern TaskHandle_t mqttTaskHandle;
extern TaskHandle_t bleTaskHandle;
extern TaskHandle_t sdCardTaskHandle;
extern TaskHandle_t watchdogTaskHandle;

/**
 * Print RTOS task information
 */
void printTaskInfo() {
    Serial.println("\n========== RTOS Task Information ==========");
    
    // Get current task info
    Serial.printf("Total Free Heap: %lu bytes\n", ESP.getFreeHeap());
    Serial.printf("Min Free Heap: %lu bytes\n", ESP.getMinFreeHeap());
    Serial.printf("Max Alloc Heap: %lu bytes\n", ESP.getMaxAllocHeap());
    
    Serial.println("\nTask List:");
    Serial.println("-------------------------------------------");
    
    // List all running tasks
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    uint32_t ulTotalTime;
    
    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
    
    if (pxTaskStatusArray != NULL) {
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalTime);
        
        Serial.printf("%-20s %10s %10s %10s\n", "Task Name", "Priority", "Stack", "Core");
        Serial.println("-------------------------------------------");
        
        for (x = 0; x < uxArraySize; x++) {
            Serial.printf("%-20s %10lu %10lu %10lu\n",
                pxTaskStatusArray[x].pcTaskName,
                (unsigned long)pxTaskStatusArray[x].uxCurrentPriority,
                (unsigned long)pxTaskStatusArray[x].usStackHighWaterMark,
                (unsigned long)pxTaskStatusArray[x].xCoreID);
        }
        
        vPortFree(pxTaskStatusArray);
    }
    
    Serial.println("===========================================\n");
}

/**
 * Suspend a specific task
 */
void suspendTask(TaskHandle_t taskHandle) {
    if (taskHandle != NULL) {
        vTaskSuspend(taskHandle);
        Serial.println("[RTOS] Task suspended");
    }
}

/**
 * Resume a specific task
 */
void resumeTask(TaskHandle_t taskHandle) {
    if (taskHandle != NULL) {
        vTaskResume(taskHandle);
        Serial.println("[RTOS] Task resumed");
    }
}

/**
 * Delete a specific task
 */
void deleteTask(TaskHandle_t taskHandle) {
    if (taskHandle != NULL) {
        vTaskDelete(taskHandle);
        Serial.println("[RTOS] Task deleted");
    }
}

/**
 * Get current CPU frequency
 */
uint32_t getCPUFrequency() {
    return getCpuFrequencyMhz();
}

/**
 * Print system information
 */
void printSystemInfo() {
    Serial.println("\n========== System Information ==========");
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("CPU Cores: %d\n", ESP.getChipCores());
    Serial.printf("CPU Frequency: %lu MHz\n", getCPUFrequency());
    Serial.printf("Flash Size: %lu bytes\n", ESP.getFlashChipSize());
    Serial.printf("Flash Speed: %lu Hz\n", ESP.getFlashChipSpeed());
    Serial.printf("Free Heap: %lu bytes\n", ESP.getFreeHeap());
    Serial.printf("SDK Version: %s\n", ESP.getSdkVersion());
    Serial.println("========================================\n");
}

/**
 * Delay function that yields to other tasks
 */
void taskDelayMs(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

/**
 * Get uptime in seconds
 */
unsigned long getUptimeSeconds() {
    return millis() / 1000;
}

/**
 * Get uptime formatted string
 */
String getUptimeFormatted() {
    unsigned long seconds = getUptimeSeconds();
    
    unsigned long days = seconds / 86400;
    unsigned long hours = (seconds % 86400) / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;
    
    char buffer[32];
    sprintf(buffer, "%02lu:%02lu:%02lu:%02lu", days, hours, minutes, secs);
    
    return String(buffer);
}

#endif // RTOS_TASKS_H

