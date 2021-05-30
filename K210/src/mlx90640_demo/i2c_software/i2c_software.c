#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "fpioa.h"
#include "i2c.h"
#include "utils.h"
#include "sysctl.h"
#include "i2c_software.h"

#define DELAY_TIME  10

slave_info_t slave_device;
i2c_slave_handler_t slave_handler;

void i2c_slave_receive(uint32_t data)
{
    if (slave_device.acces_reg == 0xFF)
    {
        if (data < SLAVE_MAX_ADDR)
            slave_device.acces_reg = data;
    } 
    else if (slave_device.acces_reg < SLAVE_MAX_ADDR)
    {
        slave_device.reg_data[slave_device.acces_reg] = data;
        slave_device.acces_reg++;
    }
}
uint32_t i2c_slave_transmit()
{
    uint32_t ret = 0;
    if (slave_device.acces_reg >= SLAVE_MAX_ADDR)
        slave_device.acces_reg = 0;
    ret = slave_device.reg_data[slave_device.acces_reg];
    slave_device.acces_reg++;
    return ret;
}
void i2c_slave_event(i2c_event_t event)
{
    if(I2C_EV_START == event)
    {
        if (slave_device.acces_reg >= SLAVE_MAX_ADDR)
            slave_device.acces_reg = 0xFF;
    }
    else if(I2C_EV_STOP == event)
    {
        slave_device.acces_reg = 0xFF;
    }
}

void i2c_slave_init(void)
{
    slave_handler.on_event = i2c_slave_event,
    slave_handler.on_receive = i2c_slave_receive,
    slave_handler.on_transmit = i2c_slave_transmit,

    i2c_init_as_slave(0, SLAVE_ADDRESS, 7, &slave_handler);
    slave_device.acces_reg = 0xFF;
}

void i2c_master_init(software_i2c_haldler_t *handler)
{
    printf("scl pin:%d\nsda pin:%d\nscl hspin:%d\nsda hspin:%d\n", 
        handler->scl_pin_num, handler->sda_pin_num, handler->scl_hspin_num, handler->sda_hspin_num);
    //init io
    fpioa_set_function(handler->scl_pin_num, FUNC_GPIOHS0 + handler->scl_hspin_num);
    fpioa_set_function(handler->sda_pin_num, FUNC_GPIOHS0 + handler->sda_hspin_num);
    gpiohs_set_drive_mode(handler->scl_hspin_num, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(handler->sda_hspin_num, GPIO_DM_OUTPUT);
    gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_HIGH);
    gpiohs_set_pin(handler->sda_hspin_num, GPIO_PV_HIGH);
}
/*
    7 SDA
    6 SCL
 */
void i2c_start(software_i2c_haldler_t *handler)
{
    fpioa_set_function(handler->scl_pin_num, FUNC_GPIOHS0 + handler->scl_hspin_num);
    fpioa_set_function(handler->sda_pin_num, FUNC_GPIOHS0 + handler->sda_hspin_num);
    gpiohs_set_drive_mode(handler->sda_hspin_num, GPIO_DM_OUTPUT);
    gpiohs_set_pin(handler->sda_hspin_num, GPIO_PV_HIGH);
    gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_HIGH);
    usleep(DELAY_TIME);
    gpiohs_set_pin(handler->sda_hspin_num, GPIO_PV_LOW);
    usleep(DELAY_TIME);
    gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_LOW);
}

void i2c_stop(software_i2c_haldler_t *handler)
{
    gpiohs_set_drive_mode(handler->sda_hspin_num, GPIO_DM_OUTPUT);
    gpiohs_set_pin(handler->sda_hspin_num, GPIO_PV_LOW);
    gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_HIGH);
    usleep(DELAY_TIME);
    gpiohs_set_pin(handler->sda_hspin_num, GPIO_PV_HIGH);
}

uint8_t i2c_send_byte(software_i2c_haldler_t *handler, uint8_t data)
{
    uint8_t index;

    gpiohs_set_drive_mode(handler->sda_hspin_num, GPIO_DM_OUTPUT);
    for (index = 0; index < 8; index++)
    {
        if (data & 0x80)
            gpiohs_set_pin(handler->sda_hspin_num, GPIO_PV_HIGH);
        else
            gpiohs_set_pin(handler->sda_hspin_num, GPIO_PV_LOW);
        usleep(DELAY_TIME);
        gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_HIGH);
        usleep(DELAY_TIME);
        gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_LOW);
        data <<= 1;
    }
    gpiohs_set_drive_mode(handler->sda_hspin_num, GPIO_DM_INPUT);
    usleep(DELAY_TIME);
    gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_HIGH);
    data = gpiohs_get_pin(handler->sda_hspin_num);
    usleep(DELAY_TIME);
    gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_LOW);

    return data;
}

uint8_t i2c_receive_byte(software_i2c_haldler_t *handler, uint8_t ack)
{
    uint8_t index, data = 0;

    gpiohs_set_drive_mode(handler->sda_hspin_num, GPIO_DM_INPUT);
    for (index = 0; index < 8; index++)
    {
        usleep(DELAY_TIME);
        gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_HIGH);
        data <<= 1;
        if (gpiohs_get_pin(handler->sda_hspin_num))
            data++;
        usleep(DELAY_TIME);
        gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_LOW);
    }
    gpiohs_set_drive_mode(handler->sda_hspin_num, GPIO_DM_OUTPUT);
    if (ack)
        gpiohs_set_pin(handler->sda_hspin_num, GPIO_PV_HIGH);
    else
        gpiohs_set_pin(handler->sda_hspin_num, GPIO_PV_LOW);
    usleep(DELAY_TIME);
    gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_HIGH);
    usleep(DELAY_TIME);
    gpiohs_set_pin(handler->scl_hspin_num, GPIO_PV_LOW);

    return data;
}

uint8_t i2c_write_reg(software_i2c_haldler_t *handler, uint8_t slave_address, uint16_t reg, uint8_t *data_buf, size_t length)
{
    uint8_t cmd[2] = {0,0};

    cmd[0] = reg >> 8;
    cmd[1] = reg & 0x00FF;
    i2c_start(handler);
    i2c_send_byte(handler, slave_address << 1);
    i2c_send_byte(handler, cmd[0]);
    i2c_send_byte(handler, cmd[1]);
    while (length--)
        i2c_send_byte(handler, *data_buf++);
    i2c_stop(handler);

    return 0;
}

uint8_t i2c_read_reg(software_i2c_haldler_t *handler, uint8_t slave_address, uint16_t reg, uint8_t *data_buf, size_t length)
{
    uint8_t cmd[2] = {0,0};

    cmd[0] = reg >> 8;
    cmd[1] = reg & 0x00FF;
    i2c_start(handler);
    i2c_send_byte(handler, slave_address << 1);
    i2c_send_byte(handler, cmd[0]);
    i2c_send_byte(handler, cmd[1]);
    i2c_start(handler);
    i2c_send_byte(handler, ((slave_address << 1) | 0x01));
    while (length > 1L)
    {
        *data_buf++ = i2c_receive_byte(handler, 0);
        length--;
    }
    *data_buf++ = i2c_receive_byte(handler, 1);
    i2c_stop(handler);

    return 0;
}

