/*
 * i2c.h
 *
 *  Created on: May 30, 2025
 *      Author: gaduy
 */

// i2c.h
#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);
void i2c_address_write(uint8_t addr);
void i2c_address_read(uint8_t addr);

#endif
 /* INC_I2C_H_ */
