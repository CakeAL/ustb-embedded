/****************************************************************************
* 文 件 名: main.c
* 描    述: 定时器T1通过中断方式控制LED3周期性闪烁
****************************************************************************/
#include <ioCC2530.h>

typedef unsigned char uchar;
typedef unsigned int  uint;

#define LED1 P1_0       // P1.0口控制LED1
#define LED2 P1_1       // P1.1口控制LED2
#define LED3 P1_4       // P1.4口控制LED3

#define KEY1 P0_1        // P0.1口控制按键KEY1
#define KEY3 P0_7       // P0.7口控制按键KEY3

unsigned int t1_led1_count = 0; //Timer1 对于 LED1 溢出次数计数
unsigned int t1_led3_count = 0; //Timer1 对于 LED3 溢出次数计数
unsigned char key1_parsed = 1; // KEY1是否按下 0为按下
unsigned char key3_parsed = 1; // KEY0是否按下 0为按下
unsigned char led1_or_3_active = 0; //现在是led1还是3应该亮灭 0为led1，1为led3

/****************************************************************************
* 名    称: InitLed()
* 功    能: 设置LED灯相应的IO口
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitLed(void)
{
    P1DIR |= 0x13;      //P1.0, P1.1, P1.4定义为输出
    LED1 = 0;           //使LED1灯上电默认为熄灭  
    LED2 = 0;           //使LED2灯上电默认为熄灭  
    LED3 = 0;           //使LED3灯上电默认为熄灭     
}

/****************************************************************************
* 名    称: InitKey()
* 功    能: 设置按键相应的IO口
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitKey(void)
{
  
    P0SEL &= ~0x02;     //设置P0.1为普通IO口  
    P0DIR &= ~0x02;     //按键接在P0.1口上，设P0.1为输入模式 
    P0INP &= ~0x02;     //打开P0.1上拉电阻
    
    P0SEL &= ~0x80;     //设置P0.7为普通IO口  
    P0DIR &= ~0x80;     //按键接在P0.7口上，设P0.7为输入模式 
    P0INP &= ~0x80;     //打开P0.7上拉电阻 
}


/****************************************************************************
* 名    称: InitT1()
* 功    能: 定时器初始化，系统不配置工作时钟时默认为16MHz
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitT1()
{
      T1CTL |= 0x0c; //128分频
      //每隔0.5s会产生一个中断请求

      // 2ms，需要设置1ms的最大计数，0.001/(1/16M * 128) = 125 = 0x7D
      T1CC0L=0x7D;      //设置最大计数数值的低8位。
      T1CC0H=0x00;     //设置最大计数数值的高8位。
      T1OVFIM =1;  //使能定时器1溢出中断，可不写
      T1IE = 1;          //使能定时器1中断
      EA = 1;  //开全局中断
      T1CTL |= 0x03; //设置Up/down模式，计数开始
}

/****************************************************************************
* 定时器T1中断处理函数
****************************************************************************/
#pragma vector=T1_VECTOR
__interrupt void T1_INT(void)
{
    T1STAT &= ~0x20;         //清除定时器1溢出中断标志位
    //IRCON = 0; //清除Timer中断标志 T1IF，可不写, 由硬件自动清零
    //T1IF=0;
    // t1_count++;         //定时器1溢出次数加1，溢出周期为0.5s
    if (!led1_or_3_active) { //当前是led1亮灭
      t1_led1_count++;
    }
    else {
      t1_led3_count++;
    }
    static unsigned char key1_buf = 0xFF; // 检测Key按下的状态
    static unsigned char key3_buf = 0xFF;
    key1_buf = (key1_buf << 1) | KEY1; // 每个2ms的中断检测一次KEY的状态，如果
    key3_buf = (key3_buf << 1) | KEY3; // KEY为0，那么keybuf变成1111 1110
    if(key1_buf == 0x00) { // 此时keybuf为0000 0000，说明连续16ms内都是按下的状态
      key1_parsed = 0; 
      if(led1_or_3_active == 1) {
        led1_or_3_active = 0; //如果原来是led3亮，切换}
        LED3 = 0; //熄灭LED3
      }
    } else if (key3_buf == 0x00) {     
      key3_parsed = 0;
      if(led1_or_3_active == 0) {
        led1_or_3_active = 1;
        LED1 = 0; //熄灭LED1
      }
    } else if (key1_parsed == 0xFF) {
      key1_parsed = 1; // 按键弹起
    } else if (key3_parsed == 0xFF) {
      key3_parsed = 1;
    }
    if(!led1_or_3_active){ // LED1 亮灭 状态
          if(t1_led1_count == 250 * 3)   //如果溢出次数达到3说明经过了1.5s
      { 
        LED1=1;    //点亮LED1
      }
      else if( t1_led1_count >= 250 * 4)   //如果溢出次数达到4说明经过了2s
      {
        LED1=0;    //熄灭LED1
        t1_led1_count = 0;    //清零定时器1溢出次数              
      }
    }
    else {
      if(t1_led3_count == 250 * 3)   //如果溢出次数达到3说明经过了1.5s
      { 
        LED3=1;    //点亮LED3
      }
      else if( t1_led3_count >= 250 * 4)   //如果溢出次数达到4说明经过了2s
      {
        LED3=0;    //熄灭LED3
        t1_led3_count = 0;    //清零定时器1溢出次数              
      }
    }
}


/****************************************************************************
* 程序入口函数
****************************************************************************/
void main(void)
{
    InitLed();  //调用LED初始化函数
    InitKey();  //调用Key初始化函数
    InitT1();   //调用Timer1初始化函数
    
    while(1);  
}
