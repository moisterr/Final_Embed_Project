#include "stm32f4xx.h"
#include "dht22.h"

// Định nghĩa cổng và chân kết nối với cảm biến DHT22
#define DHT22_PORT GPIOA
#define DHT22_PIN  1    // sử dụng chân PA1

// ====================== DELAY_US dùng TIMER2 ======================
// Hàm khởi tạo Timer2 để tạo delay theo micro giây (us)
void timer2_init_us(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;     // Bật clock cho Timer2
    TIM2->PSC = 16 - 1;                     // Prescaler để giảm xung clock xuống còn 1 MHz (1 tick = 1us)
    TIM2->ARR = 0xFFFF;                     // Giá trị tự động nạp lại cao nhất (không quan trọng trong delay)
    TIM2->CR1 |= TIM_CR1_CEN;              // Bắt đầu đếm
}

// Hàm delay theo micro giây (us) bằng Timer2
void delay_us(uint16_t us) {
    TIM2->CNT = 0;                          // Đặt lại counter về 0
    while (TIM2->CNT < us);                // Chờ cho đến khi đủ số tick
}

// ====================== CẤU HÌNH GPIO ======================
// Đặt chân DHT22 làm output
static void set_pin_output(void) {
    DHT22_PORT->MODER &= ~(3 << (DHT22_PIN * 2));   // Xóa 2 bit điều khiển chế độ chân
    DHT22_PORT->MODER |=  (1 << (DHT22_PIN * 2));   // Đặt chế độ output (01)
    DHT22_PORT->OTYPER &= ~(1 << DHT22_PIN);        // Output dạng push-pull
    DHT22_PORT->OSPEEDR |= (3 << (DHT22_PIN * 2));  // Tốc độ cao
}

// Đặt chân DHT22 làm input
static void set_pin_input(void) {
    DHT22_PORT->MODER &= ~(3 << (DHT22_PIN * 2));   // Đặt về input
    DHT22_PORT->PUPDR &= ~(3 << (DHT22_PIN * 2));   // Không kéo pull-up hoặc pull-down
}

// Kéo chân DHT22 xuống mức thấp (0)
static void dht22_write_low(void) {
    DHT22_PORT->BSRR = (1 << (DHT22_PIN + 16)); // Reset bit tương ứng (BSRR bit 16+N = Reset)
}

// Đặt chân DHT22 lên mức cao (1)
static void dht22_write_high(void) {
    DHT22_PORT->BSRR = (1 << DHT22_PIN);       // Set bit tương ứng
}

// Đọc trạng thái chân DHT22 (1 hoặc 0)
static uint8_t dht22_read_pin(void) {
    return (DHT22_PORT->IDR >> DHT22_PIN) & 0x01;  // Đọc giá trị từ thanh ghi input
}

// ====================== KHỞI TẠO ======================
// Khởi tạo module DHT22
void DHT22_Init(void) {
    //RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   // (Bật clock GPIOA nếu cần thiết – đã bật ở main)
    set_pin_input();                         // Đặt chân ở trạng thái input ban đầu
    timer2_init_us();                        // Khởi tạo Timer2 để delay us
}

// ====================== ĐỌC DỮ LIỆU TỪ DHT22 ======================
// Đọc nhiệt độ và độ ẩm từ DHT22
// Trả về 0 nếu thành công, -1 nếu timeout, -2 nếu checksum sai
int DHT22_Read(float *temperature, float *humidity) {
    uint8_t data[5] = {0};  // Mảng chứa 5 byte dữ liệu từ DHT22

    // ==== Gửi tín hiệu khởi động đến DHT22 ====
    set_pin_output();         // Đặt chân làm output
    dht22_write_low();        // Kéo xuống mức thấp
    delay_us(1000);           // Giữ ít nhất 1ms (theo datasheet)
    dht22_write_high();       // Kéo lên mức cao
    delay_us(30);             // Đợi 20-40us
    set_pin_input();          // Sau đó chuyển về input để đợi phản hồi

    // ==== Chờ phản hồi từ DHT22: 80us LOW + 80us HIGH ====
    uint32_t timeout = 0;
    while (!dht22_read_pin()) {      // Đợi chân lên mức cao (kết thúc 80us LOW)
        if (++timeout > 100) return -1;
        delay_us(1);
    }

    timeout = 0;
    while (dht22_read_pin()) {       // Đợi chân xuống mức thấp (kết thúc 80us HIGH)
        if (++timeout > 100) return -1;
        delay_us(1);
    }

    // ==== Đọc 40 bit dữ liệu (5 byte) ====
    for (int i = 0; i < 40; i++) {
        // Đợi start bit (low khoảng 50us)
        while (!dht22_read_pin());

        // Delay 35us để phân biệt bit 0/1
        delay_us(35);  // Nếu pin vẫn còn HIGH → là bit 1, nếu LOW → là bit 0

        if (dht22_read_pin()) {
            data[i / 8] |= (1 << (7 - (i % 8)));  // Ghi bit 1 vào byte tương ứng
            while (dht22_read_pin());            // Đợi cho đến khi chân xuống lại
        }
    }

    // ==== Kiểm tra checksum ====
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) return -2;

    // ==== Ghép dữ liệu và chuyển đổi sang float ====
    uint16_t raw_humi = (data[0] << 8) | data[1];      // Ghép độ ẩm 16-bit
    uint16_t raw_temp = (data[2] << 8) | data[3];      // Ghép nhiệt độ 16-bit

    *humidity = raw_humi / 10.0f;                      // Độ ẩm tính theo %
    if (raw_temp & 0x8000) {                           // Nếu là giá trị âm (bit 15 = 1)
        raw_temp &= 0x7FFF;
        *temperature = -(raw_temp / 10.0f);            // Đổi dấu
    } else {
        *temperature = raw_temp / 10.0f;
    }

    return 0;  // Thành công
}
