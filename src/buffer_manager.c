#include <pthread.h>
#include "buffer_manager.h"

#define BUFFER_SIZE 256

static sensor_data_t buffer[BUFFER_SIZE];
static int head = 0;
static int tail = 0;
static int count = 0;

static pthread_mutex_t lock;
static pthread_cond_t not_empty;

void init_buffer() {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&not_empty, NULL);
}

int buffer_push(sensor_data_t *data) {
    pthread_mutex_lock(&lock);

    // If full, overwrite the oldest data
    if (count == BUFFER_SIZE) {
        tail = (tail + 1) % BUFFER_SIZE;
        count--;
    }

    buffer[head] = *data;
    head = (head + 1) % BUFFER_SIZE;
    count++;

    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&lock);
    return 0;
}

int buffer_pop(sensor_data_t *data) {
    pthread_mutex_lock(&lock);

    // Wait until there is data
    while (count == 0) {
        pthread_cond_wait(&not_empty, &lock);
    }

    *data = buffer[tail];
    tail = (tail + 1) % BUFFER_SIZE;
    count--;

    pthread_mutex_unlock(&lock);
    return 0;
}
