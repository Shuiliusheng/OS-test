#include "apilib.h"
#include <string.h>

void testfuc()
{
	int i=0;
	for(i=0;i<100;i++){
		api_puts("testfuc");
	}
	for(;;);
	return ;
}

void HariMain(void)
{	
	int i;
	char buf[20];
	sprintf(buf,"test:0x%x, 0x%x\n",(int)testfuc,(int)HariMain);
	api_puts(buf);
	
	api_putchar((int)testfuc);
	for(i=0;i<100;i++){
		api_puts("HariMain");
	}
	
	for(;;);
	api_return();
}