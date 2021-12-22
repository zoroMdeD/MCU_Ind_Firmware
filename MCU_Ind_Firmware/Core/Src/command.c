/*
 * command.c
 *
 *  Created on: 9 окт. 2020 г.
 *      Author: mmorozov
 */
#include "main.h"
#include "stm32f1xx_hal.h"
#include "st7735.h"

#define READ	0x1B	//Continuous Array Read
#define WRITE	0x02	//Main Memory Byte
#define RDSR	0xD7	//Status Register Read
#define RDID	0x9F	//Read Device ID
#define NOP		0x00	//No Data

extern uint8_t SPI_rx_buf[1];
extern uint8_t SPI_tx_buf[1];
extern uint8_t flag_output_spi1;

//Функция передачи байта по USART
void USART_Tx(unsigned char Data)
{
	while(!(USART2->SR & USART_SR_TC));
	USART2->DR = Data;
}
//Функция отправки сткроки
//Принимает строку для отправки
void send_str(char * string)
{
	uint8_t i = 0;
	while(string[i])
	{
		USART_Tx(string[i]);
		i++;
	}
}
//Функция приема/передачи данных по SPI
//Принимает данные для передачи ведомому устройству
//Возвращает данные с ведомого устройства
uint8_t SPI_RW(uint8_t data)
{
//	uint8_t TxBuffer[1] = {0};
//	uint8_t RxBuffer[1] = {0};
//	TxBuffer[0] = data;
//
//	HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)TxBuffer, (uint8_t*)RxBuffer, 1, 100);
//
//	return RxBuffer[0];

	SPI_tx_buf[0] = data;

	CSM_L;
	HAL_SPI_TransmitReceive_IT(&hspi1, (uint8_t*)SPI_tx_buf, (uint8_t *)SPI_rx_buf, 1);
    while(!flag_output_spi1) {;}
    flag_output_spi1 = 0;

    return SPI_rx_buf[0];
}
//Функция получения обработанных данных с АЦП канала измерения яркости освещения
//Возвращает преобразованное значение с канала АЦП
float Conv_ADC1(void)
{
	float Value = 0;
	float ADC_value = 0;
	const float Resolution = 0.0008056640625;

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	ADC_value = HAL_ADC_GetValue(&hadc1);

	Value = ADC_value * Resolution;

	return Value;
}
//Функция получения необработанных данных с АЦП канала измерения яркости освещения
//Возвращает значение с канала АЦП
uint16_t Get_ADC1(void)
{
	uint16_t ADC_value = 0;

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	ADC_value = HAL_ADC_GetValue(&hadc1);

	return ADC_value;
}
//----------------------------------------Комманды микросхемы памяти----------------------------------------
//Размер памяти микросхемы 0x3FFFFF (4Mbit)

//Фукция чтения регистра статуса
//Возвращает данные регистра статуса
uint16_t Send_RDSR_command(void)
{
	uint16_t data = 0;
	CSF_L;
		SPI_RW(RDSR);
		data = (uint8_t)(SPI_RW(NOP));
		data = data << 8;
		data += (uint8_t)(SPI_RW(NOP));
	CSF_H;

	return data;
}
//Фукция записи в память микросхемы
//Принимает адрес ячейки пямяти
//Принимает данные для записи в ячейку
void Send_WRITE_command(uint32_t addr, uint8_t data)
{
	CSF_L;
		SPI_RW(WRITE);
		SPI_RW((uint8_t)(addr>>16));
		SPI_RW((uint8_t)(addr>>8));
		SPI_RW((uint8_t)(addr));
		SPI_RW(data);
	CSF_H;
}
//Фукция чтения из памяти микросхемы
//Принимает адрес ячейки пямяти
//Возвращает данные которые читаем из ячейки 1 байт
uint8_t Send_READ_command(uint32_t addr)
{
	uint8_t data = 0;
	CSF_L;
		SPI_RW(READ);
		SPI_RW((uint8_t)(addr>>16));
		SPI_RW((uint8_t)(addr>>8));
		SPI_RW((uint8_t)(addr));
		SPI_RW(NOP);
		SPI_RW(NOP);
		data = SPI_RW(NOP);
	CSF_H;

	return data;
}
//Фукция чтения из памяти микросхемы
//Принимает адрес ячейки пямяти
//Возвращает данные которые читаем из ячейки 2 байта
uint16_t Send_READ_command_2B(uint32_t addr)
{
	uint16_t data = 0;
	CSF_L;
		SPI_RW(READ);
		SPI_RW((uint8_t)(addr>>16));
		SPI_RW((uint8_t)(addr>>8));
		SPI_RW((uint8_t)(addr));
		SPI_RW(NOP);
		SPI_RW(NOP);
		data = (uint8_t)(SPI_RW(NOP));
	CSF_H;
	addr++;
	data = data << 8;
	CSF_L;
		SPI_RW(READ);
		SPI_RW((uint8_t)(addr>>16));
		SPI_RW((uint8_t)(addr>>8));
		SPI_RW((uint8_t)(addr));
		SPI_RW(NOP);
		SPI_RW(NOP);
		data += (uint8_t)(SPI_RW(NOP));
	CSF_H;

	return data;
}
//Фукция настройки страниц памяти(binary mode): 256 bytes page
void Binary_page_mode(void)
{
	CSF_L;
		SPI_RW(0x3D);
		SPI_RW(0x2A);
		SPI_RW(0x80);
		SPI_RW(0xA6);
	CSF_H;
}
//Фукция настройки страниц памяти(flash mode): 264 bytes page
void Data_flash_page_mode(void)
{
	CSF_L;
		SPI_RW(0x3D);
		SPI_RW(0x2A);
		SPI_RW(0x80);
		SPI_RW(0xA7);
	CSF_H;
}
//Фукция сброса блокировки секторов
void Freeze_Sector_Lockdown(void)
{
	CSF_L;
		SPI_RW(0x34);
		SPI_RW(0x55);
		SPI_RW(0xAA);
		SPI_RW(0x40);
	CSF_H;
}
//----------------------------------------------------------------------------------------------------------
//Фукция инициализации микросхемы памяти
void Init_Card(void)
{
//	uint16_t status = 0;
//
//	WP_H;
//	Binary_page_mode();
//	HAL_Delay(250);
//	Freeze_Sector_Lockdown();
//	HAL_Delay(250);
//	status = Send_RDSR_command();
//	USART_Tx(status >> 8);
//	USART_Tx(status);
}
/*
//Фукция записи микросхемы памяти
//Принимает адрес ячейки пямяти с которой начнется запись на равное кол-во байт в массиве
//Принимает массив с данными, который необходимо записать
void Write_Variable(uint32_t addr, uint8_t* mass)
{
	for(uint8_t i = 0; i < sizeof(mass); i++, addr++)
	Send_WRITE_command(addr, mass[i]);
}
*/
/*
//Фукция обновления переменных статуса входов/выходов
//Принимает адрес ячейки пямяти
//Принимает массивы переменных статуса входов/выходов
void ReadWrite_Variable(uint32_t addr, uint8_t* mass_1, uint8_t* mass_2, uint8_t* mass_3, uint8_t* mass_4, uint8_t* mass_5)
{
	for(uint8_t i = 0; i < 8; i++, addr++)
	{
		mass_1[i] = Send_READ_command(addr);
	}
	for(uint8_t i = 0; i < 8; i++, addr++)
	{
		mass_2[i] = Send_READ_command(addr);
	}
	for(uint8_t i = 0; i < 3; i++, addr++)		//Размерность массива!!!(см. стек памяти)
	{
		mass_3[i] = Send_READ_command(addr);
	}
	for(uint8_t i = 0; i < 8; i++, addr++)
	{
		mass_4[i] = Send_READ_command(addr);
	}
	for(uint8_t i = 0; i < 2; i++, addr++)		//Размерность массива!!!(см. стек памяти)
	{
		mass_5[i] = Send_READ_command(addr);
	}
}
*/
//Фукция регулировки дисплея
//Принимает данные с АЦП(датчик освещенности)
void BackLight(uint16_t data)
{
	TIM2->CCR1 = data;
}
//Фукция отрисовки символа нарастающего фронта
//Принимает координаты отрисовки
void Rising(uint16_t a, uint16_t b)
{
	//---------------Стирание линий---------------
	ST7735_DrawLine(123, a, 126, a, ST7735_BLACK);
	ST7735_DrawLine(126, a, 134, b, ST7735_BLACK);
	ST7735_DrawLine(134, b, 137, b, ST7735_BLACK);
	//--------------------------------------------

	ST7735_DrawLine(123, b, 126, b, ST7735_RED);
	ST7735_DrawLine(126, b, 134, a, ST7735_RED);
	ST7735_DrawLine(134, a, 137, a, ST7735_RED);
}
//Фукция отрисовки символа спадающего фронта
//Принимает координаты отрисовки
void Faling(uint16_t a, uint16_t b)
{
	//---------------Стирание линий---------------
	ST7735_DrawLine(123, a, 126, a, ST7735_BLACK);
	ST7735_DrawLine(126, a, 134, b, ST7735_BLACK);
	ST7735_DrawLine(134, b, 137, b, ST7735_BLACK);
	//--------------------------------------------

	ST7735_DrawLine(123, b, 126, b, ST7735_BLUE);
	ST7735_DrawLine(126, b, 134, a, ST7735_BLUE);
	ST7735_DrawLine(134, a, 137, a, ST7735_BLUE);
}

