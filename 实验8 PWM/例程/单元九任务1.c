#include "ioCC2530.h"

//#define LED1 P1_0     // LED1����Ϊ1_0
#define LED2 P1_1     // LED2����Ϊ1_1
//#define LED3 P1_4     // LED3����Ϊ1_4


void Timer1_Init(void)
{   
    T1CC0L = 0xC8;    //����T1CC0,PWM���ʱ�Ƚ�ֵ���� = 200
    T1CC0H = 0x00;    

    T1CCTL2 = 0x64;   //���ж�ʱ��0x24������T1�����Channel 2��ģʽ4 - Clear Output on Compare-Up,Set on Compare-Down
    //TIMIF |= 0x40;  //ʹ�ܶ�ʱ��1����ж�

    //T1CC2L = 0xF7;    //����T1���ͨ��2�Ƚ����ֵ
    //T1CC2H = 0x00;    
    T1CCTL0 = 0x64; //����CC0�ıȽ�����ж�
    //���ù���ģʽ�ͷ�Ƶϵ��
     T1CTL = 0x0E; // ģģʽ
   
     T1IE = 1; 
}


void LED_Init(void)
{
  CLKCONCMD &= 0x80;
  PERCFG |= 0x40;   //��ʱ��1 ��IOλ�� 2: T1 location=2
  P2SEL &= ~0x10; //P1.0������ʱ�ж��ֶ��壬��������ѡ��ʱ��1�Ƚ��������
  //P2DIR |= 0xC0; //��1���ȼ�����ʱ��1ͨ��2-3
  P1DIR |= 0x13;    //����LED1������P1.0, LED2������P1.1Ϊ���
  P1SEL |= 0x01;    //����P1.0Ϊ���蹦��, P1.1��Ȼ����ͨGPIO

  P1 &= ~0x13; //ȫ��  
}

#pragma vector = T1_VECTOR
__interrupt void T1_ISR(void){
    if (T1STAT & 0x04) {
      T1STAT &= ~(1 << 2);
      LED2 = 0;
    } else if (T1STAT & 0x01) {
      T1STAT &= ~(1 << 0);
      LED2 = 1;
    }
} 


void Delay(unsigned int xms)
{
 while(xms--);
}
/**************************************************************
�������ƣ�main
���ܣ�����������
��ڲ�������
���ڲ�������
����ֵ����
**************************************************************/
void main(void)
{
  unsigned char rate[16]={0xAF,0x96,0x7D,0x64,0x4B,0x32,0x19,0x0A,0x0A,0x19,0x32,0x4B,0x64,0x7D,0x96,0xAF};
  LED_Init();
  unsigned char i;
  Timer1_Init();
  LED2 = 1;
  while(1){
        for(i=0;i<16;i++)
      {
        T1CC2L = rate[i];
        T1CC2H = 0x00;
 
        EA = 1;   

        Delay(50000);
        Delay(50000);
        Delay(50000);
        Delay(50000);
        Delay(50000);
      }
  }
}