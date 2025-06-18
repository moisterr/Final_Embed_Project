/*
 * dht22.h
 *
 *  Created on: Jun 17, 2025
 *      Author: gaduy
 */

#ifndef INC_DHT22_H_
#define INC_DHT22_H_

#include <stdint.h>
#include <stdbool.h>

void DHT22_Init(void);
int DHT22_Read(float *temperature, float *humidity);
void delay_us(uint16_t us);       // sử dụng Timer2
void timer2_init_us(void);        // khởi tạo Timer2


#endif /* INC_DHT22_H_ */
