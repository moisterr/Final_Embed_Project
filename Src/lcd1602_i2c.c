#include "stm32f4xx.h"
#include "lcd1602_i2c.h"
#include <stdint.h>
#include "delay.h"
//#include "i2c.h"

#define LCD_ADDR (0x27 << 1)  // Địa chỉ I2C của module I2C LCD (thường là 0x27 hoặc 0x3F)
#define I2C_DELAY 1

extern void delay_ms(int ms);

//static void i2c1_init(void);
static void i2c1_write(uint8_t data);

static void lcd_write4bits(uint8_t data);
static void lcd_send(uint8_t data, uint8_t mode);
static void lcd_write_cmd(uint8_t cmd);
static void lcd_write_data(uint8_t data);

void lcd_init(void) {
    //i2c_init();
    delay_ms(50);

    lcd_write_cmd(0x33); // init
    lcd_write_cmd(0x32); // 4-bit mode
    lcd_write_cmd(0x28); // 2 line, 5x8 matrix
    lcd_write_cmd(0x0C); // Display on, cursor off
    lcd_write_cmd(0x06); // Increment cursor
    lcd_clear();
}

void lcd_clear(void) {
    lcd_write_cmd(0x01);
    delay_ms(2);
}

void lcd_send_string(char *str) {
    while (*str) {
        lcd_write_data(*str++);
    }
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? 0x80 + col : 0xC0 + col;
    lcd_write_cmd(addr);
}

//---------------- INTERNAL --------------------
static void lcd_write_cmd(uint8_t cmd) {
    lcd_send(cmd, 0);
}

static void lcd_write_data(uint8_t data) {
    lcd_send(data, 1);
}

static void lcd_send(uint8_t data, uint8_t mode) {
    uint8_t high = data & 0xF0;
    uint8_t low = (data << 4) & 0xF0;

    lcd_write4bits(high | (mode ? 0x01 : 0x00));
    lcd_write4bits(low | (mode ? 0x01 : 0x00));
}

static void lcd_write4bits(uint8_t data) {
    i2c1_write(data | 0x08);  // EN = 0, Backlight on
    i2c1_write(data | 0x0C);  // EN = 1
    delay_ms(I2C_DELAY);
    i2c1_write(data | 0x08);  // EN = 0
    delay_ms(I2C_DELAY);
}

//---------------- I2C1 BASIC --------------------

static void i2c1_write(uint8_t data) {
    while (I2C1->SR2 & I2C_SR2_BUSY);

    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));
    (void)I2C1->SR1;

    I2C1->DR = LCD_ADDR;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR2;

    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = data;
    while (!(I2C1->SR1 & I2C_SR1_BTF));

    I2C1->CR1 |= I2C_CR1_STOP;
}
