/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lib_spi.h"
#include "mod_flash.h"
#include "lib_usart.h"
#include "ff.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  FATFS fs;           // 文件系统对象. 一般, 一个物理设备用一个 FATFS 描述
  FIL file;           // 文件对象, 包含了文件的基本信息
  FRESULT fres;       // 文件操作结果
  UINT fnum;          // 文件成功读写数量
  BYTE fbuffer[1024]; // 读写缓冲区
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /** NOJTAG: JTAG-DP Disabled and SW-DP Enabled
  */
  LL_GPIO_AF_Remap_SWJ_NOJTAG();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  /* USER CODE BEGIN 2 */
  Lib_USART_Init();

  // 文件系统格式化
  fres = f_mount(&fs, "0:", 1); // 将逻辑驱动器挂载到 FATFS
  if (fres == FR_NO_FILESYSTEM) // 该逻辑驱动器还没有文件系统
  {
    Lib_USART_Send_String("Flash has no FAT. Initialize...\n");
    // 创建文件系统
    BYTE work[FF_MAX_SS]; // 创建文件系统需要工作缓冲区, 最少为 FF_MAX_SS
    fres = f_mkfs("0:", (void*)0, work, FF_MAX_SS); // 使用默认参数初始化
    if (fres == FR_OK) // 成功创建
    {
      Lib_USART_Send_String("FAT created sucessfully.\n");
      fres = f_unmount("0:"); // 初始化文件系统后, 重新挂载
      fres = f_mount(&fs, "0:", 0);
    }
    else // 创建失败
    {
      Lib_USART_Send_String("Error: fail to create FAT. Stop...\n");
      while (1);
    }
  }
  else if (fres != FR_OK) // 已创建文件系统, 但挂载失败
  {
    Lib_USART_Send_String("Error: fail to mount device. Stop...\n");
    while (1);
  }
  else // 文件系统已创建, 且挂载成功
  {
    Lib_USART_Send_String("Succeed to mount device. Next to R/W.\n");
  }

  // 写测试
  Lib_USART_Send_String("Now to test writing.\n");
  // 以 "w" 模式打开文件
  fres = f_open(&file, "0:/test.txt", FA_CREATE_ALWAYS | FA_WRITE);
  if (fres == FR_OK) // 打开成功
  {
    const char str[] = "Hello world\nTest FatFs. Write\nRead.";
    fres = f_write(&file, str, sizeof(str), &fnum);
    if (fres == FR_OK)
    {
      Lib_USART_Send_fString("Succeed to write %d bytes.\n", fnum);
      Lib_USART_Send_fString("The data is\n\t %s\n", str);
    }
    else
    {
      Lib_USART_Send_String("Error: fail to open the file. Stop...\n");
      while (1);
    }
    f_close(&file);
  }

  // 读测试
    Lib_USART_Send_String("Now to test reading.\n");
  // 以 "r" 模式打开文件
  fres = f_open(&file, "0:/test.txt", FA_OPEN_EXISTING | FA_READ);
  if (fres == FR_OK) // 打开成功
  {
    fres = f_read(&file, fbuffer, sizeof(fbuffer), &fnum);
    fbuffer[fnum] = '\0'; // f_read() 不会自动加 '\0'
    if (fres == FR_OK)
    {
      Lib_USART_Send_fString("Succeed to read %d bytes.\n", fnum);
      Lib_USART_Send_fString("The data is\n\t %s\n", fbuffer);
    }
    else
    {
      Lib_USART_Send_String("Error: fail to open the file. Stop...\n");
      while (1);
    }
    f_close(&file);
  }

  f_unmount("0:");
  Lib_USART_Send_String("Succeed to test Fatfs.\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_Init1msTick(36000000);
  LL_SetSystemCoreClock(36000000);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
