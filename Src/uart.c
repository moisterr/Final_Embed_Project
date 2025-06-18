#include "stm32f4xx.h"
#include "uart.h"

void uart2_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    GPIOA->MODER |= (2 << 4);     // PA2 alternate function
    GPIOA->AFR[0] |= (7 << 8);    // AF7 for USART2 TX

    USART2->BRR = 16000000 / 9600;
    USART2->CR1 |= USART_CR1_TE;
    USART2->CR1 |= USART_CR1_UE;
}

void uart2_write_string(char *str) {
    while (*str) {
        while (!(USART2->SR & USART_SR_TXE));
        USART2->DR = *str++;
    }
}
