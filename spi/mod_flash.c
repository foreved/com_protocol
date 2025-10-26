#include "mod_flash.h"

uint32_t Mod_Flash_ReadID(uint8_t cmd)
{
    // 开始通信
    Mod_Flash_COM_Start();

    

    // 结束通信
    Mod_Flash_COM_Stop();
}