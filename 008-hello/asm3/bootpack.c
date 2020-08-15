#include"bootpack.h"

///keyboard define 
#define TAB 0x0f

static char keytable0[0x80] = {
	0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   0,		//16
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0,   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'','`',   0,'\\', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
};
static char keytable1[0x80] = {
	0,    0,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,   0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',   0,   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',  0,  '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
};

void show_string_xy(sSHEET *sht, char str[], int x, int y, uint color, uint bgcolor);
extern struct TASKCTL taskctl;;

sSHEET *sht_debug;
void HariMain(void)
{	
	//0x0ff0是asmhead中定义各个变量的起始位置
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	sFIFO32 fifo;
	uint fifobuf[128];
	
	
	//memory alloc
	uint memtotal;
	struct MEM_MAN *memManager;
	//0x003c0000
	memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	
	memtotal = memtest(0x00400000, 0xbfffffff);
	initMemManger(memManager);
	setFreeRange(memManager,0x00001000, 0x0009e000);
	setFreeRange(memManager,0x00400000, memtotal - 0x00400000);
	
	/////////////////
	init_gdtidt();
	init_pic();
	io_sti();	//中断初始化完成，开中断
	io_out8(PIC0_IMR, 0xf8); /* 开启键盘和鼠标中断主芯片设置:(11111000) */
	io_out8(PIC1_IMR, 0xef); /* 从芯片设置:(11101111) */

	///////// 
	init_PIT();
	sTASK *selftask=task_init(memManager);
	fifo32_init(&fifo, 128, fifobuf,selftask);
	
	///////// 设置图层
	init_palette();
	
	sSHTCTL *shtctl;	
	sSHEET *sht_back, *sht_mouse;
	uchar *mouse_buf, *backbuf, *winbuf;
	int mouseX,mouseY;
	mouseX=(binfo->scrnx-16)/2;
	mouseY=(binfo->scrny-28-16)/2;

	shtctl = shtctl_init(memManager, binfo->vram, binfo->scrnx,binfo->scrny);
	sht_back  = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_debug = sht_back;
	
	backbuf = (uchar *)mm_alloc(memManager, binfo->scrnx*binfo->scrny);	
	mouse_buf = (uchar *)mm_alloc(memManager, 256);
	
	sheet_init(sht_back, backbuf, binfo->scrnx, binfo->scrny, -1); 
	sheet_init(sht_mouse, mouse_buf, 16, 16, 99);	//99感觉是随意设置的		
	
	init_mouse_cursor8(mouse_buf, 99);//99为背景色号
	init_screen(backbuf,binfo->scrnx,binfo->scrny);
	
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_mouse, mouseX, mouseY);	

	//////////////// keyboard mouse
	sMOUSE_DEC mouseDec;
	init_keyboard(&fifo, 256);
	enable_mouse(&mouseDec, &fifo, 512);//激活鼠标
	
	
	/////// init timer
	sTIMER *timer3;
	timer3=timer_alloc(memManager);
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);
	
	int i=0;
	char str[30];
	////////// set task
	sTASK *task_console;
	sSHEET *sht_win, *sht_console;
	uint task_b_esp;
	
	sht_win = sheet_alloc(shtctl);
	winbuf = (uchar *)mm_alloc(memManager, 144 * 52);
	sheet_init(sht_win, winbuf, 144, 52, -1);
	make_window8(winbuf, 144, 52, "task_b",1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);

	
	sht_console = sheet_alloc(shtctl);
	winbuf = (uchar *)mm_alloc(memManager, 256 * 165);
	sheet_init(sht_console, winbuf, 256, 165, -1);
	make_window8(winbuf, 256, 165, "console",0);
	make_textbox8(sht_console, 8, 28, 240, 128, COL8_000000);
	
	task_console = task_alloc(memManager);
	task_b_esp = mm_alloc(memManager, 64 * 1024) + 64 * 1024;
	*(int *)(task_b_esp-4)=(int *)task_console;
	*(int *)(task_b_esp-8)=(int *)sht_console;
	task_console->tss.esp = task_b_esp - 12;
	task_console->tss.es = 1 * 8;	
	task_console->tss.cs = 2 * 8;
	task_console->tss.ss = 1 * 8;
	task_console->tss.ds = 1 * 8;
	task_console->tss.fs = 1 * 8;
	task_console->tss.gs = 1 * 8;
	task_console->tss.eip = (int) &console_task;
	task_run(task_console, 2, 2);


	sheet_slide(sht_win,  80, 160);
	sheet_slide(sht_console,  48, 100);
	
	sheet_updown(sht_back,     0);
	sheet_updown(sht_console,  1);
	sheet_updown(sht_win,      2);
	sheet_updown(sht_mouse,    3);
	
	
	///////  show information
	uint key;
	uint cursor_x=8;
	uint cursor_color = COL8_FFFFFF;
	int task_running=0;
	uchar key_shift=0;
	for(;;){				
		io_cli();
		if(fifo32_status(&fifo)==0){ //空的
			task_sleep(selftask);
			io_sti();
		}
		else{
			key=fifo32_pop(&fifo);
			io_sti();//继续接收中断
			if(key>=256&&key<=511){
				key-=256;
				
				sprintf(str,"key:%d,%x",key,key);
				show_string(str,22);
				
				if (key == 0x2a) {	/* 左shift ON */
					key_shift |= 1;	//1
				}
				else if (key == 0x36) {	/* 右shift ON */
					key_shift |= 2;	//2
				}
				else if (key == 0xaa) {	/* 左shift OFF */
					key_shift &= ~1;
				}
				else if (key == 0xb6) {	/* 右shift OFF */
					key_shift &= ~2;
				}
				
				if(key < 0x80){//键盘编码转换为字符编码
					if(key_shift!=0){
						str[0] = keytable1[key];
					}
					else{
						str[0] = keytable0[key];
					}
				}
				else{
					str[0]=0;
				}
				
				if(str[0]!=0){
					if(task_running==0){
						if(cursor_x<(sht_win->bxsize-16)){
							str[1]=0;
							show_string_xy(sht_win,str,cursor_x,28,COL8_FFFFFF,COL8_000000);
							cursor_x += 8;
						}
					}
					else if(task_running==1){
						fifo32_push(&(task_console->fifo),str[0]+256);
					}
				}
				else if(key == 0x0e){//backspace
					if(task_running==0&& cursor_x >8){
						show_string_xy(sht_win," ",cursor_x,28,COL8_FFFFFF,COL8_000000);
						cursor_x -= 8;
					}
					else if(task_running==1){
						show_string("backspace",21);
						fifo32_push(&(task_console->fifo),256+8);
					}
					
				}
				
				else if(key==TAB){
					if(task_running==0){
						task_running=1;
						make_wtitle8(sht_win->buf, sht_win->bxsize,"task_b",0);
						make_wtitle8(sht_console->buf, sht_console->bxsize, "console",1);
					}
					else{
						task_running=0;
						make_wtitle8(sht_win->buf, sht_win->bxsize,"task_b",1);
						make_wtitle8(sht_console->buf, sht_console->bxsize, "console",0);
					}
					sheet_refresh(sht_win, 0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_console, 0, 0, sht_console->bxsize,  21);
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
					
					if ((mouseDec.btn & 0x01) != 0) {
						str[1] = 'L';
						if(mouseX>=sht_win->vx0&&mouseX<=sht_win->vx0+sht_win->bxsize
							&&mouseY>=sht_win->vy0&&mouseY<=sht_win->vy0+sht_win->bysize){

							sheet_slide(sht_win, sht_win->vx0+mouseDec.x, sht_win->vy0+mouseDec.y);
						}
						
					}
					if ((mouseDec.btn & 0x02) != 0) {
						str[3] = 'R';
					}
					if ((mouseDec.btn & 0x04) != 0) {
						str[2] = 'C';
					}
				}
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

