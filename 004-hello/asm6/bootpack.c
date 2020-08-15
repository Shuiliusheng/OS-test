#include"bootpack.h"


#define MEMMAN_ADDR			0x003c0000

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
	
	
	//////////////// keyboard mouse
	char keybuf[32];
	char mousebuf[128];
	sMOUSE_DEC mouseDec;
	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	
	init_keyboard();
	//激活鼠标
	enable_mouse(&mouseDec);
	
	//memory alloc
	uint memtotal;
	struct MEM_MAN *memManager;
	//0x003c0000
	memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	
	memtotal = memtest(0x00400000, 0xbfffffff);
	initMemManger(memManager);
	setFreeRange(memManager,0x00001000, 0x0009e000);
	setFreeRange(memManager,0x00400000, memtotal - 0x00400000);
	
	char *p1,*p2,*p3;
	p1 = mm_alloc(memManager, 100);
	p2 = mm_alloc(memManager, 0x100000);
	p3 = mm_alloc(memManager, 10);
	
	char temp[30];
	sprintf(temp,"0x%p, 0x%p, 0x%p",p1,p2,p3);
	
	
	//////////////// screen
	init_palette();
	init_screen(binfo->vram,binfo->scrnx,binfo->scrny);
	
	char str[30];
	sprintf(str,"vram:0x%p",binfo->vram);
	putfonts8_asc(binfo->vram,binfo->scrnx,24,24,COL8_FFFFFF,str);

	///////////// show mouse
	char mcursor[256];//16*16;
	int mouseX,mouseY;
	mouseX=(binfo->scrnx-16)/2;
	mouseY=(binfo->scrny-28-16)/2;
	init_mouse_cursor8(mcursor,COL8_840084);
	putblock8_8(binfo->vram,binfo->scrnx,16,16,mouseX,mouseY,mcursor,16);
	
	freeBlockNum(memManager);
	show_string(temp,9);
	
	///////  show information
	int key;
	
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
				boxfill8(binfo->vram, binfo->scrnx, COL8_840084, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, str);
			}
			else{
				key=fifo8_pop(&mousefifo);
				io_sti();//继续接收中断
				
				if(mouse_decode(&mouseDec,key)==1){
					
					/////// 显示鼠标
					boxfill8(binfo->vram, binfo->scrnx, COL8_840084, mouseX, mouseY, mouseX+15, mouseY+15);
					
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
					putblock8_8(binfo->vram,binfo->scrnx,16,16,mouseX,mouseY,mcursor,16);
					
					sprintf(str, "[lcr %3d %3d]", mouseX, mouseY);
					if ((mouseDec.btn & 0x01) != 0) {
						str[1] = 'L';
						mm_free(memManager,p2);
						freeBlockNum(memManager);
					}
					if ((mouseDec.btn & 0x02) != 0) {
						str[3] = 'R';
						mm_free(memManager,p1);
						freeBlockNum(memManager);
					}
					if ((mouseDec.btn & 0x04) != 0) {
						str[2] = 'C';
						mm_free(memManager,p3);
						freeBlockNum(memManager);
					}

					//show_string(str,2);
					//boxfill8也用于隐藏上一次输出的字符
					//boxfill8(binfo->vram, binfo->scrnx, COL8_840084, 32, 16, 32 + 12 * 8 - 1, 31);
					//putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, str);
				}
			}
		}
	}
}


void show_string(char str[], int line)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//boxfill8也用于隐藏上一次输出的字符
	int len=strlen(str);
	boxfill8(binfo->vram, binfo->scrnx, COL8_840084, 0, (line-1)*16, 8*len, line*16);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, (line-1)*16, COL8_FFFFFF, str);
}