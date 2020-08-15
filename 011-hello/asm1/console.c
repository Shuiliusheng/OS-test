#include"bootpack.h"
#include<string.h>

#define BACKSPACE 8
#define ENTER 10

int test=0;
int hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) ADR_CONS);
	uint ds_base=0, gdtnum=0, segsize=0;
	sTASK *task = task_now();
	
	//获取用户程序传入参数的之前的栈地址
	//即asm中两次pushad的第一次为了保存寄存器的pushad
	int *reg = &eax + 1;	/* 第一次pushad的edi寄存器地址 */
	//用于向用户程序传递参数
	/* reg[0] : EDI,   reg[1] : ESI,   reg[2] : EBP,   reg[3] : ESP */
	/* reg[4] : EBX,   reg[5] : EDX,   reg[6] : ECX,   reg[7] : EAX */
		
	if (edx == 1) {
		cons_putchar(cons, eax&0xff, 1);
	} 
	else if(edx ==2){	//显示字符串
		//ecx: 字符串地址
		//ax: 数据段段号
		gdtnum = (eax&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		show_string_sheet(cons, (char*)(ebx+ds_base), 1);
	}
	else if(edx ==4){
		return &(task->tss.esp0);
	}
	else if(edx == 5){
		char str[40];
		sprintf(str,"%d, edx:%5d, eax:%5d, ecx:%5d",test,edx, eax, ecx);
		show_string(str,31);
		expand_segment(gdtnum/8,1024);
	}
	else if(edx == 8){	//init malloc，memManger 16B
		//ebx: malloc可用的地址范围，起始地址用于存放mMan
		//ax：数据段段号
		gdtnum = (eax&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		segsize = get_segSize(gdtnum/8);
		
		struct MEM_MAN *man = (struct MEM_MAN *)(ds_base + ebx);
		uint freesize = (segsize - ebx - sizeof(struct MEM_MAN));
		char str[40];
		sprintf(str,"free:%d, seg:%p, 0x%p",freesize,segsize,ebx);
		show_string(str,31);
		uint addr = ds_base + ebx + sizeof(struct MEM_MAN);
		initMemManger(man);
		setFreeRange(man, addr, freesize);
		show_string("init",32);
		freeBlockNum(man,33);
	}
	else if(edx == 9){	//mm_alloc
		//ebx: mMan的起始地址
		//ecx: 分配数据大小
		//ax：数据段段号
		gdtnum = (eax&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		struct MEM_MAN *man = (struct MEM_MAN *)(ds_base + ebx);
		
		reg[7] = mm_alloc(man, ecx);
		if(reg[7]==0){	//尝试增加段大小
			//增加10K * n
			uint freesize = (ecx/10240+1)*10240;
			uint old_segsize = get_segSize(gdtnum/8);
			uint old_ds = get_segBaseAddr(gdtnum/8);
			expand_segment(gdtnum/8, freesize);
			
			ds_base = get_segBaseAddr(gdtnum/8);
			//segsize = get_segSize(gdtnum/8);
			
			man = (struct MEM_MAN *)(ds_base + ebx);
			//第一个free块需要将实地址重新转换
			man->freeblocks = (sMEM_BLOCK *)((uint)man->freeblocks - old_ds + ds_base);
			setFreeRange(man, ds_base+old_segsize, freesize);
			show_string("malloc",35);
			freeBlockNum(man,36);
			reg[7] = mm_alloc(man, ecx);
		}
		
		if(reg[7]!=0){
			//转换为段内地址
			reg[7] -= ds_base;
		}
		
	}
	else if(edx == 10){	//mm_free
		//ebx: mMan的起始地址
		//ecx: 释放的内存地址
		//ax：数据段段号
		gdtnum = (eax&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		struct MEM_MAN *man = (struct MEM_MAN *)(ds_base + ebx);
		mm_free(man, ecx+ds_base);
		show_string("free",40);
		freeBlockNum(man,41);
	}
	else if(edx == 11){	//mm_free_size
		//ebx: mMan的起始地址
		//ax：数据段段号
		gdtnum = (eax&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		struct MEM_MAN *man = (struct MEM_MAN *)(ds_base + ebx);
		reg[7] = man->freesize;
	}
	return 0;
}

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
	
	*((int *)ADR_CONS) = (int)&cons;
	
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

int cmd_app(sCONSOLEINFO *cons, char *cmdline, uint *fat)
{
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	sFILEINFO *fileinfo = (sFILEINFO *)(ADR_DISKIMG+0x002600);
	sFILEINFO *info=find_file(fileinfo,cmdline,0);
	sTASK *task = task_now();
	uint segsiz,esp,datsiz, dathrb,i=0;
	char *code,*data;
	if(info==0){
		char str[30];
		strcpy(str,cmdline);
		strcat(str,".hrb");
		info=find_file(fileinfo,str,0);
	}
	if(info!=0){
		uint gdt_code = alloc_gdt();
		uint gdt_data = alloc_gdt();
		if(gdt_code!= 0 && gdt_data!=0){
			code = (char *)mm_alloc(memManager, info->size);
			file_readdata(fat,code,info->size,info->clustno);
			if (info->size >= 36 && strncmp(code + 4, "Hari", 4) == 0 && code!=0){
				segsiz = *((int *) (code + 0x0000));
				esp    = *((int *) (code + 0x000c));
				datsiz = *((int *) (code + 0x0010));
				dathrb = *((int *) (code + 0x0014));
				uint mem=*((int *) (code + 0x0020));
				
				data = (char *)mm_alloc(memManager, segsiz);
				*((int *)ADR_DATA)=(int)data;
				//复制代码中数据段的数据
				//mm_copy(&data[esp],&code[datahrb], datsiz);
				for (i = 0; i < datsiz; i++) {
					data[esp + i] = code[dathrb + i];
				}
				
				//通过+0x60让cpu能够区分出当前是处于什么段，用户还是os
				set_segmdesc(gdt + gdt_code/8, info->size-1, (int)code, AR_CODE32_ER+0x60);
				set_segmdesc(gdt + gdt_data/8, segsiz-1, (int)data, AR_DATA32_RW+0x60);
				
				struct SEGMENT_DESCRIPTOR *sd = &(gdt[gdt_data/8]);
				
				char str[60];
				//sprintf(str,"init: %5d,0x%10x,%10d,%5d",gdt_data/8, data,segsiz-1, AR_DATA32_RW+0x60);
				sprintf(str,"memstart: %d,%d",mem,segsiz);
				show_string(str,30);
				//&(task->tss.esp0)
				//需要在当前任务的TSS段中将esp0和ss写入操作系统当前的esp和ss
				start_app(0x1b, gdt_code, esp , gdt_data, &(task->tss.esp0));
			
				//防止在运行过程中出现变化
				data = get_segBaseAddr(gdt_data/8);
				mm_free(memManager, data);
				free_gdt(gdt_data);
			}
			else{
				show_string_sheet(cons,"hrb file format error!",1);
			}
			mm_free(memManager, code);
			free_gdt(gdt_code);
		}
	}
	else{
		return 0;
	}
	console_newline(cons);
	return 1;
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

uint str2int(char str[])
{
	int l=strlen(str),s=0,neg=1,t=0;
	uint dst=0;
	if(l<1){
		return 0;
	}
	
	if(str[0]=='-'){
		s=1;
		neg=-1;
	}
	for(;s<l;s++){
		t=str[s]-'0';
		dst=dst*10+t;
	}
	return dst*neg;
}

void console_runcmd(sCONSOLEINFO *cons, char *cmdline, uint *fat, uint memtotal)
{
	char buf[512];
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
		char str[50];
		sprintf(str,"gdt used: %d\ngdt free:%d",gdt_used(),8192-gdt_used());
		show_string_sheet(cons,str,1);
	}
	else if(strcmp_sub(cmdline,"read ")==0){
		uint sector=0;
		char str[50];
		sector=str2int(&cmdline[5]);
		
		buf[0]=0,buf[1]=1,buf[2]=2;
		hd_read(sector, 1, buf);
		sprintf(str,"sect1: %d,(%d,%d,%d)", sector, buf[0],buf[1],buf[2]);
		show_string_sheet(cons,str,1);
	}
	else if(strcmp_sub(cmdline,"write ")==0){
		uint sector=0;
		char str[50];
		sector=str2int(&cmdline[5]);
		//writeSector(1, sector, buf, 512);
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

