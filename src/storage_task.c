#include <stdio.h>
#include <stdlib.h>
#include "buffer_manager.h"

void* storage_task(void* arg) {
    printf(" Storage thread started. Logging to data_log.csv\n");

    FILE *file = fopen("/tmp/data_log.csv", "w");
    if (!file) {
        perror("Failed to open log file");
        return NULL;
    }

    // Write CSV Header
    fprintf(file, "Timestamp,Accel_X,Temp_C,Airbag_Deployed,Brakes_Applied\n");
    fflush(file);

    sensor_data_t data;

    while (1) {
        // Blocks here until event_capture pushes new data
        buffer_pop(&data);

        // Write to file
        fprintf(file, "%lu,%d,%.2f,%d,%d\n",
                data.timestamp,
                data.accel_x,
                data.temperature,
                data.gpio_1_state,
                data.gpio_2_state);

        // Ensure data is immediately written to disk (critical for blackbox recorders)
        fflush(file);
    }

    fclose(file);
    return NULL;
}
