#include "apilib.h"
#include<string.h>
void HariMain(void)
{	
	//init
	api_initmalloc();
	char *filename = (char *)api_malloc(128);	
	api_puts("input filename:\n");
	api_gets(filename,128);
	
	int fh = api_file_open(filename,'r');
	if(fh == 0){
		api_puts("can open file for read\n");
		api_return();
	}
	
	char *buf = (char *)api_malloc(128);
	int len=0,i=0;
	while(api_file_eof(fh)==0){
		len=api_file_read(fh, buf, 128);
		for(i=0;i<len;i++){
			api_putchar(buf[i]);
		}
	}
	api_puts("\n");
	api_file_close(fh);
	api_return();
}