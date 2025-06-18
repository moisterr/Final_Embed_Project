/*
 * i2c.c
 *
 *  Created on: May 30, 2025
 *      Author: gaduy
 */
#include "stm32f4xx.h"
#include "i2c.h"

void i2c_init(void) {
    // Enable clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // Set PB8 (SCL), PB9 (SDA) to Alternate Function
    GPIOB->MODER &= ~((3 << (8 * 2)) | (3 << (9 * 2)));
    GPIOB->MODER |= (2 << (8 * 2)) | (2 << (9 * 2)); // AF mode
    GPIOB->OTYPER |= (1 << 8) | (1 << 9);            // Open drain
    GPIOB->PUPDR &= ~((3 << (8 * 2)) | (3 << (9 * 2)));
    GPIOB->PUPDR |= (1 << (8 * 2)) | (1 << (9 * 2)); // Pull-up

    // AF4 for I2C1 on PB8, PB9
    GPIOB->AFR[1] &= ~((0xF << ((8 - 8) * 4)) | (0xF << ((9 - 8) * 4)));
    GPIOB->AFR[1] |= (4 << ((8 - 8) * 4)) | (4 << ((9 - 8) * 4));

    // Reset and configure I2C1
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR2 = 16;     // APB1 freq = 16 MHz
    I2C1->CCR = 80;     // 100kHz
    I2C1->TRISE = 17;   // TRISE = Freq + 1
    I2C1->CR1 |= I2C_CR1_PE;
}


void I2C1_Start(void) {
 I2C1->CR1 |= I2C_CR1_START;
 while (!(I2C1->SR1 & I2C_SR1_SB));//Wait for Start bit
}

void I2C1_Stop(void) {
 I2C1->CR1 |= I2C_CR1_STOP;
}

void I2C1_WriteAddress(uint8_t address) {
 I2C1->DR = address;
 while (!(I2C1->SR1 & I2C_SR1_ADDR));//Wait for Slave to response
 (void)I2C1->SR2; //Read SR2 to erase ADDR flag
}

void I2C1_WriteData(uint8_t data) {
 while (!(I2C1->SR1 & I2C_SR1_TXE));//Data Register Empty(transmitter)
 I2C1->DR = data;
 while (!(I2C1->SR1 & I2C_SR1_BTF));//Byte Transfer Finished
}

uint8_t I2C1_ReadACK(void) {
 I2C1->CR1 |= I2C_CR1_ACK; // Send ACK after read
 while (!(I2C1->SR1 & I2C_SR1_RXNE));
 return I2C1->DR;
}

uint8_t I2C1_ReadNACK(void) {
 I2C1->CR1 &= ~I2C_CR1_ACK; // Send NACK after read the last byte
 I2C1->CR1 |= I2C_CR1_STOP;
 while (!(I2C1->SR1 & I2C_SR1_RXNE));
 return I2C1->DR;
}


