#include "apilib.h"
#include <string.h>

struct WINDOW{
	char *buf;
	int sht;
	int width;
	int height;
	int timer;
	int fsx,fsy,fex,fey;
};
#define sWindow struct WINDOW
void put_pixel(sWindow *win, int x, int y, int color);
void drawline(sWindow *win, int x1, int y1, int x2, int y2, int color);
void drawcircle_fill(sWindow *win, int x, int y, int r, int color);
void drawcircle(sWindow *win, int x, int y, int r, int color);
void sleep(sWindow *win, int nsec);
void painter1();




void HariMain(void)
{	
	api_initmalloc();
	
	painter1();
	
	api_getchar(1);
	api_return();
}



//////////////////////////////////
void refresh_win(sWindow *win)
{
	if(win->fsx == win->fex && win->fsy == win->fey){
		return;
	}
	api_draw_refresh(win->sht, win->fsx, win->fsy, win->fex-win->fsx+1, win->fey-win->fsy+1);
	win->fsx = 0;
	win->fsy = 0;
	win->fex = 0;
	win->fey = 0;
}

void putpixel(sWindow *win, int x, int y, int color)
{
	if(x<0||y<0||x>win->width||y>win->height){
		return ;
	}
	win->buf[y*win->width+x]=color;
	if(x<win->fsx){win->fsx = x;}
	else if(x>win->fex){win->fex = x;}
	
	if(y<win->fsy){win->fsy = y;}
	else if(y>win->fey){win->fey = y;}
}

void drawline(sWindow *win, int x1, int y1, int x2, int y2, int color)
{
	int x = x1;
	int y = y1;
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int s1 = x2 > x1 ? 1 : -1;
	int s2 = y2 > y1 ? 1 : -1;
	
	int interchange = 0;	// 默认不互换 dx、dy
	if (dy > dx)				// 当斜率大于 1 时，dx、dy 互换
	{
		int temp = dx;
		dx = dy;
		dy = temp;
		interchange = 1;
	}
	
	int p = 2 * dy - dx, i=0;
	for(i = 0; i < dx; i++){
		putpixel(win, x, y, color);
		if (p >= 0){
			if (!interchange){	// 当斜率 < 1 时，选取上下象素点
				y += s2;
			}
			else{				// 当斜率 > 1 时，选取左右象素点
				x += s1;
			}
			p -= 2 * dx;
		}
		if (!interchange){
			x += s1;	
		}			// 当斜率 < 1 时，选取 x 为步长
		else{
			y += s2;
		}			// 当斜率 > 1 时，选取 y 为步长
		p += 2 * dy;
	}
}

void drawcircle_fill(sWindow *win, int x, int y, int r, int color)
{
	int tx = 0, ty = r, d = 3 - 2 * r, i;

	while( tx < ty)
	{
		for (i = x - ty; i <= x + ty; i++){
			putpixel(win,i, y - tx, color);
			if (tx != 0)	
				putpixel(win,i, y + tx, color);
		}
		if (d < 0)			// 取上面的点
			d += 4 * tx + 6;
		else				// 取下面的点
		{
			for (i = x - tx; i <= x + tx; i++){
				putpixel(win,i, y - ty, color);
				putpixel(win,i, y + ty, color);
			}
			d += 4 * (tx - ty) + 10, ty--;
		}
		tx++;
	}
	if (tx == ty)			// 画水平两点连线(=45度)
		for (i = x - ty; i <= x + ty; i++){
			putpixel(win,i, y - tx, color);
			putpixel(win,i, y + tx, color);
		}
}

void drawcircle(sWindow *win, int x, int y, int r, int color)
{
	int tx = 0, ty = r, d = 3 - 2 * r;
	while( tx <= ty)
	{
		// 利用圆的八分对称性画点
		putpixel(win,x + tx, y + ty, color);
		putpixel(win,x + tx, y - ty, color);
		putpixel(win,x - tx, y + ty, color);
		putpixel(win,x - tx, y - ty, color);
		putpixel(win,x + ty, y + tx, color);
		putpixel(win,x + ty, y - tx, color);
		putpixel(win,x - ty, y + tx, color);
		putpixel(win,x - ty, y - tx, color);

		if (d < 0)		// 取上面的点
			d += 4 * tx + 6;
		else			// 取下面的点
			d += 4 * (tx - ty) + 10, ty--;
		tx++;
	}
}

void sleep(sWindow *win, int nsec)
{
	if(win->timer == 0){
		return ;
	}
	api_initTimer(win->timer, 128);
	api_setTimer(win->timer, nsec);
	int key=0;
	while(1){
		key = api_getchar(1);
		if (key!= -128) {	//char溢出的原因
			continue;
		}
		else{
			break;
		}
	}
}

void painter1()
{
	int temp = 1000;
	int h = 300*temp;				// 高度
	int v = 0;				// 速度（方向向下）
	int dv = 9800/50;		// 加速度（每 1/50 秒）

	sWindow win;
	win.buf = api_malloc(640*480);
	win.sht = api_initwindow(win.buf,640,480,-1,"test");
	win.width = 640;
	win.height = 480;
	win.timer = api_allocTimer();
	// 画地平线
	drawline(&win, 100, 421, 540, 421, 7);
	refresh_win(&win);
	char str[20];
	while(1)
	{
		if(api_getchar(0)=='c'){
			break;
		}
		v += dv;				// 根据加速度计算速度
		h -= (v - dv / 2);		// 计算高度

		// 如果高度低于地平线，实现反弹，速度方向取反
		if (h <= 0){
			h += (v - dv / 2);
			v = - v * 9 /10;		// 反弹时能量损耗 10%
		}
		drawcircle_fill(&win,320, 400 - h/temp, 20, 20);
		refresh_win(&win);
		sleep(&win, 5);
		drawcircle_fill(&win,320, 400 - h/temp, 20, 0);
		refresh_win(&win);
	}
	
	api_freeTimer(win.timer);
	api_free(win.buf);
	api_closewindow(win.sht);
}
