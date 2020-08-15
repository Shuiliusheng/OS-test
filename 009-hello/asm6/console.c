#include"bootpack.h"
#include<string.h>

#define BACKSPACE 8
#define ENTER 10


void console_task(sSHEET *sheet, sTASK *task, int memtotal)
{
	uint fifobuf[128];
	fifo32_init(&task->fifo, 128, fifobuf,task);
	
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	sTIMER *timer1;
	timer1=timer_alloc(memManager);
	timer_init(timer1, &task->fifo, 1);
	timer_settime(timer1, 50);
	
	
	//file alloc table,记录文件在磁盘中的位置，类似于链表
	//软盘共2880个扇区，因此为了记录下一个扇区号，需要1.5B
	//windows为了避免使用2B存储，将2*1.5B组合成3B
	uint *fat = (uchar *)mm_alloc(memManager, 2880*4);
	file_readfat(fat, (uchar *)(ADR_DISKIMG+FAT_OFFSET));
		
	sCONSOLEINFO cons;
	cons.sheet=sheet;
	cons.sx=8;
	cons.sy=28;
	cons.maxW=sheet->bxsize - 16;
	cons.maxH=sheet->bysize - 10;
	cons.cursorX=16;
	cons.cursorY=28;
	cons.color=COL8_000000;
	
	*((int *)0x0fec) = (int)&cons;
	
	int key,i=0;
	char str[32], cmdline[128];
	show_string_xy(sheet,">",8,cons.cursorY,COL8_FFFFFF,COL8_000000);
	for(;;){
		io_cli();
		if(fifo32_status(&task->fifo)==0){
			task_sleep(task);
			io_sti();
		}
		else{
			key=fifo32_pop(&task->fifo);
			if(key==1||key==0){
				if (key != 0) {
					timer_init(timer1, &task->fifo, 0); 
					if(cons.color!=-1){
						cons.color = COL8_000000;
					}
				} else {
					timer_init(timer1, &task->fifo, 1); 
					if(cons.color!=-1){
						cons.color = COL8_FFFFFF;
					}
				}
				timer_settime(timer1, 50);
			}
			else if(key==3){//停止闪烁
				cons.color=-1;
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cursorX, cons.cursorY, cons.cursorX + 7, cons.cursorY+15);
				sheet_refresh(sheet, cons.cursorX, cons.cursorY, cons.cursorX + 8, cons.cursorY+16);
			}
			else if(key==4){//开始闪烁
				cons.color=COL8_000000;
			}
			else if(key>=256&&key<=511){
				key-=256;
				if(key == BACKSPACE){	
					if(cons.cursorX>16){
						show_string_xy(sheet," ",cons.cursorX,cons.cursorY,COL8_FFFFFF,COL8_000000);
						cons.cursorX -= 8;	
					}
				}
				else if(key == ENTER){
					//先消除光标
					boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cursorX, cons.cursorY, cons.cursorX + 7, cons.cursorY+15);
					sheet_refresh(sheet, cons.cursorX, cons.cursorY, cons.cursorX + 8, cons.cursorY+16);
					
					console_newline(&cons);
					cmdline[(cons.cursorX-16)/8]='\0';
					
					cons.cursorX=8;
					console_runcmd(&cons, cmdline, fat, memtotal);
					
					cons.cursorX=16;
					show_string_xy(sheet,">",8,cons.cursorY,COL8_FFFFFF,COL8_000000);
				}
				else{
					if(cons.cursorX<=(cons.maxW-8)){
						str[0]=key;
						str[1]=0;
						cmdline[(cons.cursorX-16)/8]=key;
						show_string_xy(sheet,str,cons.cursorX,cons.cursorY,COL8_FFFFFF,COL8_000000);
						cons.cursorX += 8;
					}
				}
			}
			if(cons.color!=-1){
				boxfill8(sheet->buf, sheet->bxsize, cons.color, cons.cursorX, cons.cursorY, cons.cursorX + 7, cons.cursorY+15);
				sheet_refresh(sheet, cons.cursorX, cons.cursorY, cons.cursorX + 8, cons.cursorY+16);
			}
		}
	}
}

int strcmp_sub(char str[], const char substr[])
{
	int len1=strlen(str);
	int len2=strlen(substr);
	if(len1<len2){
		return 1;
	}
	
	char c=str[len2];
	str[len2]='\0';
	
	uchar flag=strcmp(str,substr);
	str[len2]=c;
	return flag;
}

void cmd_mem(sCONSOLEINFO *cons, uint memtotal)
{
	struct MEM_MAN *memManager;
	char str[32];
	memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	sprintf(str,"total:%d MB",memtotal/1024/1024);
	show_string_sheet(cons,str,1);
	sprintf(str,"free:%d KB",memManager->freesize/1024);
	show_string_sheet(cons,str,1);
}

void cmd_cls(sCONSOLEINFO *cons)
{
	boxfill8(cons->sheet->buf, cons->sheet->bxsize, COL8_000000, cons->sx, cons->sy, cons->maxW, cons->maxH);
	sheet_refresh(cons->sheet, cons->sx, cons->sy, cons->maxW, cons->maxH);
	cons->cursorY=cons->sy;
	cons->cursorX=16;
	show_string_xy(cons->sheet,">",8,cons->cursorY,COL8_FFFFFF,COL8_000000);
}

void cmd_dir(sCONSOLEINFO *cons)
{
	////从这个地方开始放文件结构体信息
	sFILEINFO *fileinfo = (sFILEINFO *)(ADR_DISKIMG+0x002600);
	int i=0,key=0;
	char str[64];
	for(i=0;i<224;i++){
		if(fileinfo[i].name[0]==0x00){
			//表示这一段不包括文件信息
			break;
		}
		if(fileinfo[i].name[0]!=0xe5){
			//0xe5表明该文件已被删除
			//0x10: 目录；0x08:非文件信息
			if( (fileinfo[i].type&0x18) == 0){
				sprintf(str, "filename.ext   %7d", fileinfo[i].size);
				for (key = 0; key < 8; key++) {
					str[key] = fileinfo[i].name[key];
				}
				str[ 9] = fileinfo[i].ext[0];
				str[10] = fileinfo[i].ext[1];
				str[11] = fileinfo[i].ext[2];
				show_string_sheet(cons,str,1);
			}
		}
	}
}

void cmd_type(sCONSOLEINFO *cons, char *cmdline, uint *fat)
{
	char *filename=&cmdline[5], *data;
	int i=0;	
	char str[2];
	sFILEINFO *fileinfo = (sFILEINFO *)(ADR_DISKIMG+0x002600);
	sFILEINFO *info=find_file(fileinfo,filename,0);
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	if(info!=0){
		data = (char *)mm_alloc(memManager, info->size);
		file_readdata(fat,data,info->size,info->clustno);
		str[1]='\0';
		for(i=0;i<info->size;i++){
			str[0]=data[i];
			show_string_sheet(cons,str,0);
		}
		mm_free(memManager,data);
		cons->cursorX=8;
		console_newline(cons);
	}
	else{
		show_string_sheet(cons,"file is not exist!",0);
	}
}

int cmd_app(sCONSOLEINFO *cons, char *cmdline, uint *fat)
{
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	sFILEINFO *fileinfo = (sFILEINFO *)(ADR_DISKIMG+0x002600);
	sFILEINFO *info=find_file(fileinfo,cmdline,1);
	if(info!=0){
		uint gdtnum = alloc_gdt();
		if(gdtnum != 0){
			char *data = (char *)mm_alloc(memManager, info->size);
			file_readdata(fat,data,info->size,info->clustno);
			
			set_segmdesc(gdt + gdtnum/8, info->size-1, (int)data, AR_CODE32_ER);
			farcall(0, gdtnum);
			
			mm_free(memManager, data);
			free_gdt(gdtnum);
		}
	}
	else{
		return 0;
	}
	console_newline(cons);
	return 1;
}

void console_runcmd(sCONSOLEINFO *cons, char *cmdline, uint *fat, uint memtotal)
{
	if(strcmp(cmdline,"mem")==0){
		cmd_mem(cons, memtotal);
	}
	else if(strcmp(cmdline,"cls")==0){
		cmd_cls(cons);
	}
	else if(strcmp(cmdline,"dir")==0){
		cmd_dir(cons);
	}
	else if(strcmp_sub(cmdline,"type ")==0){
		cmd_type(cons,cmdline,fat);
	}
	else if(strcmp(cmdline,"gdt")==0){
		char str[20];
		sprintf(str,"gdt used: %d\ngdt free:%d",gdt_used(),8192-gdt_used());
		show_string_sheet(cons,str,1);
	}
	else if(strlen(cmdline)>0){
		if(cmd_app(cons, cmdline, fat)==0){
			show_string_sheet(cons,"bad command",1);
		}
	}
}


void console_newline(sCONSOLEINFO *cons)
{
	if(cons->cursorY<=(cons->maxH-16)){
		cons->cursorY+=16;
	}
	else{//滚动
		int x,y,t,t1;
		
		//上移
		for(y=cons->sy;y<=cons->cursorY;y++){
			t=y*cons->sheet->bxsize;
			t1=(y+16)*cons->sheet->bxsize;
			for(x=cons->sx;x<=(cons->maxW+8);x++){
				cons->sheet->buf[t+x]=cons->sheet->buf[t1+x];
			}
		}
		
		for(y=cons->cursorY;y<=cons->cursorY+16;y++){
			t=y*cons->sheet->bxsize;
			for(x=cons->sx;x<=(cons->maxW+8);x++){
				cons->sheet->buf[t+x]=COL8_000000;
			}
		}
		sheet_refresh(cons->sheet,cons->sx,cons->sy,(cons->maxW+8),cons->cursorY+16);
	}
}

void show_string_sheet(sCONSOLEINFO *cons, char str[], int enter)
{
	int i=0,j=0,t=0;
	int len=strlen(str);
	char s[2];
	s[1]='\0';
	for(i=0;i<len;i++){
		if(str[i]==0x0a){//换行符
			cons->cursorX=8;
			console_newline(cons);
		}
		else if(str[i]==0x0d){
		}
		else if(str[i]==0x09){//tab符
			t=4-((cons->cursorX-8)/8)%4;
			for(j=0;j<t;j++){
				s[0]=' ';
				show_string_xy(cons->sheet,s,cons->cursorX,cons->cursorY,COL8_FFFFFF,COL8_000000);
				if(cons->cursorX<=(cons->maxW-8)){
					cons->cursorX+=8;
				}
				else{
					cons->cursorX=8;
					console_newline(cons);
				}
			}
		}
		else{
			s[0]=str[i];
			show_string_xy(cons->sheet,s,cons->cursorX,cons->cursorY,COL8_FFFFFF,COL8_000000);
			if(cons->cursorX<=(cons->maxW-8)){
				cons->cursorX+=8;
			}
			else{
				cons->cursorX=8;
				console_newline(cons);
			}
		}
	}
	if(cons->cursorX>8&&enter==1){
		console_newline(cons);
		cons->cursorX=8;
	}
}

void cons_putchar(sCONSOLEINFO *cons, char c, int move)
{
	int i=0,j=0,t=0;
	char s[2];
	s[1]='\0';
	s[0]=c;
	if(s[0]==0x0a){//换行符
		cons->cursorX=8;
		console_newline(cons);
	}
	else if(s[0]==0x0d){
	}
	else if(s[0]==0x09){//tab符
		t=4-((cons->cursorX-8)/8)%4;
		for(j=0;j<t;j++){
			s[0]=' ';
			show_string_xy(cons->sheet,s,cons->cursorX,cons->cursorY,COL8_FFFFFF,COL8_000000);
			if(cons->cursorX<=(cons->maxW-8)){
				cons->cursorX+=8;
			}
			else{
				cons->cursorX=8;
				console_newline(cons);
			}
		}
	}
	else{
		show_string_xy(cons->sheet,s,cons->cursorX,cons->cursorY,COL8_FFFFFF,COL8_000000);
		if(move!=0){
			if(cons->cursorX<=(cons->maxW-8)){
				cons->cursorX+=8;
			}
			else{
				cons->cursorX=8;
				console_newline(cons);
			}
		}
	}
}

