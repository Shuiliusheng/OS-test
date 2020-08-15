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
	io_out8(PIC0_IMR, 0xf9); /* 开启键盘和鼠标中断主芯片设置:(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 从芯片设置:(11101111) */
	
	
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
	sSHEET *sht_back, *sht_mouse;
	uchar *mouse_buf;
	uchar *backbuf;
	int mouseX,mouseY;

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
	
	mouseX=(binfo->scrnx-16)/2;
	mouseY=(binfo->scrny-28-16)/2;
	sheet_slide(shtctl, sht_back, 0, 0);
	sheet_slide(shtctl, sht_mouse, mouseX, mouseY);
	
	sheet_updown(shtctl, sht_back,  0);
	sheet_updown(shtctl, sht_mouse, 1);
	
	sheet_refresh(shtctl); 
	///////  show information
	int key;
	char str[30];
	for(;;){
		//处理中断buf的过程中，关闭中断
		io_cli();
		if(fifo8_status(&keyfifo)==0&&fifo8_status(&mousefifo)==0){ //空的
			io_stihlt();//连续执行两条指令，避免sti执行之后立马发生中断
		}
		else{
			if(fifo8_status(&keyfifo)!=0){
				key=fifo8_pop(&keyfifo);
				io_sti();//继续接收中断
				sprintf(str,"%02X",key);
				boxfill8(backbuf, binfo->scrnx, COL8_840084, 0, 16, 15, 31);
				putfonts8_asc(backbuf, binfo->scrnx, 0, 16, COL8_FFFFFF, str);
				sheet_refresh(shtctl);
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
					else if(mouseX>=binfo->scrnx-16){
						mouseX=binfo->scrnx-16;
					}
					if(mouseY<0){
						mouseY=0;
					}
					else if(mouseY>=binfo->scrny-16){
						mouseY=binfo->scrny-16;
					}

					sheet_slide(shtctl, sht_mouse, mouseX, mouseY);
						
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
	int len=strlen(str);
	uchar *buf=sht_debug->buf;
	int bxsize=sht_debug->bxsize;
	boxfill8(buf, bxsize, 99, 0, (line-1)*16, 8*len, line*16);
	putfonts8_asc(buf, bxsize, 0, (line-1)*16, COL8_FFFFFF, str);
	sheet_refresh(ctl_debug);	
}