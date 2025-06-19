/*
 * adc.c
 *
 *  Created on: May 30, 2025
 *      Author: gaduy
 */

#include "stm32f4xx.h"
#include "adc.h"

// ====================== HÀM KHỞI TẠO ADC ======================
void adc_init(void) {
    // 1. Bật clock cho GPIOA (cổng chứa PA0, PA1) và ADC1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   // Clock cho GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;    // Clock cho ADC1

    // 2. Cấu hình PA0 và PA1 sang chế độ Analog (MODER = 11)
    GPIOA->MODER |= (3 << (0 * 2)) | (3 << (1 * 2)); // 3 = 0b11

    // 3. Cấu hình ADC
    ADC1->SQR1 = 0;                 // Chỉ đọc 1 kênh mỗi lần (1 conversion)
    ADC1->CR2 |= ADC_CR2_ADON;     // Bật ADC (ADON = 1)
    ADC1->SMPR2 |= (7 << 0) | (7 << 3);  // Thiết lập thời gian lấy mẫu (sample time) dài nhất cho kênh 0 và 1
    // - 7 tương đương 480 chu kỳ → lấy mẫu chính xác hơn khi điện trở nguồn cao
}

// ====================== HÀM ĐỌC GIÁ TRỊ ADC ======================
// Đọc dữ liệu ADC từ kênh chỉ định (channel: 0 -> PA0, 1 -> PA1)
uint16_t adc_read_channel(uint8_t channel) {
    ADC1->SQR3 = channel;            // Đặt kênh cần đọc (SQR3: Regular sequence register 3)
    ADC1->CR2 |= ADC_CR2_SWSTART;   // Bắt đầu chuyển đổi ADC (start conversion)
    while (!(ADC1->SR & ADC_SR_EOC)); // Chờ cho đến khi bit EOC (End Of Conversion) được set
    return ADC1->DR;                 // Trả về giá trị ADC đã đọc (từ thanh ghi DR)
}
