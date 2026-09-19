#include <stdio.h>
#include <unistd.h>

void* timing_monitor_task(void* arg)
{
    while (1)
    {
        printf("System Alive\n");
        sleep(1);
    }
}
