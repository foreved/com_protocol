#ifndef _LIB_TOOL_H
#define _LIB_TOOL_H

#include "stm32f1xx_ll_cortex.h"

#define LIB_TOOL_AHB_FREQUENCY     72000000

void Lib_Tool_Delay_us(const uint16_t num_us);
void Lib_Tool_Delay_ms(const uint16_t num_ms);

#endif