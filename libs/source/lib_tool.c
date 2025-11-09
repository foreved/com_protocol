#include "lib_tool.h"

/*
 * @brief   不同位表示相应外设是否开启. 0: 关闭, 1: 开启
 * @note    bit0: SysTick
 *          bit1: DWT
*/
static uint8_t LIB_TOOL_STATUS;
#define    LIB_TOOL_STATUS_SYSTICK_Pos    0
#define    LIB_TOOL_STATUS_SYSTICK_Mask   (1<<LIB_TOOL_STATUS_SYSTICK_Pos)
#define    LIB_TOOL_STATUS_DWT_Pos        1
#define    LIB_TOOL_STATUS_DWT_Mask       (1<<LIB_TOOL_STATUS_DWT_Pos)

/*
 * @brief   配置并开启 SysTick, 时基为 1ms
 * @param   无
 * @return  无
 * @note    SysTick使用 AHB 作为时钟源 (不是 AHB/8), 使用前必须配置
 *          lib_tool.h 中的 LIB_TOOL_AHB_FREQUENCY, 即 AHB 时钟频率.
*/
void Lib_Tool_SysTick_Init(void)
{
    SysTick->LOAD = LIB_TOOL_AHB_FREQUENCY / 1000 - 1; // 1 ms
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk; // 使用 AHB, 并开启 SysTick
}

/*
 * @brief   使用 SysTick 实现毫秒级延时
 * @param   num_ms: 延时 num_ms 毫妙
 * @return  无
 * @note    1) SysTick使用 AHB 作为时钟源 (不是 AHB/8), 使用前必须配置
 *          lib_tool.h 中的 LIB_TOOL_AHB_FREQUENCY, 即 AHB 时钟频率.
 *          2) 推荐场景: 非高精度, 短期, 毫秒级延时
*/
void Lib_Tool_SysTick_Delay_ms(const uint16_t num_ms)
{
    SysTick->VAL = 0;
    for (uint16_t i = 0; i < num_ms; ++i)
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
}

/*
 * @brief   开启 DWT, 时基时 1/LIB_TOOL_AHB_FREQUENCY
 * @param   无
 * @return  无
*/
void Lib_Tool_DWT_Init(void)
{
    // 使能 DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // 清零计数器
    DWT->CYCCNT = 0;
    // 开启计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // DWT 已开启
    LIB_TOOL_STATUS |= LIB_TOOL_STATUS_DWT_Mask;
}

/*
 * @brief   使用 DWT 实现微秒级延时
 * @param   num_us: 延时 num_us 毫妙
 * @return  无
 * @note    1) 使用前要正确配置 DWT 的时钟源频率, lib_tool.h 中的 LIB_TOOL_AHB_FREQUENCY.
 *          2) 使用前必须已经开启 DWT 
 *          2) 推荐场景: 高精度, 短期, 微秒级延时
*/
void Lib_Tool_DWT_Delay_us(const uint16_t num_us)
{
    // 不清零也可以, 因为是无符号数
    // e.g. 0x00000003−0xFFFFFFFE=0x00000005
    uint32_t start = DWT->CYCCNT;
    // num_us 对应的 DWT 计数器的计时数
    uint32_t ticks = LIB_TOOL_AHB_FREQUENCY / 1000000 * num_us;
    while ((DWT->CYCCNT - start) < ticks);
}

/*
 * @brief   使用 DWT 实现计时器, 结束计时, 与 Lib_Tool_DWT_Timer_Start() 一起使用. 微秒级计时 (is_us=1),
 *          毫秒级计时 (is_us=0).
 * @param   start 开始计时时刻
 *          is_us 模式选择:
 *              -0: 毫秒级
 *              -1: 微秒级
 * @return  从 start 到此刻的时间间隔, 毫秒或微秒为单位
 * @note    1) 使用前要 Lib_Tool_DWT_Timer_Start() 获取开始时刻
 *          2) 推荐场景: 高精度, 微妙级或毫秒级, 短期计时
*/
uint32_t Lib_Tool_DWT_Timer_End(const uint32_t start, const uint8_t is_us)
{
    uint32_t ticks = DWT->CYCCNT - start;
    if (is_us) // 毫秒级
    {
        return (uint32_t)((uint64_t)ticks * 1000000 / LIB_TOOL_AHB_FREQUENCY);
    }
    else // 微秒级
    {
        return (uint32_t)((uint64_t)ticks * 1000 / LIB_TOOL_AHB_FREQUENCY);
    }
}

/*
 * @brief   初始化 SysTick, DWT
 * @param   无
 * @return  无
*/
void Lib_Tool_Init(void)
{
    Lib_Tool_SysTick_Init();
    Lib_Tool_DWT_Init();
}