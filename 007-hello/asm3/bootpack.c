#include"bootpack.h"


#define MEMMAN_ADDR			0x003c0000

///keyboard define 
#define BACKSPACE 0x0e

//104B, task status segment, 需要在GDT中定义后使用
struct TSS32 {	
	//与任务设置相关的信息，不会被写入寄存器，除了backline
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	//32位寄存器，eip：extended instruction pointer
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	//16位寄存器
	int es, cs, ss, ds, fs, gs;
	//有关任务设置的部分
	int ldtr, iomap;
};
#define sTSS32 struct TSS32


void show_string_xy(sSHEET *sht, char str[], int x, int y, uint color, uint bgcolor);

void task_b_main(sSHEET *sht_back)
{
	struct MEM_MAN *memManager;
	sFIFO32 fifo;
	uint fifobuf[128];
	memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	fifo32_init(&fifo, 128, fifobuf);
	
	sTIMER *timer_ts, *timer3, *timer10;
	timer_ts=timer_alloc(memManager);
	timer_init(timer_ts, &fifo, 2);
	timer_settime(timer_ts, 2);
	
	timer3=timer_alloc(memManager);
	timer_init(timer3, &fifo, 3);
	timer_settime(timer3, 300);
	timer10=timer_alloc(memManager);
	timer_init(timer10, &fifo, 10);
	timer_settime(timer10, 1000);
	
	int key;
	int count=0;
	int start=0;
	char str[20];
	for(;;){
		count++;
		io_cli();
		if(fifo32_status(&fifo)==0){
			io_sti();
		}
		else{
			key=fifo32_pop(&fifo);
			if(key==2){
				//先跳过去执行，等执行的那边设置时间后再调回来

				farjmp(0,3*8);
				timer_settime(timer_ts,2);
			}
			else if(key==3){
				count=0;
				show_string_xy(sht_back,"3sec",0,48,COL8_FFFFFF,COL8_000000);
			}
			else if(key==10){
				sprintf(str,"taskB: %10d",count);
				show_string_xy(sht_back,str,0,64,COL8_FFFFFF,COL8_000000);
				count=0;
			}
		}
	}
}


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
	sTIMER *timer1,*timer2,*timer3,*timer_ts;
	
	init_PIT();	
	timer1=timer_alloc(memManager);
	timer2=timer_alloc(memManager);
	timer3=timer_alloc(memManager);
	
	timer_init(timer1, &fifo, 10);
	timer_init(timer2, &fifo, 3);
	timer_init(timer3, &fifo, 1);
	
	timer_settime(timer1, 1000);
	timer_settime(timer2, 300);
	timer_settime(timer3, 50);
	
	timer_ts=timer_alloc(memManager);
	timer_init(timer_ts, &fifo, 2);
	timer_settime(timer_ts, 2);

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
	sheet_slide(sht_win, 100, 100);
	
	sheet_updown(sht_back,  0);
	sheet_updown(sht_mouse, 2);
	sheet_updown(sht_win, 1	);
	
	////////// set task
	sTSS32 tss_a, tss_b;
	uint task_b_esp;//b的栈指针
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	//注册到GDT中
	set_segmdesc(gdt+3, 103, (int)&tss_a,AR_TSS32); 
	set_segmdesc(gdt+4, 103, (int)&tss_b,AR_TSS32);
	//设置参数
	tss_a.ldtr = 0;
	tss_a.iomap = 0x40000000;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;
	//栈指针在栈底，栈是向下长的
	task_b_esp = mm_alloc(memManager, 64 * 1024) + 64 * 1024;
	tss_b.eip = (int) &task_b_main;
	tss_b.eflags = 0x00000202; /* IF = 1; sti之后的eflags就是这个*/
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	//参数压栈
	*(int *)(task_b_esp-4)=(int *)sht_back;
	//栈地址-8，保证[esp+4]能够获取参数
	tss_b.esp = task_b_esp -8;
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 1 * 8;	//第一段是整个地址空间
	tss_b.cs = 2 * 8;	//代码段使用gdt的第二段，即bootpack段
	tss_b.ss = 1 * 8;
	tss_b.ds = 1 * 8;
	tss_b.fs = 1 * 8;
	tss_b.gs = 1 * 8;
	
	//设置TR寄存器，task register，写入当前运行任务的GDT号
	//当任务切换时，tr会自动设置
	load_tr(3*8);	
	
	
	///////  show information
	uint key;
	char str[30];
	uint cursor_x=8;
	uint cursor_color = COL8_FFFFFF;
	int count=0;
	int start=0;
	for(;;){		
		count++;
		
		io_cli();
		if(fifo32_status(&fifo)==0){ //空的
			io_sti();
		}
		else{
			key=fifo32_pop(&fifo);
			io_sti();//继续接收中断
			if(key==2){
				farjmp(0,4*8);
				timer_settime(timer_ts,2);
			}	
			else if(key>=256&&key<=511){
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
			else if (key==10) {
				show_string("10[sec]",7);
				
				sprintf(str,"%010d",count);
				show_string(str,8);
			} 
			else if (key==3) {
				show_string("3[sec]",7);
				count=0;//167863037
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

