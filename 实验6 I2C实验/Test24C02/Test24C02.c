/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
* �� �� ����Test24C02.c
�� ���ܣ���I2C����AT24C02��ĳ��ַ(addr)д��һ������(num0)������������(num1)���������
*        ��֤���������ݾ���д�������
*
*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include"i2c.h"
// GPIOģ��I2C������ΪP0.5��P0.6�������i2c.h
//#define SCL          P0_6     //I2Cʱ��
//#define SDA          P0_5     //I2C������

//���� IO
#define K1 P0_1
#define K2 P2_0
#define K3 P0_7

#define ACK 1
#define noACK 0


void initIO(void);

char writedata[]="Write: 000";
char readdata[] ="Read: 000";
char numdata[]  ="Num: 000";
unsigned char testdata[] = {4,5,6,7,8,9,10,11,12,13};
unsigned char getdata[] = {0,0,0,0,0,0,0,0,0,0};
unsigned char *hex_to_ascii_data; 

/*******************************************************************************
* ������         : void At24c02Write(unsigned char addr,unsigned char dat)
* ��������		   : ��24c02��һ����ַд��һ������
* ����           : ��
* ���         	 : ��
*******************************************************************************/
void At24c02Write(unsigned char addr,unsigned char dat)
{
	I2cStart();
	I2cSendByte(0xa2);//����д������ַ
	I2cSendByte(addr);//����Ҫд���ڴ��ַ
	I2cSendByte(dat);	//��������
	I2cStop();
}
/* E2д�뺯����buf-Դ����ָ�룬addr-E2�е���ʼ��ַ��len-д�볤��*/
void E2Write(unsigned char *buf, unsigned char addr, unsigned char len)
{
     while (len--) {
     do {                           //��Ѱַ������ѯ��ǰ�Ƿ�ɽ��ж�д����
        I2cStart();
        if (I2cSendByte(0x51<<1)) { //Ӧ��������ѭ������Ӧ���������һ�β�ѯ
         break;
        }
        I2cStop();
     } while(1);
    I2cSendByte(addr++);  //д����ʼ��ַ
    I2cSendByte(*buf++);  //д��һ���ֽ�����
    I2cStop();          //����д�������Եȴ�д�����
     }
}

/* E2д�뺯����buf-Դ����ָ�룬addr-E2�е���ʼ��ַ��len-д�볤��*/
void E2Write1(unsigned char *buf, unsigned char addr, unsigned char len) {
while (len > 0) {
  do {    //�ȴ��ϴ�д�������ɣ���Ѱַ������ѯ��ǰ�Ƿ�ɽ��ж�д����
     I2cStart();
     if (I2cSendByte(0x51<<1)) {  //Ӧ��������ѭ������Ӧ���������һ�β�ѯ
          break;   }
     I2cStop();
     } while(1);
   //��ҳдģʽ����д���ֽ�
   I2cSendByte(addr);            //д����ʼ��ַ
   while (len > 0) {
       I2cSendByte(*buf++);      //д��һ���ֽ�����
       len--;                   //��д�볤�ȼ����ݼ�
       addr++;                  //E2��ַ����
       if ((addr&0x07) == 0) { //����ַ�Ƿ񵽴�ҳ�߽磬24C02ÿҳ8�ֽڣ����Լ���3λ�Ƿ�Ϊ�㼴��
          break; }      //����ҳ�߽�ʱ������ѭ������������д����
    }   I2cStop();   }   }


/* E2��ȡ������buf-���ݽ���ָ�룬addr-E2�е���ʼ��ַ��len-��ȡ����*/
void E2Read(unsigned char *buf, unsigned char addr, unsigned char len) {
  do {                           //��Ѱַ������ѯ��ǰ�Ƿ�ɽ��ж�д����	
     I2cStart();
     if (I2cSendByte(0x51<<1)) {   //Ӧ��������ѭ������Ӧ���������һ�β�ѯ
         break;   }
      I2cStop();
    } while(1);
   I2cSendByte(addr);              //д����ʼ��ַ
   I2cStart();                   //�����ظ������ź�
   I2cSendByte((0x51<<1)|0x01);  //Ѱַ����������Ϊ������
   while (len > 1) {   //������ȡlen-1���ֽ�
      *buf++ = I2cReadByte(ACK); //����ֽ�֮ǰΪ��ȡ����+Ӧ��
       len--;  }
      *buf = I2cReadByte(noACK);       //���һ���ֽ�Ϊ��ȡ����+��Ӧ��
   I2cStop();  }


/*******************************************************************************
* ������         : unsigned char At24c02Read(unsigned char addr)
* ��������		   : ��ȡ24c02��һ����ַ��һ������
* ����           : ��
* ���         	 : ���ض������ֽ�
*******************************************************************************/
unsigned char At24c02Read(unsigned char addr)
{
	unsigned char num;
	I2cStart();
	I2cSendByte(0xa2); //����д������ַ
	I2cSendByte(addr); //����Ҫ��ȡ�ĵ�ַ
	I2cStart();
	I2cSendByte(0xa3); //���Ͷ�������ַ
	num = I2cReadByte(noACK); //��ȡ����
	I2cStop();
	return num;	
}

/****************************************************************************
* ��    ��: DelayMS()
* ��    ��: �Ժ���Ϊ��λ��ʱ��ϵͳʱ�Ӳ�����ʱĬ��Ϊ16M(��ʾ���������൱��ȷ)
* ��ڲ���: msec ��ʱ������ֵԽ����ʱԽ��
* ���ڲ���: ��
****************************************************************************/
void DelayMS(unsigned int msec)
{ 
    unsigned int i,j;
    
    for (i=0; i<msec; i++)
        for (j=0; j<535; j++);
}

unsigned char KeyScan(void)
{
    if (K1 == 0)
    {
        DelayMS(10);      //��ʱ10MSȥ��
        if (K1 == 0)
        {
            while(!K1); //���ּ��
            return 1;     //�а�������
        }
    }

    if (K2 == 0)
    {
        DelayMS(10);      //��ʱ10MSȥ��
        if (K2 == 0)
        {
            while(!K2); //���ּ��
            return 2;     //�а�������
        }
    }

    if (K3 == 0)
    {
        DelayMS(10);      //��ʱ10MSȥ��
        if (K3 == 0)
        {
            while(!K3); //���ּ��
            return 3;     //�а�������
        }
    }
    
    return 0;             //�ް�������
}

/**************************************************************************************************
 * �������ƣ�initIO
 * ����������IO��ʼ��������P0.1,P2.0,P0.7;
 * ��    ������
 * �� �� ֵ����
 **************************************************************************************************/
void InitIO(void)
{

    P0SEL &= ~0x82;     //����P0.1,P0.7Ϊ��ͨIO��  
    P0DIR &= ~0x82;     //��������P0.1,P0.7���ϣ�����P0.1,P0.7Ϊ���뷽�� 
    P0INP &= ~0x82;     //��P0.1,P0.7��������
  
    P2SEL &= ~0x01;     //����P2.0Ϊ��ͨIO��  
    P2DIR &= ~0x01;     //��������P2.0���ϣ�����P2.0Ϊ���뷽��
    P2INP &= ~0x01;     //��P2.0��������
    
}

/*****************************************
 ���ڳ�ʼ����������ʼ������ UART0		
*****************************************/
void InitUART0(void)
{
  	P0SEL = 0x0c;				 //P0.2 P0.3��������
	PERCFG = 0x00;				 //ѡ��USART0λ��1	
	P2DIR &= ~0XC0;                          //P0������ΪUART0    
        U0CSR |= 0x80;   	                 //��������ΪUART��ʽ
        U0GCR |= 11;				
        U0BAUD |= 216;		                  //��������Ϊ115200
	UTX0IF = 0;                               //UART0 TX�жϱ�־��ʼ��λ1  
        U0CSR |= 0X40;				  //�������
        //IEN0 |= 0x84;				  //�����жϣ������ж�    
}

/****************************************************************
   ���ڷ����ַ�������						
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
  U0DBUF = 0x0A;        // ����
  while(UTX0IF == 0);
  UTX0IF = 0;
}

/**************************
ϵͳʱ�� ����Ƶ
����ʱ�� 32��Ƶ
**************************/
void InitClock(void)
{   
    CLKCONCMD &= ~0x40; // ����ϵͳʱ��ԴΪ 32MHZ����
    while(CLKCONSTA & 0x40);    // �ȴ������ȶ� 
    CLKCONCMD &= ~0x47;          // ����ϵͳ��ʱ��Ƶ��Ϊ 32MHZ
}

/* ��һ���ڴ�����ת��Ϊʮ�����Ƹ�ʽ���ַ����� str-�ַ���ָ�룬src-Դ���ݵ�ַ��len-���ݳ���*/
void MemToStr(unsigned char *str, unsigned char *src, unsigned char len) {
    unsigned char tmp;
    while (len--) {
        tmp = *src >> 4; //��ȡ��4λ
        if (tmp <= 9) *str++ = tmp + '0'; //ת��Ϊ0-9��A-F 
        else *str++ = tmp - 10 + 'A';
        tmp = *src & 0x0F; //��ȡ��4λ
        if (tmp <= 9) *str++ = tmp + '0'; //ת��Ϊ0-9��A-F 
        else *str++ = tmp - 10 + 'A';
        *str++ = ' '; //ת����һ���ֽ����һ���ո�
        src++;
    } 
}

/**************************************************************************************************
 * �������ƣ�main
 * ������������ⰴ��KEY1,KEY2,KEY3�����ݰ���ִ����Ӧ�Ķ�����
 *        KEY1��д����
 *        KEY2��������
 *        KEY3������+1
 * ��    ������
 * �� �� ֵ����
 **************************************************************************************************/
void main()
{

  unsigned char keynum;  
  unsigned char num0=15,num1=0,addr=9;
  
 
    InitClock();  // ����ϵͳʱ��ԴΪ32MHz��������
    InitIO();  
    InitUART0();  //��ʼ������

    DelayMS(10);
    UartTX_Send_String("Start:" ,6);
    //���ڴ�ӡ��ǰ��д������ݣ�����λʮ���Ʊ�ʾ
    numdata[5]= (num0)/100+48;  //��λ
    numdata[6]= (num0%100/10)+48; //ʮλ
    numdata[7]= (num0%100%10) +48; //��λ   
    UartTX_Send_String(numdata ,8);
    DelayMS(10);

    while(1)
    {
         DelayMS(2);
         keynum = KeyScan();
		
         if(keynum==1) // Key1 press detected
         {
                // UartTX_Send_String("write loop:" ,11);
                //DelayMS(200);
		At24c02Write(addr,num0);                   
                writedata[7]= (char)(num0)/100+48;
                writedata[8]= (char)(num0%100/10)+48;
                writedata[9]= (char)(num0%100%10) +48;  
                //DelayMS(50);
                UartTX_Send_String(writedata ,10);
             
	}
		
	if(keynum==2) // Key2 press detected
	{
                // UartTX_Send_String("read loop:" ,11);  
                //DelayMS(200);
		num1=At24c02Read(addr);                
                readdata[6]= (char)(num1)/100+48;
                readdata[7]= (char)(num1%100/10)+48;
                readdata[8]= (char)(num1%10) +48;    
                //DelayMS(50);
                UartTX_Send_String(readdata ,9);
	}
        
	if(keynum==3) // Key3 press detected
	{
        //        //UartTX_Send_String("increase loop:" ,15);		   
        //        //DelayMS(10);
	    //    if(num0==255) num0=0; else num0++;;
        //        numdata[5]= (num0)/100+48;
        //        numdata[6]= (num0%100/10)+48;
        //        numdata[7]= (num0%100%10) +48;    
        //        //DelayMS(50);
        //        UartTX_Send_String(numdata ,8);
        E2Write1(testdata, 11, 10);
        E2Read(getdata, 11, 10);
        // unsigned char i = 0;
        // for(i; i < 10; i++){
        //     *(getdata+i) += 48;
        // }
        MemToStr(hex_to_ascii_data, getdata, 10); //HEX to ASCII
        UartTX_Send_String(hex_to_ascii_data, 10 * 3);
    }
    }
  
}
