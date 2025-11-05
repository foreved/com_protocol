#include "mod_oled.h"
#include "ascii.h"

static void Mod_Oled_Show_Char(const uint8_t ch);

extern const uint8_t fixedsys_ascii_chars[][FIXEDSYS_ASCII_CHARS_LEN];

#define ASCII_CHARS        fixedsys_ascii_chars
#define ASCII_CHARS_LEN    FIXEDSYS_ASCII_CHARS_LEN
#define ASCII_CHARS_START  FIXEDSYS_ASCII_CHARS_START

// opt: 0表示关闭显示, 1表示开启显示
void Mod_Oled_Display_Control(const uint8_t opt)
{
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ONCE_CMD);
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
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ONCE_CMD);
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_SET_MOD);
    Mod_Oled_Buffer_Append(mode);
    Mod_Oled_Buffer_Send();
}

//  仅用于水平或垂直寻址
// start_page, end_page: [0, 7]
void Mod_Oled_Set_Page(const uint8_t start_page, const uint8_t end_page)
{
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ONCE_CMD);
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_ADDR_SET_PAGE);
    Mod_Oled_Buffer_Append(start_page);
    Mod_Oled_Buffer_Append(end_page);
    Mod_Oled_Buffer_Send();
}

// 仅用于水平或垂直寻址
// start_column, end_column: [0, 127]
void Mod_Oled_Set_Column(const uint8_t start_column, const uint8_t end_column)
{
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ONCE_CMD);
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_ADDR_SET_COLUMN);
    Mod_Oled_Buffer_Append(start_column);
    Mod_Oled_Buffer_Append(end_column);
    Mod_Oled_Buffer_Send();
}

static void Mod_Oled_Show_Char(const uint8_t ch)
{
    const uint8_t* arr = ASCII_CHARS[ch - ASCII_CHARS_START];
    for (uint8_t i = 0; i < ASCII_CHARS_LEN; ++i)
        Mod_Oled_Buffer_Append(arr[i]);
}

// 显示字符串
void Mod_Oled_Show_String(const char* const str)
{
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_NONCE_DATA);
    for (uint8_t i = 0; str[i] != '\0'; ++i)
        Mod_Oled_Show_Char(str[i]);
    Mod_Oled_Buffer_Send();
}