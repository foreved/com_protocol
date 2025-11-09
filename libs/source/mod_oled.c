#include <stdarg.h>
#include "mod_oled.h"
#include "lib_font.h"
#include "lib_tool.h"
#include "lib_usart.h"

static void Mod_Oled_Set_Addr_Mode(const uint8_t mode);
static Mod_Oled_Pos_Type Mod_Oled_Show_Char(const Mod_Oled_Pos_Type pos, const uint8_t ch);
static void Mod_Oled_Set_Pos(const Mod_Oled_Pos_Type pos);

// 使用的字模
#define MOD_OLED_CHARS Fixedsys_ASCII_Chars_8x16
#define MOD_OLED_CHARS_BG (' ')

// 数据序列缓冲区
static uint8_t Mod_Oled_Buffer[MOD_OLED_BUFFER_SIZE];
// 管理缓冲区的指针
static uint8_t Mod_Oled_PBuffer;

/*
 * @brief   OLED 上电后, 需要进行初始化配置才能使用
 */
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
            0x8D, 0x14};

    // 上电后, 延时 1s 再配置
    Lib_Tool_SysTick_Delay_ms(1000);
    Mod_Oled_Send_Data(cmd_list, sizeof(cmd_list));
    Mod_Oled_Clear_Screen();
    Mod_Oled_Display_Control(1);

    // 将寻址模式设置为水平
    Mod_Oled_Set_Addr_Mode(MOD_OLED_CMD_MOD_HORIZONTAL);
}

/*
 * @brief   控制 OLED 是否显示
 * @param   opt --0: 不显示; --1: 显示
 */
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

/*
 * @brief   设置内存寻址模式
 * @param   mode 模式: 水平寻址或垂直寻址
 */
static void Mod_Oled_Set_Addr_Mode(const uint8_t mode)
{
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ALWAYS_CMD);
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_SET_MOD);
    Mod_Oled_Buffer_Append(mode);
    Mod_Oled_Buffer_Send();
}

/*
 * @brief   显示单个字符, 字体由 MOD_OLED_CHARS 决定
 * @param   pos 指定的显示坐标
 *          ch  显示的字符
 * @return  成功显示字符的坐标的下一个字符位
 */
static Mod_Oled_Pos_Type Mod_Oled_Show_Char(const Mod_Oled_Pos_Type pos, const uint8_t ch)
{
    const uint8_t *const arr = (uint8_t *)MOD_OLED_CHARS[ch - MOD_OLED_CHARS_BG];
    Mod_Oled_Pos_Type addr = pos;

    // 能否完整显示一个字符
    if ((addr.column + 8) > 127)
    {
        addr.page += 2; // 一个字符大小为 8x16, 占两页
        addr.column = 0;
        if (addr.page > 7)
            addr.page = 0;
    }

    // 前8个字节, 在第一页
    Mod_Oled_Set_Pos(addr);
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ALWAYS_DATA);
    for (uint8_t i = 0; i < 8; ++i)
        Mod_Oled_Buffer_Append(arr[i]);
    Mod_Oled_Buffer_Send();

    // 后8个字节, 在第二页
    addr.page += 1;
    Mod_Oled_Set_Pos(addr);
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ALWAYS_DATA);
    for (uint8_t i = 8; i < 16; ++i)
        Mod_Oled_Buffer_Append(arr[i]);
    Mod_Oled_Buffer_Send();

    // 不需要保证完整显示字符, 再次调用本函数会判断
    addr.page -= 1;
    addr.column += 8;
    return addr;
}

/*
 * @brief   显示字符串, 字体由 MOD_OLED_CHARS 决定
 * @param   pos 指定的显示坐标
 *          ch  显示的字符
 * @return  成功显示的最后一个字符, 它的坐标的下一个字符位
 */
Mod_Oled_Pos_Type Mod_Oled_Show_String(const Mod_Oled_Pos_Type pos, const char *const str)
{
    Mod_Oled_Pos_Type addr = pos;
    for (uint8_t i = 0; str[i] != '\0'; ++i)
    {
        addr = Mod_Oled_Show_Char(addr, str[i]);
    }
    return addr;
}

/*
 * @brief   设置光标坐标
 * @param   pos 光标所在位置
 */
static void Mod_Oled_Set_Pos(const Mod_Oled_Pos_Type pos)
{
    Mod_Oled_Buffer_Clear();
    Mod_Oled_Buffer_Append(MOD_OLED_CTRL_ALWAYS_CMD);

    // 页
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_ADDR_SET_PAGE);
    Mod_Oled_Buffer_Append(pos.page);
    Mod_Oled_Buffer_Append(7);

    // 列
    Mod_Oled_Buffer_Append(MOD_OLED_CMD_ADDR_SET_COLUMN);
    Mod_Oled_Buffer_Append(pos.column);
    Mod_Oled_Buffer_Append(127);

    Mod_Oled_Buffer_Send();
}

/*
 * @brief   用 data 填充屏幕
 * @param   常用: 0x00 -- 清屏; 0xFF -- 检查屏幕坏点
 */
void Mod_Oled_Fill_Screen(const uint8_t data)
{
    uint8_t arr[129] = {MOD_OLED_CTRL_ALWAYS_DATA, 0};
    if (data > 0)
    {
        for (uint8_t i = 1; i < 129; ++i)
            arr[i] = data;
    }

    Mod_Oled_Set_Pos((Mod_Oled_Pos_Type){0, 0});
    for (uint8_t i = 0; i < 8; ++i)
        Mod_Oled_Send_Data(arr, 129);
}

/*
 * @brief   显示格式化字符串, 字体由 MOD_OLED_CHARS 决定
 * @param   pos 指定的显示坐标
 *          ch  显示的字符
 *          ... --%d: 十进制表示符号32位整型
 *              --%x: 十六进制表示符号32位整型
 *              --%.xf: x 位浮点数
 *              --%s: 字符串
 * @return  成功显示的最后一个字符, 它的坐标的下一个字符位
 */
Mod_Oled_Pos_Type Mod_Oled_Show_fString(const Mod_Oled_Pos_Type pos, const char *const str, ...)
{
    Mod_Oled_Pos_Type addr = pos;
    int arg_int = 0;           // 整型参数
    double arg_double = 0.0;   // 浮点型
    char *arg_str = (void *)0; // 字符串
    uint8_t buffer[20] = {0};  // 缓冲区

    va_list ap;        // 声明ap容纳不定参数
    va_start(ap, str); // 初始化ap

    for (uint8_t i = 0; str[i] != '\0'; ++i)
    {
        // 非格式化字符
        if (str[i] != '%')
        {
            addr = Mod_Oled_Show_Char(addr, str[i]);
        }
        else // 格式化字符
        {
            ++i;
            switch (str[i])
            {
            // 字符 %
            case '%':
                addr = Mod_Oled_Show_Char(addr, '%');
                continue;
            // 十进制整型
            case 'd':
                arg_int = va_arg(ap, int);
                Lib_USART_Int2Char_DEC(arg_int, buffer);
                break;
            // 十六进制整型
            case 'x':
                arg_int = va_arg(ap, int);
                Lib_USART_Int2Char_HEX(arg_int, buffer);
                break;
            // 字符串
            case 's':
                arg_str = va_arg(ap, char *);
                addr = Mod_Oled_Show_String(addr, arg_str);
                continue;
            // 浮点数
            case '.':
                // %.3f -- 3位小数
                arg_double = va_arg(ap, double);
                // 默认打印3位小数
                Lib_USART_Double2Char(arg_double, buffer, str[i + 1] - '0');
                i += 2;
                break;
            default:
                // 报错，待实现
                break;
            }
            // 发送buffer数据
            for (uint8_t j = 0; buffer[j] != '\0'; ++j)
            {
                addr = Mod_Oled_Show_Char(addr, buffer[j]);                
            }
        }
    }
    va_end(ap); // 释放ap

    return addr;
}