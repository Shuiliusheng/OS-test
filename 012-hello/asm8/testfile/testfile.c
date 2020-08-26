#include "apilib.h"

#include<string.h>
void HariMain(void)
{	
	//init
	api_initmalloc();
	char str[100];
	char *buf = (char *)api_malloc(100);	//1kb
	int i=0,j=0;
	
	int fh=0;
	char c=0;
	for(i=0;i<5;i++){	//16K,21K,12K
		sprintf(str,"test%d.txt",i);
		fh = api_file_open(str, 'w');
		
		for(j=0;j<100;j++){
			buf[j]=i+'0';
		}
		buf[9]='\0';
		api_file_write(fh,buf,10);
		api_file_close(fh);
	}
	
	fh = api_file_open("test1.txt",'a');
	if(fh==0){
		api_puts("can open file for a\n");
		api_return();
	}
	api_file_write(fh,"test for mode a", 16);
	api_file_close(fh);
	
	fh = api_file_open("test1.txt",'r');
	if(fh == 0){
		api_puts("can open file for r\n");
		api_return();
	}
	i=api_file_read(fh, buf, 100);
	sprintf(str,"Read:%d\n",i);
	api_puts(str);
	for(j=0;j<i;j++){
		api_putchar(buf[j]);
	}
	api_puts("\n");
	api_file_close(fh);
	
	
	api_return();
}