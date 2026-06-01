#include "app_car.h"

int main(void)
{
    App_CarInit();

    while (1)
    {
        App_CarTask();
    }
}
