#include <stdio.h>
#include <unistd.h>

void* timing_monitor_task(void* arg)
{
    while(1)
    {
        printf("⏱ System running...\n");
        sleep(1);
    }
}
