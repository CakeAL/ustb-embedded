#ifndef __I2C_H_
#define __I2C_H_

#include <ioCC2530.h>

#define SCL          P0_6     //I2C时钟
#define SDA          P0_5     //I2C数据线

void I2cStart();
void I2cStop();
unsigned char I2cSendByte(unsigned char dat);
unsigned char I2cReadByte(unsigned char ack);

#define IO_DIR_PORT_PIN(port, pin, dir)  \
   do {                                  \
      if (dir == IO_OUT)                 \
         P##port##DIR |= (0x01<<(pin));  \
      else                               \
         P##port##DIR &= ~(0x01<<(pin)); \
   }while(0)

// Where port={0,1,2}, pin={0,..,7} and dir is one of:
#define IO_IN   0
#define IO_OUT  1

#define IO_SET_SDA_OUT IO_DIR_PORT_PIN(0, 5, IO_OUT)
#define IO_SET_SDA_IN  IO_DIR_PORT_PIN(0, 5, IO_IN)
#define IO_SET_SCL_OUT IO_DIR_PORT_PIN(0, 6, IO_OUT)




#endif
