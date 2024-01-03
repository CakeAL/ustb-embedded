#ifndef SHT10_H
#define SHT10_H

// 坑
//sht10.h里面两个端口端对不上开发版电路图
//sth10.c里面P0DIR全部需要改
//然后这俩文件名也对不上

#include <ioCC2531.h>
#include <string.h>

#define DATA   P0_5//定义通讯数据端口
#define SCK    P0_6//定义通讯时钟端口
#define noACK 0       //继续传输数据，用于判断是否结束通讯
#define ACK   1       //结束数据传输；
                            //地址  命令
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
