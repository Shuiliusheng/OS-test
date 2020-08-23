#include "apilib.h"
#include <string.h>
void HariMain(void)
{	
	int i;
	char buf[20];
	for(i=0;i<100000;i++)
	{
		sprintf(buf,"input:%d\r",i%500);
		api_puts(buf);
	}
	api_puts("\n");
	
	api_putchar('a');
	api_putchar('1');
	api_putchar('@');
	api_putchar('>');
	api_putchar('~');
	api_return();
}