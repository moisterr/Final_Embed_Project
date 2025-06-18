/*
 * motor_control.h
 *
 *  Created on: Jun 17, 2025
 *      Author: gaduy
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>

// Khởi tạo PWM (TIM3 CH1 - PA6)
void pwm_motor_init(void);

// Đặt duty cycle PWM (%)
void pwm_set_percent(uint8_t percent);

// Khởi tạo chân điều khiển chiều quay (PB0/PB1)
void motor_dir_init(void);

// Quay tới
void motor_forward(void);

// Dừng động cơ
void motor_stop(void);

// Hàm điều khiển chính từ giá trị MQ2
void motor_control_from_gas(uint16_t gas_value);

#endif

