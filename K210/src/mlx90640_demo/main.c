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
    float emissivity = 0.95;
    float tr;
    unsigned char slaveAddress;
    static uint16_t eeMLX90640[832]; 
    static uint16_t mlx90640Frame[834]; 
    paramsMLX90640 mlx90640;
    static float mlx90640To[768];
    int status;
    int mode;
    printf("start\n");
    status = MLX90640_DumpEE (0x33, eeMLX90640);
    printf("DumpEE status:%d\n", status);
    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
    printf("Extract status:%d\n", status);
    mode = MLX90640_GetCurMode(0x33);
    printf("Mode:%d\n", mode);
    status = MLX90640_GetFrameData (0x33, mlx90640Frame); 
    printf("GetFrame status:%d\n", status);
    tr = 23.15;
    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
    MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, mlx90640To, mode, &mlx90640);
    MLX90640_BadPixelsCorrection((&mlx90640)->outlierPixels, mlx90640To, mode, &mlx90640);
    while(1) {
        //printf("rst %d\n", MLX90640_GetFrameData(0x33, frame_data));
        status = MLX90640_GetFrameData (0x33, mlx90640Frame); 
        printf("GetFrame status:%d\n", status);
        if(status != 0) {
            continue;
        }
        MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
        MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, mlx90640To, mode, &mlx90640);
        MLX90640_BadPixelsCorrection((&mlx90640)->outlierPixels, mlx90640To, mode, &mlx90640);
        for(int i = 0; i < 768; i++) {
            printf("%f\n", mlx90640To[i]);
        }
    }
}