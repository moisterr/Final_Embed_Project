/*
 * ds3231.h
 *
 *  Created on: May 30, 2025
 *      Author: gaduy
 */

#ifndef DS3231_H_
#define DS3231_H_

#include <stdint.h>

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} RTC_Time;

void DS3231_ReadTime(RTC_Time* time);
uint8_t DS3231_ReadRegister(uint8_t reg);
void DS3231_WriteRegister(uint8_t reg, uint8_t data);

#endif /* DS3231_H_ */
 /* INC_DS3231_H_ */
