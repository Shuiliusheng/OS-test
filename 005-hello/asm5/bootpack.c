#include"bootpack.h"


#define MEMMAN_ADDR			0x003c0000

extern sFIFO8 keyfifo;
extern sFIFO8 mousefifo;
sSHTCTL *ctl_debug;
sSHEET *sht_debug;
void HariMain(void)
{	
	//0x0ff0是asmhead中定义各个变量的起始位置
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	
	init_gdtidt();
	init_pic();
	io_sti();	//中断初始化完成，开中断
	io_out8(PIC0_IMR, 0xf8); /* 开启键盘和鼠标中断主芯片设置:(11111000) */
	io_out8(PIC1_IMR, 0xef); /* 从芯片设置:(11101111) */
	
	/////// init timer
	init_PIT();
	
	
	//////////////// keyboard mouse
	char keybuf[32];
	char mousebuf[128];
	sMOUSE_DEC mouseDec;
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	
	init_keyboard();
	enable_mouse(&mouseDec);//激活鼠标
	
	//memory alloc
	uint memtotal;
	struct MEM_MAN *memManager;
	//0x003c0000
	memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	
	memtotal = memtest(0x00400000, 0xbfffffff);
	initMemManger(memManager);
	setFreeRange(memManager,0x00001000, 0x0009e000);
	setFreeRange(memManager,0x00400000, memtotal - 0x00400000);
	
	///////// 设置图层
	init_palette();
	
	sSHTCTL *shtctl;	
	sSHEET *sht_back, *sht_mouse, *sht_win;
	uchar *mouse_buf, *backbuf, *winbuf;
	int mouseX,mouseY;

	shtctl = shtctl_init(memManager, binfo->vram, binfo->scrnx,binfo->scrny);
	ctl_debug = shtctl ;
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
	int key;
	char str[30];
	for(;;){
		sprintf(str,"%010d",timerctl.count);
		show_string1(sht_win,str, 3, COL8_000000, COL8_C6C6C6);
		
		io_cli();
		if(fifo8_status(&keyfifo)==0&&fifo8_status(&mousefifo)==0){ //空的
			io_sti();
		}
		else{
			if(fifo8_status(&keyfifo)!=0){
				key=fifo8_pop(&keyfifo);
				io_sti();//继续接收中断
				sprintf(str,"%02X",key);
				boxfill8(backbuf, binfo->scrnx, COL8_840084, 0, 16, 15, 31);
				putfonts8_asc(backbuf, binfo->scrnx, 0, 16, COL8_FFFFFF, str);
				sheet_refresh(sht_back,0, 16, 16, 32);
			}
			else{
				key=fifo8_pop(&mousefifo);
				io_sti();//继续接收中断
				
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
