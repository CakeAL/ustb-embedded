#include "delay.h"
#include "lcd.h"
#include <ioCC2530.h>
#include "sht10.h"
#include <string.h>
#include <stdio.h>
#include "i2c.h"

#define uint unsigned int
#define uchar unsigned char

#define LED1 P1_0 // ����P1.0��ΪLED1���ƶ�
#define LED2 P1_1 // ����P1.1��ΪLED2���ƶ�
#define LED3 P1_4 // ����P1.4��ΪLED3���ƶ�

#define KEY1 P0_1 // P0.1�ڿ��ư���KEY1
#define KEY2 P2_0 // P2.0�ڿ��ư���KEY2
#define KEY3 P0_7 // P0.7�ڿ��ư���KEY3

#define ACK 1
#define noACK 0

uchar KeyValue = 0; // �ոհ��µ�KEY��ֵ

union
{
	unsigned int i;
	float f;
} humi_val, temp_val; // ����������ͬ�壬һ������ʪ�ȣ�һ�������¶�

char temp_result[15] = {0};
float temp_value = 0;
char humi_result[15] = {0};
float humi_value = 0;

uchar count = 0;
char RxBuf;
char RxData[51];	 // �洢���յ��ַ���
char key3_state = 0; // 0��ʱ���Ƿ����״̬��1��ʱ����������ݵ�״̬

// �����������õ�Delay������û��Pause
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
	P0DIR |= 0xff; // P0����Ϊ���
	P1DIR |= 0xff; // P1����Ϊ���
}

// ��ʼ��ADC
void InitialAD(void)
{
	// 0000 0000 | 1 = 0000 0001
	P0SEL |= (1 << (4));  // ����P0.4Ϊ����IO��
	P0DIR &= ~(1 << (4)); // ����P0.4Ϊ����I/O

	// LED1 = 1; // ����ADCת���� ָʾ��

	ADCCON1 &= ~0x80; // ��EOC��־
	APCFG |= 0x10;	  // ����P0.4Ϊģ��I/O
	// 1011 0100
	ADCCON3 = 0xb4;	 // ����ת��,�ο���ѹΪ��Դ��ѹ  //14λ�ֱ���
	ADCCON1 = 0X30;	 // ֹͣA/D
	ADCCON1 |= 0X40; // ADCCON1.ST=1������A/D
}

// ��ʼ������
void InitKey()
{
	P0SEL &= ~0x02; // ����P0.1Ϊ��ͨIO��
	P0DIR &= ~0x02; // ��������P0.1���ϣ���P0.1Ϊ����ģʽ
	P0INP &= ~0x02; // ��P0.7��������

	P0SEL &= ~0x80; // ����P0.7Ϊ��ͨIO��
	P0DIR &= ~0x80; // ��������P0.7���ϣ���P0.7Ϊ����ģʽ
	P0INP &= ~0x80; // ��P0.7��������

	P2SEL &= ~0x01; // ����P2.0Ϊ��ͨIO��
	P2DIR &= ~0x01; // ��������P2.0���ϣ���P2.0Ϊ����ģʽ
	P2INP &= ~0x01; // ��P2.0����������

	P0IEN |= 0x82; // P0.7, P0.1 ����Ϊ�жϷ�ʽ
	PICTL |= 0x01; // �½��ش���
	// IEN1 |= 0x20;   // ����P0���ж�;
	P0IE = 1;	  // ����P0���ж�;
	P0IFG = 0x00; // ��ʼ���жϱ�־λ

	P2IEN |= 0x01; // P2.0 ����Ϊ�жϷ�ʽ
	PICTL |= 0x08; // �½��ش���
	// P2IE = 1;
	IEN2 |= 0x02; // ����P2���ж�
	P2IFG = 0x00;

	EA = 1;
}

/*****************************************
 ���ڳ�ʼ����������ʼ������ UART0
*****************************************/
void InitUART0(void)
{
	P0SEL = 0x0c;	// P0.2 P0.3��������
	PERCFG = 0x00;	// ѡ��USART0λ��1
	P2DIR &= ~0XC0; // P0������ΪUART0
	U0CSR |= 0x80;	// ��������ΪUART��ʽ
	U0GCR |= 11;
	U0BAUD |= 216; // ��������Ϊ115200
	UTX0IF = 0;	   // UART0 TX�жϱ�־��ʼ��λ1
	U0CSR |= 0X40; // �������

	IEN0 |= 0x84; // ���������Rx�ж�
	EA = 1;		  // ���ж�ʹ��
}

// ���ڷ����ַ�������
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
	U0DBUF = 0x0A; // ����
	while (UTX0IF == 0)
		;
	UTX0IF = 0;
}

// ���ڽ����ж�
#pragma vector = URX0_VECTOR
__interrupt void UART0RX_ISR(void)
{
	URX0IF = 0;		// ���жϱ�־
	RxBuf = U0DBUF; // ȡ�����յ��ֽ�
}

// ���ڽ����ַ�
void UartRecieveChar(char ch)
{
	if (ch != '#' && count < 50)
	{
		RxData[count++] = ch; // ��'��'Ϊ������,һ��������50���ַ�
	}
	else
	{
		if (count >= 50)
		{						   // �ж����ݺϷ��ԣ���ֹ���
			count = 0;			   // ������0
			memset(RxData, 0, 51); // ��ս��ջ�����
		}
		else
		{
			// �ж��Ƿ���GetData, ����ǣ�����State1
			if (strcmp(RxData, "GetData") == 0)
			{
				key3_state = 1;
			}
		}
	}
}

/**************************
ϵͳʱ�� ����Ƶ
����ʱ�� 32��Ƶ
**************************/
void InitClock(void)
{
	CLKCONCMD &= ~0x40; // ����ϵͳʱ��ԴΪ 32MHZ����
	while (CLKCONSTA & 0x40)
		;				// �ȴ������ȶ�
	CLKCONCMD &= ~0x47; // ����ϵͳ��ʱ��Ƶ��Ϊ 32MHZ
}

// KEY1 or 3�ж�
#pragma vector = P0INT_VECTOR
__interrupt void P0_ISR(void)
{
	if (P0IFG & 0x02) // ����P0.1�ж�
	{
		DelayMS_for_KEY(10); // ��ʱȥ��
		if (KEY1 == 0)
		{
			KeyValue = 1;												 // �����жϱ����ж�״̬
			T1CTL = 0;													 // �ؼ�ʱ��
			LCD_ShowString(20, 150, "                         ", BLACK); // ����
		}
	}
	if (P0IFG & 0x80) // ����P0.7�ж�
	{
		DelayMS_for_KEY(10); // ��ʱȥ��
		if (KEY3 == 0)
		{
			KeyValue = 3;
                        			count = 0;			   // ������0
			memset(RxData, 0, 51); // ��ս��ջ�����
			LCD_ShowString(20, 150, "                         ", BLACK); // ����
		}
	}

	P0IFG &= ~0x82; // ��Pin�жϱ�־
	P0IF = 0;		// ��˿�0�жϱ�־
}

// KEY2�ж�
#pragma vector = P2INT_VECTOR
__interrupt void P2_ISR(void)
{
	if (P2IFG & 0x01) // ����P2.0�ж�
	{
		DelayMS_for_KEY(10); // ��ʱȥ��
		if (KEY2 == 0)
		{
			KeyValue = 2;
			LED2 = 1;
			LED3 = 0;													 // LED2����LED3Ϩ��
			T1CTL = 0;													 // �ؼ�ʱ��
			LCD_ShowString(20, 150, "                         ", BLACK); // ����
		}
	}

	P2IFG &= ~0x01; // ��Pin�жϱ�־
	P2IF = 0;		// ��˿�0�жϱ�־
}

/* E2д�뺯����buf-Դ����ָ�룬addr-E2�е���ʼ��ַ��len-д�볤��*/
void E2Write1(unsigned char *buf, unsigned char addr, unsigned char len)
{
	while (len > 0)
	{
		do
		{ // �ȴ��ϴ�д�������ɣ���Ѱַ������ѯ��ǰ�Ƿ�ɽ��ж�д����
			I2cStart();
			if (I2cSendByte(0x51 << 1))
			{ // Ӧ��������ѭ������Ӧ���������һ�β�ѯ
				break;
			}
			I2cStop();
		} while (1);
		// ��ҳдģʽ����д���ֽ�
		I2cSendByte(addr); // д����ʼ��ַ
		while (len > 0)
		{
			I2cSendByte(*buf++); // д��һ���ֽ�����
			len--;				 // ��д�볤�ȼ����ݼ�
			addr++;				 // E2��ַ����
			if ((addr & 0x07) == 0)
			{ // ����ַ�Ƿ񵽴�ҳ�߽磬24C02ÿҳ8�ֽڣ����Լ���3λ�Ƿ�Ϊ�㼴��
				break;
			} // ����ҳ�߽�ʱ������ѭ������������д����
		}
		I2cStop();
	}
}

/* E2��ȡ������buf-���ݽ���ָ�룬addr-E2�е���ʼ��ַ��len-��ȡ����*/
void E2Read(unsigned char *buf,unsigned char addr, unsigned char len)
{
	do
	{ // ��Ѱַ������ѯ��ǰ�Ƿ�ɽ��ж�д����
		I2cStart();
		if (I2cSendByte(0x51 << 1))
		{ // Ӧ��������ѭ������Ӧ���������һ�β�ѯ
			break;
		}
		I2cStop();
	} while (1);
	I2cSendByte(addr);				 // д����ʼ��ַ
	I2cStart();						 // �����ظ������ź�
	I2cSendByte((0x51 << 1) | 0x01); // Ѱַ����������Ϊ������
	while (len > 1)
	{							   // ������ȡlen-1���ֽ�
		*buf++ = I2cReadByte(ACK); // ����ֽ�֮ǰΪ��ȡ����+Ӧ��
		len--;
	}
	*buf = I2cReadByte(noACK); // ���һ���ֽ�Ϊ��ȡ����+��Ӧ��
	I2cStop();
}

// ��ʱ����ʼ��
void InitT1()
{
	T1CTL |= 0x0c; // 128��Ƶ
	// ÿ��0.5s�����һ���ж�����
	T1CC0L = 0x12; // ������������ֵ�ĵ�8λ��
	T1CC0H = 0x7A; // ������������ֵ�ĸ�8λ��
	T1OVFIM = 1;   // ʹ�ܶ�ʱ��1����жϣ��ɲ�д
	T1IE = 1;	   // ʹ�ܶ�ʱ��1�ж�
	EA = 1;		   // ��ȫ���ж�
	T1CTL |= 0x03; // ����Up/downģʽ��������ʼ
}

// ��ʱ��1�жϴ�����
#pragma vector = T1_VECTOR
__interrupt void T1_INT(void)
{
	T1STAT &= ~0x20; // �����ʱ��1����жϱ�־λ
	// IRCON = 0; //���Timer�жϱ�־ T1IF���ɲ�д, ��Ӳ���Զ�����
	// T1IF=0;
	LED3 = !LED3; // 0.5s ��˸
}

//******************************************************
int main(void)
{
	u8 i, m;
	float t = 0;
	InitClock();  // ����ϵͳʱ��ԴΪ32MHz��������
	InitUART0();  // ��ʼ������
	Initial_IO(); //
	// InitialAD();
	InitKey();
	Lcd_Init(); // ��ʼ��OLED
	LCD_Clear(WHITE);
	BACK_COLOR = WHITE;
	// �ص�
	LED1 = 0;
	LED2 = 0;
	LED3 = 0;

	// ��ʪ�Ȳ������
	unsigned char error, checksum;
	unsigned char HUMI, TEMP;
	HUMI = 0X01;
	TEMP = 0X02;

	s_connectionreset();
	while (1) //���ѭ��ò���ǲ���Ҫ��
	{
		LCD_ShowChinese32x32(10, 10, 0, 16, BLACK);		   // ��
		LCD_ShowChinese32x32(30, 10, 1, 16, BLACK);		   // ��
		LCD_ShowChinese32x32(50, 10, 2, 16, BLACK);		   // ��
		LCD_ShowString(80, 10, "U202140874", GREEN);	   // ѧ��
		LCD_ShowPicture(10, 50, 10 + 60 - 1, 50 + 80 - 1); // ��Ƭ

		while (1)
		{
			static int now_addr = 0; // ��¼д�뵱ǰʪ��ֵ�ĵ�ַ
			if (KeyValue == 1)
			{
				InitialAD(); // ����һ��ת��
				while (!(ADCCON1 & 0x80))
					; // ADCCON1.EOC,ת������ж�
				char temp[2];
				uint adc = 0;
				float num = 0;
				// LED1 = 0; // ת�����ָʾ
				// LED2 = 1; // �����ݴ���ָʾ��
				temp[1] = ADCL;
				temp[0] = ADCH;

				adc |= (uint)temp[1];
				adc |= ((uint)temp[0]) << 8;
				adc >>= 2; // ADCL[1:0]û��ʹ��
				num = (float)adc;
				// num = adc * 3.3 / 8192; // ���ο���ѹΪ3.3V��14λ�ֱ���
				// adcdata[1] = (char)(num) % 10 + 48;
				// adcdata[3] = (char)(num * 10) % 10 + 48;
				// UartTX_Send_String(adcdata, 6); // ��������
				LCD_ShowString(40, 150, "ADC: ", BLACK);
				LCD_ShowNum1(80, 150, num, 5, BLUE);
				// LED2 = 0; // ������ݴ���
			}
			else if (KeyValue == 2)
			{
				delay_ms(1000);
				error = 0;
				// ���ﲻ�ܴ�ֵ���Ҳ�֪��Ϊʲô
				// �Ѿ������û����ͷ�ļ�������IAR��Ȼ������������̫����
				error += s_measure((unsigned char *)&humi_val.i, &checksum, HUMI); // ʪ�Ȳ���
				error += s_measure((unsigned char *)&temp_val.i, &checksum, TEMP); // �¶Ȳ���

				temp_value = temp_val.i * 0.01 - 39.6;
				// sprintf(temp_result, "%s", "temperature:");
				// UartTX_Send_String(temp_result, 13);
				LCD_ShowString(20, 150, "temperature:", BLACK);
				sprintf(temp_result, "%3.2f C\0", temp_value); // �Ӹ����϶ȷ���
				// UartTX_Send_String(temp_result, 10);
				LCD_ShowString(130, 150, temp_result, BLUE);

				humi_value = humi_val.i * 0.0367 - 2.0468;
				// sprintf(humi_result, "%s", "humidity: ");
				// UartTX_Send_String(humi_result, 10);
				// sprintf(humi_result, "%3.4f", humi_value);
				// 000.0000 ��Ҫ����8λ��������24C02��һҳ
				// UartTX_Send_String(humi_result, 10);
				static char humi_count = 0; // ��¼д��ʪ��ֵ�Ĵ��� <100
				if (humi_count < 100)
				{
					// ����24C02һ���ܴ洢256�ֽ����ݣ�������治��
					// E2Write1(humi_result, now_addr, 8); // ÿ��д��8�ֽ�����

					// ��Ϊ�����ֽڷֱ�洢С����ǰ(<127)��С�������λ������123.78, humi_char1 �洢 123�� humi_char2 �洢 78
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
					now_addr = 0; // ֻ��¼�����100����ֱ�Ӹ���֮ǰ��
					LED2 = 0;	  // Ϩ��LED2
				}

				if (error != 0)
				{
					s_connectionreset(); // �����������ϵͳ��λ
										 // LED1 = !LED1;
										 // LED2 = !LED2;
				}
				else
				{
					humi_val.f = (float)humi_val.i;		  // ת��Ϊ������
					temp_val.f = (float)temp_val.i;		  // ת��Ϊ������
					calc_sth11(&humi_val.f, &temp_val.f); // �������ʪ�ȼ��¶�
				}
			}
			else if (KeyValue == 3)
			{
				static char sign = 0;
				if (sign == 0)
				{
					InitT1(); // ��ʼ����ʱ��T1 ��ʼ��˸
					sign = 1;
					key3_state = 0;
				}
				if (key3_state == 1)
				{
					// LED3���볣��״̬����24C02��ȡ�������100��ʪ��ֵͨ�����ڴ���PC
					// �رն�ʱ��1
					T1CTL = 0;
					LED3 = 1;
					uchar ii = 0;
					static char ii_to_str[7];
					static char getdata[2];
					static char str_data[10];
					for (ii = 0; ii < 100; ii++)
					{
						E2Read(getdata, now_addr, 2);
						//��ȡ��ɺ�getdata[0]�Ǵ洢С����ǰ�ģ�getdata[1]�Ǵ洢С������
						sprintf(str_data, "%d.%d", *getdata, *(getdata+1));
						now_addr += 2;
						if (now_addr == 202)
							now_addr = 0;
						sprintf(ii_to_str, "%3d. ", ii);
						// ��Ҫ���һ��ʱ�䣬��֤����������
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