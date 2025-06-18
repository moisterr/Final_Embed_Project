/*
 * adc.c
 *
 *  Created on: May 30, 2025
 *      Author: gaduy
 */

#include "stm32f4xx.h"
#include "adc.h"

void adc_init(void) {
    // 1. Bật clock GPIOA và ADC1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    // 2. Cấu hình PA0 và PA1 ở chế độ analog
    GPIOA->MODER |= (3 << (0 * 2)) | (3 << (1 * 2)); // PA0 & PA1 => Analog mode

    // 3. Cấu hình ADC
    ADC1->SQR1 = 0;  // Đọc 1 conversion mỗi lần
    ADC1->CR2 |= ADC_CR2_ADON; // Bật ADC
    ADC1->SMPR2 |= (7 << 0) | (7 << 3); // Sample time dài nhất cho IN0 & IN1
}

// Hàm đọc giá trị từ kênh ADC chỉ định
uint16_t adc_read_channel(uint8_t channel) {
    ADC1->SQR3 = channel;           // Chọn kênh (0 cho PA0 - MQ2, 1 cho PA1 - LM35)
    ADC1->CR2 |= ADC_CR2_SWSTART;  // Bắt đầu chuyển đổi
    while (!(ADC1->SR & ADC_SR_EOC)); // Chờ kết thúc
    return ADC1->DR;
}
