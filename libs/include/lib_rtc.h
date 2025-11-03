#ifndef _LIB_RTC_H
#define _LIB_RTC_H

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rtc.h"
#include "stm32f1xx_ll_pwr.h"
#include "ff.h"

#define LIB_RTC           RTC         // 使用的 RTC 实例
#define LIB_RTC_CLK_FRE   LSE_VALUE   // RTC 的时钟频率

#define LIB_RTC_IT_EN               1
#if LIB_RTC_IT_EN
    #define LIB_RTC_IRQ             RTC_IRQn
    #define LIB_RTC_IT_SEC_EN       1     // 使能秒中断
    #define LIB_RTC_IT_SEC_PREEM    0     // 抢占优先级
    #define LIB_RTC_IT_SEC_SUB      0     // 子优先级
    #define Lib_RTC_Handler         RTC_IRQHandler
    extern uint8_t Lib_RTC_IT_SEC_Flag;
#endif

// UNIX 时间戳
#define Lib_RTC_UnixType int32_t  // 时间戳类型
typedef struct {
    int16_t year;   // 1970+
    int8_t month;  // 1-12
    int8_t day;    // 1-31
    int8_t hour;   // 0-23
    int8_t minute; // 0-59
    int8_t second; // 0-59
} Lib_RTC_DateType;                  // 日期类型

// 时区宏定义
#define LIB_RTC_TIMEZONE +8            // 默认 UTC+8
#define LIB_RTC_UNIX    1735689600     // 默认 UNIX 时间戳是 0

void Lib_RTC_Init(void);
void Lib_RTC_Set_Time(const Lib_RTC_UnixType ts);
Lib_RTC_UnixType Lib_RTC_Read_Time(void);
Lib_RTC_UnixType Lib_RTC_Date2Unix(const Lib_RTC_DateType *dt);
void Lib_RTC_Unix2Date(const Lib_RTC_UnixType timestamp, Lib_RTC_DateType* dt);
uint8_t Lib_RTC_Check_Same_Date(const Lib_RTC_DateType *const dt1, const Lib_RTC_DateType *const dt2);
#define Lib_RTC_Cheack_Same_Unix(ts1, ts2) ((ts1) == (ts2))
Lib_RTC_UnixType Lib_RTC_Fat2Unix(const uint32_t fat);
uint32_t Lib_RTC_Unix2Fat(const Lib_RTC_UnixType ts);
void Lib_RTC_Fat2Date(const uint32_t fat, Lib_RTC_DateType *const dt);
uint32_t Lib_RTC_Date2Fat(const Lib_RTC_DateType *const dt);

#endif