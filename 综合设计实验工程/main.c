#include "delay.h"
#include "lcd.h"
#include <ioCC2530.h>
#include "sht10.h"
#include <string.h>
#include <stdio.h>
#include "i2c.h"

#define uint unsigned int
#define uchar unsigned char

#define LED1 P1_0 // 定义P1.0口为LED1控制端
#define LED2 P1_1 // 定义P1.1口为LED2控制端
#define LED3 P1_4 // 定义P1.4口为LED3控制端

#define KEY1 P0_1 // P0.1口控制按键KEY1
#define KEY2 P2_0 // P2.0口控制按键KEY2
#define KEY3 P0_7 // P0.7口控制按键KEY3

#define ACK 1
#define noACK 0

uchar KeyValue = 0; // 刚刚按下的KEY的值

union
{
	unsigned int i;
	float f;
} humi_val, temp_val; // 定义两个共同体，一个用于湿度，一个用于温度

char temp_result[15] = {0};
float temp_value = 0;
char humi_result[15] = {0};
float humi_value = 0;

uchar count = 0;
char RxBuf;
char RxData[51];	 // 存储接收的字符串
char key3_state = 0; // 0的时候是非输出状态，1的时候是输出数据的状态

// 给按键消抖用的Delay函数，没有Pause
void DelayMS_for_KEY(uint msec)
{
	uint i, j;
	static int DelayCallCount = 0;

	for (i = 0; i < msec; i++)
		for (j = 0; j < 535; j++)
		{
		}
	DelayCallCount++;
}

void Initial_IO(void)
{
	P0DIR |= 0xff; // P0定义为输出
	P1DIR |= 0xff; // P1定义为输出
}

// 初始化ADC
void InitialAD(void)
{
	// 0000 0000 | 1 = 0000 0001
	P0SEL |= (1 << (4));  // 设置P0.4为外设IO口
	P0DIR &= ~(1 << (4)); // 设置P0.4为输入I/O

	// LED1 = 1; // 启动ADC转换， 指示灯

	ADCCON1 &= ~0x80; // 清EOC标志
	APCFG |= 0x10;	  // 设置P0.4为模拟I/O
	// 1011 0100
	ADCCON3 = 0xb4;	 // 单次转换,参考电压为电源电压  //14位分辨率
	ADCCON1 = 0X30;	 // 停止A/D
	ADCCON1 |= 0X40; // ADCCON1.ST=1，启动A/D
}

// 初始化按键
void InitKey()
{
	P0SEL &= ~0x02; // 设置P0.1为普通IO口
	P0DIR &= ~0x02; // 按键接在P0.1口上，设P0.1为输入模式
	P0INP &= ~0x02; // 打开P0.7上拉电阻

	P0SEL &= ~0x80; // 设置P0.7为普通IO口
	P0DIR &= ~0x80; // 按键接在P0.7口上，设P0.7为输入模式
	P0INP &= ~0x80; // 打开P0.7上拉电阻

	P2SEL &= ~0x01; // 设置P2.0为普通IO口
	P2DIR &= ~0x01; // 按键接在P2.0口上，设P2.0为输入模式
	P2INP &= ~0x01; // 打开P2.0的上拉电阻

	P0IEN |= 0x82; // P0.7, P0.1 设置为中断方式
	PICTL |= 0x01; // 下降沿触发
	// IEN1 |= 0x20;   // 允许P0口中断;
	P0IE = 1;	  // 允许P0口中断;
	P0IFG = 0x00; // 初始化中断标志位

	P2IEN |= 0x01; // P2.0 设置为中断方式
	PICTL |= 0x08; // 下降沿触发
	// P2IE = 1;
	IEN2 |= 0x02; // 允许P2口中断
	P2IFG = 0x00;

	EA = 1;
}

/*****************************************
 串口初始化函数：初始化串口 UART0
*****************************************/
void InitUART0(void)
{
	P0SEL = 0x0c;	// P0.2 P0.3用作串口
	PERCFG = 0x00;	// 选择USART0位置1
	P2DIR &= ~0XC0; // P0优先作为UART0
	U0CSR |= 0x80;	// 串口设置为UART方式
	U0GCR |= 11;
	U0BAUD |= 216; // 波特率设为115200
	UTX0IF = 0;	   // UART0 TX中断标志初始置位1
	U0CSR |= 0X40; // 允许接收

	IEN0 |= 0x84; // 开允许接收Rx中断
	EA = 1;		  // 总中断使能
}

// 串口发送字符串函数
void UartTX_Send_String(char *Data, int len)
{
	int j;
	for (j = 0; j < len; j++)
	{
		U0DBUF = *Data++;
		while (UTX0IF == 0)
			;
		UTX0IF = 0;
	}
	U0DBUF = 0x0A; // 换行
	while (UTX0IF == 0)
		;
	UTX0IF = 0;
}

// 串口接收中断
#pragma vector = URX0_VECTOR
__interrupt void UART0RX_ISR(void)
{
	URX0IF = 0;		// 清中断标志
	RxBuf = U0DBUF; // 取出接收到字节
}

// 串口接受字符
void UartRecieveChar(char ch)
{
	if (ch != '#' && count < 50)
	{
		RxData[count++] = ch; // 以'＃'为结束符,一次最多接收50个字符
	}
	else
	{
		if (count >= 50)
		{						   // 判断数据合法性，防止溢出
			count = 0;			   // 计数清0
			memset(RxData, 0, 51); // 清空接收缓冲区
		}
		else
		{
			// 判断是否是GetData, 如果是，进入State1
			if (strcmp(RxData, "GetData") == 0)
			{
				key3_state = 1;
			}
		}
	}
}

/**************************
系统时钟 不分频
计数时钟 32分频
**************************/
void InitClock(void)
{
	CLKCONCMD &= ~0x40; // 设置系统时钟源为 32MHZ晶振
	while (CLKCONSTA & 0x40)
		;				// 等待晶振稳定
	CLKCONCMD &= ~0x47; // 设置系统主时钟频率为 32MHZ
}

// KEY1 or 3中断
#pragma vector = P0INT_VECTOR
__interrupt void P0_ISR(void)
{
	if (P0IFG & 0x02) // 按键P0.1中断
	{
		DelayMS_for_KEY(10); // 延时去抖
		if (KEY1 == 0)
		{
			KeyValue = 1;												 // 产生中断保存中断状态
			T1CTL = 0;													 // 关计时器
			LCD_ShowString(20, 150, "                         ", BLACK); // 清屏
		}
	}
	if (P0IFG & 0x80) // 按键P0.7中断
	{
		DelayMS_for_KEY(10); // 延时去抖
		if (KEY3 == 0)
		{
			KeyValue = 3;
                        			count = 0;			   // 计数清0
			memset(RxData, 0, 51); // 清空接收缓冲区
			LCD_ShowString(20, 150, "                         ", BLACK); // 清屏
		}
	}

	P0IFG &= ~0x82; // 清Pin中断标志
	P0IF = 0;		// 清端口0中断标志
}

// KEY2中断
#pragma vector = P2INT_VECTOR
__interrupt void P2_ISR(void)
{
	if (P2IFG & 0x01) // 按键P2.0中断
	{
		DelayMS_for_KEY(10); // 延时去抖
		if (KEY2 == 0)
		{
			KeyValue = 2;
			LED2 = 1;
			LED3 = 0;													 // LED2亮起，LED3熄灭
			T1CTL = 0;													 // 关计时器
			LCD_ShowString(20, 150, "                         ", BLACK); // 清屏
		}
	}

	P2IFG &= ~0x01; // 清Pin中断标志
	P2IF = 0;		// 清端口0中断标志
}

/* E2写入函数，buf-源数据指针，addr-E2中的起始地址，len-写入长度*/
void E2Write1(unsigned char *buf, unsigned char addr, unsigned char len)
{
	while (len > 0)
	{
		do
		{ // 等待上次写入操作完成，用寻址操作查询当前是否可进行读写操作
			I2cStart();
			if (I2cSendByte(0x51 << 1))
			{ // 应答则跳出循环，非应答则进行下一次查询
				break;
			}
			I2cStop();
		} while (1);
		// 按页写模式连续写入字节
		I2cSendByte(addr); // 写入起始地址
		while (len > 0)
		{
			I2cSendByte(*buf++); // 写入一个字节数据
			len--;				 // 待写入长度计数递减
			addr++;				 // E2地址递增
			if ((addr & 0x07) == 0)
			{ // 检查地址是否到达页边界，24C02每页8字节，所以检测低3位是否为零即可
				break;
			} // 到达页边界时，跳出循环，结束本次写操作
		}
		I2cStop();
	}
}

/* E2读取函数，buf-数据接收指针，addr-E2中的起始地址，len-读取长度*/
void E2Read(unsigned char *buf,unsigned char addr, unsigned char len)
{
	do
	{ // 用寻址操作查询当前是否可进行读写操作
		I2cStart();
		if (I2cSendByte(0x51 << 1))
		{ // 应答则跳出循环，非应答则进行下一次查询
			break;
		}
		I2cStop();
	} while (1);
	I2cSendByte(addr);				 // 写入起始地址
	I2cStart();						 // 发送重复启动信号
	I2cSendByte((0x51 << 1) | 0x01); // 寻址器件，后续为读操作
	while (len > 1)
	{							   // 连续读取len-1个字节
		*buf++ = I2cReadByte(ACK); // 最后字节之前为读取操作+应答
		len--;
	}
	*buf = I2cReadByte(noACK); // 最后一个字节为读取操作+非应答
	I2cStop();
}

// 定时器初始化
void InitT1()
{
	T1CTL |= 0x0c; // 128分频
	// 每隔0.5s会产生一个中断请求
	T1CC0L = 0x12; // 设置最大计数数值的低8位。
	T1CC0H = 0x7A; // 设置最大计数数值的高8位。
	T1OVFIM = 1;   // 使能定时器1溢出中断，可不写
	T1IE = 1;	   // 使能定时器1中断
	EA = 1;		   // 开全局中断
	T1CTL |= 0x03; // 设置Up/down模式，计数开始
}

// 定时器1中断处理函数
#pragma vector = T1_VECTOR
__interrupt void T1_INT(void)
{
	T1STAT &= ~0x20; // 清除定时器1溢出中断标志位
	// IRCON = 0; //清除Timer中断标志 T1IF，可不写, 由硬件自动清零
	// T1IF=0;
	LED3 = !LED3; // 0.5s 闪烁
}

//******************************************************
int main(void)
{
	u8 i, m;
	float t = 0;
	InitClock();  // 设置系统时钟源为32MHz晶体振荡器
	InitUART0();  // 初始化串口
	Initial_IO(); //
	// InitialAD();
	InitKey();
	Lcd_Init(); // 初始化OLED
	LCD_Clear(WHITE);
	BACK_COLOR = WHITE;
	// 关灯
	LED1 = 0;
	LED2 = 0;
	LED3 = 0;

	// 温湿度测量相关
	unsigned char error, checksum;
	unsigned char HUMI, TEMP;
	HUMI = 0X01;
	TEMP = 0X02;

	s_connectionreset();
	while (1) //这层循环貌似是不需要的
	{
		LCD_ShowChinese32x32(10, 10, 0, 16, BLACK);		   // 
		LCD_ShowChinese32x32(30, 10, 1, 16, BLACK);		   // 
		LCD_ShowChinese32x32(50, 10, 2, 16, BLACK);		   // 
		LCD_ShowString(80, 10, "U202140874", GREEN);	   // 学号
		LCD_ShowPicture(10, 50, 10 + 60 - 1, 50 + 80 - 1); // 照片

		while (1)
		{
			static int now_addr = 0; // 记录写入当前湿度值的地址
			if (KeyValue == 1)
			{
				InitialAD(); // 启动一次转换
				while (!(ADCCON1 & 0x80))
					; // ADCCON1.EOC,转换完毕判断
				char temp[2];
				uint adc = 0;
				float num = 0;
				// LED1 = 0; // 转换完毕指示
				// LED2 = 1; // 打开数据处理指示灯
				temp[1] = ADCL;
				temp[0] = ADCH;

				adc |= (uint)temp[1];
				adc |= ((uint)temp[0]) << 8;
				adc >>= 2; // ADCL[1:0]没用使用
				num = (float)adc;
				// num = adc * 3.3 / 8192; // 定参考电压为3.3V。14位分辨率
				// adcdata[1] = (char)(num) % 10 + 48;
				// adcdata[3] = (char)(num * 10) % 10 + 48;
				// UartTX_Send_String(adcdata, 6); // 串口送数
				LCD_ShowString(40, 150, "ADC: ", BLACK);
				LCD_ShowNum1(80, 150, num, 5, BLUE);
				// LED2 = 0; // 完成数据处理
			}
			else if (KeyValue == 2)
			{
				delay_ms(1000);
				error = 0;
				// 这里不能传值，我不知道为什么
				// 已经解决，没包含头文件，但是IAR居然不报错，编译器太离谱
				error += s_measure((unsigned char *)&humi_val.i, &checksum, HUMI); // 湿度测量
				error += s_measure((unsigned char *)&temp_val.i, &checksum, TEMP); // 温度测量

				temp_value = temp_val.i * 0.01 - 39.6;
				// sprintf(temp_result, "%s", "temperature:");
				// UartTX_Send_String(temp_result, 13);
				LCD_ShowString(20, 150, "temperature:", BLACK);
				sprintf(temp_result, "%3.2f C\0", temp_value); // 加个摄氏度符号
				// UartTX_Send_String(temp_result, 10);
				LCD_ShowString(130, 150, temp_result, BLUE);

				humi_value = humi_val.i * 0.0367 - 2.0468;
				// sprintf(humi_result, "%s", "humidity: ");
				// UartTX_Send_String(humi_result, 10);
				// sprintf(humi_result, "%3.4f", humi_value);
				// 000.0000 需要保存8位，正好是24C02的一页
				// UartTX_Send_String(humi_result, 10);
				static char humi_count = 0; // 记录写入湿度值的次数 <100
				if (humi_count < 100)
				{
					// 由于24C02一共能存储256字节数据，这样会存不下
					// E2Write1(humi_result, now_addr, 8); // 每次写入8字节数据

					// 改为两个字节分别存储小数点前(<127)和小数点后两位，例如123.78, humi_char1 存储 123， humi_char2 存储 78
					u8 humi_char1 = (int)humi_value;
					u8 humi_char2 = (int)(humi_value * 100) % 100;
					E2Write1(&humi_char1, now_addr, 1);
					E2Write1(&humi_char2, now_addr + 1, 1);
					humi_count += 1;
					now_addr += 2;
				}
				else
				{
					humi_count = 0;
					now_addr = 0; // 只记录最近的100条，直接覆盖之前的
					LED2 = 0;	  // 熄灭LED2
				}

				if (error != 0)
				{
					s_connectionreset(); // 如果发生错误，系统复位
										 // LED1 = !LED1;
										 // LED2 = !LED2;
				}
				else
				{
					humi_val.f = (float)humi_val.i;		  // 转换为浮点数
					temp_val.f = (float)temp_val.i;		  // 转换为浮点数
					calc_sth11(&humi_val.f, &temp_val.f); // 修正相对湿度及温度
				}
			}
			else if (KeyValue == 3)
			{
				static char sign = 0;
				if (sign == 0)
				{
					InitT1(); // 初始化定时器T1 开始闪烁
					sign = 1;
					key3_state = 0;
				}
				if (key3_state == 1)
				{
					// LED3进入常亮状态，从24C02中取出最近的100次湿度值通过串口传给PC
					// 关闭定时器1
					T1CTL = 0;
					LED3 = 1;
					uchar ii = 0;
					static char ii_to_str[7];
					static char getdata[2];
					static char str_data[10];
					for (ii = 0; ii < 100; ii++)
					{
						E2Read(getdata, now_addr, 2);
						//读取完成后，getdata[0]是存储小数点前的，getdata[1]是存储小数点后的
						sprintf(str_data, "%d.%d", *getdata, *(getdata+1));
						now_addr += 2;
						if (now_addr == 202)
							now_addr = 0;
						sprintf(ii_to_str, "%3d. ", ii);
						// 需要间隔一定时间，保证串口输出完成
						UartTX_Send_String(ii_to_str, 7);
						delay_ms(100);
						UartTX_Send_String(str_data, 8);
						delay_ms(100);
						UartTX_Send_String("\n", 1);
						delay_ms(100);
					}
					key3_state = 0;
					sign = 0;
				}
				else
				{
					if (RxBuf != 0)
					{
						UartRecieveChar(RxBuf);
						RxBuf = 0;
					}
				}
			}

		} //  while loop
	}
}
