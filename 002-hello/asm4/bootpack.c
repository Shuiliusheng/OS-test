void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);


void init_palette(void);
void set_palette(int start,int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char color,int x0,int y0,int x1, int y1);
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

#define uchar unsigned char
void HariMain(void)
{
	init_palette();
	
	uchar *vram;
	int xsize,ysize;
	vram=(uchar *)0xa0000;
	xsize=320;
	ysize=200;
	
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
	
	for(;;){
		io_hlt();
	}
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
