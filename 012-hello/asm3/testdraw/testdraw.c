#include "apilib.h"
#include <string.h>

void HariMain(void)
{	
	api_initmalloc();
	char *buf = api_malloc(300*400);
	int sht = api_initwindow(buf,300,400,-1,"test");
	api_draw_string(sht, "show text", 28, 28, 7, 0);
	api_draw_block(sht, 28, 44, 50, 16, 1);
	int i=0,j=0;
	for(i=30;i<100;i++){
		for(j=30;j<100;j++){
			buf[i*300+j]=1;
		}
	}
	api_draw_refresh(sht,30,30,70,70);
	api_getchar(1);
	api_return();
}