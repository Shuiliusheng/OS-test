#include "apilib.h"
#include<string.h>
void HariMain(void)
{	
	//init
	api_initmalloc();
	char *filename = (char *)api_malloc(128);	
	api_puts("input filename:\n");
	api_gets(filename,128);
	
	int fh = api_file_open(filename,'w');
	if(fh == 0){
		api_puts("can open file for write\n");
		api_return();
	}
	
	char *buf = (char *)api_malloc(129);
	buf[128]='\0';
	int len=0,i=0;
	int l=0;
	api_puts("input filename:(#: input over)\n");
	while(1){
		l=api_gets(buf,128);
		if(strcmp(buf,"#")==0){
			break;
		}
		buf[l]='\n';
		buf[l+1]='\0';
		api_file_write(fh,buf,strlen(buf));
	}
	api_file_close(fh);
	api_return();
}