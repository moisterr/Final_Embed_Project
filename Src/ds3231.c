/*
 * ds3231.c
 *
 *  Created on: May 30, 2025
 *      Author: gaduy
 */
#include "ds3231.h"
#include "i2c.h"

#define DS3231_ADDRESS 0x68 // 7-bit I2C address for DS3231

static uint8_t BCD_to_Dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}


static uint8_t Dec_to_BCD(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}
void DS3231_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    I2C1_Start();
    I2C1_WriteAddress(0x68 << 1); // 0x68 là địa chỉ DS3231
    I2C1_WriteData(0x00);         // Bắt đầu từ thanh ghi giây
    I2C1_WriteData(Dec_to_BCD(seconds));
    I2C1_WriteData(Dec_to_BCD(minutes));
    I2C1_WriteData(Dec_to_BCD(hours));
    I2C1_Stop();
}
uint8_t DS3231_ReadRegister(uint8_t reg) {
    I2C1_Start();
    I2C1_WriteAddress((DS3231_ADDRESS << 1) | 0); // Write mode
    I2C1_WriteData(reg);
    I2C1_Start();
    I2C1_WriteAddress((DS3231_ADDRESS << 1) | 1); // Read mode
    uint8_t data = I2C1_ReadNACK();
    return data;
}

void DS3231_WriteRegister(uint8_t reg, uint8_t data) {
    I2C1_Start();
    I2C1_WriteAddress((DS3231_ADDRESS << 1) | 0);
    I2C1_WriteData(reg);
    I2C1_WriteData(data);
    I2C1_Stop();
}

void DS3231_ReadTime(RTC_Time* time) {
    I2C1_Start();
    I2C1_WriteAddress((DS3231_ADDRESS << 1) | 0);
    I2C1_WriteData(0x00); // Set pointer to seconds register

    I2C1_Start();
    I2C1_WriteAddress((DS3231_ADDRESS << 1) | 1);

    time->seconds = BCD_to_Dec(I2C1_ReadACK());
    time->minutes = BCD_to_Dec(I2C1_ReadACK());
    time->hours   = BCD_to_Dec(I2C1_ReadACK());
    time->day     = BCD_to_Dec(I2C1_ReadACK()); // Day of week
    time->date    = BCD_to_Dec(I2C1_ReadACK()); // Day of month
    time->month   = BCD_to_Dec(I2C1_ReadACK());
    time->year    = BCD_to_Dec(I2C1_ReadNACK());

    // Stop condition is inside ReadNACK
}


