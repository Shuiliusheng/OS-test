#include"bootpack.h"

extern sFIFO8 keyfifo;


void HariMain(void)
{
	//已经编入img中，直接使用
	//256个字符，256*16
	extern char hankaku[4096];
	
	//0x0ff0是asmhead中定义各个变量的起始位置
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	
	init_gdtidt();
	init_pic();
	io_sti();	//中断初始化完成，开中断
	
	char keybuf[32];
	fifo8_init(&keyfifo, 32, keybuf);
	
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

	io_out8(PIC0_IMR, 0xf9); /* 开启键盘和鼠标中断主芯片设置:(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 从芯片设置:(11101111) */
	
	int key;
	for(;;){
		//处理中断buf的过程中，关闭中断
		io_cli();
		if(fifo8_status(&keyfifo)==0){ //空的
			io_stihlt();//连续执行两条指令，避免sti执行之后立马发生中断
		}
		else{
			key=fifo8_pop(&keyfifo);
			io_sti();//继续接收中断
			sprintf(str,"%02X",key);
			boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, str);
		}
		
	}
}