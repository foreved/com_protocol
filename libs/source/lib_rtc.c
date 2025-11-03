#include "lib_rtc.h"
#include "lib_usart.h"

#define LIB_RTC_WAIT_TASK()     do {} while(LL_RTC_IsActiveFlag_RTOF(LIB_RTC) != SET)

void Lib_RTC_Init(void)
{   
    // RTC 位于 BKP 区, 所以使能 BKP
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_BKP);
    // 复位后, BKP 默认不能访问, 必须开启访问权限
    LL_PWR_EnableBkUpAccess();
    // RTC 与 APB1 总线分离, 首次访问 RTC 前, 需要同步
    LL_RTC_WaitForSynchro(LIB_RTC);
    
    // 配置 RTC
    // 任何写 RTC 寄存器的前后, 都要确保上次和本次任务已经完成

    // 进行配置前, 必须进入配置模式
    LIB_RTC_WAIT_TASK();
    LL_RTC_DisableWriteProtection(LIB_RTC);
    LIB_RTC_WAIT_TASK();
    // 设置 RTC 预分频系数, 使分频后的 RTC 时钟为 1 Hz
    LL_RTC_SetAsynchPrescaler(LIB_RTC, LIB_RTC_CLK_FRE - 1);
    LIB_RTC_WAIT_TASK();
    // RTC 时间
    LL_RTC_TIME_Set(LIB_RTC, LIB_RTC_UNIX);
    LIB_RTC_WAIT_TASK();

    // RTC 中断
    #if LIB_RTC_IT_EN
        NVIC_SetPriority(LIB_RTC_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), LIB_RTC_IT_SEC_PREEM, LIB_RTC_IT_SEC_SUB));
        NVIC_EnableIRQ(LIB_RTC_IRQ);
        #if LIB_RTC_IT_SEC_EN // 秒中断
            LL_RTC_EnableIT_SEC(LIB_RTC);
            LIB_RTC_WAIT_TASK();
        #endif
    #endif

    // 若要使配置生效, 必须退出配置模式
    LL_RTC_EnableWriteProtection(LIB_RTC);
    LIB_RTC_WAIT_TASK();
}

void Lib_RTC_Set_Time(const Lib_RTC_UnixType ts)
{
    // 进行配置前, 必须进入配置模式
    LIB_RTC_WAIT_TASK();
    LL_RTC_DisableWriteProtection(LIB_RTC);
    LIB_RTC_WAIT_TASK();
    // RTC 时间
    LL_RTC_TIME_Set(LIB_RTC, LIB_RTC_UNIX);
    LIB_RTC_WAIT_TASK();
    // 若要使配置生效, 必须退出配置模式
    LL_RTC_EnableWriteProtection(LIB_RTC);
    LIB_RTC_WAIT_TASK();
}

#if LIB_RTC_IT_EN
    uint8_t Lib_RTC_IT_SEC_Flag;

    void Lib_RTC_Handler(void)
    {
        if (LL_RTC_IsActiveFlag_SEC(LIB_RTC) == SET)
        {
            Lib_RTC_IT_SEC_Flag = SET;
            LL_RTC_ClearFlag_SEC(LIB_RTC);
            LIB_RTC_WAIT_TASK();
        }
    }
#endif

// Unix 时间戳
static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#define is_leap_year(year)  ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))

Lib_RTC_UnixType Lib_RTC_Date2Unix(const Lib_RTC_DateType *dt)
{
    int64_t total_days = 0, res = 0;

    if (dt->year >= 1970)
    {
        for (uint16_t y = 1970; y < dt->year; ++y)
            total_days += is_leap_year(y) ? 366 : 365;
    }
    else
    {
        for (uint16_t y = dt->year; y < 1970; ++y)
            total_days -= is_leap_year(y) ? 366 : 365;
    }
    for (uint8_t m = 1; m < dt->month; ++m)
        total_days += days_in_month[m - 1] + ((m == 2 && is_leap_year(dt->year)) ? 1 : 0);
    total_days += dt->day - 1;
    res = total_days * 86400
        + (int64_t)dt->hour * 3600
        + (int64_t)dt->minute * 60
        + (int64_t)dt->second
        - (int64_t)LIB_RTC_TIMEZONE * 3600;

    if (res < INT32_MIN)
        return INT32_MIN;
    else if (res > INT32_MAX)
        return INT32_MAX;
    else
        return (Lib_RTC_UnixType)res;
}

void Lib_RTC_Unix2Date(const Lib_RTC_UnixType ts, Lib_RTC_DateType* const dt)
{
    int64_t modified_ts = (int64_t)ts + LIB_RTC_TIMEZONE * 3600;
    int64_t total_days = modified_ts / 86400, secs_of_day = modified_ts % 86400;

    if (secs_of_day < 0)
    {
        secs_of_day += 86400;
        total_days -= 1;
    }
    dt->hour   = secs_of_day / 3600;
    dt->minute = (secs_of_day % 3600) / 60;
    dt->second = secs_of_day % 60;
    for (int32_t year = 1970; ;)
    {
        int32_t days_this_year = is_leap_year(year) ? 366 : 365;
        if (total_days >= days_this_year) 
        {
            total_days -= days_this_year;
            ++year;
        }
        else if (total_days < 0) 
        {   
            total_days += days_this_year;
            --year;
        }
        else 
        {
            dt->year = year;
            break;
        }
    }
    for (uint32_t month = 1; month <= 12; month++) 
    {
        int32_t tmp = days_in_month[month - 1];
        if (month == 2 && is_leap_year(dt->year))
            tmp += 1;
        if (total_days >= tmp)
            total_days -= tmp;
        else
        {
            dt->month = month;
            break;
        }
    }

    dt->day = total_days + 1;
}

uint8_t Lib_RTC_Check_Same_Date(const Lib_RTC_DateType* const dt1, const Lib_RTC_DateType* const dt2)
{
    if (dt1->year != dt2->year) return 0;
    if (dt1->month != dt2->month) return 0;
    if (dt1->day != dt2->day) return 0;
    if (dt1->hour != dt2->hour) return 0;
    if (dt1->minute != dt2->minute) return 0;
    if (dt1->second != dt2->second) return 0;
    return 1;
}