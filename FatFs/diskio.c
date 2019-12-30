/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sd.h"


/* Definitions of physical drive number for each drive */
#define SD		0	/* Example: Map MMC/SD card to drive number 1 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv) {
		case SD:
			stat = sd_init();
			if(stat){
				sd_speedLow();
				sd_readWrite(0xff);
				sd_speedHigh();
				return STA_NOINIT;
			}
			return stat;
		default:
			stat = 1;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	if (!count)return RES_PARERR;
	switch (pdrv) {
		case SD:
			res = (DRESULT)sd_read(buff,sector,count);
			if(res){
				sd_speedLow();
				sd_readWrite(0xff);
				sd_speedHigh();
				return RES_ERROR;
			}
			return RES_OK;
		default:
			res = RES_ERROR;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if (!count)return RES_PARERR;
	switch (pdrv) {
		case SD:
			sd_write((unsigned char*)buff,sector,count);
			return RES_OK;
	}

	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	if(pdrv==SD) {
		switch(cmd){
			case CTRL_SYNC:
				if(sd_waitReady()) return RES_OK;
				else return RES_ERROR;
			case GET_SECTOR_SIZE:
				*(WORD*)buff = 512;
				return RES_OK;
			case GET_BLOCK_SIZE:
				*(WORD*)buff = 8;
				return RES_OK;
			case GET_SECTOR_COUNT:
				*(DWORD*)buff = sd_getSector();
				return RES_OK;
			default:
				return RES_PARERR;
		}
	}

	return RES_PARERR;
}
#endif
