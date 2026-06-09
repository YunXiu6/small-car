#include "app_car.h"

/* Program entry point. All hardware and application state are owned by app_car. */
int main(void)
{
    /* Initialize clocks, peripherals, drivers, and the initial UI/debug output. */
    App_CarInit();

    while (1)
    {
        /* Cooperative main task; each subsystem is scheduled by elapsed time. */
        App_CarTask();
    }
}
