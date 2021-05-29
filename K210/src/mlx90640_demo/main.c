#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include "plic.h"
#include "sysctl.h"
#include "bsp.h"
#include <stdio.h>

int main()
{
    uint16_t frame_data[1024];

    plic_init();
    sysctl_enable_irq();
    MLX90640_I2CInit();
    while(1) {
        printf("rst %d\n", MLX90640_GetFrameData(0x33, frame_data));
    }
}