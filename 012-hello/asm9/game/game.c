#include "apilib.h"
#include <string.h>

#define MOUSE_EVENT_RIGHTDOWN 3
#define MOUSE_EVENT_RIGHTUP 4
#define MOUSE_EVENT_MOVE 5
struct MOUSE_EVENT{
	short x, y;
	int type;
};

struct WINDOW{
	char *buf;
	int sht,timer;
	int width, height;
	int fsx,fsy,fex,fey;
	char mousedown;
	short mx, my;
	short fx, fy;
	unsigned max_v;
};
#define sWindow struct WINDOW

void refresh_win(sWindow *win);
void listen_event(struct MOUSE_EVENT event);
void put_pixel(sWindow *win, int x, int y, int color);
void drawline(sWindow *win, int x1, int y1, int x2, int y2, int color);
void drawcircle_fill(sWindow *win, int x, int y, int r, int color);
void drawcircle(sWindow *win, int x, int y, int r, int color);
void sleep(sWindow *win, int nsec);
int dis(int x, int y, int x1, int y1);
int sqrt(int x);
int rand(void);


struct SWORD{
	int x, y;
	int vx, vy, dis_v;
	int ex, ey;
	int sx, sy;
	int width;
	int color;
	
	int sx1,sy1,ex1,ey1;
}sword;

struct CIRCLE{
	int x, y;
	int r;
	int color;
	int hit;
}circle;

sWindow win;

void cal_nextline()
{
	int dx = sword.ex-sword.sx;
	int dy = sword.ey-sword.sy;
	int len = sword.width;
	dx = dx/2;
	dy = dy/2;
	
	sword.sx1 = sword.x+dy;
	sword.sy1 = sword.y-dx;
	sword.ex1 = sword.x-dy;
	sword.ey1 = sword.y+dx;
}	


void drawsword()
{
	if(win.mx==0){
		drawline(&win, sword.sx, sword.sy, sword.ex, sword.ey, 0);
		drawline(&win, sword.sx1, sword.sy1, sword.ex1, sword.ey1, 0);
		sword.sx = sword.x;
		sword.sy = sword.y;
		sword.ex = sword.x + sword.width;
		sword.ey = sword.y;
		drawline(&win, sword.x, sword.y,sword.x+sword.width, sword.y, sword.color);
		cal_nextline();
		drawline(&win, sword.sx1, sword.sy1, sword.ex1, sword.ey1, 30);
	}
	else if(win.mousedown==1){
		drawline(&win, sword.sx, sword.sy,sword.ex, sword.ey, 0);
		drawline(&win, sword.sx1, sword.sy1, sword.ex1, sword.ey1, 0);
		int dx = win.mx - sword.x;
		int dy = win.my - sword.y;
		int len = dis(win.mx, win.my, sword.x, sword.y);
		
		int vlen=0;
		if(win.fx>win.mx&&win.fy<win.my){
			vlen = dis(win.fx, win.fy, win.mx, win.my);
			vlen = 1000*vlen/win.max_v;
			vlen = vlen*30/1000;
		}
		
		sword.vx = 1000*vlen*dx/len;
		sword.vy = 1000*vlen*dy/len;
		sword.dis_v = dis(sword.vx/10, sword.vy/10, 0, 0);
		sword.sx = sword.x - vlen*dx/len;
		sword.sy = sword.y - vlen*dy/len;
		sword.ex = sword.x + (sword.width- vlen)*dx/len;
		sword.ey = sword.y + (sword.width- vlen)*dy/len;
		drawline(&win, sword.sx, sword.sy,sword.ex, sword.ey, sword.color);
		cal_nextline();
		drawline(&win, sword.sx1, sword.sy1, sword.ex1, sword.ey1, 30);
	}
	else if(win.mousedown==0&&sword.sy<win.height&&sword.sx<win.width&&sword.vx!=0){
		//drawline(&win, sword.sx, sword.sy,sword.ex, sword.ey, 0);
		int step = 3;
		sword.sx += step;
		int t = abs(step*1000*1000/sword.vx);
		sword.vy += 300*t/1000;
		sword.sy += (sword.vy*t/1000)/1000;
		putpixel(&win, sword.sx, sword.sy, sword.color);
	}
}

void painter()
{
	drawcircle_fill(&win,circle.x, circle.y, circle.r, circle.color);
	while(1){
		drawsword();
		refresh_win(&win);
		
		if(api_getchar(0)!=-1){
			break;
		}
		if(circle.hit == 1){
			circle.x = win.width/3+(rand()%(win.width*2/3-30));
			circle.y = 40 + (rand()%(win.height-80));
			circle.r = 5 + (rand()%5);
			circle.hit = 0;
			drawcircle_fill(&win,circle.x, circle.y, circle.r, circle.color);
		}
	}
}


void init()
{
	win.buf = api_malloc(640*480);
	win.sht = api_initwindow(win.buf,640,480,-1,"test");
	win.width = 640;
	win.height = 480;
	win.mousedown = 0;
	win.mx = 0;
	win.my = 0;
	win.timer = api_allocTimer();
	api_initTimer(win.timer, 128);
	
	sword.x = 70;
	sword.y = 400;
	sword.ex = 70;
	sword.ey = 400;
	sword.width = 50;
	sword.color = 6;
	
	circle.x = 600;
	circle.y = 300;
	circle.r = 15;
	circle.color = 4;
	circle.hit = 0;
	
	api_draw_block(win.sht, 10, 20, win.width-10*2, win.height-30, 0);
	painter();
	
	api_freeTimer(win.timer);
	api_free(win.buf);
	api_closewindow(win.sht);
}



void HariMain(void)
{	
	api_initmalloc();
	char *stack = api_malloc(1024*5);
	int te = api_thread_listen((int)&listen_event, stack+1024*5);
	init();
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
	int t=y*win->width+x;
	if(win->buf[t] == circle.color && color == sword.color){
		circle.hit=1;
	}
	win->buf[t]=color;
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

int sqrt(int x)
{
	long i = 0;
    long j = x / 2 + 1;
    while (i <= j)
    {
        long mid = (i + j) / 2;
        long sq = mid * mid;
        if (sq == x) return mid;
        else if (sq < x) i = mid + 1;
        else j = mid - 1;
    }
    return j;
}

int dis(int x, int y, int x1, int y1)
{
	return sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
}

void listen_event(struct MOUSE_EVENT event)
{
	char str[10];
	if(event.type == MOUSE_EVENT_RIGHTDOWN){
		if(event.x>=0&&event.y>=0&&event.x<win.width&&event.y<win.height){
			api_draw_block(win.sht, 10, 20, win.width-10*2, win.height-30, 0);
			drawcircle_fill(&win,circle.x, circle.y, circle.r, circle.color);
			win.mousedown=1;
			win.mx = event.x;
			win.my = event.y;
			win.fx = event.x;
			win.fy = event.y;
			win.max_v = dis(win.fx,win.fy, 0, 0);
		}
	}
	else if(event.type == MOUSE_EVENT_RIGHTUP){
		win.mousedown=0;
		win.mx = event.x;
		win.my = event.y;
	}
	else if(event.type == MOUSE_EVENT_MOVE){
		if(win.mousedown==1&&event.x>=0&&event.y>=0&&event.x<win.width&&event.y<win.height){
			win.mx = event.x;
			win.my = event.y;
		}
	}
	api_return();
}

