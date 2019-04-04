#ifndef __PRINTER_H
#define __PRINTER_H

#include "stm32f10x.h"


#define DeleteSpaceNum          8//打印时若开头为空格，定义略过的个数




#define  ESC_CMD          0x1B
#define  FS_CMD           0x1C

#define  REAL_LINE        0x2C
#define  DOTS_LINE        0x27

//#define SP_RMDIIID 
#define SP_D10


#ifdef  SP_RMDIIID 
	#define   PrinterCTS_Port          GPIOB
	#define   PrinterCTS_Pin           GPIO_Pin_4 
	#define   PrinterOnline_Port       GPIOB
	#define   PrinterOnline_Pin        GPIO_Pin_3
	
	#define   PrinterCTS_Value         GPIO_ReadInputDataBit(PrinterCTS_Port, PrinterCTS_Pin);
  #define   PrinterOnline_Value      GPIO_ReadInputDataBit(PrinterOnline_Port, PrinterOnline_Pin);
#endif

#define   NowUSART_Port_CLK        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE) ;
#define   NowUSART_Port            GPIOA
#define   NowUSART_RXPin           GPIO_Pin_10
#define   NowUSART_TXPin           GPIO_Pin_9
#define   NowUSART_ClockEnable     RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#define   NowUSART			           USART1
#define   NowUSART_IRQn            USART1_IRQn



#define PrinterON  0xa
#define PrinterOFF 0x0


//#define PrinterON  0xa
//#define PrinterOFF 0x0




void USART_Printer_Config(unsigned int baud);
void Printer_Init(void);
void PrinterString(char* prtstr);
void PrinterChar(char chr);
void PrinterCurve(u8 NumOfDots,u8 *ptrstr);//,u8 CurveType
void PrinterLiner(u16 Startpoint,u16 Endpoint,u8 LinerType);
void NRow_ZouZhi(u8 NumOfRow);     //n行走纸

#endif
