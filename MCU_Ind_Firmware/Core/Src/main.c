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
#include "adc.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7735.h"
#include "fonts.h"
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
extern uint8_t SPI_rx_buf[1];
extern uint8_t SPI_tx_buf[1];
uint8_t flag_output_spi1 = 0;

uint8_t start_device = 1;	//Флаг запуска устройства, чтобы запросить включить подсветку дисплея

uint8_t r =  1;				//Переменная ориентации дисплея
uint8_t DOWN = 0;			//Флаг нажатия кнопки "Вниз"
uint8_t UP = 0;				//Флаг нажатия кнопки "Вверх"
uint8_t ENTER = 0;			//Флаг нажатия кнопки "Ввод"
uint8_t BACK = 0;			//Флаг нажатия кнопки "Назад"
uint32_t time = 0;			//Переменная задержки

uint8_t choice = 0;			//Переменная выбранного пункта меню
uint8_t idMenu[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};			//Флаг возврата в главное меню
uint8_t Number_Menu = 1;	//Переменная номера меню в котором находимся
uint8_t Sub_Menu = 0;		//Переменная номера под меню в котором находимся

uint16_t What_Time = 0;
uint16_t What_Date = 0;
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
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  	ST7735_Init();
	ST7735_SetRotation(r);
	loading();
	//Init_Card();
	//HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	time = 200;
	HAL_Delay(4750);
	Menu_Main();
	HAL_TIM_Base_Start_IT(&htim1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
    	BackLight(Get_ADC1());		//BackLight((4095 - Get_ADC1()));
    	DataUpdate();			//Подумать про обновление дисплея
    	if(Number_Menu == 1)	//Главное меню
    	{
    		if(idMenu[0])
    		{
    			Menu_Main();
    			idMenu[0] = 0;
    		}
    		if(DOWN)
    		{
    			choice = Main_Menu(DOWN, UP, time);
    			DOWN = 0;
    		}
    		else if(UP)
    		{
    			choice = Main_Menu(DOWN, UP, time);
    			UP = 0;
    		}
    	}
    	else if(Number_Menu == 2)	//Подменю: "Выводы базового блока"
    	{
    		if(idMenu[1])
    		{
    			Menu_Output();
    			idMenu[1] = 0;
    		}
    		if(DOWN)
    		{
    			choice = One_Menu(DOWN, UP, time);
    			DOWN = 0;
    		}
    		else if(UP)
    		{
    			choice = One_Menu(DOWN, UP, time);
    			UP = 0;
    		}
    	}
    	else if(Number_Menu == 3)	//Подменю: "Подключенные блоки"
    	{
    		if(idMenu[2])
    		{
    			Menu_Blocks();
    			idMenu[2] = 0;
    		}
    		if(DOWN)
    		{
    			choice = Two_Menu(DOWN, UP, time);
    			DOWN = 0;
    		}
    		else if(UP)
    		{
    			choice = Two_Menu(DOWN, UP, time);
    			UP = 0;
    		}
    	}
    	else if(Number_Menu == 4)	//Подменю: "Аналоговые входы"
    	{
    		if(idMenu[3])
    		{
    			Menu_Analog();
    			idMenu[3] = 0;
    		}
    		if(DOWN)
    		{
    			choice = Three_Menu(DOWN, UP, time);
    			DOWN = 0;
    		}
    		else if(UP)
    		{
    			choice = Three_Menu(DOWN, UP, time);
    			UP = 0;
    		}
    	}
    	else if(Number_Menu == 5)	//Подменю: "Цифровые входы"
    	{
    		if(idMenu[4])
    		{
    			Menu_Digital();
    			idMenu[4] = 0;
    		}
    		if(DOWN)
    		{
    			choice = Four_Menu(DOWN, UP, time);
    			DOWN = 0;
    		}
    		else if(UP)
    		{
    			choice = Four_Menu(DOWN, UP, time);
    			UP = 0;
    		}
    	}
    	else if(Number_Menu == 6)	//Подменю: "Открытый коллектор"
    	{
    		if(idMenu[5])
    		{
    			Menu_OpenDrain();
    			idMenu[5] = 0;
    		}
    		if(DOWN)
    		{
    			choice = Five_Menu(DOWN, UP, time);
    			DOWN = 0;
    		}
    		else if(UP)
    		{
    			choice = Five_Menu(DOWN, UP, time);
    			UP = 0;
    		}
    	}
    	else if(Number_Menu == 7)	//Подменю: "1-Wire"
    	{
    		if(idMenu[6])
    		{
    			Menu_1Wire();
    			idMenu[6] = 0;
    		}
    		if(DOWN)
    		{
    			choice = Six_Menu(DOWN, UP, time);
    			DOWN = 0;
    		}
    		else if(UP)
    		{
    			choice = Six_Menu(DOWN, UP, time);
    			UP = 0;
    		}
    	}
    	else if(Number_Menu == 8)	//Подменю: "PWM"
    	{
    		if(idMenu[7])
    		{
    			Menu_PWM();
    			idMenu[7] = 0;
    		}
    		if(DOWN)
    		{
    			choice = Seven_Menu(DOWN, UP, time);
    			DOWN = 0;
    		}
    		else if(UP)
    		{
    			choice = Seven_Menu(DOWN, UP, time);
    			UP = 0;
    		}
    	}
    	else if(Number_Menu == 9)	//Подменю: "RELAY"
    	{
    		if(idMenu[8])
    		{
    			idMenu[8] = 0;
    		}
    	}
    	if(ENTER)
    	{
    		Number_Menu = Item_Selection(choice);
    		if((Number_Menu >= 13) && (Number_Menu <= 20))
    		{
    			Visible_Analog();
    			Number_Menu = 4;
    		}
    		if((Number_Menu >= 21) && (Number_Menu <= 28))
    		{
    			Visible_Digital();
    			Number_Menu = 5;
    		}
    		else if((Number_Menu >= 29) && (Number_Menu <= 32))
    		{
    			Visible_PWM();
    			Number_Menu = 8;
    		}
    		else if((Number_Menu >= 33) && (Number_Menu <= 34))
    		{
    			Visible_1Wire();
    			Number_Menu = 7;
    		}
    		else if((Number_Menu >= 35) && (Number_Menu <= 42))
    		{
    			Visible_OpenDrain();
    			Number_Menu = 6;
    		}
    		HAL_Delay(time);
    		ENTER = 0;
    	}
    	if(BACK)
    	{
    		if(Number_Menu == 4)
    		{
    			Number_Menu = 2;
    			idMenu[1] = 1;
    		}
    		else if(Number_Menu == 2)
    		{
    			Number_Menu = 1;
    			idMenu[0] = 1;
    		}
    		else if(Number_Menu == 3)
    		{
    			Number_Menu = 1;
    			idMenu[0] = 1;
    		}
    		if(Number_Menu == 1)
    		{
    			idMenu[0] = 1;
    		}
    		if(Number_Menu == 5)
    		{
    			Number_Menu = 2;
    			idMenu[1] = 1;
    		}
    		if(Number_Menu == 6)
    		{
    			Number_Menu = 2;
    			idMenu[1] = 1;
    		}
    		if(Number_Menu == 7)
    		{
    			Number_Menu = 2;
    			idMenu[1] = 1;
    		}
    		if(Number_Menu == 8)
    		{
    			Number_Menu = 2;
    			idMenu[1] = 1;
    		}
    		if(Number_Menu == 9)
    		{
    			Number_Menu = 3;
    			idMenu[2] = 1;
    		}
    		if(Number_Menu == 10)
    		{
    			Number_Menu = 3;
    			idMenu[2] = 1;
    		}
    		if(Number_Menu == 11)
    		{
    			Number_Menu = 3;
    			idMenu[2] = 1;
    		}
    		if(Number_Menu == 12)
    		{
    			Number_Menu = 3;
    			idMenu[2] = 1;
    		}
    		HAL_Delay(time);
    		BACK = 0;
    	}
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_9)
	{
		if((GPIOA->IDR & GPIO_PIN_9) != 1)
		{
			if((GPIOA->IDR & GPIO_PIN_9) != 1)
				DOWN = 1;
		}
	}
	else if (GPIO_Pin == GPIO_PIN_8)
	{
		if((GPIOA->IDR & GPIO_PIN_8) != 1)
		{
			if((GPIOA->IDR & GPIO_PIN_8) != 1)
				UP = 1;
		}
	}
	else if (GPIO_Pin == GPIO_PIN_10)
	{
		if((GPIOA->IDR & GPIO_PIN_10) != 1)
		{
			if((GPIOA->IDR & GPIO_PIN_10) != 1)
				ENTER = 1;
		}
	}
	else if (GPIO_Pin == GPIO_PIN_11)
	{
		if((GPIOA->IDR & GPIO_PIN_11) != 1)
		{
			if((GPIOA->IDR & GPIO_PIN_11) != 1)
				BACK = 1;
		}
	}
	else
	{
		__NOP();
	}
	What_Time = 0;
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1) //check if the interrupt comes from TIM1
    {
		What_Time++;
		What_Date++;

		if(What_Date == 60)	//Запрос данных каждую 1 минуту
        {
			if(Number_Menu == 1)
			{
				Str_Time();
			}
			else
				__NOP();
			What_Date = 0;
        }
		if(What_Time == 300)	//Запрос данных каждые 5 минут
        {
        	//Возврат на главное меню по истечении времени
			if(Number_Menu > 1)
			{
				Menu_Main();
				Number_Menu = 1;
				//idMenu[0] = 0;
				What_Time = 0;
			}
			else
				__NOP();
			What_Time = 0;
        }
    }
}
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi == &hspi1)
	{
		if(hspi1.TxXferCount == 0)
		{
			CSM_H;
			flag_output_spi1 = 1;
		}
	}
}
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
