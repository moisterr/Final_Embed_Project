#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void adc_init(void);
uint16_t adc_read_channel(uint8_t channel);

#endif /* ADC_H_ */
