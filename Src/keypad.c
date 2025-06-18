/*
 * keypad.c
 *
 *  Created on: Jun 17, 2025
 *      Author: gaduy
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_
#include <stdint.h>
void keypad_init(void);
char keypad_getkey(void);
#endif

/* ---------- keypad.c ---------- */
#include "stm32f4xx.h"
#include "keypad.h"
#include "delay.h"

static const char keymap[4][4] = {
    { '1','2','3','A' },
    { '4','5','6','B' },
    { '7','8','9','C' },
    { '*','0','#','D' }
};

void keypad_init(void) {
    /* Cấu hình PC0..PC3 là output (ROWS) */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER &= ~(
        (3<<(0*2)) | (3<<(1*2)) | (3<<(2*2)) | (3<<(3*2))
    );
    GPIOC->MODER |= (
        (1<<(0*2)) | (1<<(1*2)) | (1<<(2*2)) | (1<<(3*2))
    );
    /* Cấu hình PC4..PC7 là input pull-up (COLS) */
    GPIOC->MODER &= ~(
        (3<<(4*2)) | (3<<(5*2)) | (3<<(6*2)) | (3<<(7*2))
    );
    GPIOC->PUPDR &= ~(
        (3<<(4*2)) | (3<<(5*2)) | (3<<(6*2)) | (3<<(7*2))
    );
    GPIOC->PUPDR |= (
        (1<<(4*2)) | (1<<(5*2)) | (1<<(6*2)) | (1<<(7*2))
    );
}

char keypad_getkey(void) {
    for (uint8_t row = 0; row < 4; row++) {
        /* Đặt tất cả ROWs = HIGH */
        GPIOC->ODR |=  (1<<0)|(1<<1)|(1<<2)|(1<<3);
        /* Kéo row hiện tại xuống LOW */
        GPIOC->ODR &= ~(1<<row);
        delay_ms(1);  // debounce

        /* Đọc 4 cột */
        for (uint8_t col = 0; col < 4; col++) {
            if (!(GPIOC->IDR & (1<<(4+col)))) {
                /* đợi phím nhả để tránh ghi lặp */
                while (!(GPIOC->IDR & (1<<(4+col))));
                return keymap[row][col];
            }
        }
    }
    return 0; // không có phím
}

