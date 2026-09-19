#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <devctl.h>
#include <hw/i2c.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include "event_capture.h"
#include "buffer_manager.h"

#define IMU_ADDR 0x68
#define BME_ADDR 0x76
#define BCM2711_GPIO_BASE 0xFE200000
#define GPLEV0_OFFSET     0x34
#define GPIO_PIN_1        17
#define GPIO_PIN_2        27

static int i2c_fd = -1;
static uintptr_t gpio_base_ptr;

// BME280 Calibration
static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static int32_t  t_fine;

// Minimal I2C Write
static int i2c_write_reg(uint8_t dev, uint8_t reg, uint8_t data) {
    struct { i2c_send_t hdr; uint8_t buf[2]; } msg = {
        .hdr.slave.addr = dev, .hdr.slave.fmt = I2C_ADDRFMT_7BIT, 
        .hdr.len = 2, .hdr.stop = 1, .buf = {reg, data}
    };
    return devctl(i2c_fd, DCMD_I2C_SEND, &msg, sizeof(msg), NULL);
}

// Minimal I2C Read
static int i2c_read_regs(uint8_t dev, uint8_t reg, uint8_t *data, int len) {
    struct { i2c_sendrecv_t hdr; uint8_t buf[32]; } msg = {
        .hdr.slave.addr = dev, .hdr.slave.fmt = I2C_ADDRFMT_7BIT,
        .hdr.send_len = 1, .hdr.recv_len = len, .hdr.stop = 1, .buf = {reg}
    };
    if (devctl(i2c_fd, DCMD_I2C_SENDRECV, &msg, sizeof(msg.hdr) + (len > 1 ? len : 1), NULL) != 0) return -1;
    for (int i = 0; i < len; i++) data[i] = msg.buf[i];
    return 0;
}

static float compensate_temperature(int32_t adc_T) {
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return ((t_fine * 5 + 128) >> 8) / 100.0f;
}

int init_hardware() {
    if (ThreadCtl(_NTO_TCTL_IO, 0) == -1) return -1;
    if ((i2c_fd = open("/dev/i2c1", O_RDWR)) < 0) return -1;

    i2c_write_reg(IMU_ADDR, 0x6B, 0x00); // Wake IMU

    uint8_t calib[6];
    if (i2c_read_regs(BME_ADDR, 0x88, calib, 6) == 0) {
        dig_T1 = (calib[1] << 8) | calib[0];
        dig_T2 = (int16_t)((calib[3] << 8) | calib[2]);
        dig_T3 = (int16_t)((calib[5] << 8) | calib[4]);
    }
    i2c_write_reg(BME_ADDR, 0xF4, 0x27); // Wake BME280

    if ((gpio_base_ptr = mmap_device_io(0x100, BCM2711_GPIO_BASE)) == (uintptr_t)MAP_FAILED) return -1;

    // Set GPIO17 and GPIO27 as inputs
    *(volatile uint32_t *)(gpio_base_ptr + 0x04) &= ~(7 << 21);
    *(volatile uint32_t *)(gpio_base_ptr + 0x08) &= ~(7 << 21);

    printf(" Hardware Initialized Successfully\n");
    return 0;
}

void* capture_task(void* arg) {
    struct timespec next_period;
    sensor_data_t current_data;
    uint8_t buf[14], prev_gpio_1 = 0, prev_gpio_2 = 0;
    
    clock_gettime(CLOCK_MONOTONIC, &next_period);

    while (1) {
        // Read IMU
        if (i2c_read_regs(IMU_ADDR, 0x3B, buf, 6) == 0) {
            current_data.accel_x = (int16_t)((buf[0] << 8) | buf[1]);
            current_data.accel_y = (int16_t)((buf[2] << 8) | buf[3]);
            current_data.accel_z = (int16_t)((buf[4] << 8) | buf[5]);
        }

        // Read BME280
        if (i2c_read_regs(BME_ADDR, 0xF7, buf, 8) == 0) {
            current_data.pressure = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
            current_data.temperature = compensate_temperature((buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4));
        }

        // Read GPIOs
        uint32_t gplev0 = *(volatile uint32_t *)(gpio_base_ptr + GPLEV0_OFFSET);
        current_data.gpio_1_state = (gplev0 & (1 << GPIO_PIN_1)) ? 1 : 0;
        current_data.gpio_2_state = (gplev0 & (1 << GPIO_PIN_2)) ? 1 : 0;

        // Alerts
        if (current_data.gpio_1_state && !prev_gpio_1) printf("\n CRITICAL: AIRBAG DEPLOYED! \n\n");
        if (current_data.gpio_2_state && !prev_gpio_2) printf("\n CRITICAL: BRAKE APPLIED! \n\n");
        prev_gpio_1 = current_data.gpio_1_state;
        prev_gpio_2 = current_data.gpio_2_state;

        // Timestamp & Storage
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        current_data.timestamp = (ts.tv_sec * 1000000000LL) + ts.tv_nsec;
        buffer_push(&current_data);

        printf("T: %lu | Accel X: %6d | Temp: %.2f °C | GPIO17: %s | GPIO27: %s\n",
               current_data.timestamp, current_data.accel_x, current_data.temperature,
               current_data.gpio_1_state ? "AIRBAG DEPLOYED" : "OFF",
               current_data.gpio_2_state ? "BRAKES APPLIED"  : "OFF");

        // 10ms Real-Time Sleep
        next_period.tv_nsec += 10000000;
        while (next_period.tv_nsec >= 1000000000L) {
            next_period.tv_sec++;
            next_period.tv_nsec -= 1000000000L;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_period, NULL);
    }
    return NULL;
}

void close_hardware() {
    if (i2c_fd >= 0) close(i2c_fd);
    if (gpio_base_ptr != (uintptr_t)MAP_FAILED) munmap_device_io(gpio_base_ptr, 0x100);
}
