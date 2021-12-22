/*
 * command.h
 *
 *  Created on: 9 окт. 2020 г.
 *      Author: mmorozov
 */

#ifndef INC_COMMAND_H_
#define INC_COMMAND_H_

void USART_Tx(unsigned char Data);
void send_str(char * string);
uint8_t SPI_RW(uint8_t data);
float Conv_ADC1(void);
uint16_t Get_ADC1(void);
uint16_t Send_RDSR_command(void);
void Send_WRITE_command(uint32_t addr, uint8_t data);
uint8_t Send_READ_command(uint32_t addr);
uint16_t Send_READ_command_2B(uint32_t addr);
void Binary_page_mode(void);
void Data_flash_page_mode(void);
void Freeze_Sector_Lockdown(void);
void Init_Card(void);
//void Write_Variable(uint32_t addr, uint8_t* mass);
//void ReadWrite_Variable(uint32_t addr, uint8_t* mass_1, uint8_t* mass_2, uint8_t* mass_3, uint8_t* mass_4, uint8_t* mass_5);
void BackLight(uint16_t data);
void Rising(uint16_t a, uint16_t b);
void Faling(uint16_t a, uint16_t b);

#endif /* INC_COMMAND_H_ */
