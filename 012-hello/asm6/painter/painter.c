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

void drawline(char *buf, int width, int x0, int y0, int x1, int y1, int color, int linewidth);
void put_pixel(char *buf, int width, int x, int y, int color);

int sht;
char *buf;
int bufw,bufh;

int color_sx, color_sy, color_size;
int current_color, current_lw;
char isClicked;
short lastx, lasty;

void listen_event(struct MOUSE_EVENT event)
{
	char str[10];
	int t1=0;
	if(event.type == MOUSE_EVENT_RIGHTDOWN){
		isClicked = 1;
		if(event.x>=50&&event.x<bufw&&event.y>=20&&event.y<bufh){
			lastx = event.x;
			lasty = event.y;
		}
		else if(event.x<color_sx+40&&event.x>=color_sx
				&&event.y>=color_sy&&event.y<color_sy+640){
			
			current_color = (event.x-color_sx)/color_size + (event.y-color_sy)/color_size*4;
			api_draw_block(sht, 10, 25, 3, 3, current_color);	//线宽1
			api_draw_block(sht, 20, 25, 5, 5, current_color); //线宽2
			api_draw_block(sht, 30, 25, 7, 7, current_color); //线宽2
			api_draw_block(sht, 40, 25, 9, 9, current_color); //线宽2
			
			lastx = -1;
		}
		else if(event.y>=25&&event.y<=35){
			if(event.x>=10&&event.x<20){
				current_lw = 1;
			}
			else if(event.x>=20&&event.x<30){
				current_lw = 3;
			}
			else if(event.x>=30&&event.x<40){
				current_lw = 5;
			}
			else if(event.x>=40&&event.x<50){
				current_lw = 7;
			}
			lastx = -1;
		}
		else if(event.y>=37&&event.y<=47){
			if(event.x>=10&&event.x<20){
				current_lw = -1;
			}
			else if(event.x>=20&&event.x<30){
				current_lw = -2;
			}
			else if(event.x>=30&&event.x<40){
				current_lw = -3;
			}
			else if(event.x>=40&&event.x<50){
				current_lw = -4;
			}
			lastx = -1;
		}
		else{
			lastx = -1;
		}
	}
	else if(event.type == MOUSE_EVENT_RIGHTUP){
		isClicked = 0;
		lastx = -1;
	}
	else if(event.type == MOUSE_EVENT_MOVE){
		if(isClicked==1&&lastx!=-1){
			if(event.x>=60&&event.x<bufw-10&&event.y>=20&&event.y<bufh-10){
				if(lastx!=event.x||lasty!=event.y){
					if(current_lw>0){
						drawline(buf, bufw, lastx, lasty, event.x, event.y, current_color, current_lw);
						lastx = event.x;
						lasty = event.y;
					}
					else{
						if(current_lw==-1){
							api_draw_block(sht, event.x-1, event.y-1, 3, 3, 7);
						}
						else if(current_lw==-2){
							api_draw_block(sht, event.x-3, event.y-3, 7, 7, 7);
						}
						else if(current_lw==-3){
							api_draw_block(sht, event.x-5, event.y-5, 11, 11, 7);
						}
						else if(current_lw==-4){
							api_draw_block(sht, event.x-6, event.y-6, 13, 13, 7);
						}
					}
				}
			}
			else{
				lastx = -1;
			}
		}
	}
	api_return();
}

void HariMain(void)
{	
	api_initmalloc();
	bufw = 700;
	bufh = 700;
	buf = api_malloc(bufw*bufh);
	sht = api_initwindow(buf,bufw,bufh,-1,"test");

	color_sx = 10;
	color_sy = 50;
	color_size = 10;
	current_color=0, current_lw=1;
	
	/// painter color table
	int i=0, x = color_sx, y = color_sy;
	for(i=1;i<=248;i++){
		api_draw_block(sht, x, y, color_size, color_size, i-1);
		x+=color_size;
		if(i%4==0){
			y+=color_size;
			x = color_sx;
		}
	}
	
	api_draw_block(sht, 10, 25, 40, 10, 7);
	api_draw_block(sht, 10, 25, 3, 3, current_color);	//线宽1
	api_draw_block(sht, 20, 25, 5, 5, current_color); //线宽2
	api_draw_block(sht, 30, 25, 7, 7, current_color); //线宽2
	api_draw_block(sht, 40, 25, 9, 9, current_color); //线宽2
	
	drawline(buf,bufw, 5,36, 55, 36, 0,1);
	
	api_draw_block(sht, 10, 37, 40, 10, 0);	//橡皮擦
	api_draw_block(sht, 10, 37, 3, 3, 7); //橡皮擦1
	api_draw_block(sht, 20, 37, 5, 5, 7); //橡皮擦1
	api_draw_block(sht, 30, 37, 7, 7, 7); //橡皮擦1
	api_draw_block(sht, 40, 37, 9, 9, 7); //橡皮擦2
	
	drawline(buf,bufw, 5, 49, 55, 49, 0, 1);
	
	//background
	api_draw_block(sht, 60, 20, bufw-70, bufh-30, 7); //橡皮擦2
	
	isClicked=0;
	lastx = -1;
	char *stack = api_malloc(1024);
	int te = api_thread_listen((int)&listen_event, stack+1024);
	
	
	while(api_getchar(1)!='c');
	api_thread_delete(te);
	api_return();
}

int abs1(int x)
{
	return ( x<0 ) ? -x : x;
}

void put_pixel(char *buf, int width, int x, int y, int color)
{
	if(x<0||y<0||x>=width){
		return ;
	}
	buf[y*width+x]=color;
}

void drawline(char *buf, int width, int x0, int y0, int x1, int y1, int color, int linewidth)
{
    int dx,dy,epsl,k,i;
    int xIncre,yIncre,x,y;
    dx = x1 - x0;
    dy = y1 - y0;
    x = x0;
    y = y0;
    if(abs1(dx) > abs1(dy))
        epsl = abs1(dx);
    else
        epsl = abs1(dy);
	
    xIncre = (dx*1000) / (epsl);
    yIncre = (dy*1000) / (epsl);
 
	x *= 1000;
	y *= 1000;
	linewidth *= 1000;
    for(k = 0; k < epsl; k++){
		if(y0 - y1 <= 0){
			for(i = 0; i < linewidth; i+=1000){
				put_pixel(buf, width, (x+500)/1000,(y+500+i/2)/1000,color);
			}
		}
		else{
			for(i = 0; i < linewidth; i+=1000){
				put_pixel(buf, width, (x+500+i/2)/1000,(y+500)/1000,color);
			}
		}
        x += xIncre;
        y += yIncre;
	}
	
	x = (x0>x1)?x1:x0;
	y = (y0>y1)?y1:y0;
	
	dx = (x0>x1)?x0:x1;
	dy = (y0>y1)?y0:y1;
	
	api_draw_refresh(sht, x, y, dx-x+1, dy-y+1);
}