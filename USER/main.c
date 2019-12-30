#include "stm32f10x.h"
#include "key.h"
#include "exti.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "sd.h"
#include "ff.h"
#include "string.h"
#include "iap.h"

#define FLASH_APP_ADDR 0x08006000

int is_updata_program(void);
unsigned char buf[1024];
int main(){
	unsigned char i = 0;
	UINT rbw = 0;
	unsigned int res;
	FIL fil;
	key_init();
	led_init();
//	led_on(1);
	key_on();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	exti0_init();
	uart3_init(115200);
	delay_init();
	while(sd_init());
	if(is_updata_program() == 1){
		//更新程序,更新结束后删除box.bin,
		res = f_open(&fil,(const TCHAR *)"box.bin",FA_READ);
		f_sync(&fil);
		res = f_read(&fil,(void *)buf,1024,&rbw);
		while(rbw > 0){
			iap_write_appbin(FLASH_APP_ADDR+i*0x400,buf,1024);
			i++;
			res = f_read(&fil,(void *)buf,1024,&rbw);
		}
		res = f_close(&fil);
		//删除box.bin
		res = f_unlink((const TCHAR *)"box.bin");
	}
	//跳转到应用程序
	iap_load_app(FLASH_APP_ADDR);
	return 0;
}

//判断是否需要更新程序
//判断方法:查看SD卡根目录下是否有box.bin文件
int is_updata_program(void)
{
	unsigned char path0[3] = "0:";
	unsigned int res;
	FATFS fs;
	DIR dir;
	FILINFO fno;
	res = f_mount(&fs,"0:",1);
	res = f_opendir(&dir,(const TCHAR*)path0);
	res = f_readdir(&dir,&fno);
	while(fno.fname[0] != '\0'){
		if(strcmp(fno.fname,(const char*)"BOX.BIN") == 0){
			return 1;
		}
		res = f_readdir(&dir,&fno);
	}
	res = f_mount(0,"0:",1);
	return 0;
}
