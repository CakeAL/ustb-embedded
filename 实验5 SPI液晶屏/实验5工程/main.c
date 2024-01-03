//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//�о�԰����
//  �� �� ��   : main.c
//  �� �� ��   : v2.0
//  ��    ��   : HuangKai
//  ��������   : 2018-10-31
//  ����޸�   : 
//  ��������   : LCD SPI�ӿ���ʾ����
//              ˵��: 
//              ----------------------------------------------------------------
//              GND     ��Դ��
//              VCC     3.3v��Դ
//              SCLK      P1.5��SCL��
//              MOSI      P1.6��SDA��
//              RES     P1.3
//              DC      P1.2
//              CS      ����  
//		BLK     ���� ���Ʊ���
//		MISO    P1.7
//              ----------------------------------------------------------------
// �޸���ʷ   :
// ��    ��   : 
// ��    ��   : HuangKai
// �޸�����   : �����ļ�
//��Ȩ���У�����ؾ���
//Copyright(C) �о�԰����2018-10-31
//All rights reserved
//******************************************************************************/
#include "delay.h"
#include "lcd.h"
#include <ioCC2530.h>

#define uint unsigned int
#define uchar unsigned char

void Initial_IO(void)
{
    P0DIR |= 0xff;     //P0����Ϊ���
    P1DIR |= 0xff;     //P1����Ϊ���
	// Master Mode
	PERCFG |= 0x02; //PERCFG.U1CFG = 1�����ù���ѡ��
	P1SEL |= 0xE0; //1110 0000ʹ��P1_7, P1_6, P1_5��Ϊ����
	P1SEL &= ~0x10; //1110 1111ʹ��P1_4��ΪSSN����λGPIO�������
	P1DIR |= 0x10; //SSN����Ϊ���
	// Slave mode
	// PERCFG |= 0x02; //PERCFG.U1CFG = 1�����ù���ѡ��
	// P1SEL |= 0xF0; //1111 0000ʹ��P1_7, P1_6, P1_5, P1_4��Ϊ����
	// ����SPIʱ�Ӳ����� 3.25MHz
	U1BAUD = 0x00; //BAUD_M = 0
	U1GCR = 0x11; //BAUD_E = 17
	//����SPI Masterģʽ
	U1CSR &= ~0xA0;
	//����SPI Slaveģʽ
	// U1CSR &= ~0x80;
	// U1CSR |= 0x20;
	//����ʱ�Ӽ��ԣ�ʱ����λ��bit˳��
	U1GCR &= ~0xC0; //CPOL = CPHA = 0
	U1GCR |= 0x20; //ORDER = 1
}

//******************************************************
int main(void)
 {	
	u8 i,m;
	float t=0;
    
    Initial_IO();  

	Lcd_Init();			//��ʼ��OLED  
	LCD_Clear(WHITE);
	BACK_COLOR=WHITE;

	while(1) {
		LCD_ShowChinese32x32(10,0,0,32,GBLUE);   //��
		LCD_ShowChinese32x32(45,0,1,32,GBLUE);   //��
		LCD_ShowChinese32x32(80,0,2,32,GBLUE);   //԰
		LCD_ShowChinese32x32(115,0,3,32,GBLUE);  //��
		LCD_ShowChinese32x32(150,0,4,32,GBLUE);  //��
		LCD_ShowChinese32x32(10,75,0,16,RED);   //��
		LCD_ShowChinese32x32(45,75,1,16,RED);   //��
		LCD_ShowChinese32x32(80,75,2,16,RED);   //԰
		LCD_ShowChinese32x32(115,75,3,16,RED);  //��
		LCD_ShowChinese32x32(150,75,4,16,RED);  //��
		LCD_ShowString(10,35,"2.8 TFT SPI 240*320",RED);
		LCD_ShowString(10,55,"LCD_W:",RED);	LCD_ShowNum(70,55,LCD_W,3,RED);
		LCD_ShowString(110,55,"LCD_H:",RED);LCD_ShowNum(160,55,LCD_H,3,RED);
		  
        for(i=0;i<2;i++)
		{
			for(m=0;m<6;m++)
			{
		           LCD_ShowPicture(0+m*40,120+i*40,39+m*40,159+i*40);
			}
		}
		
                while(1)
		{
                          LCD_ShowNum1(80,200,t,5,RED);
		          t+=0.01;
                }
          }//  while loop
}
