#include "sd.h"
#include "spi.h"
#include "stm32f10x_spi.h"
#include "sys.h"

unsigned char sd_type;

unsigned char sd_readWrite(unsigned char data)
{
	return spi1_readWriteByte(data);
}

//SD����ʼ��ʱ,ʼ�ղ��ܴ���400KHz
void sd_speedLow(void)
{
	spi1_setSpeed(SPI_BaudRatePrescaler_256);
}

void sd_speedHigh(void)
{
	spi1_setSpeed(SPI_BaudRatePrescaler_2);
}

//�ȴ���׼����
//SD����æ״̬,D0������
unsigned char sd_waitReady(void)
{
	unsigned int count = 0;
	do{
		if(sd_readWrite(0xff) == 0xFF)return 0;
	}while(count<0xFFFFFF);
	return 1;
}

//����һ������
unsigned char sd_sendCmd(unsigned char cmd,unsigned int arg,unsigned char crc)
{
	unsigned char retry = 0;
	unsigned char r1 = 0x80;
	//��������ǰ��Ҫ�ȴ�SD���ǿ���״̬
	while(sd_waitReady() != 0);
	//��������Ǵ��������ֶ�Ӧ�Ĵ�λ������߿�ʼ
	sd_readWrite(cmd | 0x40);
	sd_readWrite(arg >> 24);
	sd_readWrite(arg >> 16);
	sd_readWrite(arg >> 8);
	sd_readWrite(arg);
	sd_readWrite(crc);
	if(cmd==CMD12)sd_readWrite(0xff);
	//��SEND_STATUS������,ÿ�������SD�����ᷢ��R1��Ӧ
	retry = 0x1F;
	do{
		r1 = sd_readWrite(0xff);
	}while((r1&0x80) && retry--);
	return r1;
}

unsigned char sd_init(void)
{
	unsigned char buf[4] = "";
	unsigned short i = 0;
	unsigned short retry = 0;
	unsigned char r1 = 0;
	//1.��ʼ��Ӳ��
	spi1_init();
	SD_CS = 0;
	//2.����Ϊ����ģʽ
	sd_speedLow();
	//3.��������74������
	for(i=0;i<10;i++) sd_readWrite(0xff);
	//����CMD0,����IDLE״̬
	retry = 20;
	do{
		r1 = sd_sendCmd(CMD0,0,0x95);
	}while((r1!=0x01) && retry--);
	sd_type = 0;	//Ĭ���޿�
	if(r1 == 0x01){
		if(sd_sendCmd(CMD8,0x1AA,0x87) == 1){	//v2.0
			for(i=0;i<4;i++)buf[i] = sd_readWrite(0xff);
			if(buf[2]==0x01&&buf[3]==0xAA){	//�жϿ��Ƿ�֧��2.7~3.6v
				retry = 0xFFFE;
				do{
					sd_sendCmd(CMD55,0,0x01);
					r1 = sd_sendCmd(ACMD41,0x40000000,0x01);	//HCSλ����Ϊ1,����λΪ0
				}while(r1&&retry--);
				if(retry&&sd_sendCmd(CMD58,0,0x01)==0){		//����2.0�汾������߰汾
					for(i=0;i<4;i++)buf[i] = sd_readWrite(0xff);
					if(buf[0]&0x40)sd_type = SD_TYPE_V2HC;
					else sd_type = SD_TYPE_V2;
				}
			}
		}else{ //MMC/v1.0
			sd_sendCmd(CMD55,0,0x01);
			r1 = sd_sendCmd(ACMD41,0,0x01);
			if(r1<=1){
				sd_type = SD_TYPE_V1;
				retry = 0xFFFE;
				do{
					sd_sendCmd(CMD55,0,0x01);
					r1 = sd_sendCmd(ACMD41,0,0x01);
				}while(r1&&retry--);
			}else{	//MMC����֧��CMD8+ACMD41
				sd_type = SD_TYPE_MMC;
				retry = 0xFFFE;
				do{
					r1 = sd_sendCmd(CMD1,0,0x01);
				}while(r1&&retry--);
			}
			if(retry==0||sd_sendCmd(CMD16,512,0x01)!=0) sd_type = SD_TYPE_ERR;
		}
	}
	if(sd_type) return 0;
	else if(r1) return r1;
	return 0xaa;
}
/**********************************
** buf:���ݻ�����
** sector:����
** cnt:������
** ����ֵ��0,�ɹ�.����,ʧ��.
**********************************/
unsigned char sd_read(unsigned char *buf,unsigned int sector,unsigned char cnt)
{
	unsigned char r1;
	if(sd_type!=SD_TYPE_V2HC)sector <<= 9;	//ת��Ϊ�ֽڵ�ַ
	if(cnt == 1){
		r1 = sd_sendCmd(CMD17,sector,0x01);
		if(r1 == 0){	//ָ��ͳɹ���������
			r1 = sd_recvData(buf,512);
		}
	}else{
		r1 = sd_sendCmd(CMD18,sector,0x01);
		do{
			r1 = sd_recvData(buf,512);
			buf+=512;
		}while(--cnt && r1==0);
		sd_sendCmd(CMD12,0,0x01);
	}
	return r1;
}

/*********************************
** ��SD����ȡһ�����ݰ�������
** buf:���ݻ�����
** len:Ҫ��ȡ�����ݳ���
** ����ֵ:0,�ɹ�.����,ʧ��.
********************************/
unsigned char sd_recvData(unsigned char *buf,unsigned short len)
{
	if(sd_getResponse(0xFE))return 1;
	//��ʼ��������
	while(len--){
		*buf=sd_readWrite(0xFF);
		buf++;
	}
	//2��αCRC
	sd_readWrite(0xFF);
	sd_readWrite(0xFF);
	return 0;
}

/*****************************
** �ȴ�SD����Ӧ(��ʼ����)
** response:��Ҫ�õ��Ļ�Ӧֵ
** ����ֵ:0,�ɹ�.����,ʧ��
*****************************/
unsigned char sd_getResponse(unsigned char response)
{
	unsigned short count = 0xFFFE;
	while((sd_readWrite(0xff)!=response)&&count) count--;
	if(count==0)return SD_RESPONSE_FAIL;
	else return SD_RESPONSE_SUCCESS;
}

/*****************************
** SD��д����
** buf:���ݻ�����
** sector:��ʼ����
** cnt:������
** ����ֵ:0,�ɹ�.����,ʧ��
*****************************/
unsigned char sd_write(unsigned char *buf,unsigned int sector,unsigned char cnt)
{
	unsigned char r1;
	if(sd_type!=SD_TYPE_V2HC)sector *= 512;
	if(cnt==1){
		r1 = sd_sendCmd(CMD24,sector,0x01);
		if(r1==0){
			r1 = sd_sendBlock(buf,0xFE);
		}
	}else{
		if(sd_type!=SD_TYPE_MMC){
			sd_sendCmd(CMD55,0,0x01);
			sd_sendCmd(CMD23,cnt,0x01);
		}
		r1 = sd_sendCmd(CMD25,sector,0x01);
		if(r1==0){
			do{
				r1 = sd_sendBlock(buf,0xFC);
				buf+=512;
			}while(--cnt&&r1==0);
			r1 = sd_sendBlock(0,0xFD);
		}
	}
	return 0;
} 

unsigned char sd_sendBlock(unsigned char *buf,unsigned char cmd)
{
	unsigned char r = 0;
	unsigned short i;
	if(sd_waitReady())return 1;
	sd_readWrite(cmd);
	if(cmd!=0xFD){
		for(i=0;i<512;i++)sd_readWrite(buf[i]);
		//CRC
		sd_readWrite(0xFF);
		sd_readWrite(0xFF);
		//������Ӧ
		r = sd_readWrite(0xFF);
		if(r&0x1F)return 2;	//��Ӧ����
	}
	return 0;
}

/*************************
** ��ȡSD����CID��Ϣ,������������Ϣ
** cid:���CID�Ĵ���ֵ�û���
** ����ֵ:0,�ɹ�.1����.
*************************/
unsigned char sd_getCID(unsigned char *cid)
{
	unsigned char r1 = 0;
	//����CMD10
	r1 = sd_sendCmd(CMD10,0,0x01);
	if(r1==0){
		r1 = sd_recvData(cid,16);
	}
	if(r1)return  1;
	else return 0;
}

/****************************
** ��ȡCSD�Ĵ�����ֵ
** csd:���CSD�Ĵ���ֵ�û���
** ����ֵ:0,�ɹ�.1,ʧ��.
***************************/
unsigned char sd_getCSD(unsigned char *csd)
{
	unsigned char r1 = 0;
	r1 = sd_sendCmd(CMD9,0,0x01);
	if(r1==0){
		r1 = sd_recvData(csd,16);
	}
	if(r1)return 1;
	else return 0;
}

/***************************
** ��ȡSD��������
** 
**
** ����ֵ:SD������
***************************/
unsigned int sd_getSector(void)
{
	unsigned int capacity = 0;
	unsigned char csd[16] = "";
	unsigned short csize = 0;
	unsigned char n = 0;
	//��ȡCSD��Ϣ
	if(sd_getCSD(csd)!=0) return 0;
	if(csd[0]&0x40){	//V2.00��
		csize = csd[9]+((unsigned short)csd[8]<<8);
		capacity = ((unsigned int)csize+1)*1024;
	}else{	//V1.xx��
		n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
		csize = (csd[8] >> 6) + ((unsigned short)csd[7] << 2) + ((unsigned short)(csd[6] & 3) << 10) + 1;
		capacity= (unsigned int)csize << (n - 9);//�õ�������   
	}
	return capacity;
}
