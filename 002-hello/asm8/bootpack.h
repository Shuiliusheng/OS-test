#include<stdio.h>
#define uchar unsigned char

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);


void init_screen(uchar *vram,int xsize,int ysize);
void init_palette(void);
void set_palette(int start,int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char color,int x0,int y0,int x1, int y1);
void putfont8(uchar *vram,int xsize,int x,int y, char color, char *font);
void putfonts8_asc(uchar *vram,int xsize,int x,int y, char color, char *str);

void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int scrnx, int pxsize, 
	int pysize,int startx,int starty, char *block, int bxsize);

//定义色号
#define COL8_000000		0	//黑
#define COL8_FF0000		1	//亮红
#define COL8_00FF00		2	//亮绿
#define COL8_FFFF00		3	//亮黄
#define COL8_0000FF		4	//亮蓝
#define COL8_FF00FF		5	//亮紫
#define COL8_00FFFF		6	//浅亮蓝
#define COL8_FFFFFF		7	//白
#define COL8_C6C6C6		8	//亮灰
#define COL8_840000		9	//暗红
#define COL8_008400		10	//暗绿
#define COL8_848400		11	//暗黄
#define COL8_000084		12	//暗蓝
#define COL8_840084		13	//暗紫
#define COL8_008484		14	//浅暗蓝
#define COL8_848484		15	//暗灰



struct BOOTINFO{
	char cyls,leds,vmode,reserve;
	short scrnx,scrny;
	uchar *vram;
};

//GDT 
struct SEGMENT_DESCRIPTOR{
	short limit_low,base_low;
	char base_mid,access_right;
	char limit_high,base_high;
};

//IDT
struct GATE_DESCRIPTOR{
	short offset_low,selector;
	char dw_count,access_right;
	short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);