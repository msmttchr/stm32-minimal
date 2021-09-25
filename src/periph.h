#include <stdlib.h>
#include <stdint.h>

#ifndef __PERIPH_H__
#define __PERIPH_H__
void     Configure_USART(void);
void     LED_Init(void);
void     LED_On(void);
void     LED_Off(void);
void     LED_Blinking(uint32_t Period);
void     UserButton_Init(void);
size_t   USART_TxBuffer(uint8_t*buffer, size_t size);
char    *getUidString(void);   
#endif
