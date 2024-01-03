#ifndef SHT10_H
#define SHT10_H

// ��
//sht10.h���������˿ڶ˶Բ��Ͽ������·ͼ
//sth10.c����P0DIRȫ����Ҫ��
//Ȼ�������ļ���Ҳ�Բ���

#include <ioCC2531.h>
#include <string.h>

#define DATA   P0_5//����ͨѶ���ݶ˿�
#define SCK    P0_6//����ͨѶʱ�Ӷ˿�
#define noACK 0       //�����������ݣ������ж��Ƿ����ͨѶ
#define ACK   1       //�������ݴ��䣻
                            //��ַ  ����
#define MEASURE_TEMP 0x03   //000   00011
#define MEASURE_HUMI 0x05   //000   00101

#define uint unsigned int
#define uchar unsigned char
#define Uint16 unsigned int


void delay1Us(Uint16 microSecs);
void init_uart(void);
void s_connectionreset(void);
void s_transstart(void);
char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode);
char s_write_byte(unsigned char value);
char s_read_byte(unsigned char ack);
void calc_sth11(float *p_humidity ,float *p_temperature);
float calc_dewpoint(float h,float t);
void delay (unsigned int time);

#endif
