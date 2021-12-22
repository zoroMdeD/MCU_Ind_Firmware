/*
 * menu.c
 *
 *  Created on: 9 окт. 2020 г.
 *      Author: mmorozov
 */
#include "main.h"
#include "command.h"

//RTC
RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef DateToUpdate = {0};
char trans_str[64] = {0,};

uint8_t SPI_rx_buf[1] = {0};
uint8_t SPI_tx_buf[1] = {0};
extern uint8_t flag_output_spi1;

uint8_t cnt1 = 1;			//Флаг выбора позиции отрисовки в меню
uint8_t cnt2 = 1;			//Флаг выбора позиции отрисовки в меню
uint8_t step_DOWN = 0;		//Флаг нажатия кнопки "Вниз"
uint8_t step_UP = 0;		//Флаг нажатия кнопки "Вверх"
uint8_t Check = 0;			//Флаг выбора конкретной строки в меню для совершения действия
uint8_t Num_Menu = 0;		//Переменная перехода в рабочую область нового меню

uint8_t flag_MCU = 1;		//Флаг отправки данных на MCU
//uint8_t start_device = 1;	//Флаг запуска устройства, чтобы запросить актуальные данные вводов/выводов

uint8_t Status_AIN[8] = {0, 0, 0, 0, 0, 0, 0, 0};	//Статус аналоговых входов
uint8_t Status_DIN[8] = {0, 0, 0, 0, 0, 0, 0, 0};	//Статус цифровых входов
uint8_t Status_PWM[8] = {0, 0, 0, 0, 0, 0, 0, 0};	//Статус выходов ШИМ
uint8_t Status_OCD[8] = {0, 0, 0, 0, 0, 0, 0, 0};	//Статус выходов открытый коллектор
uint8_t Status_1WR[8] = {0, 0, 0, 0, 0, 0, 0, 0};	//Статус выходов интерфейса 1-Wire

#define MCU_OUTPUT			0xC0	//Комманда чтения статусов входов/выходов
#define MCU_NOP				0x00	//Пустая посылка
#define MCU_RELAY_CNT		0xA1	//Кол-во блоков реле
#define MCU_DIGITAL_CNT		0xA2	//Кол-во блоков реле
#define MCU_DIMMING_CNT		0xA3	//Кол-во блоков реле
#define MCU_INTERFACE_CNT	0xA4	//Кол-во блоков реле
#define MCU_RELAY_ADR		0xB1	//Адреса блоков реле
#define MCU_DIGITAL_ADR		0xB2	//Адреса блоков реле
#define MCU_DIMMING_ADR		0xB3	//Адреса блоков реле
#define MCU_INTERFACE_ADR	0xB4	//Адреса блоков реле

#define RELAY		0
#define DIGITAL 	1
#define DIMMING 	2
#define INTERFACE 	3

uint8_t Count[4] = {0, 0, 0, 0};	//Массив для хранения кол-ва подключенных блоков(Relay, Digital, Dimming, Interface)

//--------------------------------------Комманды контроллера управления-------------------------------------
void loading(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(25, 55, "LOADING...", Font_11x18, ST7735_WHITE, ST7735_BLACK);
  	BackLight(0);
  	HAL_Delay(250);
  	BackLight(Get_ADC1());
}
//Фукция получения данных от управляющего контроллера по spi
//Принимает массивы переменных статуса входов/выходов
void Send_READ_Status_Outputs(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(3, 0, "DEV", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	ST7735_DrawString(36, 0, "ELECTRONICS", Font_11x18, ST7735_BLACK, ST7735_WHITE);
	ST7735_DrawString(26, 60, "LOADING...", Font_11x18, ST7735_WHITE, ST7735_BLACK);

	uint8_t i = 0;
	int8_t j = -1;

	SPI_tx_buf[0] = MCU_OUTPUT;
	CSM_L;
	HAL_SPI_TransmitReceive_IT(&hspi1, (uint8_t*)SPI_tx_buf, (uint8_t *)SPI_rx_buf, 1);
    while(!flag_output_spi1) {;}
    flag_output_spi1 = 0;

	SPI_tx_buf[0] = MCU_NOP;

	while(1)
    {
		CSM_L;
		HAL_SPI_TransmitReceive_IT(&hspi1, (uint8_t*)SPI_tx_buf, (uint8_t *)SPI_rx_buf, 1);
		while(!flag_output_spi1) {;}
		flag_output_spi1 = 0;
    	if(i == 0)
    	{
    		if(j != -1)
    		{
    			Status_AIN[j] = SPI_rx_buf[0];
    			j++;
				if(j > 7)
				{
					j = 0;
					i++;
				}
    		}
    		if(j == -1)
    			j++;
    	}
    	else if(i == 1)
    	{
    		Status_DIN[j] = SPI_rx_buf[0];
			j++;
			if(j > 7)
			{
				j = 0;
				i++;
			}
    	}
    	else if(i == 2)
    	{
    		Status_PWM[j] = SPI_rx_buf[0];
			j++;
			if(j > 7)
			{
				j = 0;
				i++;
			}
    	}
    	else if(i == 3)
    	{
    		Status_OCD[j] = SPI_rx_buf[0];
			j++;
			if(j > 7)
			{
				j = 0;
				i++;
			}
    	}
    	else if(i == 4)
    	{
    		Status_1WR[j] = SPI_rx_buf[0];
			j++;
			if(j > 7)
			{
				j = 0;
				i++;
			}
    	}
		if(i > 4)
		{
			break;
		}
		HAL_Delay(125);
    }
}
//Фукция получения данных от управляющего контроллера по spi
//Возвращает кол-во подключенных типовых блоков
uint8_t Data_Block(uint8_t CMD)
{
	uint8_t Data = 0;

	CSM_L;
		Data = SPI_RW(CMD);
	CSM_H;

	return Data;
}
//Фукция обновления данных о состоянии входов/выходов
void DataUpdate(void)
{
	if((GPIOB->IDR & GPIO_PIN_10) == 0)
	{
		Send_READ_Status_Outputs();
		HAL_Delay(2000);
		Menu_Main();
	}
}
//----------------------------------------------------------------------------------------------------------

//Функция обработки действий главного меню
//Принимает флаги нажатия кнопок "Вниз";"Вверх";
//Принимает переменную времени задержки обработки прерываний
//Возвращает выбранную позицию в списке меню
uint8_t Main_Menu(uint8_t DOWN, uint8_t UP, uint32_t time)
{
	step_DOWN = DOWN;
	step_UP = UP;
	if((step_DOWN) || (step_UP))
	{
		if(cnt1 == 1)
		{
			ST7735_DrawString(3, 0, "DEV", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(36, 0, "ELECTRONICS", Font_11x18, ST7735_BLACK, ST7735_WHITE);
			ST7735_DrawString(30, 36, "OUTPUT", Font_16x26, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(30, 75, "BLOCKS", Font_16x26, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 2;
			Check = 1;
		}
		else if(cnt1 == 2)
		{
			ST7735_DrawString(3, 0, "DEV", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(36, 0, "ELECTRONICS", Font_11x18, ST7735_BLACK, ST7735_WHITE);
			ST7735_DrawString(30, 36, "OUTPUT", Font_16x26, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(30, 75, "BLOCKS", Font_16x26, ST7735_BLUE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 1;
			Check = 2;
		}
	}
	return Check;
}
//Функция обработки действий подменю: "Выводы базового блока"
//Принимает флаги нажатия кнопок "Вниз";"Вверх";
//Принимает переменную времени задержки обработки прерываний
//Возвращает выбранную позицию в списке меню
uint8_t One_Menu(uint8_t DOWN, uint8_t UP, uint32_t time)
{
	step_DOWN = DOWN;
	step_UP = UP;
	if((step_DOWN) || (step_UP))
	{
		if(((cnt1 == 1) && (step_DOWN == 1)) || ((cnt2 == 1) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "OUTPUTS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- ANALOG", Font_11x18, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 48, "- DIGITAL", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 66, "- OPEN DRAIN", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 84, "- 1-WIRE", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 102, "- PWM", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 2;
			cnt2 = 5;
			Check = 3;
		}
		else if(((cnt1 == 2) && (step_DOWN == 1)) || ((cnt2 == 2) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "OUTPUTS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- ANALOG", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 48, "- DIGITAL", Font_11x18, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 66, "- OPEN DRAIN", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 84, "- 1-WIRE", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 102, "- PWM", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 3;
			cnt2 = 1;
			Check = 4;
		}
		else if(((cnt1 == 3) && (step_DOWN == 1)) || ((cnt2 == 3) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "OUTPUTS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- ANALOG", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 48, "- DIGITAL", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 66, "- OPEN DRAIN", Font_11x18, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 84, "- 1-WIRE", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 102, "- PWM", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 4;
			cnt2 = 2;
			Check = 5;
		}
		else if(((cnt1 == 4) && (step_DOWN == 1)) || ((cnt2 == 4) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "OUTPUTS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- ANALOG", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 48, "- DIGITAL", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 66, "- OPEN DRAIN", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 84, "- 1-WIRE", Font_11x18, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 102, "- PWM", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 5;
			cnt2 = 3;
			Check = 6;
		}
		else if(((cnt1 == 5) && (step_DOWN == 1)) || ((cnt2 == 5) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "OUTPUTS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- ANALOG", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 48, "- DIGITAL", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 66, "- OPEN DRAIN", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 84, "- 1-WIRE", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 102, "- PWM", Font_11x18, ST7735_BLUE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 1;
			cnt2 = 4;
			Check = 7;
		}
	}
	return Check;
}
//Функция обработки действий подменю: "Подключенные блоки"
//Принимает флаги нажатия кнопок "Вниз";"Вверх";
//Принимает переменную времени задержки обработки прерываний
//Возвращает выбранную позицию в списке меню
uint8_t Two_Menu(uint8_t DOWN, uint8_t UP, uint32_t time)
{
	step_DOWN = DOWN;
	step_UP = UP;
	if((step_DOWN) || (step_UP))
	{
		if(((cnt1 == 1) && (step_DOWN == 1)) || ((cnt2 == 1) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "BLOCKS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 36, "- RELAY", Font_11x18, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 54, "- DIGITAL", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 72, "- DIMMING", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- INTERFACE", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 2;
			cnt2 = 4;
			Check = 8;
		}
		else if(((cnt1 == 2) && (step_DOWN == 1)) || ((cnt2 == 2) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "BLOCKS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 36, "- RELAY", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 54, "- DIGITAL", Font_11x18, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 72, "- DIMMING", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- INTERFACE", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 3;
			cnt2 = 1;
			Check = 9;
		}
		else if(((cnt1 == 3) && (step_DOWN == 1)) || ((cnt2 == 3) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "BLOCKS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 36, "- RELAY", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 54, "- DIGITAL", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 72, "- DIMMING", Font_11x18, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- INTERFACE", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 4;
			cnt2 = 2;
			Check = 10;
		}
		else if(((cnt1 == 4) && (step_DOWN == 1)) || ((cnt2 == 4) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "BLOCKS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 36, "- RELAY", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 54, "- DIGITAL", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 72, "- DIMMING", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- INTERFACE", Font_11x18, ST7735_BLUE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 1;
			cnt2 = 3;
			Check = 11;
		}
	}
	return Check;
}
//Функция обработки действий подменю: "Аналоговые входы"
//Принимает флаги нажатия кнопок "Вниз";"Вверх";
//Принимает переменную времени задержки обработки прерываний
//Возвращает выбранную позицию в списке меню
uint8_t Three_Menu(uint8_t DOWN, uint8_t UP, uint32_t time)
{
	step_DOWN = DOWN;
	step_UP = UP;
	if((step_DOWN) || (step_UP))
	{
		if(((cnt1 == 1) && (step_DOWN == 1)) || ((cnt2 == 1) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "ANALOG", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- AIN0", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- AIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- AIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- AIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- AIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- AIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- AIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- AIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 2;
			cnt2 = 8;
			Check = 12;
		}
		else if(((cnt1 == 2) && (step_DOWN == 1)) || ((cnt2 == 2) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "ANALOG", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- AIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- AIN1", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- AIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- AIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- AIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- AIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- AIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- AIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 3;
			cnt2 = 1;
			Check = 13;
		}
		else if(((cnt1 == 3) && (step_DOWN == 1)) || ((cnt2 == 3) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "ANALOG", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- AIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- AIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- AIN2", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- AIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- AIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- AIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- AIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- AIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 4;
			cnt2 = 2;
			Check = 14;
		}
		else if(((cnt1 == 4) && (step_DOWN == 1)) || ((cnt2 == 4) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "ANALOG", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- AIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- AIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- AIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- AIN3", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- AIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- AIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- AIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- AIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 5;
			cnt2 = 3;
			Check = 15;
		}
		else if(((cnt1 == 5) && (step_DOWN == 1)) || ((cnt2 == 5) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "ANALOG", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- AIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- AIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- AIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- AIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- AIN4", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- AIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- AIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- AIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 6;
			cnt2 = 4;
			Check = 16;
		}
		else if(((cnt1 == 6) && (step_DOWN == 1)) || ((cnt2 == 6) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "ANALOG", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- AIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- AIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- AIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- AIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- AIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- AIN5", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- AIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- AIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 7;
			cnt2 = 5;
			Check = 17;
		}
		else if(((cnt1 == 7) && (step_DOWN == 1)) || ((cnt2 == 7) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "ANALOG", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- AIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- AIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- AIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- AIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- AIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- AIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- AIN6", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- AIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 8;
			cnt2 = 6;
			Check = 18;
		}
		else if(((cnt1 == 8) && (step_DOWN == 1)) || ((cnt2 == 8) && step_UP == 1))
		{
			ST7735_DrawString(48, 0, "ANALOG", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- AIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- AIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- AIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- AIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- AIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- AIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- AIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- AIN7", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 1;
			cnt2 = 7;
			Check = 19;
		}
	}

	return Check;
}
//Функция обработки действий подменю: "Цифровые входы"
//Принимает флаги нажатия кнопок "Вниз";"Вверх";
//Принимает переменную времени задержки обработки прерываний
//Возвращает выбранную позицию в списке меню
uint8_t Four_Menu(uint8_t DOWN, uint8_t UP, uint32_t time)
{
	step_DOWN = DOWN;
	step_UP = UP;
	if((step_DOWN) || (step_UP))
	{
		if(((cnt1 == 1) && (step_DOWN == 1)) || ((cnt2 == 1) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- DIN0", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- DIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- DIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- DIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- DIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- DIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- DIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- DIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 2;
			cnt2 = 8;
			Check = 20;
		}
		else if(((cnt1 == 2) && (step_DOWN == 1)) || ((cnt2 == 2) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- DIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- DIN1", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- DIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- DIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- DIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- DIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- DIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- DIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 3;
			cnt2 = 1;
			Check = 21;
		}
		else if(((cnt1 == 3) && (step_DOWN == 1)) || ((cnt2 == 3) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- DIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- DIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- DIN2", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- DIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- DIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- DIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- DIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- DIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 4;
			cnt2 = 2;
			Check = 22;
		}
		else if(((cnt1 == 4) && (step_DOWN == 1)) || ((cnt2 == 4) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- DIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- DIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- DIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- DIN3", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- DIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- DIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- DIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- DIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 5;
			cnt2 = 3;
			Check = 23;
		}
		else if(((cnt1 == 5) && (step_DOWN == 1)) || ((cnt2 == 5) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- DIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- DIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- DIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- DIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- DIN4", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- DIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- DIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- DIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 6;
			cnt2 = 4;
			Check = 24;
		}
		else if(((cnt1 == 6) && (step_DOWN == 1)) || ((cnt2 == 6) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- DIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- DIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- DIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- DIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- DIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- DIN5", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- DIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- DIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 7;
			cnt2 = 5;
			Check = 25;
		}
		else if(((cnt1 == 7) && (step_DOWN == 1)) || ((cnt2 == 7) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- DIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- DIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- DIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- DIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- DIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- DIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- DIN6", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- DIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 8;
			cnt2 = 6;
			Check = 26;
		}
		else if(((cnt1 == 8) && (step_DOWN == 1)) || ((cnt2 == 8) && step_UP == 1))
		{
			ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- DIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- DIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- DIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- DIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- DIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- DIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- DIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- DIN7", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 1;
			cnt2 = 7;
			Check = 27;
		}
	}
	return Check;
}
//Функция обработки действий подменю: "Открытый коллектор"
//Принимает флаги нажатия кнопок "Вниз";"Вверх";
//Принимает переменную времени задержки обработки прерываний
//Возвращает выбранную позицию в списке меню
uint8_t Five_Menu(uint8_t DOWN, uint8_t UP, uint32_t time)
{
	step_DOWN = DOWN;
	step_UP = UP;
	if((step_DOWN) || (step_UP))
	{
		if(((cnt1 == 1) && (step_DOWN == 1)) || ((cnt2 == 1) && step_UP == 1))
		{
			ST7735_DrawString(24, 0, "OPEN DRAIN", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- OC0", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- OC1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- OC2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- OC3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- OC4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- OC5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- OC6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- OC7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 2;
			cnt2 = 8;
			Check = 34;
		}
		else if(((cnt1 == 2) && (step_DOWN == 1)) || ((cnt2 == 2) && step_UP == 1))
		{
			ST7735_DrawString(24, 0, "OPEN DRAIN", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- OC0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- OC1", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- OC2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- OC3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- OC4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- OC5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- OC6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- OC7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 3;
			cnt2 = 1;
			Check = 35;
		}
		else if(((cnt1 == 3) && (step_DOWN == 1)) || ((cnt2 == 3) && step_UP == 1))
		{
			ST7735_DrawString(24, 0, "OPEN DRAIN", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- OC0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- OC1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- OC2", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- OC3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- OC4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- OC5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- OC6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- OC7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 4;
			cnt2 = 2;
			Check = 36;
		}
		else if(((cnt1 == 4) && (step_DOWN == 1)) || ((cnt2 == 4) && step_UP == 1))
		{
			ST7735_DrawString(24, 0, "OPEN DRAIN", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- OC0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- OC1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- OC2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- OC3", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- OC4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- OC5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- OC6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- OC7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 5;
			cnt2 = 3;
			Check = 37;
		}
		else if(((cnt1 == 5) && (step_DOWN == 1)) || ((cnt2 == 5) && step_UP == 1))
		{
			ST7735_DrawString(24, 0, "OPEN DRAIN", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- OC0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- OC1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- OC2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- OC3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- OC4", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- OC5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- OC6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- OC7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 6;
			cnt2 = 4;
			Check = 38;
		}
		else if(((cnt1 == 6) && (step_DOWN == 1)) || ((cnt2 == 6) && step_UP == 1))
		{
			ST7735_DrawString(24, 0, "OPEN DRAIN", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- OC0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- OC1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- OC2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- OC3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- OC4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- OC5", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- OC6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- OC7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 7;
			cnt2 = 5;
			Check = 39;
		}
		else if(((cnt1 == 7) && (step_DOWN == 1)) || ((cnt2 == 7) && step_UP == 1))
		{
			ST7735_DrawString(24, 0, "OPEN DRAIN", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- OC0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- OC1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- OC2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- OC3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- OC4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- OC5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- OC6", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- OC7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 8;
			cnt2 = 6;
			Check = 40;
		}
		else if(((cnt1 == 8) && (step_DOWN == 1)) || ((cnt2 == 8) && step_UP == 1))
		{
			ST7735_DrawString(24, 0, "OPEN DRAIN", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- OC0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- OC1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- OC2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- OC3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 70, "- OC4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 80, "- OC5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 90, "- OC6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 100, "- OC7", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 1;
			cnt2 = 7;
			Check = 41;
		}
	}
	return Check;
}
//Функция обработки действий подменю: "1-Wire"
//Принимает флаги нажатия кнопок "Вниз";"Вверх";
//Принимает переменную времени задержки обработки прерываний
//Возвращает выбранную позицию в списке меню
uint8_t Six_Menu(uint8_t DOWN, uint8_t UP, uint32_t time)
{
	step_DOWN = DOWN;
	step_UP = UP;
	if((step_DOWN) || (step_UP))
	{
		if(cnt1 == 1)
		{
			ST7735_DrawString(48, 0, "1-WIRE", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- WIRE0", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- WIRE1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 2;
			Check = 32;
		}
		else if(cnt1 == 2)
		{
			ST7735_DrawString(48, 0, "1-WIRE", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- WIRE0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- WIRE1", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 1;
			Check = 33;
		}
	}
	return Check;
}
//Функция обработки действий подменю: "PWM"
//Принимает флаги нажатия кнопок "Вниз";"Вверх";
//Принимает переменную времени задержки обработки прерываний
//Возвращает выбранную позицию в списке меню
uint8_t Seven_Menu(uint8_t DOWN, uint8_t UP, uint32_t time)
{
	step_DOWN = DOWN;
	step_UP = UP;
	if((step_DOWN) || (step_UP))
	{
		if(((cnt1 == 1) && (step_DOWN == 1)) || ((cnt2 == 1) && step_UP == 1))
		{
			ST7735_DrawString(71, 0, "PWM", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- PWM0", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- PWM1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- PWM2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- PWM3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 2;
			cnt2 = 4;
			Check = 28;
		}
		else if(((cnt1 == 2) && (step_DOWN == 1)) || ((cnt2 == 2) && step_UP == 1))
		{
			ST7735_DrawString(71, 0, "PWM", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- PWM0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- PWM1", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- PWM2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- PWM3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 3;
			cnt2 = 1;
			Check = 29;
		}
		else if(((cnt1 == 3) && (step_DOWN == 1)) || ((cnt2 == 3) && step_UP == 1))
		{
			ST7735_DrawString(71, 0, "PWM", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- PWM0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- PWM1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- PWM2", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- PWM3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 4;
			cnt2 = 2;
			Check = 30;
		}
		else if(((cnt1 == 4) && (step_DOWN == 1)) || ((cnt2 == 4) && step_UP == 1))
		{
			ST7735_DrawString(71, 0, "PWM", Font_11x18, ST7735_BLUE, ST7735_WHITE);
			ST7735_DrawString(0, 30, "- PWM0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 40, "- PWM1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 50, "- PWM2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(0, 60, "- PWM3", Font_7x10, ST7735_BLUE, ST7735_BLACK);
			HAL_Delay(time);
			step_DOWN = 0;
			cnt1 = 1;
			cnt2 = 3;
			Check = 31;
		}
	}
	return Check;
}
//Функция отрисовки при переходе в новое подменю
//Принимает переменную выбранного пункта меню
//Возвращает переменную, которая отвечает за выбор рабочей области
uint8_t Item_Selection(uint8_t choice)
{
	if(choice == 1)			//item: OUTPUT
	{
		Menu_Output();
		Num_Menu = 2;
	}
	else if(choice == 2)	//item: BLOCKS
	{
		Menu_Blocks();
		Num_Menu = 3;
	}
	else if(choice == 3)	//item: ANALOG
	{
		Menu_Analog();
		Num_Menu = 4;
	}
	else if(choice == 4)	//item: DIGITAL
	{
		Menu_Digital();
		Num_Menu = 5;
	}
	else if(choice == 5)	//item: OPEN DRAIN
	{
		Menu_OpenDrain();
		Num_Menu = 6;
	}
	else if(choice == 6)	//item: 1-WIRE
	{
		Menu_1Wire();
		Num_Menu = 7;
	}
	else if(choice == 7)	//item: PWM
	{
		Menu_PWM();
		Num_Menu = 8;
	}
	else if(choice == 8)	//item: RELAY
	{
		//Показать адрес блока(Запрос)
		Count[0] = Data_Block(MCU_RELAY_CNT);
		if(Count[0] > 0)
			Filling_Menu_Block(MCU_RELAY_ADR, Count[0], RELAY);
		else
			Not_Found_Block(RELAY);
		Num_Menu = 9;
	}
	else if(choice == 9)	//item: DIGITAL
	{
		//Показать адрес блока(Запрос)
		Count[1] = Data_Block(MCU_DIGITAL_CNT);
		if(Count[1] > 0)
			Filling_Menu_Block(MCU_DIGITAL_ADR, Count[1], DIGITAL);
		else
			Not_Found_Block(RELAY);
		Num_Menu = 10;
	}
	else if(choice == 10)	//item: DIMMING
	{
		//Показать адрес блока(Запрос)
		Count[2] = Data_Block(MCU_DIMMING_CNT);
		if(Count[2] > 0)
			Filling_Menu_Block(MCU_DIMMING_ADR, Count[2], DIMMING);
		else
			Not_Found_Block(DIMMING);
		Num_Menu = 11;
	}
	else if(choice == 11)	//item: INTERFACE
	{
		//Показать адрес блока(Запрос)
		Count[3] = Data_Block(MCU_INTERFACE_CNT);
		if(Count[3] > 0)
			Filling_Menu_Block(MCU_INTERFACE_ADR, Count[3], INTERFACE);
		else
			Not_Found_Block(INTERFACE);
		Num_Menu = 12;
	}
	else if(choice == 12)	//item: AIN0_ON/OFF
	{
		if(!Status_AIN[0])
		{
			SPI_RW(AIN0_ON);
			Status_AIN[0] = 1;
//			send_str("AIN0_ON\n");
			//Send_WRITE_command(0xB000, 0x01);
			//Возможно добавить посылку комманды на MCU
		}
		else
		{
			SPI_RW(AIN0_OFF);
			Status_AIN[0] = 0;
//			send_str("AIN0_OFF\n");
			//Send_WRITE_command(0xB000, 0x00);
		}
		Num_Menu = 13;
	}
	else if(choice == 13)	//item: AIN1_ON/OFF
	{
		if(!Status_AIN[1])
		{
			SPI_RW(AIN1_ON);
			Status_AIN[1] = 1;
//			send_str("AIN1_ON\n");
			//Send_WRITE_command(0xB001, 0x01);
		}
		else
		{
			SPI_RW(AIN1_OFF);
			Status_AIN[1] = 0;
//			send_str("AIN1_OFF\n");
			//Send_WRITE_command(0xB001, 0x00);
		}
		Num_Menu = 14;
	}
	else if(choice == 14)	//item: AIN2_ON/OFF
	{
		if(!Status_AIN[2])
		{
			SPI_RW(AIN2_ON);
			Status_AIN[2] = 1;
//			send_str("AIN2_ON\n");
			//Send_WRITE_command(0xB002, 0x01);
		}
		else
		{
			SPI_RW(AIN2_OFF);
			Status_AIN[2] = 0;
//			send_str("AIN2_OFF\n");
			//Send_WRITE_command(0xB002, 0x00);
		}
		Num_Menu = 15;
	}
	else if(choice == 15)	//item: AIN3_ON/OFF
	{
		if(!Status_AIN[3])
		{
			SPI_RW(AIN3_ON);
			Status_AIN[3] = 1;
//			send_str("AIN3_ON\n");
			//Send_WRITE_command(0xB003, 0x01);
		}
		else
		{
			SPI_RW(AIN3_OFF);
			Status_AIN[3] = 0;
//			send_str("AIN3_OFF\n");
			//Send_WRITE_command(0xB003, 0x00);
		}
		Num_Menu = 16;
	}
	else if(choice == 16)	//item: AIN4_ON/OFF
	{
		if(!Status_AIN[4])
		{
			SPI_RW(AIN4_ON);
			Status_AIN[4] = 1;
//			send_str("AIN4_ON\n");
			//Send_WRITE_command(0xB004, 0x01);
		}
		else
		{
			SPI_RW(AIN4_OFF);
			Status_AIN[4] = 0;
//			send_str("AIN4_OFF\n");
			//Send_WRITE_command(0xB004, 0x00);
		}
		Num_Menu = 17;
	}
	else if(choice == 17)	//item: AIN5_ON/OFF
	{
		if(!Status_AIN[5])
		{
			SPI_RW(AIN5_ON);
			Status_AIN[5] = 1;
//			send_str("AIN5_ON\n");
			//Send_WRITE_command(0xB005, 0x01);
		}
		else
		{
			SPI_RW(AIN5_OFF);
			Status_AIN[5] = 0;
//			send_str("AIN5_OFF\n");
			//Send_WRITE_command(0xB005, 0x00);
		}
		Num_Menu = 18;
	}
	else if(choice == 18)	//item: AIN6_ON/OFF
	{
		if(!Status_AIN[6])
		{
			SPI_RW(AIN6_ON);
			Status_AIN[6] = 1;
//			send_str("AIN6_ON\n");
			//Send_WRITE_command(0xB006, 0x01);
		}
		else
		{
			SPI_RW(AIN6_OFF);
			Status_AIN[6] = 0;
//			send_str("AIN6_OFF\n");
			//Send_WRITE_command(0xB006, 0x00);
		}
		Num_Menu = 19;
	}
	else if(choice == 19)	//item: AIN7_ON/OFF
	{
		if(!Status_AIN[7])
		{
			SPI_RW(AIN7_ON);
			Status_AIN[7] = 1;
//			send_str("AIN7_ON\n");
			//Send_WRITE_command(0xB007, 0x01);
		}
		else
		{
			SPI_RW(AIN7_OFF);
			Status_AIN[7] = 0;
//			send_str("AIN7_OFF\n");
			//Send_WRITE_command(0xB007, 0x00);
		}
		Num_Menu = 20;
	}
	else if(choice == 20)	//item: DIN0_ON/OFF
	{
		if(!Status_DIN[0])
		{
			SPI_RW(DIN0_ON);
			Status_DIN[0] = 1;
//			send_str("DIN0_ON\n");
			//Send_WRITE_command(0xB008, 0x01);
		}
		else
		{
			SPI_RW(DIN0_OFF);
			Status_DIN[0] = 0;
//			send_str("DIN0_OFF\n");
			//Send_WRITE_command(0xB008, 0x00);
		}
		Num_Menu = 21;
	}
	else if(choice == 21)	//item: DIN1_ON/OFF
	{
		if(!Status_DIN[1])
		{
			SPI_RW(DIN1_ON);
			Status_DIN[1] = 1;
//			send_str("DIN1_ON\n");
			//Send_WRITE_command(0xB009, 0x01);
		}
		else
		{
			SPI_RW(DIN1_OFF);
			Status_DIN[1] = 0;
//			send_str("DIN1_OFF\n");
			//Send_WRITE_command(0xB009, 0x00);
		}
		Num_Menu = 22;
	}
	else if(choice == 22)	//item: DIN2_ON/OFF
	{
		if(!Status_DIN[2])
		{
			SPI_RW(DIN2_ON);
			Status_DIN[2] = 1;
//			send_str("DIN2_ON\n");
			//Send_WRITE_command(0xB00A, 0x01);
		}
		else
		{
			SPI_RW(DIN2_OFF);
			Status_DIN[2] = 0;
//			send_str("DIN2_OFF\n");
			//Send_WRITE_command(0xB00A, 0x00);
		}
		Num_Menu = 23;
	}
	else if(choice == 23)	//item: DIN3_ON/OFF
	{
		if(!Status_DIN[3])
		{
			SPI_RW(DIN3_ON);
			Status_DIN[3] = 1;
//			send_str("DIN3_ON\n");
			//Send_WRITE_command(0xB00B, 0x01);
		}
		else
		{
			SPI_RW(DIN3_OFF);
			Status_DIN[3] = 0;
//			send_str("DIN3_OFF\n");
			//Send_WRITE_command(0xB00B, 0x00);
		}
		Num_Menu = 24;
	}
	else if(choice == 24)	//item: DIN4_ON/OFF
	{
		if(!Status_DIN[4])
		{
			SPI_RW(DIN4_ON);
			Status_DIN[4] = 1;
//			send_str("DIN4_ON\n");
			//Send_WRITE_command(0xB00C, 0x01);
		}
		else
		{
			SPI_RW(DIN4_OFF);
			Status_DIN[4] = 0;
//			send_str("DIN4_OFF\n");
			//Send_WRITE_command(0xB00C, 0x00);
		}
		Num_Menu = 25;
	}
	else if(choice == 25)	//item: DIN5_ON/OFF
	{
		if(!Status_DIN[5])
		{
			SPI_RW(DIN5_ON);
			Status_DIN[5] = 1;
//			send_str("DIN5_ON\n");
			//Send_WRITE_command(0xB00D, 0x01);
		}
		else
		{
			SPI_RW(DIN5_OFF);
			Status_DIN[5] = 0;
//			send_str("DIN5_OFF\n");
			//Send_WRITE_command(0xB00D, 0x00);
		}
		Num_Menu = 26;
	}
	else if(choice == 26)	//item: DIN6_ON/OFF
	{
		if(!Status_DIN[6])
		{
			SPI_RW(DIN6_ON);
			Status_DIN[6] = 1;
//			send_str("DIN6_ON\n");
			//Send_WRITE_command(0xB00E, 0x01);
		}
		else
		{
			SPI_RW(DIN6_OFF);
			Status_DIN[6] = 0;
//			send_str("DIN6_OFF\n");
			//Send_WRITE_command(0xB00E, 0x00);
		}
		Num_Menu = 27;
	}
	else if(choice == 27)	//item: DIN7_ON/OFF
	{
		if(!Status_DIN[7])
		{
			SPI_RW(DIN7_ON);
			Status_DIN[7] = 1;
//			send_str("DIN7_ON\n");
			//Send_WRITE_command(0xB00F, 0x01);
		}
		else
		{
			SPI_RW(DIN7_OFF);
			Status_DIN[7] = 0;
//			send_str("DIN7_OFF\n");
			//Send_WRITE_command(0xB00F, 0x00);
		}
		Num_Menu = 28;
	}
	else if(choice == 28)	//item: PWM0_ON/OFF
	{
		if(!Status_PWM[0])
		{
			SPI_RW(PWM0_ON);
			Status_PWM[0] = 1;
//			send_str("PWM0_ON\n");
			//Send_WRITE_command(0xB010, 0x01);
		}
		else
		{
			SPI_RW(PWM0_OFF);
			Status_PWM[0] = 0;
//			send_str("PWM0_OFF\n");
			//Send_WRITE_command(0xB010, 0x00);
		}
		Num_Menu = 29;
	}
	else if(choice == 29)	//item: PWM1_ON/OFF
	{
		if(!Status_PWM[1])
		{
			SPI_RW(PWM1_ON);
			Status_PWM[1] = 1;
//			send_str("PWM1_ON\n");
			//Send_WRITE_command(0xB011, 0x01);
		}
		else
		{
			SPI_RW(PWM1_OFF);
			Status_PWM[1] = 0;
//			send_str("PWM1_OFF\n");
			//Send_WRITE_command(0xB011, 0x00);
		}
		Num_Menu = 30;
	}
	else if(choice == 30)	//item: PWM2_ON/OFF
	{
		if(!Status_PWM[2])
		{
			SPI_RW(PWM2_ON);
			Status_PWM[2] = 1;
//			send_str("PWM2_ON\n");
			//Send_WRITE_command(0xB012, 0x01);
		}
		else
		{
			SPI_RW(PWM2_OFF);
			Status_PWM[2] = 0;
//			send_str("PWM2_OFF\n");
			//Send_WRITE_command(0xB012, 0x00);
		}
		Num_Menu = 31;
	}
	else if(choice == 31)	//item: PWM3_ON/OFF
	{
		if(!Status_PWM[3])
		{
			SPI_RW(PWM3_ON);
			Status_PWM[3] = 1;
//			send_str("PWM3_ON\n");
			//Send_WRITE_command(0xB013, 0x01);
		}
		else
		{
			SPI_RW(PWM3_OFF);
			Status_PWM[3] = 0;
//			send_str("PWM3_OFF\n");
			//Send_WRITE_command(0xB013, 0x00);
		}
		Num_Menu = 32;
	}
	else if(choice == 32)	//item: WR0_ON/OFF
	{
		if(!Status_1WR[0])
		{
			SPI_RW(WR0_ON);
			Status_1WR[0] = 1;
//			send_str("WR0_ON\n");
			//Send_WRITE_command(0xB014, 0x01);
		}
		else
		{
			SPI_RW(WR0_OFF);
			Status_1WR[0] = 0;
//			send_str("WR0_OFF\n");
			//Send_WRITE_command(0xB014, 0x00);
		}
		Num_Menu = 33;
	}
	else if(choice == 33)	//item: WR1_ON/OFF
	{
		if(!Status_1WR[1])
		{
			SPI_RW(WR1_ON);
			Status_1WR[1] = 1;
//			send_str("WR1_ON\n");
			//Send_WRITE_command(0xB015, 0x01);
		}
		else
		{
			SPI_RW(WR1_OFF);
			Status_1WR[1] = 0;
//			send_str("WR1_OFF\n");
			//Send_WRITE_command(0xB015, 0x00);
		}
		Num_Menu = 34;
	}
	else if(choice == 34)	//item: OC0_ON/OFF
	{
		if(!Status_OCD[0])
		{
			SPI_RW(OC0_ON);
			Status_OCD[0] = 1;
//			send_str("OC0_ON\n");
			//Send_WRITE_command(0xB016, 0x01);
		}
		else
		{
			SPI_RW(OC0_OFF);
			Status_OCD[0] = 0;
//			send_str("OC0_OFF\n");
			//Send_WRITE_command(0xB016, 0x00);
		}
		Num_Menu = 35;
	}
	else if(choice == 35)	//item: OC1_ON/OFF
	{
		if(!Status_OCD[1])
		{
			SPI_RW(OC1_ON);
			Status_OCD[1] = 1;
//			send_str("OC1_ON\n");
			//Send_WRITE_command(0xB017, 0x01);
		}
		else
		{
			SPI_RW(OC1_OFF);
			Status_OCD[1] = 0;
//			send_str("OC1_OFF\n");
			//Send_WRITE_command(0xB017, 0x00);
		}
		Num_Menu = 36;
	}
	else if(choice == 36)	//item: OC2_ON/OFF
	{
		if(!Status_OCD[2])
		{
			SPI_RW(OC2_ON);
			Status_OCD[2] = 1;
//			send_str("OC2_ON\n");
			//Send_WRITE_command(0xB018, 0x01);
		}
		else
		{
			SPI_RW(OC2_OFF);
			Status_OCD[2] = 0;
//			send_str("OC2_OFF\n");
			//Send_WRITE_command(0xB018, 0x00);
		}
		Num_Menu = 37;
	}
	else if(choice == 37)	//item: OC3_ON/OFF
	{
		if(!Status_OCD[3])
		{
			SPI_RW(OC3_ON);
			Status_OCD[3] = 1;
//			send_str("OC3_ON\n");
			//Send_WRITE_command(0xB019, 0x01);
		}
		else
		{
			SPI_RW(OC3_OFF);
			Status_OCD[3] = 0;
//			send_str("OC3_OFF\n");
			//Send_WRITE_command(0xB019, 0x00);
		}
		Num_Menu = 38;
	}
	else if(choice == 38)	//item: OC4_ON/OFF
	{
		if(!Status_OCD[4])
		{
			SPI_RW(OC4_ON);
			Status_OCD[4] = 1;
//			send_str("OC4_ON\n");
			//Send_WRITE_command(0xB01A, 0x01);
		}
		else
		{
			SPI_RW(OC4_OFF);
			Status_OCD[4] = 0;
//			send_str("OC4_OFF\n");
			//Send_WRITE_command(0xB01A, 0x00);
		}
		Num_Menu = 39;
	}
	else if(choice == 39)	//item: OC5_ON/OFF
	{
		if(!Status_OCD[5])
		{
			SPI_RW(OC5_ON);
			Status_OCD[5] = 1;
//			send_str("OC5_ON\n");
			//Send_WRITE_command(0xB01B, 0x01);
		}
		else
		{
			SPI_RW(OC5_OFF);
			Status_OCD[5] = 0;
//			send_str("OC5_OFF\n");
			//Send_WRITE_command(0xB01B, 0x00);
		}
		Num_Menu = 40;
	}
	else if(choice == 40)	//item: OC6_ON/OFF
	{
		if(!Status_OCD[6])
		{
			SPI_RW(OC6_ON);
			Status_OCD[6] = 1;
//			send_str("OC6_ON\n");
			//Send_WRITE_command(0xB01C, 0x01);
		}
		else
		{
			SPI_RW(OC6_OFF);
			Status_OCD[6] = 0;
//			send_str("OC6_OFF\n");
			//Send_WRITE_command(0xB01C, 0x00);
		}
		Num_Menu = 41;
	}
	else if(choice == 41)	//item: OC7_ON/OFF
	{
		if(!Status_OCD[7])
		{
			SPI_RW(OC7_ON);
			Status_OCD[7] = 1;
//			send_str("OC7_ON\n");
			//Send_WRITE_command(0xB01D, 0x01);
		}
		else
		{
			SPI_RW(OC7_OFF);
			Status_OCD[7] = 0;
//			send_str("OC7_OFF\n");
			//Send_WRITE_command(0xB01D, 0x00);
		}
		Num_Menu = 42;
	}

	return Num_Menu;
}
//Функция возврата в предидущее меню
//Принимает переменную времени задержки обработки прерываний
//Возвращает переменную позиции уменьшенную на один
uint8_t Back(uint32_t time)
{
	Check--;
	HAL_Delay(time);
	return Check;
}
//Функция отрисовки подменю: "Выводы базового блока"
void Menu_Output(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(42, 0, "OUTPUTS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	ST7735_DrawString(0, 30, "- ANALOG", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	ST7735_DrawString(0, 48, "- DIGITAL", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	ST7735_DrawString(0, 66, "- OPEN DRAIN", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	ST7735_DrawString(0, 84, "- 1-WIRE", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	ST7735_DrawString(0, 102, "- PWM", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	cnt1 = 1;
	cnt2 = 1;
}
//Функция отрисовки подменю: "Подключенные блоки"
void Menu_Blocks(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(48, 0, "BLOCKS", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	ST7735_DrawString(0, 36, "- RELAY", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	ST7735_DrawString(0, 54, "- DIGITAL", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	ST7735_DrawString(0, 72, "- DIMMING", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	ST7735_DrawString(0, 90, "- INTERFACE", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	cnt1 = 1;
	cnt2 = 1;
}
//Функция отрисовки меню: "Главное меню"
void Str_Time(void)
{
    HAL_RTC_GetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN);
    snprintf(trans_str, 63, "%.02d-%.02d-20%d", DateToUpdate.Date, DateToUpdate.Month, DateToUpdate.Year);
	//----------------------Date-----------------------
	ST7735_DrawString(2, 120, trans_str, Font_7x10, ST7735_WHITE, ST7735_BLACK);

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN); // RTC_FORMAT_BIN , RTC_FORMAT_BCD
    snprintf(trans_str, 63, "%.02d:%.02d", sTime.Hours, sTime.Minutes);
	//----------------------Time-----------------------
    ST7735_DrawString(123, 120, trans_str, Font_7x10, ST7735_WHITE, ST7735_BLACK);
}
//Функция отрисовки меню: "Главное меню"
void Menu_Main(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(3, 0, "DEV", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	ST7735_DrawString(36, 0, "ELECTRONICS", Font_11x18, ST7735_BLACK, ST7735_WHITE);
	ST7735_DrawString(30, 36, "OUTPUT", Font_16x26, ST7735_WHITE, ST7735_BLACK);
	ST7735_DrawString(30, 75, "BLOCKS", Font_16x26, ST7735_WHITE, ST7735_BLACK);

    HAL_RTC_GetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN);
    snprintf(trans_str, 63, "%.02d-%.02d-20%d", DateToUpdate.Date, DateToUpdate.Month, DateToUpdate.Year);
	//----------------------Date-----------------------
	ST7735_DrawString(2, 120, trans_str, Font_7x10, ST7735_WHITE, ST7735_BLACK);

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN); // RTC_FORMAT_BIN , RTC_FORMAT_BCD
    snprintf(trans_str, 63, "%.02d:%.02d", sTime.Hours, sTime.Minutes);
	//----------------------Time-----------------------
    ST7735_DrawString(123, 120, trans_str, Font_7x10, ST7735_WHITE, ST7735_BLACK);
	//---------------------Version---------------------
	//ST7735_DrawString(94, 120, "Ver:1.0.0", Font_7x10, ST7735_WHITE, ST7735_BLACK);

	cnt1 = 1;
	cnt2 = 1;

	//Сделать запрос на MCU о состоянии входов/выходов
	//Записать все полученные данные в память микросхемы

	//Как только резервный "RSD" пин поднимается, значит необходимо обновить данные на индикаторе!

	//В процессе загрузки системы, проверить память на наличие изменений выводов
	//ReadWrite_Variable(0xB000, Status_AIN, Status_DIN, Status_PWM, Status_OCD, Status_1WR);

//	if(start_device)
//	{
//		CSM_L;
//			SPI_RW(MCU_OUTPUT);
//			for(uint8_t i = 0; i < 5; i++)
//			{
//				for(uint8_t j = 0; j < 8; j++)
//				{
//					if(i == 0)
//						Status_AIN[j] = SPI_RW(MCU_NOP);
//					if(i == 1)
//						Status_DIN[j] = SPI_RW(MCU_NOP);
//					if(i == 2)
//						Status_PWM[j] = SPI_RW(MCU_NOP);
//					if(i == 3)
//						Status_OCD[j] = SPI_RW(MCU_NOP);
//					if(i == 4)
//						Status_1WR[j] = SPI_RW(MCU_NOP);
//				}
//			}
//		CSM_H;
//		start_device = 0;
//	}
}
//Функция отрисовки подменю: "Аналоговые входы"
void Menu_Analog(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(48, 0, "ANALOG", Font_11x18, ST7735_BLUE, ST7735_WHITE);

	//Отправка запроса на данные на аналоговых входах

	ST7735_DrawString(0, 30, "- AIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_AIN[0] == 0x01)
		ST7735_DrawString(120, 30, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[0] == 0x00)
		ST7735_DrawString(120, 30, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 40, "- AIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_AIN[1] == 0x01)
		ST7735_DrawString(120, 40, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[1] == 0x00)
		ST7735_DrawString(120, 40, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 50, "- AIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_AIN[2] == 0x01)
		ST7735_DrawString(120, 50, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[2] == 0x00)
		ST7735_DrawString(120, 50, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 60, "- AIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_AIN[3] == 0x01)
		ST7735_DrawString(120, 60, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[3] == 0x00)
		ST7735_DrawString(120, 60, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 70, "- AIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_AIN[4] == 0x01)
		ST7735_DrawString(120, 70, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[4] == 0x00)
		ST7735_DrawString(120, 70, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 80, "- AIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_AIN[5] == 0x01)
		ST7735_DrawString(120, 80, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[5] == 0x00)
		ST7735_DrawString(120, 80, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 90, "- AIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_AIN[6] == 0x01)
		ST7735_DrawString(120, 90, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[6] == 0x00)
		ST7735_DrawString(120, 90, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 100, "- AIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_AIN[7] == 0x01)
		ST7735_DrawString(120, 100, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[7] == 0x00)
		ST7735_DrawString(120, 100, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	cnt1 = 1;
	cnt2 = 1;
}
//Функция отрисовки подменю: "Цифровые входы"
void Menu_Digital(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	ST7735_DrawString(0, 30, "- DIN0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_DIN[0] == 0x01)
	{
		uint16_t a = 29, b = 37;
		Rising(a, b);
	}
	else if(Status_DIN[0] == 0x00)
	{
		uint16_t a = 29, b = 37;
		Faling(b, a);
	}
	ST7735_DrawString(0, 40, "- DIN1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_DIN[1] == 0x01)
	{
		uint16_t a = 39, b = 47;
		Rising(a, b);
	}
	else if(Status_DIN[1] == 0x00)
	{
		uint16_t a = 39, b = 47;
		Faling(b, a);
	}
	ST7735_DrawString(0, 50, "- DIN2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_DIN[2] == 0x01)
	{
		uint16_t a = 49, b = 57;
		Rising(a, b);
	}
	else if(Status_DIN[2] == 0x00)
	{
		uint16_t a = 49, b = 57;
		Faling(b, a);
	}
	ST7735_DrawString(0, 60, "- DIN3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_DIN[3] == 0x01)
	{
		uint16_t a = 59, b = 67;
		Rising(a, b);
	}
	else if(Status_DIN[3] == 0x00)
	{
		uint16_t a = 59, b = 67;
		Faling(b, a);
	}
	ST7735_DrawString(0, 70, "- DIN4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_DIN[4] == 0x01)
	{
		uint16_t a = 69, b = 77;
		Rising(a, b);
	}
	else if(Status_DIN[4] == 0x00)
	{
		uint16_t a = 69, b = 77;
		Faling(b, a);
	}
	ST7735_DrawString(0, 80, "- DIN5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_DIN[5] == 0x01)
	{
		uint16_t a = 79, b = 87;
		Rising(a, b);
	}
	else if(Status_DIN[5] == 0x00)
	{
		uint16_t a = 79, b = 87;
		Faling(b, a);
	}
	ST7735_DrawString(0, 90, "- DIN6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_DIN[6] == 0x01)
	{
		uint16_t a = 89, b = 97;
		Rising(a, b);
	}
	else if(Status_DIN[6] == 0x00)
	{
		uint16_t a = 89, b = 97;
		Faling(b, a);
	}
	ST7735_DrawString(0, 100, "- DIN7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_DIN[7] == 0x01)
	{
		uint16_t a = 99, b = 107;
		Rising(a, b);
	}
	else if(Status_DIN[7] == 0x00)
	{
		uint16_t a = 99, b = 107;
		Faling(b, a);
	}
	cnt1 = 1;
	cnt2 = 1;
}
//Функция отрисовки подменю: "Открытый коллектор"
void Menu_OpenDrain(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(24, 0, "OPEN DRAIN", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	ST7735_DrawString(0, 30, "- OC0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_OCD[0] == 0x01)
		ST7735_DrawString(120, 30, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[0] == 0x00)
		ST7735_DrawString(120, 30, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 40, "- OC1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_OCD[1] == 0x01)
		ST7735_DrawString(120, 40, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[1] == 0x00)
		ST7735_DrawString(120, 40, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 50, "- OC2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_OCD[2] == 0x01)
		ST7735_DrawString(120, 50, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[2] == 0x00)
		ST7735_DrawString(120, 50, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 60, "- OC3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_OCD[3] == 0x01)
		ST7735_DrawString(120, 60, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[3] == 0x00)
		ST7735_DrawString(120, 60, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 70, "- OC4", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_OCD[4] == 0x01)
		ST7735_DrawString(120, 70, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[4] == 0x00)
		ST7735_DrawString(120, 70, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 80, "- OC5", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_OCD[5] == 0x01)
		ST7735_DrawString(120, 80, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[5] == 0x00)
		ST7735_DrawString(120, 80, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 90, "- OC6", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_OCD[6] == 0x01)
		ST7735_DrawString(120, 90, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[6] == 0x00)
		ST7735_DrawString(120, 90, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 100, "- OC7", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_OCD[7] == 0x01)
		ST7735_DrawString(120, 100, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[7] == 0x00)
		ST7735_DrawString(120, 100, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	cnt1 = 1;
	cnt2 = 1;
}
//Функция отрисовки подменю: "1-Wire"
void Menu_1Wire(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(48, 0, "1-WIRE", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	ST7735_DrawString(0, 30, "- WIRE0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_1WR[0] == 0x01)
		ST7735_DrawString(120, 30, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_1WR[0] == 0x00)
		ST7735_DrawString(120, 30, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 40, "- WIRE1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_1WR[1] == 0x01)
		ST7735_DrawString(120, 40, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_1WR[1] == 0x00)
		ST7735_DrawString(120, 40, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	cnt1 = 1;
	cnt2 = 1;
}
//Функция отрисовки подменю: "ШИМ"
void Menu_PWM(void)
{
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(71, 0, "PWM", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	ST7735_DrawString(0, 30, "- PWM0", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_PWM[0] == 0x01)
		ST7735_DrawString(120, 30, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_PWM[0] == 0x00)
		ST7735_DrawString(120, 30, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 40, "- PWM1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_PWM[1] == 0x01)
		ST7735_DrawString(120, 40, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_PWM[1] == 0x00)
		ST7735_DrawString(120, 40, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 50, "- PWM2", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_PWM[2] == 0x01)
		ST7735_DrawString(120, 50, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_PWM[2] == 0x00)
		ST7735_DrawString(120, 50, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	ST7735_DrawString(0, 60, "- PWM3", Font_7x10, ST7735_WHITE, ST7735_BLACK);
	if(Status_PWM[3] == 0x01)
		ST7735_DrawString(120, 60, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_PWM[3] == 0x00)
		ST7735_DrawString(120, 60, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	cnt1 = 1;
	cnt2 = 1;
}
//Функция отрисовки подменю во вкладках блоки
void Filling_Menu_Block(uint8_t CMD, uint8_t count, uint8_t Name)
{
	char item[1] = "";
	char str[5] = "";

	ST7735_FillScreen(ST7735_BLACK);
	if(Name == 0)
		ST7735_DrawString(53, 0, "RELAY", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	if(Name == 1)
		ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	if(Name == 2)
		ST7735_DrawString(42, 0, "DIMMING", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	if(Name == 3)
		ST7735_DrawString(31, 0, "INTERFACE", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	CSM_L;
	SPI_RW(CMD);
	for(uint8_t i = 0, column = 30; i < count; i++, column+=18)
	{
		itoa(i, item, 10);
		itoa(SPI_RW(MCU_NOP), str, 10);
		if(i <= 4)
		{
			ST7735_DrawString(7, column, item, Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(18, column, "-", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(30, column, str, Font_11x18, ST7735_WHITE, ST7735_BLACK);
		}
		else
		{
			ST7735_DrawString(95, (column - 90), item, Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(105, (column - 90), "-", Font_11x18, ST7735_WHITE, ST7735_BLACK);
			ST7735_DrawString(115, (column - 90), str, Font_11x18, ST7735_WHITE, ST7735_BLACK);
		}
	}
	CSM_H;
}
//Функция отрисовки сообщения о не найденных блоков
void Not_Found_Block(uint8_t Name)
{
	ST7735_FillScreen(ST7735_BLACK);
	if(Name == 0)
		ST7735_DrawString(53, 0, "RELAY", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	if(Name == 1)
		ST7735_DrawString(42, 0, "DIGITAL", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	if(Name == 2)
		ST7735_DrawString(42, 0, "DIMMING", Font_11x18, ST7735_BLUE, ST7735_WHITE);
	if(Name == 3)
		ST7735_DrawString(31, 0, "INTERFACE", Font_11x18, ST7735_BLUE, ST7735_WHITE);

	ST7735_DrawString(48, 54, "BLOCKS", Font_11x18, ST7735_WHITE, ST7735_BLACK);
	ST7735_DrawString(31, 72, "NOT FOUND", Font_11x18, ST7735_WHITE, ST7735_BLACK);


}
//Функция отображения состояния меню аналоговых входов
void Visible_Analog(void)
{
	if(Status_AIN[0] == 0x01)
		ST7735_DrawString(120, 30, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[0] == 0x00)
		ST7735_DrawString(120, 30, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_AIN[1] == 0x01)
		ST7735_DrawString(120, 40, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[1] == 0x00)
		ST7735_DrawString(120, 40, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_AIN[2] == 0x01)
		ST7735_DrawString(120, 50, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[2] == 0x00)
		ST7735_DrawString(120, 50, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_AIN[3] == 0x01)
		ST7735_DrawString(120, 60, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[3] == 0x00)
		ST7735_DrawString(120, 60, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_AIN[4] == 0x01)
		ST7735_DrawString(120, 70, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[4] == 0x00)
		ST7735_DrawString(120, 70, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_AIN[5] == 0x01)
		ST7735_DrawString(120, 80, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[5] == 0x00)
		ST7735_DrawString(120, 80, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_AIN[6] == 0x01)
		ST7735_DrawString(120, 90, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[6] == 0x00)
		ST7735_DrawString(120, 90, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_AIN[7] == 0x01)
		ST7735_DrawString(120, 100, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_AIN[7] == 0x00)
		ST7735_DrawString(120, 100, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
}
//Функция отображения состояния меню цифровых входов
void Visible_Digital(void)
{
	if(Status_DIN[0] == 0x01)
	{
		uint16_t a = 29, b = 37;
		Rising(a, b);
	}
	else if(Status_DIN[0] == 0x00)
	{
		uint16_t a = 29, b = 37;
		Faling(b, a);
	}
	if(Status_DIN[1] == 0x01)
	{
		uint16_t a = 39, b = 47;
		Rising(a, b);
	}
	else if(Status_DIN[1] == 0x00)
	{
		uint16_t a = 39, b = 47;
		Faling(b, a);
	}
	if(Status_DIN[2] == 0x01)
	{
		uint16_t a = 49, b = 57;
		Rising(a, b);
	}
	else if(Status_DIN[2] == 0x00)
	{
		uint16_t a = 49, b = 57;
		Faling(b, a);
	}
	if(Status_DIN[3] == 0x01)
	{
		uint16_t a = 59, b = 67;
		Rising(a, b);
	}
	else if(Status_DIN[3] == 0x00)
	{
		uint16_t a = 59, b = 67;
		Faling(b, a);
	}
	if(Status_DIN[4] == 0x01)
	{
		uint16_t a = 69, b = 77;
		Rising(a, b);
	}
	else if(Status_DIN[4] == 0x00)
	{
		uint16_t a = 69, b = 77;
		Faling(b, a);
	}
	if(Status_DIN[5] == 0x01)
	{
		uint16_t a = 79, b = 87;
		Rising(a, b);
	}
	else if(Status_DIN[5] == 0x00)
	{
		uint16_t a = 79, b = 87;
		Faling(b, a);
	}
	if(Status_DIN[6] == 0x01)
	{
		uint16_t a = 89, b = 97;
		Rising(a, b);
	}
	else if(Status_DIN[6] == 0x00)
	{
		uint16_t a = 89, b = 97;
		Faling(b, a);
	}
	if(Status_DIN[7] == 0x01)
	{
		uint16_t a = 99, b = 107;
		Rising(a, b);
	}
	else if(Status_DIN[7] == 0x00)
	{
		uint16_t a = 99, b = 107;
		Faling(b, a);
	}
}
//Функция отображения состояния меню выходов ШИМ
void Visible_PWM(void)
{
	if(Status_PWM[0] == 0x01)
		ST7735_DrawString(120, 30, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_PWM[0] == 0x00)
		ST7735_DrawString(120, 30, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_PWM[1] == 0x01)
		ST7735_DrawString(120, 40, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_PWM[1] == 0x00)
		ST7735_DrawString(120, 40, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_PWM[2] == 0x01)
		ST7735_DrawString(120, 50, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_PWM[2] == 0x00)
		ST7735_DrawString(120, 50, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_PWM[3] == 0x01)
		ST7735_DrawString(120, 60, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_PWM[3] == 0x00)
		ST7735_DrawString(120, 60, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
}
//Функция отображения состояния меню выходов интерфейса 1-Wire
void Visible_1Wire(void)
{
	if(Status_1WR[0] == 0x01)
		ST7735_DrawString(120, 30, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_1WR[0] == 0x00)
		ST7735_DrawString(120, 30, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_1WR[1] == 0x01)
		ST7735_DrawString(120, 40, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_1WR[1] == 0x00)
		ST7735_DrawString(120, 40, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
}
//Функция отображения состояния меню выходов открытый коллектор
void Visible_OpenDrain(void)
{
	if(Status_OCD[0] == 0x01)
		ST7735_DrawString(120, 30, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[0] == 0x00)
		ST7735_DrawString(120, 30, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_OCD[1] == 0x01)
		ST7735_DrawString(120, 40, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[1] == 0x00)
		ST7735_DrawString(120, 40, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_OCD[2] == 0x01)
		ST7735_DrawString(120, 50, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[2] == 0x00)
		ST7735_DrawString(120, 50, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_OCD[3] == 0x01)
		ST7735_DrawString(120, 60, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[3] == 0x00)
		ST7735_DrawString(120, 60, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_OCD[4] == 0x01)
		ST7735_DrawString(120, 70, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[4] == 0x00)
		ST7735_DrawString(120, 70, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_OCD[5] == 0x01)
		ST7735_DrawString(120, 80, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[5] == 0x00)
		ST7735_DrawString(120, 80, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_OCD[6] == 0x01)
		ST7735_DrawString(120, 90, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[6] == 0x00)
		ST7735_DrawString(120, 90, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
	if(Status_OCD[7] == 0x01)
		ST7735_DrawString(120, 100, " ON", Font_7x10, ST7735_GREEN, ST7735_BLACK);
	else if(Status_OCD[7] == 0x00)
		ST7735_DrawString(120, 100, "OFF", Font_7x10, ST7735_RED, ST7735_BLACK);
}
