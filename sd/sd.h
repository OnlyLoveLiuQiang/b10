#ifndef __SD_H__
#define __SD_H__

#define	SD_CS  PAout(15) 	//SD¿¨Æ¬Ñ¡Òý½Å

#define SD_TYPE_ERR     0X00
#define SD_TYPE_MMC     0X01
#define SD_TYPE_V1      0X02
#define SD_TYPE_V2      0X04
#define SD_TYPE_V2HC    0X06

#define CMD0	0
#define CMD1	1
#define CMD8	8
#define CMD9	9
#define CMD10	10
#define CMD12	12
#define CMD16	16
#define CMD17	17
#define CMD18	18
#define CMD23	23
#define CMD24	24
#define CMD25	25
#define CMD55	55
#define CMD58	58

#define ACMD41	41

#define SD_RESPONSE_FAIL		0xFF
#define SD_RESPONSE_SUCCESS		0x00

unsigned char sd_readWrite(unsigned char data);
void sd_speedLow(void);
void sd_speedHigh(void);
unsigned char sd_waitReady(void);
unsigned char sd_sendCmd(unsigned char cmd,unsigned int arg,unsigned char crc);
unsigned char sd_init(void);
unsigned char sd_read(unsigned char *buf,unsigned int sector,unsigned char cnt);
unsigned char sd_recvData(unsigned char *buf,unsigned short len);
unsigned char sd_getResponse(unsigned char response);
unsigned char sd_write(unsigned char *buf,unsigned int sector,unsigned char cnt);
unsigned char sd_sendBlock(unsigned char *buf,unsigned char cmd);
unsigned char sd_getCID(unsigned char *cid);
unsigned char sd_getCSD(unsigned char *csd);
unsigned int sd_getSector(void);
#endif
