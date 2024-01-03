#include <math.h>
#include "sht10.h"

/**************************************************************************************************
* @fn      delay1Us
*
* @brief   wait for x us. @ 32MHz MCU clock it takes 32 "nop"s for 1 us delay.
*
* @param   x us. range[0-65536]
*
* @return   延时约为0.4us
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
延时 1uS 带参数(int)子程序
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
// 连接复位;
//       _____________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
{  
  unsigned char i; 
  
  P0DIR |= 0x60; // 0110 0000
     
  DATA=1; SCK=0;                    //准备
  for(i=0;i<9;i++)                  //DATA保持高，SCK时钟触发9次，发送启动传输，通迅即复位
  { SCK=1;
    SCK=0;
  }
  s_transstart();                   //启动传输
}
//----------------------------------------------------------------------------------
void s_transstart(void)
//----------------------------------------------------------------------------------
// 启动传输
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
// 进行温度或者湿度转换，由参数mode决定转换内容；
{ 
  unsigned error=0;
  s_transstart();                   //启动传输
  switch(mode){
    case 0x02 : error+=s_write_byte(MEASURE_TEMP); break;
    case 0x01 : error+=s_write_byte(MEASURE_HUMI); break;
    default : break;  
  }
  P0DIR |= 0x40;//0x40=0100 0000 clock
  P0DIR &= 0xDF;//d=1101,f=1111 data
 // for (i=0;i<110;i++){
  //   delay(2000);
 // if(DATA==0) break; //等待测量结束；
//  }
   while(DATA); //等待测量结束；
   if(DATA) error+=1;                // 如果长时间数据线没有拉低，说明测量错误
  *(p_value+1)  =s_read_byte(ACK);    //读第一个字节，高字节 (MSB)
  *(p_value)=s_read_byte(ACK);    //读第二个字节，低字节 (LSB)
  *p_checksum =s_read_byte(noACK);  //read CRC校验码
  //UartTX_Send_String(p_value,2); 
  return error;
}
//----------------------------------------------------------------------------------
char s_write_byte(unsigned char value)
//----------------------------------------------------------------------------------
// 写字节函数 
{ 
  char i;
  char error=0;

  P0DIR |= 0x60; // C0 -> 60
 
  SCK=0;
  DATA=0;
 for(i=0;i<8;i++)  //发送8位数据，丛机将在上升沿读取数据
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
 
 SCK=0;     //在接下来的上升沿读取从机发送的“已收到”信号。
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
// 读数据；
{ 
  unsigned char i,val=0;
  
  //DATA=1; //数据线为高
  
  P0DIR |= 0x40;//0x40=0100 0000 clock
  P0DIR &= 0xDF;//d=1101,f=1111 data
  SCK=0;
  for (i=0x80;i>0;i>>=1)             //右移位
  { 
    SCK=1;
    delay1Us(1);
    if (DATA) 
      val=(val | i);        //读数据线的值
      SCK=0;  
      delay1Us(1);      
  }

  P0DIR |= 0x60; // C0 -> 60
  DATA=!ack;                          //如果是校验，读取完后结束通讯；
  SCK=1;                           
  delay1Us(3);        
  SCK=0;          
  DATA=1;                           //释放数据线
  return val;
}
//----------------------------------------------------------------------------------------
void calc_sth11(float *p_humidity ,float *p_temperature)
//----------------------------------------------------------------------------------------
// 补偿及输出温度和相对湿度
{ const float C1=-4.0;              // for 12 Bit 湿度修正公式
  const float C2=+0.0405;           // for 12 Bit 湿度修正公式
  const float C3=-0.0000028;        // for 12 Bit 湿度修正公式
  const float T1=+0.01;             // for 14 Bit @ 5V 温度修正公式
  const float T2=+0.00008;           // for 14 Bit @ 5V  温度修正公式
  float rh=*p_humidity;             
  float t=*p_temperature;           
  float rh_lin;                     
  float rh_true;                    
  float t_C;                        
  t_C=t*0.01 - 39.66;                  //补偿温度
  rh_lin=C3*rh*rh + C2*rh + C1;     //相对湿度非线性补偿
  rh_true=(t_C-25)*(T1+T2*rh)+rh_lin;   //相对湿度对于温度依赖性补偿
  if(rh_true>100)rh_true=100;       //湿度最大修正
  if(rh_true<0.1)rh_true=0.1;       //湿度最小修正
  *p_temperature=t_C;               //返回温度结果
  *p_humidity=rh_true;              //返回湿度结果
  //UartTX_Send_String((int*)&t_C,1);
  //UartTX_Send_String((int*)&rh_true,1);
}
//--------------------------------------------------------------------
float calc_dewpoint(float h,float t)
//--------------------------------------------------------------------
// 计算绝对湿度值
{ float logEx,dew_point;
  logEx=0.66077+7.5*t/(237.3+t)+(log10(h)-2);
  dew_point = (logEx - 0.66077)*237.3/(0.66077+7.5-logEx);
  return dew_point;
}