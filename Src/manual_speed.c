#include "manual_speed.h"
#include "keypad.h"
#include "lcd1602_i2c.h"
#include "motor_control.h"
#include "delay.h"
#include <stdlib.h>

void manual_speed_mode(void) {
    lcd_clear();
    lcd_send_string("Enter speed(%)");
    lcd_set_cursor(1, 0);

    char speed_str[4] = "";
    uint8_t idx = 0;

    while (1) {
        char key = keypad_getkey();
        if (!key) continue;

        if (key >= '0' && key <= '9' && idx < 3) {
            speed_str[idx++] = key;
            speed_str[idx] = '\0';
            lcd_set_cursor(1, 0);
            lcd_send_string("   ");  // Clear input area
            lcd_set_cursor(1, 0);
            lcd_send_string(speed_str);
        } else if (key == 'D' && idx > 0) {
            idx--;
            speed_str[idx] = '\0';
            lcd_set_cursor(1, 0);
            lcd_send_string("   ");
            lcd_set_cursor(1, 0);
            lcd_send_string(speed_str);
        } else if (key == 'A') {
            int pwm = atoi(speed_str);
            if (pwm >= 0 && pwm <= 100) {
            	pwm_set_percent(pwm);
                motor_forward();
                lcd_clear();
                lcd_send_string("Set OK");
                delay_ms(1000);
                break;
            } else {
                lcd_clear();
                lcd_send_string("Invalid (0-100)");
                delay_ms(1000);
                break;
            }
        }

        delay_ms(1000);
    }
}
