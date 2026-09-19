#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include "event_capture.h"
#include "buffer_manager.h"

// Declare storage task prototype
extern void* storage_task(void* arg);

int main(int argc, char *argv[])
{
    printf(" Starting Real-Time Sensor Acquisition...\n");

    init_buffer(); // <--- Initialize Ring Buffer

    if (init_hardware() != 0) {
        fprintf(stderr, "Failed to initialize hardware.\n");
        return EXIT_FAILURE;
    }

    pthread_t capture_thread_id, storage_thread_id;
    pthread_attr_t attr;
    struct sched_param param;

    // Create the background Storage Thread (standard priority)
    pthread_create(&storage_thread_id, NULL, storage_task, NULL);

    // Setup high-priority Real-Time FIFO attributes for Capture Thread
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 50;
    pthread_attr_setschedparam(&attr, &param);

    // Create Capture Thread
    pthread_create(&capture_thread_id, &attr, capture_task, NULL);

    // Wait forever
    pthread_join(capture_thread_id, NULL);
    pthread_join(storage_thread_id, NULL);

    close_hardware();
    return EXIT_SUCCESS;
}
