#include"bootpack.h"

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
	
	for(;;){
		io_hlt();
	}
}