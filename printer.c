#include "printer.h"


static void TimeDelay (u32  DelayTime)
{
  u32  i,j;
   for(i=DelayTime;i!=0;i--)
   {
        for(j=200;j!=0;j--)
        {
        }
   }
}

void USART_Printer_Config(unsigned int baud)
{
	  GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);  // 
	  
	  NowUSART_Port_CLK;
	  NowUSART_ClockEnable;
    //GPIO的配置
	//usart Tx
	  GPIO_InitStruct.GPIO_Pin = NowUSART_TXPin ;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(NowUSART_Port, &GPIO_InitStruct);
   //RX
    GPIO_InitStruct.GPIO_Pin = NowUSART_RXPin ;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(NowUSART_Port, &GPIO_InitStruct);

	#ifdef  SP_RMDIIID 
	  GPIO_InitStruct.GPIO_Pin = PrinterCTS_Pin ;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PrinterCTS_Port, &GPIO_InitStruct);
	
	  GPIO_InitStruct.GPIO_Pin = PrinterOnline_Pin ;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PrinterOnline_Port, &GPIO_InitStruct);
	
	#endif
	
	
   
  //NVIC的配置
//   NVIC_InitStructure.NVIC_IRQChannel = NowUSART_IRQn;
//   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//   NVIC_Init(&NVIC_InitStructure);
   
   
   //usart的配置
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    /* USART configuration */
    USART_Init( NowUSART, &USART_InitStructure);
    
//    USART_ITConfig(NowUSART,USART_IT_RXNE,ENABLE);	  // USART_IT_TC|		 |USART_IT_RXNE
//    USART_ClearFlag(NowUSART,USART_FLAG_TXE);
//    USART_ClearFlag(NowUSART,USART_IT_RXNE);
    /* Enable USART */
    USART_Cmd( NowUSART, ENABLE); 
		
		
	  Printer_Init();
}


/*******************************************************************************
**函数名称:PrinterChar()
**函数功能:输出一个字节
**函数入口:无
**函数出口:无
*******************************************************************************/
void PrinterChar(char chr)
{
	//USART_SendData(USARTused,chr);
	//rt_hw_console_putc(chr);
	  USART_SendData(NowUSART, (u8)chr);
	  /* Loop until the end of transmission */
	  while(USART_GetFlagStatus(NowUSART, USART_FLAG_TXE) == RESET)
	  {
	  }
}



void Printer_Init(void)
{
	#ifdef  SP_RMDIIID 
	 //打印 机初始化
   PrinterChar(ESC_CMD);
   PrinterChar(0x40);
	 TimeDelay (50); 
	
	//设置反向打印
   PrinterChar(ESC_CMD);
   PrinterChar(0x63);
   PrinterChar(0x01);    //1  fan xiang    0 jin zhi fan xiang
//	rt_thread_delay(20);
   TimeDelay (50);
	
//	//设置左限
//	 PrinterChar(ESC_CMD);
//   PrinterChar(0x6C);
//   PrinterChar(0x00);
////	rt_thread_delay(20);
//   TimeDelay (50);
//	//设置右限
//	 PrinterChar(ESC_CMD);
//   PrinterChar(0x51);
//   PrinterChar(0x00);
////	rt_thread_delay(20);
//   TimeDelay (50);
	
	//设置字符间距
	 PrinterChar(ESC_CMD);
   PrinterChar(0x70);
   PrinterChar(0x04);    //0--255   04
//	rt_thread_delay(20);
   TimeDelay (50);
    // 行距	
	 PrinterChar(ESC_CMD);
   PrinterChar(0x31);
   PrinterChar(0x08);    //0--255  08
//	rt_thread_delay(20);
   TimeDelay (50);
	
	//字体宽度设置   横向放大
   PrinterChar(ESC_CMD);
   PrinterChar(0x55);
   PrinterChar(0x02);      //1——8   02
//		rt_thread_delay(20);
   TimeDelay (50);
	 
	 //字体宽度设置   纵向放大
   PrinterChar(ESC_CMD);
   PrinterChar(0x56);
   PrinterChar(0x02);      //1——8    02 
//		rt_thread_delay(20);
   TimeDelay (50);
	 
	 //十六进制打印
   PrinterChar(ESC_CMD);
   PrinterChar(0x22);
   PrinterChar(0x00);      //0禁止1允许
//		rt_thread_delay(20);
   TimeDelay (50);
	 
	 //设置中文打印方式
   PrinterChar(FS_CMD);
   PrinterChar(0x26);
//		rt_thread_delay(20);
   TimeDelay (50);
	 
	#elif defined SP_D10
	//打印 机初始化
   PrinterChar(ESC_CMD);
   PrinterChar(0x40);
	 TimeDelay (50); 
	
	//设置反向打印
   PrinterChar(ESC_CMD);
   PrinterChar(0x63);
   PrinterChar(0x00);    //1  fan xiang    0 jin zhi fan xiang
//	rt_thread_delay(20);
   TimeDelay (50);
	
//	//设置左限
//	 PrinterChar(ESC_CMD);
//   PrinterChar(0x6C);
//   PrinterChar(0x00);
////	rt_thread_delay(20);
//   TimeDelay (50);
//	//设置右限
//	 PrinterChar(ESC_CMD);
//   PrinterChar(0x51);
//   PrinterChar(0x00);
////	rt_thread_delay(20);
//   TimeDelay (50);
	
    // 行距	
	 PrinterChar(ESC_CMD);
   PrinterChar(0x31);
   PrinterChar(0x03);    //0--255  03
//	rt_thread_delay(20);
   TimeDelay (50);
	
	//字体宽度设置   横向放大
   PrinterChar(ESC_CMD);
   PrinterChar(0x55);
   PrinterChar(0x01);      //1——4   01
//		rt_thread_delay(20);
   TimeDelay (50);
	 
	 //字体宽度设置   纵向放大
   PrinterChar(ESC_CMD);
   PrinterChar(0x56);
   PrinterChar(0x01);      //1——4    01 
//		rt_thread_delay(20);
   TimeDelay (50);
	 
	 
	 //设置中文打印方式
   PrinterChar(FS_CMD);
   PrinterChar(0x26);
//		rt_thread_delay(20);
   TimeDelay (50);
//	 
//	 //设置汉字放大倍数   横向和纵向各放大1倍。n＝1时建立，n＝0时取消
//   PrinterChar(FS_CMD);
//   PrinterChar(0x57);
//	 PrinterChar(0x00);
////		rt_thread_delay(20);
//   TimeDelay (50);
	 
	 
	
	#else
   //打印 机初始化
   PrinterChar(ESC_CMD);
   PrinterChar(0x40);

//	rt_thread_delay(20);
   TimeDelay (50);
   //设置反向打印
   PrinterChar(ESC_CMD);
   PrinterChar(0x63);
   PrinterChar(0x00);
//	rt_thread_delay(20);
  TimeDelay (50);
//   //字体宽度设置   横向放大
//   PrinterChar(ESC_CMD);
//   PrinterChar(0x37);
//   PrinterChar(0x02);    //0——4
   //字体宽度设置   纵向放大
   PrinterChar(ESC_CMD);
   PrinterChar(0x38);
   PrinterChar(0x02);      //1——4
//		rt_thread_delay(20);
   TimeDelay (50);
//   //灰度设置
//   PrinterChar(0x1B);
//   PrinterChar(0x6D);
//   PrinterChar(0x02);    //1——12  1颜色最浅


   #endif

}



////E31
//void Printer_Init(void)
//{
//   //打印 机初始化
//   PrinterChar(0x1B);
//   PrinterChar(0x40);

//	rt_thread_delay(20);
////   TimeDelay (50);
//   //设置反向打印
//   PrinterChar(0x1B);
//   PrinterChar(0x7B);
//   PrinterChar(0x00);
//	rt_thread_delay(20);
////  TimeDelay (50);
////   //字体宽度设置   横向放大
////   PrinterChar(0x1B);
////   PrinterChar(0x37);
////   PrinterChar(0x02);    //0——4
//   //字体宽度设置   纵向放大
//   PrinterChar(0x1D);
//   PrinterChar(0x21);
//   PrinterChar(0x01);      //1——4
//		rt_thread_delay(20);
////   TimeDelay (50);
////   //灰度设置
////   PrinterChar(0x1B);
////   PrinterChar(0x6D);
////   PrinterChar(0x02);    //1——12  1颜色最浅


//}
/*******************************************************************************
**函数名称:PrinterGB()
**函数功能:打印一个汉字
**函数入口:*cn
**函数出口:无
*******************************************************************************/
static void PrinterGB( unsigned char GBH,unsigned char GBL)
{

    PrinterChar(GBH);
    PrinterChar(GBL);
}

/***********************************************************************************************
**函数名称:PrinterString()
**函数功能:	可汉字和字符同行打印
**函数入口:
**函数出口:无
***********************************************************************************************/
u8  NextLine=0;// 5数字与汉字同行输出     10  数字换行输出   20强制换行

void PrinterString(char* prtstr)
{
	volatile unsigned char i=0;
	unsigned char GBH,GBL;

	#if  DeleteSpaceNum
	 for (i=0;i<DeleteSpaceNum;i++)
	{
		  if(prtstr[i]!=0x20)
			{
				 break;
			}
//			else
//			{
//				  
//			}
		
	}
	
	#else
	
	
	#endif
//	i=j;
//	if(j<DeleteSpaceNum)
//	{
//		 i=0;
//	}
//	else
//	{
//		 i=j;
//	}
	if(prtstr[i]<128)        
	{
		 if(prtstr[i]==0x20)  //第一个是空格
		 {
			    Printer_Init();
 				  TimeDelay (2000);
			    while(prtstr[i]>0)
					{
						if(prtstr[i] < 128)// ASCII 
						{ 

				//			PrinterASCII(prtstr[i++]);
										PrinterChar(prtstr[i++]);
						}
						else // 中文
						{ 
							GBH=prtstr[i++];
							GBL=prtstr[i++];
							PrinterGB(GBH,GBL);
						}
					}
			    TimeDelay (100);
		 }
		 else                //第一个是数字
		 {
			    //取消中文打印方式
					 PrinterChar(FS_CMD);
					 PrinterChar(0x2E);
					 TimeDelay (100);
					 
//			    //zi fu ji 1
//					 PrinterChar(ESC_CMD);
//					 PrinterChar(0x36);
//					 TimeDelay (100);
			 
			 
			     PrinterChar(0x20);
//			     TimeDelay (10);
			     while(prtstr[i]>0)
					 {
								 if(prtstr[i]!=0x20)
								 {
									        if(prtstr[i] < 128)// ASCII 
													{ 

											//			PrinterASCII(prtstr[i++]);
																	PrinterChar(prtstr[i++]);
													}
													else // 中文
													{ 
														    
														    //zi fu ji 2
														    PrinterChar(ESC_CMD);
														    PrinterChar(0x37);
														    TimeDelay (100);
														    
														    PrinterChar(0x24);
														    i+=2;
														    //zi fu ji 1
														    PrinterChar(ESC_CMD);
														    PrinterChar(0x36);
														    TimeDelay (100);
														
														    
														
//														   //设置中文打印方式
//															 PrinterChar(FS_CMD);
//															 PrinterChar(0x26);
//															 TimeDelay (100);
//														
//																GBH=prtstr[i++];
//																GBL=prtstr[i++];
//																PrinterGB(GBH,GBL);
//														
//														
//														   //取消中文打印方式
//															 PrinterChar(FS_CMD);
//															 PrinterChar(0x2E);
//															 TimeDelay (100);
													}
								 } 
								 else
								 {
										 if(prtstr[i+1]!=0x20)
										 {
											  PrinterChar(prtstr[i++]);
										 }
										 else
										 {
											  i++;
										 }
									 
								 }
						 
					 }
				
//					 Printer_Init();
//					 TimeDelay (2000);
//			     //设置中文打印方式
//					 PrinterChar(FS_CMD);
//					 PrinterChar(0x26);
//					 TimeDelay (100);
										
		 }
		
	}
	else                 //第一个是汉字
	{
		  Printer_Init();
			TimeDelay (2000);
		   while(prtstr[i]>0)
			 {
				  if(prtstr[i]!=0x20)
					{
								 if(prtstr[i] < 128)// ASCII 
								{ 
									  
                    if((prtstr[i]>='0')&&(prtstr[i]<='9') ) 
										{
											  if(prtstr[i+1]!='#')
												{
													 if(NextLine!=5)
													 {
														  NextLine=10;
													    break;
													 }
													 else
													 {
														 
														  PrinterChar(prtstr[i++]);
													 }
													
												}
												else
												{
													 PrinterChar(0x0A);
													 PrinterChar(prtstr[i++]);
												}
											
										}
//										else  if(prtstr[i]=='/')
//										{
//											   i++;
//											   NextLine=10;
//											break;
//										}
										else
										{
											 PrinterChar(prtstr[i++]);
										}
						//			PrinterASCII(prtstr[i++]);
												
								}
								else // 中文
								{ 
									GBH=prtstr[i++];
									GBL=prtstr[i++];
									if(((GBH==0xCA)&&(GBL==0xFD))||((GBH==0xC2)&&(GBL==0xCA)))
									{
										  NextLine=5;
										
									}
									else if((GBH==0xB6)&&(GBL==0xAF))
									{
										  NextLine=20;
									}
									PrinterGB(GBH,GBL);
								}
					}
					else
					{
						  if(prtstr[i+2]!=0x20)
							{
								 PrinterChar(prtstr[i++]);
								
							}
							else
							{
								  i++;
							}
						   
					//	 break;
					}
				 
			 }
			 if(NextLine==10)
			 {
				        NextLine=0;
								PrinterChar(0x0A);
				//			 TimeDelay (20);
							 
							 //取消中文打印方式
							 PrinterChar(FS_CMD);
							 PrinterChar(0x2E);
							 TimeDelay (100);
							 
							 PrinterChar(0x20);
							 while(prtstr[i]>0)
							 {
									if(prtstr[i]!=0x20)
									{
											PrinterChar(prtstr[i++]);
									}
									else
									{
												if(prtstr[i+2]!=0x20)
												{
													 PrinterChar(prtstr[i++]);
													
												}
												else
												{
														i++;
												}
									}
							 } 
				 
			 }
			 else if(NextLine==20)
			 {
				  PrinterChar(0x0A);
			 }
			 NextLine=0;
//			 TimeDelay (20);
//			 //设置中文打印方式
//			 PrinterChar(FS_CMD);
//			 PrinterChar(0x26);
//			 TimeDelay (100);
			 
	}
	
	 
	 
	
//	while(prtstr[i]>0)
//	{
//		if(prtstr[i] < 128)// ASCII 
//		{ 

////			PrinterASCII(prtstr[i++]);
//         PrinterChar(prtstr[i++]);
//		}
//		else // 中文
//		{ 
//			GBH=prtstr[i++];
//			GBL=prtstr[i++];
//			PrinterGB(GBH,GBL);
//		}
//	}
} 



///***********************************************************************************************
//**函数名称:PrinterASCII()
//**函数功能:打印一个字符
//**函数入口:en
//**函数出口:无
//***********************************************************************************************/
//void PrinterASCII(unsigned char en)
//{
//PrinterChar(en); //输出字符
//}


void PrinterStrWithLen(u8 *ptrstr,u8 NumOfData)
{
	 while (NumOfData)
	 {
		    USART_SendData(NowUSART, *ptrstr);
				/* Loop until the end of transmission */
				while(USART_GetFlagStatus(NowUSART, USART_FLAG_TXE) == RESET)
				{
				}
		    NumOfData--;
				ptrstr++;
	 }
	
}

///***********************************************************************************************
//**函数名称:PrinterCurve(u8 NumOfDots,u8 *ptrstr,u8 CurveType)
//**函数功能:打印曲线的一行
//**函数入口:NumOfDots 点数
//           *ptrstr   数据  
//           CurveType 曲线类型    实线：REAL_LINE       虚线:DOTS_LINE
//**函数出口:无
//***********************************************************************************************/
void PrinterCurve(u8 NumOfDots,u8 *ptrstr)//,u8 CurveType
{
	  PrinterChar(ESC_CMD);
//	  PrinterChar(CurveType);
	  PrinterChar(DOTS_LINE);
	
	  PrinterChar(NumOfDots);
	//  PrinterString(ptrstr);
	  PrinterStrWithLen(ptrstr,NumOfDots*2);//NumOfDots
	  PrinterChar(0x0D);
}
/*
Startpoint  Endpoint : SP_RMDIIID取值范围在0---384 但Endpoint-Startpoint<=255,   m个点   n1L n1H-----nmL nmH   total: 2m
                       SP-D10  m个点  n1----nm   total:m
LinerType  :0表示打印一条实线，1表示隔一个点打印一个点，2表示隔两个点打印两个点，一次类推
此函数后不能接着发送打印数据，要有足够的延时
*/
void PrinterLiner(u16 Startpoint,u16 Endpoint,u8 LinerType)
{
	  u8 jjjjj=0,kkkkk=0;
	  u16 iiiii=0;
	  LinerType++;
	
	 #ifdef 	 SP_RMDIIID	
	  iiiii=(Endpoint-Startpoint);
	  if(iiiii>255)
		{
			 Endpoint=Startpoint+255;
			 jjjjj=255;
		}
		else
		{
			 jjjjj=(u8)iiiii;
		}
	  
	  PrinterChar(ESC_CMD);
//		if(LinerType==0)
//		{ 
//			 PrinterChar(REAL_LINE);
//		}
//		else
//		{
//			 PrinterChar(DOTS_LINE);
//		}
		PrinterChar(DOTS_LINE);
	  PrinterChar(jjjjj);

		
	
	 
	  for(iiiii=Startpoint;iiiii<Endpoint;iiiii++)
		{
			  jjjjj=(u8)iiiii;
			  kkkkk=(u8)(iiiii>>8);
				PrinterChar(jjjjj);
				PrinterChar(kkkkk);
			
		}
	
  
//	  PrinterStrWithLen(ptrstr,NumOfDots*2);//NumOfDots
	  PrinterChar(0x0D);
		TimeDelay (20);
		
		PrinterChar(ESC_CMD);
    PrinterChar(0x4A);
    PrinterChar(0x03);      //0禁止1允许
//		rt_thread_delay(20);
    TimeDelay (20);
	#elif defined 	SP_D10
		
	  if(Startpoint>255)
		{
			 Startpoint=255;
			 
		}
		if(Endpoint>255)
		{
			 Endpoint=255;
		}
	  jjjjj=(Endpoint-Startpoint)/LinerType;
		
	  PrinterChar(ESC_CMD);
		
		PrinterChar(DOTS_LINE);
	  PrinterChar(jjjjj);

		
	  for(iiiii=Startpoint;iiiii<=Endpoint;iiiii++)
		{
			  jjjjj=(u8)iiiii;
				PrinterChar(jjjjj);
			  iiiii= iiiii+LinerType-1;
		}	
		PrinterChar(0x0D);
		TimeDelay (20);
		
		
  #endif
		
}

void NRow_ZouZhi(u8 NumOfRow)     //n行走纸
{
	  PrinterChar(ESC_CMD);
		
		PrinterChar(0x4A);
	  PrinterChar(NumOfRow);
	  TimeDelay (20);
}