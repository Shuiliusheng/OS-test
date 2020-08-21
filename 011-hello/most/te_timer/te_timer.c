#include "apilib.h"
#include <string.h>
void HariMain(void)
{	
	char  s[12];
	int win, timer, sec = 0, min = 0, hou = 0;
	api_initmalloc();
	timer = api_allocTimer();
	api_initTimer(timer, 128);
	for (;;) {
		sprintf(s, "%5d:%02d:%02d\r", hou, min, sec);
		api_setTimer(timer, 100);	
		win = api_getchar(1);
		if ( win != -128) {
			break;
		}
		sec++;
		if (sec == 60) {
			sec = 0;
			min++;
			if (min == 60) {
				min = 0;
				hou++;
			}
		}
		api_puts(s);
	}
	api_return();
}