#include "lib_tool.h"

// 微妙级延时
void Lib_Tool_Delay_us(const uint16_t num_us)
{
    // 重新配置
    SysTick->CTRL = 0;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk; // 使用 AHB 时钟
    SysTick->LOAD = LIB_TOOL_AHB_FREQUENCY / 1000000 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    for (uint16_t i = 0; i < num_us; ++i)
        while (((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) >> SysTick_CTRL_COUNTFLAG_Pos) != SET);
    SysTick->CTRL = 0;
}

// 毫秒级延时
void Lib_Tool_Delay_ms(const uint16_t num_ms)
{
    // 重新配置
    SysTick->CTRL = 0;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk; // 使用 AHB 时钟
    SysTick->LOAD = LIB_TOOL_AHB_FREQUENCY / 1000 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    for (uint16_t i = 0; i < num_ms; ++i)
        while (((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) >> SysTick_CTRL_COUNTFLAG_Pos) != SET);
    SysTick->CTRL = 0;
}