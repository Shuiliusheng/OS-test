#include"bootpack.h"

extern sFIFO8 keyfifo;
extern sFIFO8 mousefifo;

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
	
	io_out8(PIC0_IMR, 0xf9); /* 开启键盘和鼠标中断主芯片设置:(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 从芯片设置:(11101111) */
	
	init_keyboard();
	//
	
	char keybuf[32];
	char mousebuf[128];
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	
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
	sprintf(str, "(%d, %d)", mouseX, mouseY);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, str);
	
	enable_mouse();
	
	int key;
	uchar mouse_data[3],mouse_phase;
	mouse_phase=0;
	
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
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, str);
			}
			else{
				key=fifo8_pop(&mousefifo);
				io_sti();//继续接收中断
				
				if(mouse_phase==0){
					if(key==0xfa){
						mouse_phase=1;
					}
				}
				else if(mouse_phase==1){
					mouse_data[0]=key;
					mouse_phase=2;
				}
				else if(mouse_phase==2){
					mouse_data[1]=key;
					mouse_phase=3;
				}
				else if(mouse_phase==3){
					mouse_data[2]=key;
					mouse_phase=1;
					
					sprintf(str,"%02X,%02X,%02X",mouse_data[0],mouse_data[1],mouse_data[2]);
					
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 31+8*8, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, str);
				}
			}
		}
	}
}
	
/*
	使用鼠标的前提：控制电路，鼠标本身两个装置有效
	鼠标的控制电路包含在键盘的控制电路中，如果键盘控制电路
	初始化完成，则鼠标的也会完成
*/
	
/* 等待键盘控制器8042输入缓冲器寄存器空。*/
// kbc:keyboard controller
void wait_KBC_sendready(void)
{
	//等待键盘控制电路准备完毕
	for(;;){
		/* 8042状态控制器bit[1]=0时表示其输入缓冲寄存器
		为空即已可接受输入  0x02 */
		if((io_in8(PORT_KEYSTA)&KEYSTA_SEND_NOTREADY) == 0){
			break;
		}
	}
}

/* 设置键盘控制器8042运作方式:键盘以PC兼容方式运作,在未完成
 * 当前键盘数据的接收时不接收下一个键盘输入。*/
void init_keyboard(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return ;
}

// 激活鼠标
void enable_mouse(void)
{
	wait_KBC_sendready();
	//0xd4,下一个数据会自动发生给鼠标
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return;
}

