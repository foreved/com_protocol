#include "mod_oled.h"
#include "ascii.h"
#include "lib_tool.h"

static void Mod_Oled_Show_Char(const uint8_t page, const uint8_t column, const uint8_t ch);

extern const uint8_t Fixedsys_ASCII_Chars_8x16[][16];

#define MOD_OLED_CHARS        Fixedsys_ASCII_Chars_8x16
#define MOD_OLED_CHARS_START  (' ')

// OLED 上电
void Mod_Oled_Power_Up(void)
{
    // 含义见手册
    uint8_t cmd_list[] =
    {
        MOD_OLED_CTRL_ALWAYS_CMD,
        0xAE,
        0xD5, 0x80,
        0xA8, 0x3F,
        0xD3, 0x00,
        0x40,
        0xA1,
        0xC8,
        0xDA, 0x12,
        0x81, 0xCF,
        0xD9, 0xF1,
        0xDB, 0x30,
        0xA4,
        0xA6,
        0x8D, 0x14
    };
    Lib_Tool_SysTick_Delay_ms(1000);
    Mod_Oled_Send_Data(cmd_list, sizeof(cmd_list));
    Mod_Oled_Clear_Screen();
    Mod_Oled_Display_Control(1);
}

// opt: 0表示关闭显示, 1表示开启显示
void Mod_Oled_Display_Control(const uint8_t opt)
{
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ALWAYS_CMD);
    if (opt == 1)
        Mod_Oled_Buffer_Append(MOD_OLED_CMD_DISPLAY_ON);
    else
        Mod_Oled_Buffer_Append(MOD_OLED_CMD_DISPLAY_OFF);
    Mod_Oled_Buffer_Send();
}

// 设置内存地址模式
// mode: 水平, 垂直寻址
void Mod_Oled_Set_Addr_Mode(const uint8_t mode)
{
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ALWAYS_CMD);
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_SET_MOD);
    Mod_Oled_Buffer_Append(mode);
    Mod_Oled_Buffer_Send();
}

//  仅用于水平或垂直寻址
// start_page, end_page: [0, 7]
void Mod_Oled_Set_Page(const uint8_t start_page, const uint8_t end_page)
{
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ALWAYS_CMD);
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_ADDR_SET_PAGE);
    Mod_Oled_Buffer_Append(start_page);
    Mod_Oled_Buffer_Append(end_page);
    Mod_Oled_Buffer_Send();
}

// 仅用于水平或垂直寻址
// start_column, end_column: [0, 127]
// 仅能显示可见字符, 不能使用 '\n'
void Mod_Oled_Set_Column(const uint8_t start_column, const uint8_t end_column)
{
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ALWAYS_CMD);
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_ADDR_SET_COLUMN);
    Mod_Oled_Buffer_Append(start_column);
    Mod_Oled_Buffer_Append(end_column);
    Mod_Oled_Buffer_Send();
}

static void Mod_Oled_Show_Char(const uint8_t page, const uint8_t column, const uint8_t ch)
{
    const uint8_t* const arr = (uint8_t*)MOD_OLED_CHARS[ch - MOD_OLED_CHARS_START];

    // 前8个字节
    Mod_Oled_Set_Pos(page, column);
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ALWAYS_DATA);
    for (uint8_t i = 0; i < 8; ++i) Mod_Oled_Buffer_Append(arr[i]);
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_ADDR_SET_PAGE);
    Mod_Oled_Buffer_Send();

    // 后8个字节
    Mod_Oled_Set_Pos(page + 1, column);
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ALWAYS_DATA);
    for (uint8_t i = 8; i < 16; ++i) Mod_Oled_Buffer_Append(arr[i]);
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_ADDR_SET_PAGE);
    Mod_Oled_Buffer_Send();
}

// 显示字符串
// page: [0, 7]; column: [0, 127]
// 可以使用 '\n'
void Mod_Oled_Show_String(const uint8_t page, const uint8_t column, const char* const str)
{
    uint8_t addr_page = page, addr_column = column;
    Mod_Oled_Set_Pos(addr_page, addr_column);
    for (uint8_t i = 0; str[i] != '\0'; ++i)
    {
        if (str[i] != '\n')
        {
            Mod_Oled_Show_Char(addr_page, addr_column, str[i]);
            addr_column += 8;
        }
        
        if ((addr_column + 8) > 127 || str[i] == '\n')
        {
            addr_page += 2;
            addr_column = 0;
            if (addr_page > 7) addr_page = 0;
        }
        Lib_Tool_SysTick_Delay_ms(100);
    }
}

// 设置写入位置
// 采用水平寻址
// page: [0, 7]; column: [0, 127]
void Mod_Oled_Set_Pos(const uint8_t page, const uint8_t column)
{
    Mod_Oled_Set_Addr_Mode(MOD_OLED_CMD_MOD_HORIZONTAL);
    Mod_Oled_Set_Page(page, 7);
    Mod_Oled_Set_Column(column, 127);
}

// data: 0x00表示清屏; 0xFF表示填满屏幕
void Mod_Oled_Fill_Screen(const uint8_t data)
{
    uint8_t arr[129] = {MOD_OLED_CTRL_ALWAYS_DATA, 0};
    if (data > 0)
    {
        for (uint8_t i = 1; i < 129; ++i)
            arr[i] = data;
    }

    Mod_Oled_Set_Pos(0, 0);
    for (uint8_t i = 0; i < 8; ++i)
        Mod_Oled_Send_Data(arr, 129);
}
