#include <math.h>
#include "sht10.h"

/**************************************************************************************************
* @fn      delay1Us
*
* @brief   wait for x us. @ 32MHz MCU clock it takes 32 "nop"s for 1 us delay.
*
* @param   x us. range[0-65536]
*
* @return   ��ʱԼΪ0.4us
**************************************************************************************************/
void delay1Us(Uint16 microSecs)   
{
  while(microSecs--)
  {
    /* 32 NOPs == 1 usecs */
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");

  }
}
/*******************************************************************************
��ʱ 1uS ������(int)�ӳ���
*******************************************************************************/
void delay (unsigned int time){
unsigned int a;
for(a=0;a<time;a++)
{
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop");
}
}

//----------------------------------------------------------------------------------
void s_connectionreset(void)
//----------------------------------------------------------------------------------
// ���Ӹ�λ;
//       _____________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
{  
  unsigned char i; 
  
  P0DIR |= 0x60; // 0110 0000
     
  DATA=1; SCK=0;                    //׼��
  for(i=0;i<9;i++)                  //DATA���ָߣ�SCKʱ�Ӵ���9�Σ������������䣬ͨѸ����λ
  { SCK=1;
    SCK=0;
  }
  s_transstart();                   //��������
}
//----------------------------------------------------------------------------------
void s_transstart(void)
//----------------------------------------------------------------------------------
// ��������
//       _____         ________
// DATA:      |_______|
//           ___     ___
// SCK : ___|   |___|   |______
{    
  P0DIR |= 0x60;
  
   DATA=1; SCK=0;                   
   delay1Us(1);
   SCK=1;
   delay1Us(1);
   DATA=0;
   delay1Us(1);
   SCK=0;  
   delay1Us(3);
   SCK=1;
   delay1Us(1);
   DATA=1;     
   delay1Us(1);
   SCK=0;     
}
//----------------------------------------------------------------------------------
char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
//----------------------------------------------------------------------------------
// �����¶Ȼ���ʪ��ת�����ɲ���mode����ת�����ݣ�
{ 
  unsigned error=0;
  s_transstart();                   //��������
  switch(mode){
    case 0x02 : error+=s_write_byte(MEASURE_TEMP); break;
    case 0x01 : error+=s_write_byte(MEASURE_HUMI); break;
    default : break;  
  }
  P0DIR |= 0x40;//0x40=0100 0000 clock
  P0DIR &= 0xDF;//d=1101,f=1111 data
 // for (i=0;i<110;i++){
  //   delay(2000);
 // if(DATA==0) break; //�ȴ�����������
//  }
   while(DATA); //�ȴ�����������
   if(DATA) error+=1;                // �����ʱ��������û�����ͣ�˵����������
  *(p_value+1)  =s_read_byte(ACK);    //����һ���ֽڣ����ֽ� (MSB)
  *(p_value)=s_read_byte(ACK);    //���ڶ����ֽڣ����ֽ� (LSB)
  *p_checksum =s_read_byte(noACK);  //read CRCУ����
  //UartTX_Send_String(p_value,2); 
  return error;
}
//----------------------------------------------------------------------------------
char s_write_byte(unsigned char value)
//----------------------------------------------------------------------------------
// д�ֽں��� 
{ 
  char i;
  char error=0;

  P0DIR |= 0x60; // C0 -> 60
 
  SCK=0;
  DATA=0;
 for(i=0;i<8;i++)  //����8λ���ݣ��Ի����������ض�ȡ����
 {
  SCK=0;
  if(value&(0x80>>i))
   DATA=1;
  else
   DATA=0;
  delay1Us(1);
  SCK=1;
  delay1Us(1);
 }
 
 SCK=0;     //�ڽ������������ض�ȡ�ӻ����͵ġ����յ����źš�
  P0DIR |= 0x40;//0x40=0100 0000 clock
  P0DIR &= 0xDF;//d=1101,f=1111 data
 delay1Us(1);
 SCK=1;
 delay1Us(1);
 error = DATA;
 delay1Us(1);
 SCK=0;
 
 P0DIR |= 0x60;
 return error;
  
}
//----------------------------------------------------------------------------------
char s_read_byte(unsigned char ack)
//----------------------------------------------------------------------------------
// �����ݣ�
{ 
  unsigned char i,val=0;
  
  //DATA=1; //������Ϊ��
  
  P0DIR |= 0x40;//0x40=0100 0000 clock
  P0DIR &= 0xDF;//d=1101,f=1111 data
  SCK=0;
  for (i=0x80;i>0;i>>=1)             //����λ
  { 
    SCK=1;
    delay1Us(1);
    if (DATA) 
      val=(val | i);        //�������ߵ�ֵ
      SCK=0;  
      delay1Us(1);      
  }

  P0DIR |= 0x60; // C0 -> 60
  DATA=!ack;                          //�����У�飬��ȡ������ͨѶ��
  SCK=1;                           
  delay1Us(3);        
  SCK=0;          
  DATA=1;                           //�ͷ�������
  return val;
}
//----------------------------------------------------------------------------------------
void calc_sth11(float *p_humidity ,float *p_temperature)
//----------------------------------------------------------------------------------------
// ����������¶Ⱥ����ʪ��
{ const float C1=-4.0;              // for 12 Bit ʪ��������ʽ
  const float C2=+0.0405;           // for 12 Bit ʪ��������ʽ
  const float C3=-0.0000028;        // for 12 Bit ʪ��������ʽ
  const float T1=+0.01;             // for 14 Bit @ 5V �¶�������ʽ
  const float T2=+0.00008;           // for 14 Bit @ 5V  �¶�������ʽ
  float rh=*p_humidity;             
  float t=*p_temperature;           
  float rh_lin;                     
  float rh_true;                    
  float t_C;                        
  t_C=t*0.01 - 39.66;                  //�����¶�
  rh_lin=C3*rh*rh + C2*rh + C1;     //���ʪ�ȷ����Բ���
  rh_true=(t_C-25)*(T1+T2*rh)+rh_lin;   //���ʪ�ȶ����¶������Բ���
  if(rh_true>100)rh_true=100;       //ʪ���������
  if(rh_true<0.1)rh_true=0.1;       //ʪ����С����
  *p_temperature=t_C;               //�����¶Ƚ��
  *p_humidity=rh_true;              //����ʪ�Ƚ��
  //UartTX_Send_String((int*)&t_C,1);
  //UartTX_Send_String((int*)&rh_true,1);
}
//--------------------------------------------------------------------
float calc_dewpoint(float h,float t)
//--------------------------------------------------------------------
// �������ʪ��ֵ
{ float logEx,dew_point;
  logEx=0.66077+7.5*t/(237.3+t)+(log10(h)-2);
  dew_point = (logEx - 0.66077)*237.3/(0.66077+7.5-logEx);
  return dew_point;
}