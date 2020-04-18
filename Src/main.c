/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */
UART_HandleTypeDef UartHandle;
TS_StateTypeDef  TS_State={0};
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
static void MX_GPIO_Init(void);
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
	  uint8_t  lcd_status = LCD_OK;
	  uint32_t ts_status  = TS_OK;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  UartHandle.Instance          = USARTx;

  UartHandle.Init.BaudRate     = 115200;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

  if(HAL_UART_DeInit(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }

  /* Initialize the SDRAM */
  if (BSP_SDRAM_Init() != SDRAM_OK)
  {
    /* User can add here some code to deal with this error */
    while (1)
    {
    }
  }

  /*##-1- LCD DSI initialization ########################################*/

  /* Initialize and start the LCD display in mode 'lcd_mode'
   *  Using LCD_FB_START_ADDRESS as frame buffer displayed contents.
   *  This buffer is modified by the BSP (draw fonts, objects depending on BSP calls).
   */

  /* Set Portrait orientation if needed, by default orientation is set to
     Landscape */

  /* Initialize DSI LCD */
  //  BSP_LCD_InitEx(LCD_ORIENTATION_PORTRAIT); /* uncomment if Portrait orientation is needed */
  lcd_status = BSP_LCD_Init(); /* Uncomment if default config (landscape orientation) is needed */
  while(lcd_status != LCD_OK);

  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_SelectLayer(0);

  /*##-2- Touch screen initialization ########################################*/
  BSP_TS_ResetTouchData(&TS_State);

  /* If calibration is not yet done, proceed with calibration */
  if (TouchScreen_IsCalibrationDone() == 0)
  {
    ts_status = Touchscreen_Calibration();
    if(ts_status == TS_OK)
    {
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 65, (uint8_t *)"Touchscreen calibration success.", CENTER_MODE);
    }
  } /* of if (TouchScreen_IsCalibrationDone() == 0) */


  /* USER CODE END 2 */
  kws_main_realtime();//kws_main_static();

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
	  RCC_ClkInitTypeDef RCC_ClkInitStruct;
	  RCC_OscInitTypeDef RCC_OscInitStruct;
	  HAL_StatusTypeDef  hal_status = HAL_OK;

	  /* Enable Power Control clock */
	  __HAL_RCC_PWR_CLK_ENABLE();

	  /* The voltage scaling allows optimizing the power consumption when the device is
	     clocked below the maximum system frequency, to update the voltage scaling value
	     regarding system frequency refer to product datasheet.  */
	  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	  /* Enable HSE Oscillator and activate PLL with HSE as source */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
	  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
	#if defined(USE_STM32469I_DISCO_REVA)
	  RCC_OscInitStruct.PLL.PLLM = 25;
	#else
	  RCC_OscInitStruct.PLL.PLLM = 8;
	#endif /* USE_STM32469I_DISCO_REVA */
	  RCC_OscInitStruct.PLL.PLLN       = 360;
	  RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2; /* to have ck_pll = 360 MHz / 2 = 180 MHz */
	  RCC_OscInitStruct.PLL.PLLQ       = 7;
	  RCC_OscInitStruct.PLL.PLLR       = 6; /* to have ck_plllcd = 60 MHz : replace DPHY PLL clock when this PLL is Off in ULPM mode */
	  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	  {
		  Error_Handler();
	  }

	  /* Activate the Over-Drive mode */
	  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
	  {
		  Error_Handler();
	  }

	  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	     clocks dividers */
	  RCC_ClkInitStruct.ClockType =  (RCC_CLOCKTYPE_SYSCLK |
	                                 RCC_CLOCKTYPE_HCLK   |
	                                 RCC_CLOCKTYPE_PCLK1  |
	                                 RCC_CLOCKTYPE_PCLK2);

	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

	  /* Set nominal timing configurations for STM32F469xx */
	  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1; /* f(hclk) = 180 MHz / 1 = 180 MHz        */
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;   /* f(pclk1) = f(hclk)/4  = 180/4 = 45 MHz */
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;   /* f(pclk2) = f(hclk)/2  = 90 MHz         */

	  /* Set Flash latency parameters (wait states) depending on CPU clock = ck_sys = 180 MHz */
	  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	  {
		  Error_Handler();
	  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();

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

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
