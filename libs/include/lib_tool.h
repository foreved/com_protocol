#ifndef _LIB_TOOL_H
#define _LIB_TOOL_H

#include "stm32f1xx_ll_cortex.h"

// AHB 时钟频率
#define LIB_TOOL_AHB_FREQUENCY     72000000

void Lib_Tool_SysTick_Init(void);
void Lib_Tool_SysTick_Delay_ms(const uint16_t num_ms);
void Lib_Tool_DWT_Init(void);
void Lib_Tool_DWT_Delay_us(const uint16_t num_us);
uint32_t Lib_Tool_DWT_Timer_Start(void);
uint32_t Lib_Tool_DWT_Timer_End(const uint32_t start, const uint8_t is_us);

#endif