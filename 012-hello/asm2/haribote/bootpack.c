#include"bootpack.h"

///keyboard define 
#define TAB 0x0f
//控制led灯的前两位
#define KEYCMD_LED		0xed

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

int find_sheet_xy(sSHTCTL *ctl, int x, int y)
{
	sSHEET *sht;
	int h=0;
	
	if(x<0||x>ctl->vxsize||y<0||y>ctl->vysize){
		return 0;
	}
	
	for(h=ctl->top-1;h>=0;h--){
		sht = ctl->sheets[h];
		if(x>=sht->vx0&&y>=sht->vy0
			&&x<=sht->vx0+sht->bxsize&&y<=sht->vy0+sht->bysize)
		{
			return h;
		}
	}
	return 0;
}

sTASK *open_task(sSHTCTL *shtctl, sSHEET *sht, sFIFO32 *fifo, uint memtotal)
{
	sTASK *task;
	uint task_b_esp;
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	
	task = task_alloc(memManager);
	task->stack_addr = mm_alloc(memManager, 64 * 1024);
	task_b_esp = task->stack_addr + 64 * 1024;
	*(int *)(task_b_esp-4)=memtotal;
	*(int *)(task_b_esp-8)=(int)task;
	*(int *)(task_b_esp-12)=(int)sht;
	task->tss.esp = task_b_esp - 16;
	task->tss.es = 1 * 8;	
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	task->tss.eip = (int) &console_task;
	
	task->fifo = (sFIFO32 *)mm_alloc(memManager, sizeof(sFIFO32));
	task->os_fifo = fifo;
	task->shtctl = shtctl;
	uint *fifobuf = (uint *)mm_alloc(memManager, 128*4);
	fifo32_init(task->fifo, 128, fifobuf, task);
	
	sht->task = task;
	task_run(task, 2, 2);
	return task;
}

sSHEET *open_console(sSHTCTL *shtctl, sFIFO32 *fifo, uint memtotal, int w, int h)
{
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;	
	sSHEET *sht_console = sheet_alloc(shtctl);
	uchar *winbuf = (uchar *)mm_alloc(memManager, w * h);
	
	sheet_init(sht_console, winbuf, w, h, -1);
	make_window8(winbuf, w, h, "console",0);//256, 465,
	make_textbox8(sht_console, 8, 28, w-16, 128, COL8_000000);
	
	open_task(shtctl, sht_console, fifo, memtotal);
	sht_console->flags |= 0x20;	//有光标
	return sht_console;
}

void close_task(sTASK *task)
{
	struct MEMMAN *man = (struct MEMMAN *) MEMMAN_ADDR;
	task_sleep(task);
	mm_free(man, (char *)(task->stack_addr));
	mm_free(man, (char *)(task->fifo->buf));
	mm_free(man, (char *)(task->fifo));
	task_delete(man, task);
	return ;
}

void close_console(sSHTCTL *shtctl, int sheetNum)
{
	//关闭sheet
	sSHEET *sht = &(shtctl->sheets0[sheetNum]);
	sTASK *task = sht->task;
	struct MEMMAN *man = (struct MEMMAN *) MEMMAN_ADDR;
	
	close_task(task);
	mm_free(man, (uint)sht->buf);
	sheet_free(sht);
	return ;
}

void keywin_off(sSHEET *key_win)
{
	if(key_win==0||key_win->flags==0){
		return;
	}
	fifo32_push((key_win->task->fifo),3);
	make_wtitle8(key_win->buf, key_win->bxsize, "console",0);
	sheet_refresh(key_win, 0, 0, key_win->bxsize,  21);
	return;
}

void keywin_on(sSHEET *key_win)
{
	if(key_win==0||key_win->flags==0){
		return;
	}
	fifo32_push((key_win->task->fifo),4);
	make_wtitle8(key_win->buf, key_win->bxsize, "console",1);
	sheet_refresh(key_win, 0, 0, key_win->bxsize,  21);
	return;
}


void HariMain(void)
{	
	//0x0ff0是asmhead中定义各个变量的起始位置
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	sFIFO32 fifo, keycmd;
	uint fifobuf[128], keycmdbuf[128];
	
	
	//memory alloc
	uint memtotal;
	struct MEM_MAN *memManager;
	//0x003c0000
	memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	
	memtotal = memtest(0x00400000, 0xbfffffff);
	initMemManger(memManager);
	setFreeRange(memManager,0x00001000, 0x0009e000);
	setFreeRange(memManager,0x00400000, memtotal - 0x00400000);
	
	//// file system
	init_diskinfo();
	init_filesystem_disk();
	read_fileinfo();
	
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
	fifo32_init(&keycmd, 128, keycmdbuf,0);
	
	///////// 设置图层
	init_palette();
	
	sSHTCTL *shtctl;	
	sSHEET *sht_back, *sht_mouse;
	uchar *mouse_buf, *backbuf;
	int mouseX,mouseY;
	int lastX=-1,lastY=-1;
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
	
	int i=0;
	char str[30];
	
	////////// set task
	sSHEET *key_win;
	key_win = open_console(shtctl, &fifo, memtotal, 400,500);
	keywin_on(key_win);
	sheet_slide(key_win,  300, 100);	
	
	sheet_updown(sht_back,     0);
	sheet_updown(key_win,  1);
	sheet_updown(sht_mouse,    2);
	
	///////  show information
	uint key;
	uchar key_shift=0;
	uchar key_ctrl=0;
	//binfo->leds:
	//第4位：scrollLock状态
	//第5位：numlock
	//第6位：casplock
	uchar key_leds = (binfo->leds >>4)&7;
	int keycmd_wait=-1;
	//设置灯的状态
	fifo32_push(&keycmd, KEYCMD_LED);
	fifo32_push(&keycmd, key_leds);
	
	int sheet_index=0;
	for(;;){
		if(fifo32_status(&keycmd)>0&&keycmd_wait<0){ //空的
			keycmd_wait=fifo32_pop(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		
		io_cli();
		if(fifo32_status(&fifo)==0){ //空的
			if(lastX!=-1){
				io_sti();
				sheet_slide(key_win, key_win->vx0+mouseX-lastX, 
								key_win->vy0+mouseY-lastY);
				lastX = mouseX;
				lastY = mouseY;
			}
			else{
				task_sleep(selftask);
				io_sti();
			}
		}
		else{
			key=fifo32_pop(&fifo);
			io_sti();//继续接收中断
			
			if(key_win!=0&&key_win->flags==0){//未被使用，被释放了
				if(shtctl->top == 1){//没有其它console
					key_win=0;
				}
				else{
					key_win = shtctl->sheets[shtctl->top-1];
					keywin_on(key_win);
				}
			}
			
			if(key>=256&&key<=511){
				key-=256;

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
				else if (key == 0x1d){
					key_ctrl=1;
				}				
				else if (key == 0x9d){
					key_ctrl=0;
				}
				
				else if (key == 0x3a) {	/* CapsLock */
					key_leds ^= 4;
					fifo32_push(&keycmd, KEYCMD_LED);
					fifo32_push(&keycmd, key_leds);
				}
				else if (key == 0x45) {	/* NumLock */
					key_leds ^= 2;
					fifo32_push(&keycmd, KEYCMD_LED);
					fifo32_push(&keycmd, key_leds);
				}
				else if (key == 0xba) {	/* ScrollLock */
					key_leds ^= 1;
					fifo32_push(&keycmd, KEYCMD_LED);
					fifo32_push(&keycmd, key_leds);
				}
				else if(key==0xfa){//键盘接收到了数据
					keycmd_wait=-1;
				}
				else if(key==0xfe){//键盘没有接收到了数据
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				
				else if(key == 0x1c){//enter,回车键
					if(key_win!=0&&key_win->flags!=0){
						fifo32_push((key_win->task->fifo),256+10);
					}
				}
				else if(key == 0x0e){//backspace
					if(key_win!=0&&key_win->flags!=0){
						fifo32_push((key_win->task->fifo),256+8);
					}
				}
				else if(key==TAB){
					keywin_off(key_win);
					key_win = shtctl->sheets[1];
					keywin_on(key_win);
					sheet_updown(key_win, shtctl->top - 1);
				}
				
				
				if(key < 0x80){//键盘编码转换为字符编码
					if(key_shift!=0){
						str[0] = keytable1[key];
					}
					else{
						str[0] = keytable0[key];
					}
					
					if(str[0]<='Z'&&str[0]>='A'){
						if((key_leds&4)!=0&&key_shift!=0 ||
							(key_leds&4)==0&&key_shift==0){
							str[0]=str[0]-'A'+'a';
						}
					}
				}
				else{
					str[0]=0;
				}
				
				if (key_ctrl ==1 && str[0] == 'c' 
					&& key_win!=0&&key_win->flags!=0 
					&& key_win->task->tss.ss0 != 0) {	//ctrl+c,强制终止
					sCONSOLEINFO * cons = key_win->task->cons;
					show_string_sheet(cons, "\nBreak(key)\n",1);
					io_cli();
					while(key_win->task->flag_allocRes==1){
						show_string_sheet(cons, "res is allocing",1);
					}
					key_win->task->tss.eax = (int) &(key_win->task->tss.esp0);
					key_win->task->tss.eip = (int) asm_end_app;
					io_sti();
					continue;
				}
				
				if (key_ctrl ==1 && str[0] == 't') {	//create console
					keywin_off(key_win);
					key_win = open_console(shtctl, &fifo, memtotal, 400, 500);
					sheet_slide(key_win,  500, 100);
					keywin_on(key_win);
					sheet_updown(key_win, shtctl->top);
					continue;
				}
				
				
				if(str[0]!=0 && key_win!=0&&key_win->flags!=0 ){
					fifo32_push((key_win->task->fifo),str[0]+256);
				}
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
					
					if ((mouseDec.btn & 0x01) != 0) {
						//str[1] = 'L';
						int h = find_sheet_xy(shtctl, mouseX, mouseY);
						if(h!=0){
							if(shtctl->sheets[h]!=key_win){
								keywin_off(key_win);
								sheet_refresh(key_win, 0, 0, key_win->bxsize,  21);
								key_win = shtctl->sheets[h];
								keywin_on(key_win);
								sheet_updown(key_win, shtctl->top-1);
							}
							
							sSHEET *sht = shtctl->sheets[h];
							int x=mouseX-sht->vx0;
							int y=mouseY-sht->vy0;
							if(x>=(sht->bxsize-21)&&x<sht->bxsize-5 &&5<=y&&y<19){
								if (key_win->task->tss.ss0 != 0) {	
									sCONSOLEINFO * cons = key_win->task->cons;
									show_string_sheet(cons, "\nBreak(key)\n",1);
									while(key_win->task->flag_allocRes==1){
										show_string_sheet(cons, "res is allocing",1);
									}
									io_cli();
									key_win->task->tss.eax = (int) &(key_win->task->tss.esp0);
									key_win->task->tss.eip = (int) asm_end_app;
									fifo32_push((key_win->task->fifo),5);
									io_sti();
								}
								else{
									sCONSOLEINFO * cons = key_win->task->cons;
									show_string_sheet(cons, "\nBreak(key)\n",1);
									io_cli();
									fifo32_push((key_win->task->fifo),5);
									io_sti();
								}
							}
							
							if(lastX==-1){
								lastX=mouseX;
								lastY=mouseY;
							}
						}
					}
					else{
						lastX = -1;
						lastY = -1;
					}
					if ((mouseDec.btn & 0x02) != 0) {
						str[3] = 'R';
					}
					if ((mouseDec.btn & 0x04) != 0) {
						str[2] = 'C';
					}
					sheet_slide(sht_mouse, mouseX, mouseY);
				}
			}
			else if (768 <= key && key <= 1023){//task to harimain
				//sheet information
				int sheetNum = key-768;
				while(key_win->task->flag_allocRes==1){
					show_string_sheet(key_win->task->cons, "res is allocing",1);
				}
				close_console(shtctl,sheetNum);
				
				sprintf(str,"close %d, free:%d KB",sheetNum-1, memManager->freesize/1024);
				show_string(str,10);
			}
		}
	}
}

void show_string(char str[], int line)
{
	sTASK *task = task_now();
	if(task->cons==0||line>1){
		uchar *buf=sht_debug->buf;
		int bxsize=sht_debug->bxsize;
		int t1=(line-1)*16, t2=8*strlen(str), t3=line*16;
		boxfill8(buf, bxsize, COL8_008484, 0, t1, t2, t3);
		putfonts8_asc(buf, bxsize, 0, t1, COL8_FFFFFF, str);	
		sheet_refresh(sht_debug,0, t1, t2, t3);
	}
	else{
		show_string_sheet(task->cons, str, 1);
	}
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

