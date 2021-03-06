#include<stdio.h>
#define uchar unsigned char

//naskfunc
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void asm_inthandler21(void);
void asm_inthandler2c(void);

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

/* int.c */
#define ADR_BOOTINFO	0x00000ff0

#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff	//2K

#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff	//64KB

#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff	//512MB

#define AR_DATA32_RW	0x4092	
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

void init_pic(void);
void inthandler21(int *esp);
void inthandler2c(int *esp);


/// fifo
struct FIFO8{
	uchar *buf;
	int size,free;
	int wp,rp;
	int flags;
};
#define sFIFO8 struct FIFO8
void fifo8_init(sFIFO8 *fifo, int size, uchar *buf);
int fifo8_push(sFIFO8 *fifo, uchar data);
int fifo8_pop(sFIFO8 *fifo);
int fifo8_status(sFIFO8 *fifo);


///////////////////// keyboard mouse
// 规定,data,status,cmd

/* 8042 端口地址,
 * in  al, 60h, 读8042输出缓冲寄存器内容到al;
 * in  al, 64h, 读8042状态寄存器内容到al;
 * out 60h, al, 写数据al到8042输入缓冲寄存器;
 * out 64h, al, 写命令al到8042输入缓冲寄存器. */
#define PORT_KEYDAT				0x0060
#define PORT_KEYCMD				0x0064

//键盘控制器8042状态寄存器端口
#define PORT_KEYSTA				0x0064

//8042状态控制器输入缓冲器满的标志
#define KEYSTA_SEND_NOTREADY 	0x02
//写入键盘控制命令字节的命令,具体的命令字节随后从60h端口写入
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

//写鼠标命令,具体的鼠标命令随后由60h端口下发;
#define KEYCMD_SENDTO_MOUSE 	0xd4
//开启鼠标发送数据功能的命令
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(void);
void init_keyboard(void);



