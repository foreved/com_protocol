#ifndef _MOD_FLASH_H
#define _MOD_FLASH_H

#include "lib_spi.h"

#define Mod_Flash_COM_Start()                                   LIB_SPI_START()           // 开始通信
#define Mod_Flash_COM_Stop()                                    LIB_SPI_STOP()            // 结束通信
#define Mod_Flash_Send_Byte(data)                               Lib_SPI_Send_Byte(data)   // 发送一个字节
#define Mod_Flash_Reveive_Byte()                                Lib_SPI_Receive_Byte()    // 接收一个字节

// W25Q64 指令
#define MOD_FLASH_W25Q64_WRITE_ENABLE							0x06
#define MOD_FLASH_W25Q64_WRITE_DISABLE						    0x04
#define MOD_FLASH_W25Q64_READ_STATUS_REGISTER_1				    0x05
#define MOD_FLASH_W25Q64_READ_STATUS_REGISTER_2				    0x35
#define MOD_FLASH_W25Q64_WRITE_STATUS_REGISTER				    0x01
#define MOD_FLASH_W25Q64_PAGE_PROGRAM							0x02
#define MOD_FLASH_W25Q64_QUAD_PAGE_PROGRAM					    0x32
#define MOD_FLASH_W25Q64_BLOCK_ERASE_64KB						0xD8
#define MOD_FLASH_W25Q64_BLOCK_ERASE_32KB						0x52
#define MOD_FLASH_W25Q64_SECTOR_ERASE_4KB						0x20
#define MOD_FLASH_W25Q64_CHIP_ERASE							    0xC7
#define MOD_FLASH_W25Q64_ERASE_SUSPEND						    0x75
#define MOD_FLASH_W25Q64_ERASE_RESUME							0x7A
#define MOD_FLASH_W25Q64_POWER_DOWN							    0xB9
#define MOD_FLASH_W25Q64_HIGH_PERFORMANCE_MODE				    0xA3
#define MOD_FLASH_W25Q64_CONTINUOUS_READ_MODE_RESET			    0xFF
#define MOD_FLASH_W25Q64_RELEASE_POWER_DOWN_HPM_DEVICE_ID		0xAB
#define MOD_FLASH_W25Q64_MANUFACTURER_DEVICE_ID				    0x90
#define MOD_FLASH_W25Q64_READ_UNIQUE_ID						    0x4B
#define MOD_FLASH_W25Q64_JEDEC_ID								0x9F
#define MOD_FLASH_W25Q64_READ_DATA							    0x03
#define MOD_FLASH_W25Q64_FAST_READ							    0x0B
#define MOD_FLASH_W25Q64_FAST_READ_DUAL_OUTPUT				    0x3B
#define MOD_FLASH_W25Q64_FAST_READ_DUAL_IO					    0xBB
#define MOD_FLASH_W25Q64_FAST_READ_QUAD_OUTPUT				    0x6B
#define MOD_FLASH_W25Q64_FAST_READ_QUAD_IO					    0xEB
#define MOD_FLASH_W25Q64_OCTAL_WORD_READ_QUAD_IO				0xE3

#endif