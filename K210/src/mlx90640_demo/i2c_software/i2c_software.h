#ifndef _I2C_SOFTWARE_H
#define _I2C_SOFTWARE_H

#include <stdint.h>
#include "gpiohs.h"

#define SLAVE_MAX_ADDR  14
#define SLAVE_ADDRESS   0x32

typedef struct _slave_info
{
    uint8_t acces_reg;
    uint8_t reg_data[SLAVE_MAX_ADDR];
} slave_info_t;

typedef struct _software_i2c_handler
{
    uint8_t sda_pin_num;
    uint8_t scl_pin_num;
    uint8_t sda_hspin_num;
    uint8_t scl_hspin_num;
} software_i2c_haldler_t;

void i2c_slave_init(void);
void i2c_master_init(software_i2c_haldler_t *handler);
void i2c_start(software_i2c_haldler_t *handler);
void i2c_stop(software_i2c_haldler_t *handler);
uint8_t i2c_send_byte(software_i2c_haldler_t *handler, uint8_t data);
uint8_t i2c_receive_byte(software_i2c_haldler_t *handler, uint8_t ack);
uint8_t i2c_write_reg(software_i2c_haldler_t *handler, uint8_t slave_address, uint16_t reg, uint8_t *data_buf, size_t length);
uint8_t i2c_read_reg(software_i2c_haldler_t *handler, uint8_t slave_address, uint16_t reg, uint8_t *data_buf, size_t length);

#endif
