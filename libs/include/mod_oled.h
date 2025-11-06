#ifndef _MOD_OLED_H
#define _MOD_OLED_H

#include "lib_i2c.h"

#define MOD_OLED_ADDR                        0x3C
#define MOD_OLED_BUFFER                      LIB_I2C_BUFFER
#define MOD_OLED_PBUFFER                     LIB_I2C_PBUFFER
#define Mod_Oled_Buffer_Clear()              (MOD_OLED_PBUFFER = 0)
#define Mod_Oled_Buffer_Append(data)         (MOD_OLED_BUFFER[MOD_OLED_PBUFFER++] = data)
#define Mod_Oled_Buffer_Send()               Lib_I2C_Send_Data(MOD_OLED_ADDR, MOD_OLED_BUFFER, MOD_OLED_PBUFFER)
#define Mod_Oled_Send_Data(pbuffer, num)     Lib_I2C_Send_Data(MOD_OLED_ADDR, (const uint8_t* const)pbuffer, num)

void Mod_Oled_Power_Up(void);
void Mod_Oled_Display_Control(const uint8_t opt);
void Mod_Oled_Set_Addr_Mode(const uint8_t mode);
void Mod_Oled_Set_Page(const uint8_t start_page, const uint8_t end_page);
void Mod_Oled_Set_Column(const uint8_t start_column, const uint8_t end_column);
void Mod_Oled_Show_String(const uint8_t page, const uint8_t column, const char *const str);
void Mod_Oled_Set_Pos(const uint8_t page, const uint8_t column);
void Mod_Oled_Fill_Screen(const uint8_t data);
#define Mod_Oled_Clear_Screen()              Mod_Oled_Fill_Screen(0x00)
#define Mod_Oled_Full_Screen()               Mod_Oled_Fill_Screen(0xFF)

// 控制字节
// bit7: 0 表示只有一个控制字节, 之后是若干个指令/数据字节; 1 表示一个控制字节+一个指令/数据字节
// bit6: 0 表示后续是指令; 1 表示后续是数据
// bit5~0: 无意义
#define MOD_OLED_CTRL_ALWAYS_CMD               0x00   // 一个控制 + 若干个指令
#define MOD_OLED_CTRL_ONCE_CMD                 0x80   // 一个控制 + 一个指令
#define MOD_OLED_CTRL_ALWAYS_DATA              0x40   // 一个控制 + 若干个数据
#define MOD_OLED_CTRL_ONCE_DATA                0xC0   // 一个控制 + 一个数据

// 常用指令
// 显示控制
#define MOD_OLED_CMD_DISPLAY_ON                0xAF   // 开启显示
#define MOD_OLED_CMD_DISPLAY_OFF               0xAE   // 关闭显示
// 设置寻址方式
#define MOD_OLED_CMD_SET_MOD                   0x20   // 设置内存寻址模式
#define MOD_OLED_CMD_MOD_HORIZONTAL            0X00   // 水平寻址 (自动换页)
#define MOD_OLED_CMD_MOD_VERTICAL              0x01   // 垂直寻址 (自动换列)
// #define MOD_OLED_CMD_MOD_PAGE                  0x02   // 页寻址 (不换页)
// 设置地址 (水平或垂直寻址)
#define MOD_OLED_CMD_ADDR_SET_PAGE             0x22   // 设置页地址
#define MOD_OLED_CMD_ADDR_SET_COLUMN           0x21   // 设置列地址


#endif