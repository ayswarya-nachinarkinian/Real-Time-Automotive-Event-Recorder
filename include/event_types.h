#ifndef EVENT_TYPES_H
#define EVENT_TYPES_H

typedef struct {
    long timestamp;

    float ax;
    float ay;
    float az;

    int temp;
    int press;

    int brake;
    int airbag;

} sensor_data_t;

#endif
