#ifndef __ALERT_H
#define __ALERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
void interrupt_init(void);
void Alert_Handler(void) ;

#ifdef __cplusplus
}
#endif

#endif /* __ALERT_H */
