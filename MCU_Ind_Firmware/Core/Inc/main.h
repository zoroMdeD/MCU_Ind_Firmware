/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "rtc.h"
#include "st7735.h"
#include "fonts.h"
#include <menu.h>
#include <command.h>
#include <stdlib.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PWM_L_Pin GPIO_PIN_0
#define PWM_L_GPIO_Port GPIOA
#define ADC1_Pin GPIO_PIN_1
#define ADC1_GPIO_Port GPIOA
#define CS__M_Pin GPIO_PIN_4
#define CS__M_GPIO_Port GPIOA
#define SCK_M_Pin GPIO_PIN_5
#define SCK_M_GPIO_Port GPIOA
#define MISO_M_Pin GPIO_PIN_6
#define MISO_M_GPIO_Port GPIOA
#define MOSI_M_Pin GPIO_PIN_7
#define MOSI_M_GPIO_Port GPIOA
#define CS__F_Pin GPIO_PIN_0
#define CS__F_GPIO_Port GPIOB
#define WP__F_Pin GPIO_PIN_1
#define WP__F_GPIO_Port GPIOB
#define FLAG_MSG_Pin GPIO_PIN_10
#define FLAG_MSG_GPIO_Port GPIOB
#define CS__L_Pin GPIO_PIN_12
#define CS__L_GPIO_Port GPIOB
#define SCK_L_Pin GPIO_PIN_13
#define SCK_L_GPIO_Port GPIOB
#define MOSI_L_Pin GPIO_PIN_15
#define MOSI_L_GPIO_Port GPIOB
#define BT1_Pin GPIO_PIN_8
#define BT1_GPIO_Port GPIOA
#define BT1_EXTI_IRQn EXTI9_5_IRQn
#define BT2_Pin GPIO_PIN_9
#define BT2_GPIO_Port GPIOA
#define BT2_EXTI_IRQn EXTI9_5_IRQn
#define BT3_Pin GPIO_PIN_10
#define BT3_GPIO_Port GPIOA
#define BT3_EXTI_IRQn EXTI15_10_IRQn
#define BT4_Pin GPIO_PIN_11
#define BT4_GPIO_Port GPIOA
#define BT4_EXTI_IRQn EXTI15_10_IRQn
#define RST_L_Pin GPIO_PIN_4
#define RST_L_GPIO_Port GPIOB
#define DC_L_Pin GPIO_PIN_5
#define DC_L_GPIO_Port GPIOB
#define ERR_Pin GPIO_PIN_6
#define ERR_GPIO_Port GPIOB
#define OK_Pin GPIO_PIN_7
#define OK_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
//------------------------Открытый коллектор------------------------
#define OC0_ON		0x01
#define OC0_OFF		0x02
#define OC1_ON		0x03
#define OC1_OFF		0x04
#define OC2_ON		0x05
#define OC2_OFF		0x06
#define OC3_ON		0x07
#define OC3_OFF		0x08
#define OC4_ON		0x09
#define OC4_OFF		0x10
#define OC5_ON		0x11
#define OC5_OFF		0x12
#define OC6_ON		0x13
#define OC6_OFF		0x14
#define OC7_ON		0x15
#define OC7_OFF		0x16
//------------------------------------------------------------------
//--------------------------Цифровые входы--------------------------
#define DIN0_ON		0x17
#define DIN0_OFF	0x18
#define DIN1_ON		0x19
#define DIN1_OFF	0x20
#define DIN2_ON		0x21
#define DIN2_OFF	0x22
#define DIN3_ON		0x23
#define DIN3_OFF	0x24
#define DIN4_ON		0x25
#define DIN4_OFF	0x26
#define DIN5_ON		0x27
#define DIN5_OFF	0x28
#define DIN6_ON		0x29
#define DIN6_OFF	0x30
#define DIN7_ON		0x31
#define DIN7_OFF	0x32
//------------------------------------------------------------------
//-------------------------Аналоговые входы-------------------------
#define AIN0_ON		0x33
#define AIN0_OFF	0x34
#define AIN1_ON		0x35
#define AIN1_OFF	0x36
#define AIN2_ON		0x37
#define AIN2_OFF	0x38
#define AIN3_ON		0x39
#define AIN3_OFF	0x40
#define AIN4_ON		0x41
#define AIN4_OFF	0x42
#define AIN5_ON		0x43
#define AIN5_OFF	0x44
#define AIN6_ON		0x45
#define AIN6_OFF	0x46
#define AIN7_ON		0x47
#define AIN7_OFF	0x48
//------------------------------------------------------------------
//----------------------------Выходы ШИМ----------------------------
#define PWM0_ON		0x49
#define PWM0_OFF	0x50
#define PWM1_ON		0x51
#define PWM1_OFF	0x52
#define PWM2_ON		0x53
#define PWM2_OFF	0x54
#define PWM3_ON		0x55
#define PWM3_OFF	0x56
//------------------------------------------------------------------
//--------------------------Выходы 1-Wire---------------------------
#define WR0_ON		0x57
#define WR0_OFF		0x58
#define WR1_ON		0x59
#define WR1_OFF		0x60
//------------------------------------------------------------------
//----------------------------GSM Modem-----------------------------
#define GSM_ON		0x61
#define GSM_OFF		0x62
//------------------------------------------------------------------
//------------------Проверка подключения модулей--------------------
#define RELAY_TEST		0x63
#define DIMMING_TEST	0x64
#define DIGITAL_TEST	0x65
#define INTERFACE_TEST	0x66
//------------------------------------------------------------------
//----------------------Запрос даты и времени-----------------------
#define MCU_TIME	0x67
//------------------------------------------------------------------
//---------------------CS# Микросхемы памяти------------------------
#define CSF_L	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)	//CS = Low;
#define CSF_H 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)		//CS = High;
#define WP_L	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)	//WP = Low;
#define WP_H	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)		//WP = High;
//------------------------------------------------------------------
//------------------------CS# Контроллера---------------------------
#define CSM_L	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)	//CS = Low;
#define CSM_H 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)		//CS = High;
//------------------------------------------------------------------
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
