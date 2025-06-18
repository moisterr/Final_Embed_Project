/*
 * DHT22.c
 *
 *  Created on: Jun 17, 2025
 *      Author: gaduy
 */

#include "stm32f4xx.h"
#include "dht22.h"

#define DHT22_PORT GPIOA
#define DHT22_PIN  1    // PA1

// ====================== DELAY_US dùng TIMER2 ======================
void timer2_init_us(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 16 - 1;           // Nếu hệ thống chạy ở 16MHz → 1us/tick
    TIM2->ARR = 0xFFFF;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void delay_us(uint16_t us) {
    TIM2->CNT = 0;
    while (TIM2->CNT < us);
}

// ====================== GPIO xử lý ======================
static void set_pin_output(void) {
    DHT22_PORT->MODER &= ~(3 << (DHT22_PIN * 2));
    DHT22_PORT->MODER |=  (1 << (DHT22_PIN * 2)); // output
    DHT22_PORT->OTYPER &= ~(1 << DHT22_PIN);      // push-pull
    DHT22_PORT->OSPEEDR |= (3 << (DHT22_PIN * 2)); // high speed
}

static void set_pin_input(void) {
    DHT22_PORT->MODER &= ~(3 << (DHT22_PIN * 2)); // input
    DHT22_PORT->PUPDR &= ~(3 << (DHT22_PIN * 2)); // no pull
}

static void dht22_write_low(void) {
    DHT22_PORT->BSRR = (1 << (DHT22_PIN + 16)); // reset bit
}

static void dht22_write_high(void) {
    DHT22_PORT->BSRR = (1 << DHT22_PIN); // set bit
}

static uint8_t dht22_read_pin(void) {
    return (DHT22_PORT->IDR >> DHT22_PIN) & 0x01;
}

// ====================== Khởi tạo ======================
void DHT22_Init(void) {
    //RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   // Enable clock GPIOA
    set_pin_input();                       // Đặt ban đầu là input
    timer2_init_us();                      // Init delay_us bằng Timer2
}

// ====================== Đọc DHT22 ======================
int DHT22_Read(float *temperature, float *humidity) {
    uint8_t data[5] = {0};

    // Start signal
    set_pin_output();
    dht22_write_low();
    delay_us(1000);  // ít nhất 1ms
    dht22_write_high();
    delay_us(30);    // 20-40us
    set_pin_input();

    // Đợi phản hồi từ DHT22 (80us low, 80us high)
    uint32_t timeout = 0;
    while (!dht22_read_pin()) {
        if (++timeout > 100) return -1;
        delay_us(1);
    }
    timeout = 0;
    while (dht22_read_pin()) {
        if (++timeout > 100) return -1;
        delay_us(1);
    }

    // Đọc 40 bit (5 byte)
    for (int i = 0; i < 40; i++) {
        // đợi start bit (low ~50us)
        while (!dht22_read_pin());

        // đo độ rộng xung high
        delay_us(35); // nếu > 30us thì là 1, nếu ~20us là 0

        if (dht22_read_pin()) {
            data[i / 8] |= (1 << (7 - (i % 8)));
            // đợi đến khi pin xuống lại
            while (dht22_read_pin());
        }
    }

    // Kiểm tra checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) return -2;

    // Ghép giá trị
    uint16_t raw_humi = (data[0] << 8) | data[1];
    uint16_t raw_temp = (data[2] << 8) | data[3];

    *humidity = raw_humi / 10.0f;
    if (raw_temp & 0x8000) {
        raw_temp &= 0x7FFF;
        *temperature = -(raw_temp / 10.0f);
    } else {
        *temperature = raw_temp / 10.0f;
    }

    return 0;
}

