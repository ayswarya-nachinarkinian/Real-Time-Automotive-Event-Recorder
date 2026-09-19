#ifndef EVENT_CAPTURE_H
#define EVENT_CAPTURE_H

#include <stdint.h>

// Sensor Data Structure
typedef struct {
    // IMU Data
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    // BME Data
    float temperature;
    int32_t pressure;
    int32_t humidity;

    // GPIO States
    uint8_t gpio_1_state;
    uint8_t gpio_2_state;

    // Timestamp
    uint64_t timestamp;
} sensor_data_t;

// Function Prototypes
int init_hardware(void);
void* capture_task(void* arg);
void close_hardware(void);

#endif
