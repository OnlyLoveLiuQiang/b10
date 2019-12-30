#ifndef __SPI_H__
#define __SPI_H__

void spi1_init(void);
void spi1_setSpeed(unsigned char SpeedSet);
unsigned char spi1_readWriteByte(unsigned char TxData);
#endif

