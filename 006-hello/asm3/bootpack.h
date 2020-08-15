#include<stdio.h>
#include<string.h>
#define uchar unsigned char
#define uint unsigned int
//temp 
void show_string(char str[], int line);

//naskfunc
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
unsigned int load_cr0(void);
void store_cr0(unsigned int cr0);
unsigned int memtest_sub(unsigned int place);

void asm_inthandler21(void);
void asm_inthandler2c(void);
void asm_inthandler20(void);


//screen
void init_screen(uchar *vram,int xsize,int ysize);
void init_palette(void);
void set_palette(int start,int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char color,int x0,int y0,int x1, int y1);
void putfont8(uchar *vram,int xsize,int x,int y, char color, char *font);
void putfonts8_asc(uchar *vram,int xsize,int x,int y, char color, char *str);

void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int scrnx, int pxsize, 
	int pysize,int startx,int starty, char *block, int bxsize);

void make_window8(uchar *buf, int xsize, int ysize, char *title);


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


/// fifo
struct FIFO8{
	uchar *buf;
	int size,free;
	int wp,rp;
	int flags;
};
struct FIFO32{
	uint *buf;
	int size,free;
	int wp,rp;
	int flags;
};

#define sFIFO8 struct FIFO8
#define sFIFO32 struct FIFO32

void fifo8_init(sFIFO8 *fifo, int size, uchar *buf);
uint fifo8_push(sFIFO8 *fifo, uchar data);
uint fifo8_pop(sFIFO8 *fifo);
uint fifo8_status(sFIFO8 *fifo);

void fifo32_init(sFIFO32 *fifo, int size, uint *buf);
uint fifo32_push(sFIFO32 *fifo, uint data);
uint fifo32_pop(sFIFO32 *fifo);
uint fifo32_status(sFIFO32 *fifo);

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

#define sMOUSE_DEC struct MOUSE_DEC
struct MOUSE_DEC{
	uchar data[3],phase;
	int x,y,btn;
};

void init_keyboard(sFIFO32 *fifo, uint data0);
void inthandler21(int *esp);


void enable_mouse(sMOUSE_DEC *mouseDec, sFIFO32 *fifo, uint data0);
int mouse_decode(sMOUSE_DEC *mouseDec, uchar key);
void inthandler2c(int *esp);

////////////////// memory
unsigned int memtest(unsigned int start, unsigned int end);

#define sMEM_BLOCK struct MEM_BLOCK
struct MEM_BLOCK{
	unsigned int size;
	unsigned int flags;
	sMEM_BLOCK *lastBlock;
	sMEM_BLOCK *nextBlock;
};

//16B
struct MEM_MAN{
	unsigned int freesize;
	unsigned int lostsize;
	unsigned int losts;
	sMEM_BLOCK *freeblocks;
};
void initMemManger(struct MEM_MAN *man);
void setFreeRange(struct MEM_MAN *man, int addr, int size);
uint mm_alloc(struct MEM_MAN *man, uint bytes);
uchar mm_free(struct MEM_MAN *man, uint addr);
//debug
void freeBlockNum(struct MEM_MAN *man);



/// sheet manager
#define MAX_SHEETS 256
#define sSHEET struct SHEET
#define sSHTCTL struct SHTCTL
struct SHEET{
	uchar *buf;
	int bxsize,bysize;//buf size
	int vx0,vy0;//显示的位置
	/* 窗口中是否含透明色标识(-1则无) */
	int col_inv;//color invisible，用于指定buf中哪些为透明的
	int height;	//当前图层处理显示的层级
	int flags;	//used or not
	
	sSHTCTL *ctl;
};

struct SHTCTL{
	uchar *vram;//显存
	uchar *map;
	int vxsize,vysize;//显存大小
	int top;//最上层图层的高度
	//用于图层按高度排序
	sSHEET *sheets[MAX_SHEETS];
	//用于保存图层信息
	sSHEET sheets0[MAX_SHEETS]; 
};

sSHTCTL *shtctl_init(struct MEM_MAN *man, uchar *vram, int xsize, int ysize);
sSHEET *sheet_alloc(sSHTCTL *ctl);
void sheet_free(sSHEET *sht);

sSHTCTL *shtctl_init(struct MEM_MAN *man, uchar *vram, int xsize, int ysize);
void sheet_updown(sSHEET *sht, int height);
void sheet_slide(sSHEET *sht, int vx0, int vy0);
void sheet_refreshsub(sSHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_refresh(sSHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_refreshmap(sSHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);
void show_string1(sSHEET *sht, char str[], int line, uint color, uint bgcolor);


/// timer

extern struct TIMERCTL timerctl;

#define sTIMER struct TIMER
struct TIMER {
	uint timeout, flags;
	struct FIFO32 *fifo;
	uint data;
	sTIMER *last;
	sTIMER *next;
};
struct TIMERCTL {
	uint count, next, using;
	sTIMER *timer;
	sTIMER *idleTimer;
};

void init_PIT(void);
void inthandler20(int *esp);

int timer_number();
void timer_free(struct MEM_MAN *man, sTIMER *timer);
sTIMER *timer_alloc(struct MEM_MAN *man);
void timer_init(sTIMER *timer, sFIFO32 *fifo, uint data);
void timer_settime(sTIMER *timer, uint timeout);