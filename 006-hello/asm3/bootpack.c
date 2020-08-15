#include"bootpack.h"


#define MEMMAN_ADDR			0x003c0000

///keyboard define 
#define BACKSPACE 0x0e


void set490(struct MEM_MAN *man, sFIFO32 *fifo, int mode);
void show_string_xy(sSHEET *sht, char str[], int x, int y, uint color, uint bgcolor);

static char keytable[0x54] = {
	0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.'
};

sSHEET *sht_debug;
void HariMain(void)
{	
	//0x0ff0是asmhead中定义各个变量的起始位置
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	sFIFO32 fifo;
	uint fifobuf[128];
	fifo32_init(&fifo, 128, fifobuf);
	
	init_gdtidt();
	init_pic();
	io_sti();	//中断初始化完成，开中断
	io_out8(PIC0_IMR, 0xf8); /* 开启键盘和鼠标中断主芯片设置:(11111000) */
	io_out8(PIC1_IMR, 0xef); /* 从芯片设置:(11101111) */
	
	//memory alloc
	uint memtotal;
	struct MEM_MAN *memManager;
	//0x003c0000
	memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	
	memtotal = memtest(0x00400000, 0xbfffffff);
	initMemManger(memManager);
	setFreeRange(memManager,0x00001000, 0x0009e000);
	setFreeRange(memManager,0x00400000, memtotal - 0x00400000);
	
	/////// init timer
	sTIMER *timer1,*timer2,*timer3;
	
	init_PIT();
	set490(memManager, &fifo, 1);
	
	timer1=timer_alloc(memManager);
	timer2=timer_alloc(memManager);
	timer3=timer_alloc(memManager);
	
	timer_init(timer1, &fifo, 10);
	timer_init(timer2, &fifo, 3);
	timer_init(timer3, &fifo, 1);
	
	timer_settime(timer1, 1000);
	timer_settime(timer2, 300);
	timer_settime(timer3, 50);
	

	//////////////// keyboard mouse
	sMOUSE_DEC mouseDec;
	
	init_keyboard(&fifo, 256);
	enable_mouse(&mouseDec, &fifo, 512);//激活鼠标

	
	///////// 设置图层
	init_palette();
	
	sSHTCTL *shtctl;	
	sSHEET *sht_back, *sht_mouse, *sht_win;
	uchar *mouse_buf, *backbuf, *winbuf;
	int mouseX,mouseY;

	shtctl = shtctl_init(memManager, binfo->vram, binfo->scrnx,binfo->scrny);
	sht_back  = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	sht_debug = sht_back;
	
	backbuf = (uchar *)mm_alloc(memManager, binfo->scrnx*binfo->scrny);	
	mouse_buf = (uchar *)mm_alloc(memManager, 256);
	winbuf = (uchar *)mm_alloc(memManager, 160 * 68);
	
	sheet_init(sht_back, backbuf, binfo->scrnx, binfo->scrny, -1); 
	sheet_init(sht_mouse, mouse_buf, 16, 16, 99);	//99感觉是随意设置的	
	sheet_init(sht_win, winbuf, 160, 68, -1);	//99感觉是随意设置的	
	
	
	init_mouse_cursor8(mouse_buf, 99);//99为背景色号
	init_screen(backbuf,binfo->scrnx,binfo->scrny);
	make_window8(winbuf, 160, 68, "counter");	
	mouseX=(binfo->scrnx-16)/2;
	mouseY=(binfo->scrny-28-16)/2;
	
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_mouse, mouseX, mouseY);
	sheet_slide(sht_win, 80, 72);
	
	sheet_updown(sht_back,  0);
	sheet_updown(sht_mouse, 1);
	sheet_updown(sht_win, 2	);
	
	///////  show information
	uint key;
	char str[30];
	uint cursor_x=8;
	uint cursor_color = COL8_FFFFFF;
	for(;;){		
		io_cli();
		if(fifo32_status(&fifo)==0){ //空的
			io_sti();
		}
		else{
			key=fifo32_pop(&fifo);
			io_sti();//继续接收中断
				
			if(key>=256&&key<=511){
				key-=256;
				if(key < 0x54){
					if(keytable[key]!=0 && cursor_x<(sht_win->bxsize-16)){
						str[0]=keytable[key];
						str[1]=0;
						show_string_xy(sht_win,str,cursor_x,28,COL8_FFFFFF,COL8_000000);
						cursor_x += 8;
					}
				}
				if(key == BACKSPACE && cursor_x >8){
					show_string_xy(sht_win," ",cursor_x,28,COL8_FFFFFF,COL8_000000);
					cursor_x -= 8;
				}
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_color, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
			else if (key>=512&&key<=767){
				key-=512;		
				if(mouse_decode(&mouseDec,key)==1){
					
					/////// 显示鼠标					
					mouseX+=mouseDec.x;
					mouseY+=mouseDec.y;
					if(mouseX<0){
						mouseX=0;
					}
					else if(mouseX>=binfo->scrnx-1){
						mouseX=binfo->scrnx-1;
					}
					if(mouseY<0){
						mouseY=0;
					}
					else if(mouseY>=binfo->scrny-1){
						mouseY=binfo->scrny-1;
					}

					sheet_slide(sht_mouse, mouseX, mouseY);
					
					sprintf(str, "[lcr %3d %3d]", mouseX, mouseY);
					if ((mouseDec.btn & 0x01) != 0) {
						str[1] = 'L';
					}
					if ((mouseDec.btn & 0x02) != 0) {
						str[3] = 'R';
					}
					if ((mouseDec.btn & 0x04) != 0) {
						str[2] = 'C';
					}

					show_string(str,2);
				}
			}
			else if (key==10) {
				show_string("10[sec]",3);
				
				//sprintf(str,"%010d",count);
				//show_string1(sht_win,str, 3, COL8_000000, COL8_C6C6C6);
			} 
			else if (key==3) {
				show_string("3[sec]",3);
			} 
			else if (key==0||key==1) {
				if (key != 0) {
					timer_init(timer3, &fifo, 0); 
					cursor_color = COL8_000000;
				} else {
					timer_init(timer3, &fifo, 1); 
					cursor_color = COL8_FFFFFF;
				}
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_color, cursor_x, 28, cursor_x + 7, 43);
				timer_settime(timer3, 60);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}
}

void show_string(char str[], int line)
{
	uchar *buf=sht_debug->buf;
	int bxsize=sht_debug->bxsize;
	int t1=(line-1)*16, t2=8*strlen(str), t3=line*16;
	boxfill8(buf, bxsize, COL8_008484, 0, t1, t2, t3);
	putfonts8_asc(buf, bxsize, 0, t1, COL8_FFFFFF, str);	
	sheet_refresh(sht_debug,0, t1, t2, t3);
}

void show_string1(sSHEET *sht, char str[], int line, uint color, uint bgcolor)
{
	uchar *buf=sht->buf;
	int bxsize=sht->bxsize;
	int startx=10;
	int t1=(line-1)*16, t2=startx+8*strlen(str), t3=line*16;
	boxfill8(buf, bxsize, bgcolor, startx, t1, t2, t3);
	putfonts8_asc(buf, bxsize, startx, t1, color, str);	
	sheet_refresh(sht,startx, t1, t2, t3);
}

void show_string_xy(sSHEET *sht, char str[], int x, int y, uint color, uint bgcolor)
{
	int t2=x+8*strlen(str)-1;
	boxfill8(sht->buf, sht->bxsize, bgcolor, x, y, t2, y+15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, color, str);	
	sheet_refresh(sht,x, y, t2+1, y+16);
}

void set490(struct MEM_MAN *man, sFIFO32 *fifo, int mode)
{
	int i;
	struct TIMER *timer;
	if (mode != 0) {
		for (i = 0; i < 490; i++) {
			timer = timer_alloc(man);
			timer_init(timer, fifo, 1024 + i);
			timer_settime(timer, 100 * 60 * 60 * 24 * 50 + i * 100);
		}
	}
	return;
}
