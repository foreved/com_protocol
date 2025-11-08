#ifndef _MOD_DHT11_H
#define _MOD_DHT11_H

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"

/*
 * @brief   DHT11 的 GPIO 配置
 * @note    DHT11 仅使用一个 DATA 引脚作为数据总线
*/
#define    MOD_DHT11_DATA_PORT               GPIOB
#define    MOD_DHT11_DATA_PIN                LL_GPIO_PIN_1
#define    MOD_DHT11_GPIO_EN_CLK()           LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB)

typedef enum
{
    T_BE_ERROR,                // 主机起始信号拉低时间
    T_GO_ERROR,                // 主机释放总线时间
    T_REL_ERROR,               // 响应低电平时间
    T_REH_ERROR,               // 相应高电平时间
    T_LOW_ERROR,               // 信号0/1低电平时间
    T_H0_ERROR,                // 信号0高电平时间
    T_H1_ERROR,                // 信号1高电平时间
    T_EN_ERROR,                // 传感器释放总线时间
} Mod_DHT11_Error_Type;

typedef struct 
{
    int8_t temp_int;        // 温度整数部分
    uint8_t temp_frac;      // 温度小数部分
    uint8_t humi_int;       // 湿度整数部分
    uint8_t humi_frac;      // 湿度小数部分
} Mod_DHT11_Data_Type;

void Mod_DHT11_Task(void);

#endif