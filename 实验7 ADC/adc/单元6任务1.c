#include "ioCC2530.h"
#define uint unsigned int
#define uchar unsigned char

//������ƵƵĶ˿�
#define LED1 P1_0       //����P1.0��ΪLED1���ƶ�
#define LED2 P1_1       //����P1.1��ΪLED2���ƶ�

void delay(uint);
void initUART(void);
void InitialAD(void);
void UartTX_Send_String(char *Data,int len);

char adcdata[]=" 0.0V\n ";
char adc_ascii[]="adc:    0\n";

#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void) {
  char temp[2];
  uint adc = 0;
  float num = 0;

  LED1 = 0;	  	//ת�����ָʾ
  LED2= 1;           //�����ݴ���ָʾ��
  temp[1] = ADCL; //ADC Data Low
  temp[0] = ADCH; //ADC Data High

  adc |= (uint)temp[1];
  adc |= ( (uint) temp[0] )<<8;
  adc>>=2; // ADCL[1:0]û��ʹ��
  num = adc*3.3/8192;//���ο���ѹΪ3.3V��14λ�ֱ���
  adc_ascii[4] = (char)(adc/10000 + 48);
  adc_ascii[5] = (char)(adc/1000%10 + 48);
  adc_ascii[6] = (char)(adc/100%10 + 48);
  adc_ascii[7] = (char)(adc/10%10 + 48);
  adc_ascii[8] = (char)(adc%10 + 48);
  UartTX_Send_String(adc_ascii, 10);
  adcdata[1] = (char)(num)%10+48;
  adcdata[3] = (char)(num*10)%10+48;
  UartTX_Send_String(adcdata,6);	//��������    
  LED2= 0;                       //������ݴ���   
  ADCIF = 0; //�����жϱ�־
  InitialAD(); //������һ��ת��
}

/****************************************************************
*�������� ��������						*
*��ڲ��� ����							*
*�� �� ֵ ����							*
*˵    �� ����							*
****************************************************************/
void main(void)
{	
  P1DIR |= 0x03;
  
  
  initUART();   //��ʼ������
  InitialAD();       //��ʼ��ADC

  while(1)
  {       
  //  if(ADCCON1>=0x80)    //ADCCON1.EOC,ת������ж�
  //   {
	   
  //     delay(5000);

  //     InitialAD(); //������һ��ת��
  //   }
  }
}


/****************************************************************
*�������� ����ʱ						*
*��ڲ��� ��������ʱ						*
*�� �� ֵ ����							*
*˵    �� ��							*
****************************************************************/
void delay(uint time)
{ uint i;
  uchar j;
  for(i = 0; i < time; i++)
  {  for(j = 0; j < 240; j++) 
      {   asm("NOP");    // asm����Ƕ��࣬nop�ǿղ���,ִ��һ��ָ������
          asm("NOP");
          asm("NOP");
       }  
   }  
}


/****************************************************************
*�������� ����ʼ������1						*
*��ڲ��� ����							*
*�� �� ֵ ����							*
*˵    �� ��57600-8-n-1						*
****************************************************************/
void initUART(void)
{
 CLKCONCMD &= 0x80;           //����32MHz
  
  PERCFG = 0x00;			//λ��1 P0��
  P0SEL = 0x0C;				//P0��������
  
  U0CSR |= 0x80;			//UART��ʽ
  U0GCR |= 10;				//baud_e = 10;
  U0BAUD |= 216;			//��������Ϊ57600
  UTX0IF = 1;
  
  U0CSR |= 0X40;			//�������
  IEN0 |= 0x04;				//�����ж�
}


/****************************************************************
*�������� ����ʼ��ADC						*
*��ڲ��� ����					        	*
*�� �� ֵ ����							*
*˵    �� ���ο���ѹAVDD��ת��������1/3AVDD			*
****************************************************************/
void InitialAD(void)
{
// 0000 0000 | 1 = 0000 0001 
  P0SEL  |= (1 << (0));	  //����P0.0Ϊ����IO��
  P0DIR  &= ~ (1 << (0)); //����P0.0Ϊ����I/O

  LED1=1; //����ADCת���� ָʾ��
    
  ADCCON1 &= ~0x80;		//��EOC��־	
  APCFG  |=1;             //����P0.0Ϊģ��I/O  
  ADCCON3=0xb0;		//����ת��,�ο���ѹΪ��Դ��ѹ  //14λ�ֱ���
  ADCCON1 = 0X30;	//ֹͣA/D
  ADCCON1 |= 0X40;	//ADCCON1.ST=1������A/D
  
  IEN0 |= 0x02; //����ADC�ж�
  ADCIF = 0; //�����жϱ�־
  EA = 1;
}
/****************************************************************
*�������� �����ڷ����ַ�������					*
*��ڲ��� : data:����						*
*	    len :���ݳ���				        *
*�� �� ֵ ����						        *
*˵    �� ��							*
****************************************************************/
void UartTX_Send_String(char *Data,int len)
{
  int j;
  for(j=0;j<len;j++)
  {
    U0DBUF = *Data++;
    while(UTX0IF == 0);
    UTX0IF = 0;
  }
}