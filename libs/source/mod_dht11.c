#include "mod_dht11.h"
#include "lib_tool.h"
#include "lib_usart.h"

static void Mod_DHT11_Error(const Mod_DHT11_Error_Type error_idx);

/*
 * @brief   使 DATA 引脚输出高电平
 * @param   无
 * @return  无
 * @note    该函数其实是 LL_GPIO_SetOutputPin()
*/
#define    Mod_DHT11_Data_Set()    LL_GPIO_SetOutputPin(MOD_DHT11_DATA_PORT, MOD_DHT11_DATA_PIN)

/*
 * @brief   使 DATA 引脚输出低电平
 * @param   无
 * @return  无
 * @note    该函数其实是 LL_GPIO_ResetOutputPin()
*/
#define    Mod_DHT11_Data_Reset()  LL_GPIO_ResetOutputPin(MOD_DHT11_DATA_PORT, MOD_DHT11_DATA_PIN)

/*
 * @brief   读取 DATA 总线状态
 * @param   无
 * @return  SET: 总线高电平; RESET: 总线低电平
*/
#define    Mod_DHT11_Data_Read()   LL_GPIO_IsInputPinSet(MOD_DHT11_DATA_PORT, MOD_DHT11_DATA_PIN)

/*
 * @brief   配置 DHT11 使用的 GPIO, 仅一个 DATA 引脚
 * @param   无
 * @return  无
 * @note    使用前, 需要在 mod_dht11.h 中修改 GPIO 相关配置
*/
void Mod_DHT11_GPIO_Init(void)
{
    LL_GPIO_InitTypeDef gpio_config = {0};

    MOD_DHT11_GPIO_EN_CLK();

    // DATA 初始化为开漏输出
    gpio_config.Pin = MOD_DHT11_DATA_PIN;
    gpio_config.Mode = LL_GPIO_MODE_OUTPUT;
    gpio_config.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;  // 开漏输出
    gpio_config.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio_config.Pull = LL_GPIO_PULL_UP;                 // 上拉输入
    LL_GPIO_Init(MOD_DHT11_DATA_PORT, &gpio_config);
    // 释放总线
    Mod_DHT11_Data_Set();
}

/*
 * @brief   切换 DATA 引脚的工作模式
 * @param   opt 模式选择:
 *              -0: 开漏输出
 *              -1: 上拉输入
 * @return  无
*/
void Mod_DHT11_Change_Output_Type(const uint8_t opt)
{
    if (opt) // 上拉输入
    {
        LL_GPIO_SetPinMode(MOD_DHT11_DATA_PORT, MOD_DHT11_DATA_PIN, LL_GPIO_MODE_INPUT);
        LL_GPIO_SetPinPull(MOD_DHT11_DATA_PORT, MOD_DHT11_DATA_PIN, LL_GPIO_PULL_UP);
    }
    else // 开漏输出
    {
        LL_GPIO_SetPinMode(MOD_DHT11_DATA_PORT, MOD_DHT11_DATA_PIN, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinOutputType(MOD_DHT11_DATA_PORT, MOD_DHT11_DATA_PIN, LL_GPIO_OUTPUT_OPENDRAIN);
    }
}

/*
 * @brief   主机与 DHT11 进行一次通信, 并读取数据
 * @param   无
 * @return  无
 * @note    0) 使用前必须正确配置 lib_tool.h 中的 LIB_TOOL_AHB_FREQUENCY
 *          1) T_be: 主机拉低 DATA 的时间必须在 [18ms, 30ms], 典型值是 20ms.
 *          2) T_go: 主机释放总线后需要等待 DHT11 切换状态, [10us, 35 us], 典型值是 13us;
 *          3) T_rel: DHT11 低电平响应时间 [78us, 88us], 典型值是 83 us
 *          4) T_reh: DHT11 高电平响应时间 [80us, 92us], 典型值是 87 us
 *          5) T_low: 有效数据标志低电平时间 [50us, 58us], 典型值是 54 us
 *          6) T_h0: 有效数据 0 高电平时间 [23us, 27us], 典型值是 24us
 *          7) T_h1: 有效数据 1 高电平时间 [68us, 74us], 典型值是 71 us
 *          8) T_en: DHT11 释放总线时间 [52us, 56us], 典型值是 54us
*/
Mod_DHT11_Data_Type Mod_DHT11_Once_Com(void)
{   
    uint32_t start1 = 0, start2, num_us = 0;
    Mod_DHT11_Data_Type res = {0};
    uint8_t tmp[5], sum = 0;
    
    // 拉低总线
    Mod_DHT11_Data_Reset();
    // 延时 20 ms
    Lib_Tool_SysTick_Delay_ms(20);
    // 切换为上拉输入, 释放总线
    Mod_DHT11_Change_Output_Type(1);

    // 等待 DHT11 切换状态
    start1 = Lib_Tool_DWT_Timer_Start();            // T_go 计时开始
    while (Mod_DHT11_Data_Read() == SET);
    start2 = Lib_Tool_DWT_Timer_Start();            // T_rel 计时开始
    num_us = Lib_Tool_DWT_Timer_End(start1, 1);     // T_go 计时结束
    // 判断 T_go
    if (num_us < 10 || num_us > 35) Mod_DHT11_Error(T_GO_ERROR);

    // 总线拉低, DHT11 发送应答信号
    while (Mod_DHT11_Data_Read() == RESET);
    start1 = Lib_Tool_DWT_Timer_Start();            // T_reh 计时开始
    num_us = Lib_Tool_DWT_Timer_End(start2, 1);     // T_rel 计时结束
    // 判断 T_rel
    if (num_us < 78 || num_us > 88) Mod_DHT11_Error(T_REL_ERROR);

    // 总线拉高, DHT11 通知主机准备接收数据
    while (Mod_DHT11_Data_Read() == SET);
    start2 = Lib_Tool_DWT_Timer_Start();          // T_low 计时开始
    num_us = Lib_Tool_DWT_Timer_End(start1, 1);     // T_reh 计时结束
    // 判断 T_reh
    if (num_us < 80 || num_us > 92) Mod_DHT11_Error(T_REH_ERROR);
    
    // 接收40位数据
    for (uint8_t i = 0; i < 5; ++i)
    {
        for (uint8_t j = 0; j < 8; ++j)
        {
            // 有效数据位标志低电平
            while (Mod_DHT11_Data_Read() == RESET);
            start1 = Lib_Tool_DWT_Timer_Start();          // T_h 计时开始
            num_us = Lib_Tool_DWT_Timer_End(start2, 1);   // T_low 计时结束
            // 判断 T_low
            if (num_us < 50 || num_us > 58) Lib_USART_Send_fString("Error: flag  %d and %d\n", i, j);

            // 有效数据位
            while (Mod_DHT11_Data_Read() == SET);
            start2 = Lib_Tool_DWT_Timer_Start();
            num_us = Lib_Tool_DWT_Timer_End(start1, 1);   // T_h 计时结束
            // 判断数据
            if (num_us >= 23 && num_us <= 27) // 有效数据 0
            {
                tmp[i] = tmp[i] << 1;
            }
            else if (num_us >= 68 && num_us <= 74) // 有效数据 1
            {
                tmp[i] = (tmp[i] << 1) | 1;
            }
            else
            {
                Lib_USART_Send_fString("Error:  %d and %d\n", i, j);
            }
        }
        if (i < 4)
            sum += tmp[i];
    }

    // DHT11 释放总线
    // start1 = Lib_Tool_DWT_Timer_Start();        // T_en 计时开始
    while (Mod_DHT11_Data_Read() == RESET);
    num_us = Lib_Tool_DWT_Timer_End(start2, 1); // T_en 计时结束
    if (num_us < 52 || num_us > 56) Mod_DHT11_Error(T_EN_ERROR);
    Mod_DHT11_Change_Output_Type(0);

    // 数据处理
    if (sum != tmp[4]) Lib_USART_Send_String("Error: wrong data\n");
    res.humi_int = tmp[0];
    res.humi_frac = tmp[1];
    if (tmp[3] & 0x8) // 温度小数部分 MSB 为 1, 表示负温度
    {
        res.temp_int = -tmp[2];
        res.temp_frac = tmp[3] & (~0x8);
    }
    else
    {
        res.temp_int = tmp[2];
        res.temp_frac = tmp[3];
    }

    return res;
}

static void Mod_DHT11_Error(const Mod_DHT11_Error_Type error_idx)
{
    Lib_USART_Send_fString("Error idx: %x\n", error_idx);
    while (1);
}

/*
 * @brief   DHT11 温湿度传感器的任务
 * @param   无
 * @return  无
*/
void Mod_DHT11_Task(void)
{
    Mod_DHT11_Data_Type res = {0};

    Lib_Tool_DWT_Init();
    // 配置 DATA 总线
    Mod_DHT11_GPIO_Init();     // 主机开漏输出, DHT11 输入
    // DHT11 上电后, 需要等待 1s
    Lib_Tool_SysTick_Delay_ms(1000);
    
    res = Mod_DHT11_Once_Com();
    Lib_Tool_SysTick_Delay_ms(2000);
    while (1)
    {
        // 连续读取两次数据, 获得此时的温湿度数据
        res = Mod_DHT11_Once_Com();
        Lib_USART_Send_fString("Temperature: %d.%d\n", res.temp_int, res.temp_frac);
        Lib_USART_Send_fString("Humidity: %d.%d\n\n", res.humi_int, res.humi_frac);
        // 读取间隔大于 2s
        Lib_Tool_SysTick_Delay_ms(2000);
    }
}