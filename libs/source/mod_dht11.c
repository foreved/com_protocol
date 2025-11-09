#include "mod_dht11.h"
#include "lib_tool.h"
#include "lib_usart.h"
#include "lib_i2c.h"
#include "mod_oled.h"
#include "lib_spi.h"
#include "mod_flash.h"
#include "ff.h"

/*
 * @brief   实时温湿度数据, 2s 更新一次
*/
Mod_DHT11_Data_Type Real_Time_TempHumi;

static void Mod_DHT11_GPIO_Init(void);
static void Mod_DHT11_Change_Output_Type(const uint8_t opt);
static Mod_DHT11_Data_Type Mod_DHT11_Once_Com(void);
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
static void Mod_DHT11_GPIO_Init(void)
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
static void Mod_DHT11_Change_Output_Type(const uint8_t opt)
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
static Mod_DHT11_Data_Type Mod_DHT11_Once_Com(void)
{   
    uint32_t start1 = 0, start2 = 0, num_us = 0;
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
            start1 = Lib_Tool_DWT_Timer_Start();          // T_h0/T_h1 计时开始
            num_us = Lib_Tool_DWT_Timer_End(start2, 1);   // T_low 计时结束
            // 判断 T_low
            if (num_us < 50 || num_us > 58) Mod_DHT11_Error(T_LOW_ERROR);

            // 有效数据位
            while (Mod_DHT11_Data_Read() == SET);
            start2 = Lib_Tool_DWT_Timer_Start();          // T_low 计时开始 或 T_en 计时开始
            num_us = Lib_Tool_DWT_Timer_End(start1, 1);   // T_h0/T_h1 计时结束
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
                // T_H0_ERROR 和 T_H1_ERROR 都可以
                Mod_DHT11_Error(T_H0_ERROR);
            }
        }
        // 湿度整数+湿度小数+温度整数+温度小数
        if (i < 4)
            sum += tmp[i];
    }

    // DHT11 释放总线
    while (Mod_DHT11_Data_Read() == RESET);
    num_us = Lib_Tool_DWT_Timer_End(start2, 1); // T_en 计时结束
    if (num_us < 52 || num_us > 56) Mod_DHT11_Error(T_EN_ERROR);
    // 通信结束, 主机变回开漏输出
    Mod_DHT11_Change_Output_Type(0);

    // 数据处理
    if (sum != tmp[4]) Mod_DHT11_Error(DATA_ERROR);  // 校验数据错误
    res.humi = (uint16_t)tmp[0] * 10 + tmp[1];
    if (tmp[3] & 0x8) // 温度小数部分 MSB 为 1, 表示负温度
    {
        res.temp = -((int16_t)tmp[2] * 10 + (tmp[3] & (~0x8)));
    }
    else
    {
        res.temp = (int16_t)tmp[2] * 10 + tmp[3];
    }

    return res;
}

static void Mod_DHT11_Error(const Mod_DHT11_Error_Type error_idx)
{
    switch (error_idx)
    {
        case T_BE_ERROR:
            Lib_USART_Send_fString("Mod_DHT11_Error <%d>: T_be is wrong.\n", error_idx);
            break;
        case T_GO_ERROR:
            Lib_USART_Send_fString("Mod_DHT11_Error <%d>: T_go is wrong.\n", error_idx);
            break;
        case T_REL_ERROR:
            Lib_USART_Send_fString("Mod_DHT11_Error <%d>: T_rel is wrong.\n", error_idx);
            break;
        case T_REH_ERROR:
            Lib_USART_Send_fString("Mod_DHT11_Error <%d>: T_reh is wrong.\n", error_idx);
            break;
        case T_LOW_ERROR:
            Lib_USART_Send_fString("Mod_DHT11_Error <%d>: T_low is wrong.\n", error_idx);
            break;
        case T_H0_ERROR:
            Lib_USART_Send_fString("Mod_DHT11_Error <%d>: T_h0 is wrong.\n", error_idx);
            break;
        case T_H1_ERROR:
            Lib_USART_Send_fString("Mod_DHT11_Error <%d>: T_h1 is wrong.\n", error_idx);
            break;
        case T_EN_ERROR:
            Lib_USART_Send_fString("Mod_DHT11_Error <%d>: T_en is wrong.\n", error_idx);
            break;
        case DATA_ERROR:
            Lib_USART_Send_fString("Mod_DHT11_Error <%d>: data is wrong.\n", error_idx);
            break;
    }

    while (1);
}

// 返回绝对值
#define    Mod_DHT11_Abs(num)      (num < 0 ? -num : num)

/*
 * @brief   DHT11 温湿度传感器的任务: 读取实时温湿度数据, 周期为2s.
 * @param   无
 * @return  无
*/
void Mod_DHT11_Task(void)
{
    Mod_Oled_Pos_Type pos = {0, 0};
    FATFS fs;

    Lib_Tool_Init();
    Lib_USART_Init();
    // OLED
    Lib_I2C_Init();
    Mod_Oled_Power_Up();
    // Flash
    Lib_SPI_Init();
    // FatFs
    Mod_Flash_FatFs_Check(&fs);
    Mod_DHT11_Log_Init("0:/DHT11");

    // 配置 DATA 总线
    Mod_DHT11_GPIO_Init();     // 主机开漏输出, DHT11 输入
    // DHT11 上电后, 需要等待 2s
    Lib_Tool_SysTick_Delay_ms(2000);
    
    while (1)
    {
        // 连续读取两次数据, 获得此时的温湿度数据
        Real_Time_TempHumi = Mod_DHT11_Once_Com();
        Lib_Tool_SysTick_Delay_ms(100); // 间隔 100ms, 采集数据
        Real_Time_TempHumi = Mod_DHT11_Once_Com();
        
        // 上位机显示
        Lib_USART_Send_fString("Temperature: %d.%d\n", Real_Time_TempHumi.temp / 10, Mod_DHT11_Abs(Real_Time_TempHumi.temp % 10));
        Lib_USART_Send_fString("Humidity: %d.%d\n\n", Real_Time_TempHumi.humi / 10, Real_Time_TempHumi.humi % 10);
        
        // OLED 显示
        Mod_Oled_Clear_Screen();
        pos = (Mod_Oled_Pos_Type){0, 0};
        pos = Mod_Oled_Show_fString(pos, "Temp: %d.%d deg", Real_Time_TempHumi.temp / 10, Mod_DHT11_Abs(Real_Time_TempHumi.temp % 10));
        pos = (Mod_Oled_Pos_Type){pos.page += 2, 0};
        pos = Mod_Oled_Show_fString(pos, "Humi: %d.%d%%", Real_Time_TempHumi.humi / 10, Real_Time_TempHumi.humi % 10);

        // 读取间隔大于 2s
        Lib_Tool_SysTick_Delay_ms(2000);
    }
}

void Mod_DHT11_Log_Init(const TCHAR* path)
{
    FRESULT fres;
    DIR dir;

    fres = f_opendir(&dir, path);
    if (fres == FR_NO_PATH)
    {
        Lib_USART_Send_String("There is no log. Initizlize.\n");
        fres = f_mkdir(path);
        if (fres != FR_OK)
        {
            Lib_USART_Send_fString("Error: fail to initizlize logs. FRESULT is %d Stop...\n", fres);
            while (1);
        }
    }
    else if (fres != FR_OK)
    {
        Lib_USART_Send_fString("Error: FRESULT is %d. Stop...\n", fres);
        while (1);
    }
    Lib_USART_Send_String("Succeed to initialize logs.\n");
}

void Mod_DHT11_Log_Append(const Mod_DHT11_Data_Type data, const TCHAR* path)
{
    FIL file;
    FRESULT fres;

    fres = f_open(&file, path, FA_OPEN_APPEND | FA_WRITE);
    f_write()
}