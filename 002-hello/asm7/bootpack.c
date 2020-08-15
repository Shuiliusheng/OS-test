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
	

void HariMain(void)
{
	//已经编入img中，直接使用
	//256个字符，256*16
	extern char hankaku[4096];
	
	//0x0ff0是asmhead中定义各个变量的起始位置
	struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
	
	init_palette();
	init_screen(binfo->vram,binfo->scrnx,binfo->scrny);
	
	char str[30];
	sprintf(str,"vram:0x%p",binfo->vram);
	putfonts8_asc(binfo->vram,binfo->scrnx,24,24,COL8_FFFFFF,str);


	char mcursor[256];//16*16;
	int mouseX,mouseY;
	mouseX=(binfo->scrnx-16)/2;
	mouseY=(binfo->scrny-28-16)/2;
	init_mouse_cursor8(mcursor,COL8_840084);
	putblock8_8(binfo->vram,binfo->scrnx,16,16,mouseX,mouseY,mcursor,16);

	

	for(;;){
		io_hlt();
	}
}

void init_screen(uchar *vram,int xsize,int ysize)
{
	//simple draw
	boxfill8(vram, xsize, COL8_840084,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);
	
}

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:黒 */
		0xff, 0x00, 0x00,	/*  1:明るい赤 */
		0x00, 0xff, 0x00,	/*  2:明るい緑 */
		0xff, 0xff, 0x00,	/*  3:明るい黄色 */
		0x00, 0x00, 0xff,	/*  4:明るい青 */
		0xff, 0x00, 0xff,	/*  5:明るい紫 */
		0x00, 0xff, 0xff,	/*  6:明るい水色 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:明るい灰色 */
		0x84, 0x00, 0x00,	/*  9:暗い赤 */
		0x00, 0x84, 0x00,	/* 10:暗い緑 */
		0x84, 0x84, 0x00,	/* 11:暗い黄色 */
		0x00, 0x00, 0x84,	/* 12:暗い青 */
		0x84, 0x00, 0x84,	/* 13:暗い紫 */
		0x00, 0x84, 0x84,	/* 14:暗い水色 */
		0x84, 0x84, 0x84	/* 15:暗い灰色 */
	};
	set_palette(0, 15, table_rgb);
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i,eflags;
	eflags=io_load_eflags();
	
	io_cli();
	io_out8(0x03c8,start);
	for(i=start;i<=end;i++){
		//是为了调整颜色，
		//原来设置的颜色偏暗，但是rgb[0]如果不除以4，也一样能用
		io_out8(0x03c9,rgb[0]/4);
		io_out8(0x03c9,rgb[1]/4);
		io_out8(0x03c9,rgb[2]/4);
		rgb+=3;
	}
	
	io_store_eflags(eflags);
	return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char color,int x0,int y0,int x1, int y1)
{
	int x,y;
	for(y=y0;y<=y1;y++){
		for(x=x0;x<=x1;x++){
			vram[y*xsize+x]=color;
		}
	}
	return;
}


void putfont8(uchar *vram,int xsize,int x,int y, char color, char *font)
{
	int i;
	uchar *p,line;//8*16的字体
	for(i=0;i<16;i++)
	{
		p=vram+(y+i)*xsize+x;
		line=font[i];
		//按行设点，每行8个点，共16行
		if((line&0x80)!=0)	{ p[0]=color;}//左上角第一个点
		if((line&0x40)!=0)	{ p[1]=color;}
		if((line&0x20)!=0)	{ p[2]=color;}
		if((line&0x10)!=0)	{ p[3]=color;}
		if((line&0x08)!=0)	{ p[4]=color;}
		if((line&0x04)!=0)	{ p[5]=color;}
		if((line&0x02)!=0)	{ p[6]=color;}
		if((line&0x01)!=0)	{ p[7]=color;}
	}
}

void putfonts8_asc(uchar *vram,int xsize,int x,int y, char color, char *str)
{
	extern char hankaku[4096];
	int i;
	for(i=0;str[i]!='\0';i++){
		putfont8(vram,xsize,x,y,color,hankaku+str[i]*16);
		x+=8;
		
		if(x>=xsize){
			x=0;
			y+=16;
		}
	}
	return;
}


//bc:background color
void init_mouse_cursor8(char *mouse, char bc)
{
	//16*16
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

void putblock8_8(char *vram, int scrnx, int pxsize, 
	int pysize,int startx,int starty, char *block, int bxsize)
{
	int x,y;
	for(y=0;y<pysize;y++){
		for(x=0;x<pxsize;x++){
			vram[(starty+y)*scrnx+startx+x]=block[y*bxsize+x];
		}
	}
	return ;
}

void init_gdtidt(void)
{
	//BOTPAK  EQU 0x00280000
	//0x270000 - 0x270000+8192没有被使用，因此用来作为GDT
	struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *) 0x00270000;
	//0x26f800-0x26ffff也是
	struct GATE_DESCRIPTOR *idt=(struct GATE_DESCRIPTOR *) 0x0026f800;
	int i;
	
	//GDT init
	for(i=0;i<8192;i++){
		set_segmdesc(gdt+i,0,0,0);
	}
	//段号1：0-4GB，用于管理全部内存
	set_segmdesc(gdt+1,0xffffffff,0x00000000,0x4092);
	//段号2：0x280000,512KB, bootpack的区域
	set_segmdesc(gdt+2,0x0007ffff,0x00280000,0x409a);
	//给GDTR赋值，需要使用汇编
	load_gdtr(0xffff, 0x00270000);
	
	//IDT init
	for(i=0;i<256;i++){
		set_gatedesc(idt+i,0,0,0);
	}
	//0x7ff:2048
	load_idtr(0x7ff,0x0026f800);
	
	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	// > 1GB
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;	//4KB
	}
	
	sd->base_low     = base & 0xffff;	//基址低16位
	sd->base_mid     = (base >> 16) & 0xff;//基址17-24位
	sd->base_high    = (base >> 24) & 0xff;//基址25-32位

	sd->limit_low    = limit & 0xffff;	//大小低16位
	//limit的高16位中的低8位
	//((ar >> 8) & 0xf0): ar的9-16位，并且9-12设置为0
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	
	sd->access_right = ar & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	
	return;
}

