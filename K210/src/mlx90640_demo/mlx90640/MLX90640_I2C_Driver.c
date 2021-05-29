/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "i2c_software.h"
#include "MLX90640_I2C_Driver.h"
#include "sleep.h"

software_i2c_haldler_t mlx90640_i2c_handler;

void MLX90640_I2CInit()
{   
    //init i2c
    mlx90640_i2c_handler.scl_pin_num = 30;
    mlx90640_i2c_handler.sda_pin_num = 31;
    mlx90640_i2c_handler.scl_hspin_num = 3;
    mlx90640_i2c_handler.sda_hspin_num = 4;
    i2c_master_init(&mlx90640_i2c_handler);
    i2c_stop(&mlx90640_i2c_handler);
}

int MLX90640_I2CGeneralReset(void)
{    
    int ack;
    char cmd[2] = {0,0};
    
    cmd[0] = 0x00;
    cmd[1] = 0x06;

    i2c_stop(&mlx90640_i2c_handler);
    usleep(5);   
    ack = i2c_write_reg(&mlx90640_i2c_handler, cmd[0], 0, &cmd[1], 1);
    
    if (ack != 0x00)
    {
        return -1;
    }         
    i2c_stop(&mlx90640_i2c_handler);  
    
    usleep(50);    
    
    return 0;
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    uint8_t sa;                           
    int ack = 0;                               
    int cnt = 0;
    int i = 0;
    uint8_t cmd[2] = {0,0};
    uint8_t i2cData[1664] = {0};
    uint16_t *p;
    
    p = data;
    sa = (slaveAddr << 1);
    cmd[0] = startAddress >> 8;
    cmd[1] = startAddress & 0x00FF;
     
    i2c_stop(&mlx90640_i2c_handler);
    usleep(5);
    ack = i2c_read_reg(&mlx90640_i2c_handler, slaveAddr, startAddress, i2cData, 2*nMemAddressRead);
    if (ack != 0x00)
    {
        return -1; 
    }  
    i2c_stop(&mlx90640_i2c_handler); 
    
    for(cnt=0; cnt < nMemAddressRead; cnt++)
    {
        i = cnt << 1;
        *p++ = (uint16_t)i2cData[i]*256 + (uint16_t)i2cData[i+1];
    }
    
    return 0;   
} 

void MLX90640_I2CFreqSet(int freq)
{
    
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    uint8_t sa;
    int ack = 0;
    char cmd[4] = {0,0,0,0};
    static uint16_t dataCheck;
    

    // sa = (slaveAddr << 1);
    cmd[0] = writeAddress >> 8;
    cmd[1] = writeAddress & 0x00FF;
    cmd[2] = data >> 8;
    cmd[3] = data & 0x00FF;

    i2c_stop(&mlx90640_i2c_handler); 
    usleep(5);
    ack = i2c_write_reg(&mlx90640_i2c_handler, slaveAddr, writeAddress, &cmd[2], 2);
    if (ack != 0x00)
    {
        return -1;
    }         
    i2c_stop(&mlx90640_i2c_handler);  
    
    MLX90640_I2CRead(slaveAddr,writeAddress, 1, &dataCheck);
    
    if ( dataCheck != data)
    {
        printf("%d:%d\n", data, dataCheck);
        return -2;
    }    
    
    return 0;
}

