#include "lcd1602_i2c.h"
#include "adc.h"
#include "uart.h"
#include "delay.h"
#include "i2c.h"
#include "ds3231.h"
#include "dht22.h"
#include "keypad.h"
#include <stdio.h>
#include <string.h>
#include "motor_control.h"
#include "manual_speed.h"
#include "buzzer.h"
#include "alert.h"

#define PW_LENGTH 1
const char DEFAULT_PW[PW_LENGTH + 1] = "1";

#define TEMP_THRESHOLD   35.0f
//#define HUMI_THRESHOLD   90.0f
#define GAS_THRESHOLD    2000

uint8_t is_alerting = 0;


typedef enum {
    MODE_AUTO,
    MODE_MANUAL
} ControlMode;

ControlMode current_mode = MODE_AUTO;

char line1[17]; // Dùng để hiển thị thời gian
char line2[17]; // Dùng để hiển thị giá trị gas và nhiệt độ
RTC_Time currentTime; // Cấu trúc thời gian từ DS3231

int main(void) {
    // ------------------ KHỞI TẠO CÁC NGOẠI VI ------------------
    // Khởi tạo I2C để giao tiếp với DS3231 và LCD I2C
    i2c_init();

    // Khởi tạo ADC để đọc giá trị từ cảm biến MQ2 (PA0)
    adc_init();

    // Khởi tạo và xóa màn hình LCD 16x2
    lcd_init();
    lcd_clear();

    // Khởi tạo cảm biến DHT22 để đo nhiệt độ và độ ẩm
    DHT22_Init();

    // Khởi tạo timer2 để hỗ trợ độ trễ microsecond (dùng cho DHT22)
    timer2_init_us();

    // Khởi tạo bàn phím 4x4 để nhập dữ liệu
    keypad_init();

    // Khởi tạo PWM cho động cơ
    pwm_motor_init();

    // Khởi tạo điều khiển hướng động cơ
    motor_dir_init();

    // Khởi tạo buzzer để phát tín hiệu cảnh báo
    buzzer_init();

    // Khởi tạo ngắt (sử dụng PendSV)
    interrupt_init();

    // ------------------ XÁC THỰC MẬT KHẨU ------------------
    char entered[PW_LENGTH + 1]; // Mảng lưu mật khẩu nhập vào
    uint8_t idx = 0; // Chỉ số vị trí trong mảng entered

    while (1) {
        // Xóa màn hình LCD để hiển thị yêu cầu nhập mật khẩu
        lcd_clear();
        lcd_send_string("Enter PW:");

        // Đặt lại biến nhập
        idx = 0;
        entered[0] = '\0';

        while (1) {
            // Đọc phím từ bàn phím 4x4
            char key = keypad_getkey();
            if (!key) continue; // Bỏ qua nếu không có phím nào được nhấn

            // Xử lý nhập số (0-9)
            if (key >= '0' && key <= '9' && idx < PW_LENGTH) {
                entered[idx++] = key; // Thêm ký tự vào mảng
                entered[idx] = '\0';  // Kết thúc chuỗi
                lcd_set_cursor(1, 0); // Đặt con trỏ dòng 1, cột 0
                lcd_send_string(entered); // Hiển thị mật khẩu nhập
            }
            // Xử lý xóa ký tự (phím D)
            else if (key == 'D' && idx > 0) {
                idx--; // Giảm chỉ số
                entered[idx] = '\0'; // Xóa ký tự cuối
                lcd_set_cursor(1, 0); // Đặt con trỏ dòng 1, cột 0
                lcd_send_string(entered); // Hiển thị lại chuỗi
                lcd_send_string(" "); // Xóa ký tự cũ bằng khoảng trắng
                lcd_set_cursor(1, idx); // Đặt con trỏ về vị trí hiện tại
            }
            // Xử lý xác nhận (phím A)
            else if (key == 'A') {
                entered[idx] = '\0'; // Kết thúc chuỗi
                if (idx == PW_LENGTH && strcmp(entered, DEFAULT_PW) == 0) {
                    // Nếu mật khẩu đúng (dài đúng và khớp)
                    lcd_clear();
                    lcd_send_string("Access Granted");
                    delay_ms(1000); // Hiển thị thông báo 1 giây
                    lcd_clear();
                    break; // Thoát khỏi vòng nhập mật khẩu
                } else {
                    // Nếu mật khẩu sai
                    lcd_clear();
                    lcd_send_string("Wrong Password!");
                    delay_ms(1000); // Hiển thị thông báo 1 giây
                    break; // Quay lại nhập lại từ đầu
                }
            }
            delay_ms(100); // Chống rung phím
        }

        // Thoát vòng lặp ngoài nếu mật khẩu đúng
        if (strcmp(entered, DEFAULT_PW) == 0) break;
    }

    // ------------------ CHƯƠNG TRÌNH CHÍNH ------------------
    float dht_temp = 0.0f; // Biến lưu nhiệt độ từ DHT22
    float dht_hum = 0.0f;  // Biến lưu độ ẩm từ DHT22
    uint16_t mq2_value = 0; // Biến lưu giá trị từ cảm biến MQ2

    while (1) {
        // Đọc phím để chuyển đổi chế độ
        char key = keypad_getkey();
        if (key == 'B') {
            current_mode = MODE_AUTO; // Chuyển sang chế độ tự động
            lcd_clear();
            lcd_send_string("Mode: AUTO");
            delay_ms(1000); // Hiển thị thông báo 1 giây
            lcd_clear();
        } else if (key == 'C') {
            current_mode = MODE_MANUAL; // Chuyển sang chế độ thủ công
            manual_speed_mode(); // Chạy chế độ điều khiển thủ công
        }

        // Đọc dữ liệu từ cảm biến
        mq2_value = adc_read_channel(0); // Đọc giá trị gas từ MQ2 (PA0)
        DS3231_ReadTime(&currentTime); // Cập nhật thời gian từ DS3231
//        sprintf(line1, "%02d:%02d:%02d", currentTime.hours, currentTime.minutes, currentTime.seconds);
//        lcd_set_cursor(0, 0); // Đặt con trỏ dòng 0, cột 0
//        lcd_send_string(line1); // Hiển thị thời gian

        // Đọc dữ liệu từ DHT22
        if (DHT22_Read(&dht_temp, &dht_hum) != 0) {
            dht_temp = 0.0f; // Nếu đọc thất bại, đặt về 0
            dht_hum = 0.0f;
        }
        snprintf(line2, sizeof(line2), "G:%4d; T:%4.1f", mq2_value, dht_temp);
        lcd_set_cursor(1, 0); // Đặt con trỏ dòng 1, cột 0
        lcd_send_string(line2); // Hiển thị giá trị gas và nhiệt độ

        // Điều khiển động cơ dựa trên chế độ
        if (current_mode == MODE_AUTO) {
            motor_control_from_gas(mq2_value); // Điều khiển động cơ dựa trên giá trị gas
        }

        // Kiểm tra ngưỡng và kích hoạt ngắt
        if (mq2_value > GAS_THRESHOLD || dht_temp > TEMP_THRESHOLD) {
        	if (!is_alerting) {
        	        is_alerting = 1;
        	        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Gọi ngắt phần mềm để cảnh báo
        	    }
        } else {
        	sprintf(line1, "%02d:%02d:%02d", currentTime.hours, currentTime.minutes, currentTime.seconds);
        	lcd_set_cursor(0, 0); // Đặt con trỏ dòng 0, cột 0
        	lcd_send_string(line1); // Hiển thị thời gian
        	if (is_alerting) {
        	        is_alerting = 0;
        	        buzzer_off();
        	        // Clear LCD & hiển thị lại nội dung thường
        	        lcd_clear();
        	        lcd_set_cursor(0, 0);
        	        lcd_send_string("Now Normal...");
        	        lcd_set_cursor(1, 0);
        	        lcd_send_string("Back to monitor");
        	        delay_ms(1000); // Hiển thị trong 1s rồi quay lại
        	        lcd_clear();
        	    }
        }

        delay_ms(2000); // Đợi 2 giây trước khi lặp lại (có thể điều chỉnh)
    }

    // Không cần return vì đây là vòng lặp vô hạn
}
