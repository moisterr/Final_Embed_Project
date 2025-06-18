#include "buzzer.h"
#include "stm32f4xx.h"

void buzzer_init(void) {
    //RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;      // Bật clock cho GPIOA
    GPIOA->MODER &= ~(3 << (5 * 2));          // Clear mode của PA5
    GPIOA->MODER |= (1 << (5 * 2));           // PA5 là output (01)
    GPIOA->ODR &= ~(1 << 5);                  // Tắt còi lúc đầu
}

void buzzer_on(void) {
    GPIOA->ODR |= (1 << 5);                   // Kéo PA5 lên HIGH
}

void buzzer_off(void) {
    GPIOA->ODR &= ~(1 << 5);                  // Kéo PA5 xuống LOW
}
