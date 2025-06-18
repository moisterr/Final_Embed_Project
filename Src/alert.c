#include "stm32f4xx.h"
#include "buzzer.h"
#include "lcd1602_i2c.h"

void interrupt_init(void) {
    // Bật clock cho SYSCFG (dùng cho EXTI nếu cần sau này)
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Cấu hình PendSV (ngắt phần mềm)
    NVIC_SetPriority(PendSV_IRQn, 0xFF); // Đặt mức ưu tiên thấp nhất
    NVIC_EnableIRQ(PendSV_IRQn); // Bật ngắt PendSV
}

void Alert_Handler(void) {
    buzzer_on();  // bật còi
    lcd_set_cursor(0, 0);
    lcd_send_string("!!! ALERT !!!");
    // Không cần xóa cờ ngắt cho PendSV, nó tự động được xử lý
}
