/*
 * motor_control.c
 *
 *  Created on: Jun 17, 2025
 *      Author: gaduy
 */
#include "motor_control.h"
#include "stm32f4xx.h"

void pwm_motor_init(void) {
    // Bật clock GPIOA và TIM3
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // PA6: Alternate Function TIM3_CH1
    GPIOA->MODER &= ~(3 << (6 * 2));
    GPIOA->MODER |=  (2 << (6 * 2));     // Alternate function
    GPIOA->AFR[0] |= (2 << (6 * 4));     // AF2 = TIM3

    // Cấu hình PWM TIM3 CH1
    TIM3->PSC = 16 - 1;        // Giả sử HCLK = 16MHz → 1MHz
    TIM3->ARR = 100 - 1;       // PWM tần số = 10kHz
    TIM3->CCR1 = 0;            // Duty ban đầu = 0%
    TIM3->CCMR1 |= (6 << 4);   // PWM mode 1
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE;
    TIM3->CCER |= TIM_CCER_CC1E;
    TIM3->CR1 |= TIM_CR1_CEN;
}

void pwm_set_percent(uint8_t percent) {
    if (percent > 100) percent = 100;
    TIM3->CCR1 = percent;  // ARR = 100 nên % duty = CCR1
}

void motor_dir_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    GPIOB->MODER |= (1 << (0 * 2));  // PB0 output
    GPIOB->MODER |= (1 << (1 * 2));  // PB1 output

    GPIOB->ODR &= ~((1 << 0) | (1 << 1)); // dừng ban đầu
}

void motor_forward(void) {
    GPIOB->ODR |=  (1 << 0);  // IN1 = 1
    GPIOB->ODR &= ~(1 << 1);  // IN2 = 0
}

void motor_stop(void) {
    GPIOB->ODR &= ~((1 << 0) | (1 << 1)); // IN1 = IN2 = 0
}

void motor_control_from_gas(uint16_t gas_value) {
    if (gas_value < 2000) {
        motor_stop();
        pwm_set_percent(0);
    } else {
        motor_forward();

        // Bắt đầu từ 50%, tăng 10% mỗi 300 đơn vị gas vượt quá 2000
        uint8_t steps = (gas_value - 2000) / 300;
        uint8_t pwm = 50 + steps * 10;

        if (pwm > 100) pwm = 100;
        pwm_set_percent(pwm);
    }
}



