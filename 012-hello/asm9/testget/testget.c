#include "apilib.h"

void HariMain(void)
{	
	char buf[100];
	int t;
	char c;
	api_puts("test for getchar: ");
	while(1){
		c=api_getchar(1);	//1: 等待输入；0：不等待输入，如果没有输入返回-1
		if(c==10){	//enter
			break;
		}
		else{
			api_putchar(c);
		}
	}
	api_puts("test for getchar: over\n");
	
	
	api_puts("test for gets: ");
	t=api_gets(buf,100);
	api_puts("input: ");
	api_puts(buf);
	api_puts("\n");
	api_puts("test for gets: over\n");
	api_return();
}