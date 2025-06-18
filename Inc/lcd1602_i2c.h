#ifndef __LCD1602_I2C_H__
#define __LCD1602_I2C_H__
#include <stdint.h>
void lcd_init(void);
void lcd_send_string(char *str);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);
void lcd_send_cmd(uint8_t cmd);
void lcd_send_data(uint8_t data);
#endif
