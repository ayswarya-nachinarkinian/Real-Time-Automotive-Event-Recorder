#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "event_capture.h"

void init_buffer(void);
int buffer_push(sensor_data_t *data);
int buffer_pop(sensor_data_t *data);

#endif
