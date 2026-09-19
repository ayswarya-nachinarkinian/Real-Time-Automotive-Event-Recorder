#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "buffer_manager.h"

void* analyzer_task(void* arg)
{
    sensor_data_t data;

    while (1)
    {
        if (buffer_pop(&data))
        {
        	float ax = data.accel_x;
        	float ay = data.accel_y;
        	float az = data.accel_z;

        	float magnitude = sqrt(ax*ax + ay*ay + az*az);

        	// Correct rollover logic
        	if (az < -0.7)
        	{
        	    printf(" ROLLOVER DETECTED (UPSIDE DOWN)\n");
        	}
        	else if (magnitude < 0.3)
        	{
        	    printf(" FREE FALL DETECTED\n");
        	}

            if (data.gpio_1_state)
                printf(" AIRBAG DEPLOYED\n");

            if (data.gpio_2_state)
                printf(" BRAKE APPLIED\n");
        }

        usleep(100000);
    }
}
