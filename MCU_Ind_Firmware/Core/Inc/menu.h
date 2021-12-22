/*
 * menu.h
 *
 *  Created on: 9 окт. 2020 г.
 *      Author: mmorozov
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

void loading(void);
void Send_READ_Status_Outputs(void);
uint8_t Data_Block(uint8_t CMD);
void DataUpdate(void);

uint8_t Main_Menu(uint8_t DOWN, uint8_t UP, uint32_t time);
uint8_t One_Menu(uint8_t DOWN, uint8_t UP, uint32_t time);
uint8_t Two_Menu(uint8_t DOWN, uint8_t UP, uint32_t time);
uint8_t Three_Menu(uint8_t DOWN, uint8_t UP, uint32_t time);
uint8_t Four_Menu(uint8_t DOWN, uint8_t UP, uint32_t time);
uint8_t Five_Menu(uint8_t DOWN, uint8_t UP, uint32_t time);
uint8_t Six_Menu(uint8_t DOWN, uint8_t UP, uint32_t time);
uint8_t Seven_Menu(uint8_t DOWN, uint8_t UP, uint32_t time);
void Handler_UpDown(uint8_t DOWN, uint8_t UP, uint32_t time);
void Handler_EnterBack(uint8_t ENTER, uint8_t BACK, uint32_t time);
uint8_t Back(uint32_t time);
void Menu_Output(void);
void Menu_Blocks(void);
void Str_Time(void);
void Menu_Main(void);
void Menu_Analog(void);
void Menu_Digital(void);
void Menu_OpenDrain(void);
void Menu_1Wire(void);
void Menu_PWM(void);
void Filling_Menu_Block(uint8_t CMD, uint8_t count, uint8_t Name);
void Not_Found_Block(uint8_t Name);
uint8_t Item_Selection(uint8_t choice);
void Visible_Analog(void);
void Visible_Digital(void);
void Visible_PWM(void);
void Visible_1Wire(void);
void Visible_OpenDrain(void);

#endif /* INC_MENU_H_ */
