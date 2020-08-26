#include "apilib.h"
#include <string.h>

#define MOUSE_EVENT_LEFTDOWN 1
#define MOUSE_EVENT_LEFTUP 2
#define MOUSE_EVENT_RIGHTDOWN 3
#define MOUSE_EVENT_RIGHTUP 4
#define MOUSE_EVENT_MOVE 5

struct MOUSE_EVENT{
	short x, y;
	int type;
};

char *buf;
int sht;
int lastx=0,lasty=0;
void listen_event(struct MOUSE_EVENT event)
{
	char str[10];
	if(event.type == MOUSE_EVENT_LEFTDOWN){
		api_draw_block(sht, 30, 30, 30, 30, 1);
		api_puts("left down\n");
	}
	else if(event.type == MOUSE_EVENT_LEFTUP){
		api_draw_block(sht, 30, 30, 30, 30, 2);
		api_puts("left up\n");
	}
	else if(event.type == MOUSE_EVENT_RIGHTDOWN){
		api_draw_block(sht, 30, 60, 30, 30, 3);
		api_puts("right down\n");
	}
	else if(event.type == MOUSE_EVENT_RIGHTUP){
		api_draw_block(sht, 30, 60, 30, 30, 4);
		api_puts("right up\n");
	}
	else if(event.type == MOUSE_EVENT_MOVE){
		if(event.x<0||event.x>300||event.y<0||event.y>400){
			api_return();
		}
		api_draw_block(sht, lastx, lasty, 10, 10, 0);
		lastx = event.x;
		lasty = event.y;
		api_draw_block(sht, lastx, lasty, 10, 10, 2);
	}
	
	api_return();
}

void HariMain(void)
{	
	api_initmalloc();
	char *stack = api_malloc(1024*5);
	buf = api_malloc(300*400);
	sht = api_initwindow(buf,300,400,-1,"test");
	int te = api_thread_listen((int)&listen_event, stack+1024*5);
	
	api_getchar(1);
	api_return();
}