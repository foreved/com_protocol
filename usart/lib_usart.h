#ifndef _LIB_USART_H
#define _LIB_USART_H

#include <stdarg.h>
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_usart.h"

#define LIB_USART                USART1   // 使用USART1
#define LIB_USART_TX_PORT        GPIOA    // USART1的TX为PA9
#define LIB_USART_TX_PIN         LL_GPIO_PIN_9
#define LIB_USART_RX_PORT        GPIOA    // USART1的RX为PA10
#define LIB_USART_RX_PIN         LL_GPIO_PIN_10
#define LIB_USART_ENCLK()        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1)
#define LIB_USART_PORT_ENCLK()   LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA)
#define LIB_USART_BAUD_RATE      115200   // 波特率为115.2Kbps

void Lib_USART_Init(void);
void Lib_USART_Send_Byte(const int8_t data);
void Lib_USART_Send_String(const char *str);
void Lib_USART_Send_fString(const char *str, ...);

#endif