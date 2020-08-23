#include"bootpack.h"
#include<string.h>

#define BACKSPACE 8
#define ENTER 10

int test=0;
int hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	uint ds_base=0, gdtnum=0, segsize=0;
	sTASK *task = task_now();
	sCONSOLEINFO *cons = task->cons;
	struct MEM_MAN *man_cons = (struct MEM_MAN *) MEMMAN_ADDR;
	
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
		show_string_sheet(cons, (char*)(ecx+ds_base), 0);
	}
	else if(edx ==4){	//ret
		return &(task->tss.esp0);
	}
	else if(edx == 5){
		//expand_segment(gdtnum/8,1024);
	}
	else if(edx == 8){	//init malloc，memManger 16B
		//ebx: malloc可用的地址范围，起始地址用于存放mMan
		//ax：数据段段号
		gdtnum = (eax&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		segsize = get_segSize(gdtnum/8);
		
		struct MEM_MAN *man = (struct MEM_MAN *)(ds_base + ebx);
		uint freesize = (segsize - ebx - sizeof(struct MEM_MAN));
		uint addr = ds_base + ebx + sizeof(struct MEM_MAN);
		initMemManger(man);
		setFreeRange(man, addr, freesize);
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
			
			man = (struct MEM_MAN *)(ds_base + ebx);
			//第一个free块需要将实地址重新转换
			man->freeblocks = (sMEM_BLOCK *)((uint)man->freeblocks - old_ds + ds_base);
			setFreeRange(man, ds_base+old_segsize, freesize);
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
	}
	else if(edx == 11){	//mm_free_size
		//ebx: mMan的起始地址
		//ax：数据段段号
		gdtnum = (eax&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		struct MEM_MAN *man = (struct MEM_MAN *)(ds_base + ebx);
		reg[7] = man->freesize;
	}
	else if(edx == 12){
		//sFileHandler *openFile(char filename[], char flag, int *err);
		//eax: filename
		//ebx: flag
		//edi: ds
		int err=0;
		gdtnum = (edi&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		
		task->flag_allocRes=1;
		reg[7] = file_open((char*)(eax+ds_base), ebx&0xff, task->currentDir);
		allocHandler(task, (struct MEM_MAN *)MEMMAN_ADDR,(uint)reg[7], HANDLER_FILE);
		task->flag_allocRes=0;
		
	}
	else if(edx == 13){
		//void file_close(sFileHandler *handler)
		//eax: handler
		file_close((sFileHandler *)eax);
	}
	else if(edx == 14){
		//int file_read(sFileHandler *handler, char *buf, uint size);
		//eax: handler
		//ebx: buf
		//ecx: size
		//edi: 段号
		gdtnum = (edi&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		reg[7]=file_read((sFileHandler *)eax, (char*)(ebx+ds_base), ecx);
	}
	else if(edx == 15){
		//int file_write(sFileHandler *handler, char *buf, uint size);
		//eax: handler
		//ebx: buf
		//ecx: size
		//edi: 段号
		gdtnum = (edi&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		reg[7]=file_write((sFileHandler *)eax, (char*)(ebx+ds_base), ecx);
	}
	else if(edx == 16){
		//int file_eof(sFileHandler *handler);
		//eax: handler
		reg[7]=file_eof((sFileHandler *)eax);
	}
	else if(edx == 17){
		//int file_seek(sFileHandler *handler, int offset, uchar flag);
		//eax: handler
		//ebx: offset
		//ecx: flag
		reg[7]=file_seek((sFileHandler *)eax, ebx, ecx&0xff);
	}
	else if(edx == 18){
		//int file_tell(sFileHandler *handler);
		//eax: handler
		reg[7]=file_tell((sFileHandler *)eax);
	}
	else if(edx == 19){//监听字符输入
		//eax: flag标志，控制监听功能
		// 0: 不休眠，如果没有键盘输入则返回-1
		// 1: 休眠直到键盘输入数据
		int key=0;
		while(1){
			io_cli();
			if(fifo32_status(task->fifo)==0){
				if(eax&0x01 == 0){
					io_sti();
					reg[7]=-1;
					return 0;
				}
				else{
					task_sleep(task);
					io_sti();
				}
			}
			else{
				key=fifo32_pop(task->fifo);
				io_sti();
				if(key<=1){
					timer_init(cons->timer, task->fifo, 1);
					timer_settime(cons->timer, 50);
				}
				else if(key==3){//停止闪烁
					cons->color=-1;
				}
				else if(key==4){//开始闪烁
					cons->color=COL8_000000;
				}
				else if(key==5){//exit
					cmd_exit();
				}
				else if(key>=256){
					reg[7] = key-256;
					return 0;
				}
			}
		}
	}
	else if(edx == 20){//输入字符串
		//eax: 缓冲区地址
		//ecx：缓冲区大小
		//edi：段号
		//返回值：字符串长度
		gdtnum = (edi&0xffff);
		ds_base = get_segBaseAddr(gdtnum/8);
		char *input = (char*)(eax+ds_base);
		char str[2]=" ";
		int index = 0;
		int key=0;
		while(1){
			io_cli();
			if(fifo32_status(task->fifo)==0){
				task_sleep(task);
				io_sti();
			}
			else{
				key=fifo32_pop(task->fifo);
				io_sti();
				if(key<=1){
					timer_init(cons->timer, task->fifo, 1);
					timer_settime(cons->timer, 50);
				}
				else if(key==3){//停止闪烁
					cons->color=-1;
				}
				else if(key==4){//开始闪烁
					cons->color=COL8_000000;
				}
				else if(key==5){//exit
					cmd_exit();
				}
				else if(key>=256&&key<=511){
					key = key-256;
					if(key == BACKSPACE){	
						if(index>0){
							index--;
							if(cons->cursorX==8){
								cons->cursorX=cons->maxW;
								cons->cursorY-=16;
							}
							else{
								cons->cursorX-=8;
							}
							show_string_sheet(cons," ",0);
							
							if(cons->cursorX==8){
								cons->cursorX=cons->maxW;
								cons->cursorY-=16;
							}
							else{
								cons->cursorX-=8;
							}
						}
					}
					else if(key == ENTER){
						show_string_sheet(cons,"\n",0);
						input[index]='\0';
						reg[7]=index;
						return 0;
					}
					else{
						if(index<ecx-1){
							input[index]=key;
							index++;
							str[0]=key;
							show_string_sheet(cons,str,0);
						}
						else{
							input[index]=key;
							return index;
						}
					}
				}
			}
		}
	}
	else if(edx == 21){//alloc timer
		struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
		task->flag_allocRes=1;
		reg[7]=(uint)timer_alloc(memManager);
		allocHandler(task, memManager,reg[7], HANDLER_TIMER_APP);
		task->flag_allocRes=0;
	}
	else if(edx == 22){//设置定时器发送的数据
		//eax: timer
		//ecx: data
		timer_init((sTIMER *)eax, task->fifo, ecx+256);
	}
	else if(edx == 23){//设置定时器间隔
		//eax: timer
		//ecx: sec
		timer_settime((sTIMER *)eax, ecx);
	}
	else if(edx == 24){// timer free
		//eax:timer
		struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
		timer_free(memManager, eax);
	}
	else if(edx == 25){//thread create
		//eax: eip
		//ecx: stack_addr
		//ebx: parameter
		reg[7] = (int)thread_create(task, ecx, eax);
	}
	else if(edx == 26){//thread_run
		//eax: thread id
		thread_run((sThread *)eax);
	}
	else if(edx == 27){//thread_sleep
		//eax: thread id
		thread_sleep((sThread *)eax);
	}
	else if(edx == 28){//thread_delete
		//eax: thread id
		thread_delete((sThread *)eax);
	}
	else if(edx == 29){//thread_isover
		//eax: thread id
		reg[7]=thread_isover((sThread *)eax);
	}
	return 0;
}

void console_task(sSHEET *sheet, sTASK *task, int memtotal)
{
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	
	//file alloc table,记录文件在磁盘中的位置，类似于链表
	//软盘共2880个扇区，因此为了记录下一个扇区号，需要1.5B
	//windows为了避免使用2B存储，将2*1.5B组合成3B
	task->flag_allocRes=1;
	uint *fat = (uchar *)mm_alloc(memManager, 2880*4);
	allocHandler(task, memManager,(uint)fat, HANDLER_CONS);
	task->flag_allocRes=0;
	
	file_readfat(fat, (uchar *)(ADR_DISKIMG+FAT_OFFSET));
	
	sCONSOLEINFO cons;
	cons.sheet=sheet;
	cons.sx=8;
	cons.sy=28;
	cons.maxW= sheet->bxsize - 24 - (sheet->bxsize%8);
	
	cons.maxH=sheet->bysize - 10;
	cons.cursorX=48;
	cons.cursorY=28;
	cons.color=COL8_000000;
	cons.inputx=48;
	
	//分配定时器
	task->flag_allocRes=1;
	cons.timer=timer_alloc(memManager);
	allocHandler(task, memManager,(uint)cons.timer, HANDLER_TIMER);
	task->flag_allocRes=0;
	
	timer_init(cons.timer, task->fifo, 1);
	timer_settime(cons.timer, 50);

	task->cons = &cons;
	sFileInfoS *info = index_fileinfo(task->currentDir);
	strcpy(task->dir,"   .>");
	mm_copy(task->dir, info->filename, (strlen(info->filename)>4)?3:strlen(info->filename));
	
	int key,i=0;
	char str[32], cmdline[300];
	int index=0;
	show_string_xy(sheet,task->dir,8,cons.cursorY,COL8_FFFFFF,COL8_000000);
	for(;;){
		io_cli();
		if(fifo32_status(task->fifo)==0){
			task_sleep(task);
			io_sti();
		}
		else{
			key=fifo32_pop(task->fifo);
			io_sti();
			if(key==1||key==0){
				if (key != 0) {
					timer_init(cons.timer, task->fifo, 0); 
					if(cons.color!=-1){
						cons.color = COL8_000000;
					}
				} else {
					timer_init(cons.timer, task->fifo, 1); 
					if(cons.color!=-1){
						cons.color = COL8_FFFFFF;
					}
				}
				timer_settime(cons.timer, 50);
			}
			else if(key==3){//停止闪烁
				cons.color=-1;
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cursorX, cons.cursorY, cons.cursorX + 7, cons.cursorY+15);
				sheet_refresh(sheet, cons.cursorX, cons.cursorY, cons.cursorX + 8, cons.cursorY+16);
			}
			else if(key==4){//开始闪烁
				cons.color=COL8_000000;
			}
			else if(key==5){//exit
				cmd_exit();
			}
			else if(key>=256&&key<=511){
				key-=256;
				if(key == BACKSPACE){	
					if(cons.cursorX>cons.inputx){
						show_string_xy(sheet," ",cons.cursorX,cons.cursorY,COL8_FFFFFF,COL8_000000);
						cons.cursorX -= 8;	
						index--;
						if(index<0){
							index = 0;
						}
					}
				}
				else if(key == ENTER){
					//先消除光标
					boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cursorX, cons.cursorY, cons.cursorX + 7, cons.cursorY+15);
					sheet_refresh(sheet, cons.cursorX, cons.cursorY, cons.cursorX + 8, cons.cursorY+16);
					
					console_newline(&cons);
					cmdline[index]='\0';
					index = 0;
					cons.cursorX=8;
					console_runcmd(&cons, cmdline, fat, memtotal);
					cons.cursorX=cons.inputx;
					show_string_xy(sheet,task->dir,8,cons.cursorY,COL8_FFFFFF,COL8_000000);
				}
				else{
					if(index < 255){
						cmdline[index]=key;
						index++;
						str[0]=key;
						str[1]=0;
						show_string_sheet(&cons,str,0);
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
		//char str[256];
		//strcpy(str,cmdline);
		strcat(cmdline,".hrb");
		info=find_file(fileinfo,cmdline,0);
	}
	if(info!=0){
		////alloc gdt
		task->flag_allocRes = 1;
		task->gdt_code = alloc_gdt();
		task->gdt_data = alloc_gdt();
		task->flag_allocRes = 0;
		
		if(task->gdt_code!= 0 && task->gdt_data!=0){
			
			//alloc memory
			task->flag_allocRes = 1;
			code = (char *)mm_alloc(memManager, info->size);
			task->flag_allocRes = 0;
			
			file_readdata(fat,code,info->size,info->clustno);
			if (info->size >= 36 && strncmp(code + 4, "Hari", 4) == 0 && code!=0){
				segsiz = *((int *) (code + 0x0000));
				esp    = *((int *) (code + 0x000c));
				datsiz = *((int *) (code + 0x0010));
				dathrb = *((int *) (code + 0x0014));
				uint mem=*((int *) (code + 0x0020));
				
				task->flag_allocRes = 1;
				data = (char *)mm_alloc(memManager, segsiz);
				task->flag_allocRes = 0;
				//*((int *)ADR_DATA)=(int)data;
				//复制代码中数据段的数据
				mm_copy(&data[esp],&code[dathrb], datsiz);
				
				//通过+0x60让cpu能够区分出当前是处于什么段，用户还是os
				set_segmdesc(gdt + task->gdt_code/8, info->size-1, (int)code, AR_CODE32_ER+0x60);
				set_segmdesc(gdt + task->gdt_data/8, segsiz-1, (int)data, AR_DATA32_RW+0x60);	
				
				//&(task->tss.esp0)
				//需要在当前任务的TSS段中将esp0和ss写入操作系统当前的esp和ss
				start_app(0x1b, task->gdt_code, esp , task->gdt_data, &(task->tss.esp0));
				
				//释放申请的资源
				task->flag_allocRes = 1;
				releaseHandler(task, HANDLER_FILE);
				releaseHandler(task, HANDLER_TIMER_APP);
				task->flag_allocRes = 0 ;
				
				//删除线程
				releaseThreads(task);
				
				//防止在运行过程中出现变化
				data = get_segBaseAddr(task->gdt_data/8);
				mm_free(memManager, data);
				
				task->flag_allocRes = 1;
				free_gdt(task->gdt_data);
				task->gdt_data = 0;
				task->flag_allocRes = 0 ;				
			}
			else{
				show_string_sheet(cons,"hrb file format error!",1);
			}
			mm_free(memManager, code);
			
			task->flag_allocRes = 1 ;
			free_gdt(task->gdt_code);
			task->gdt_code = 0;
			task->flag_allocRes = 0 ;
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
	sprintf(str,"free:%d KB (%d B)",memManager->freesize/1024,memManager->freesize);
	show_string_sheet(cons,str,1);
}

void cmd_cls(sCONSOLEINFO *cons)
{
	boxfill8(cons->sheet->buf, cons->sheet->bxsize, COL8_000000, cons->sx, cons->sy,cons->sheet->bxsize-8, cons->maxH);
	sheet_refresh(cons->sheet, cons->sx, cons->sy, cons->sheet->bxsize-8, cons->maxH);
	cons->cursorY=cons->sy;
	cons->cursorX=cons->inputx;
	sTASK *task = task_now();
	show_string_xy(cons->sheet,task->dir,8,cons->cursorY,COL8_FFFFFF,COL8_000000);
}

void cmd_dir(sCONSOLEINFO *cons)
{
	////从这个地方开始放文件结构体信息
	sFILEINFO *fileinfo = (sFILEINFO *)(ADR_DISKIMG+0x002600);
	int i=0,key=0;
	char str[64];
	sTASK *task = task_now();
	if(task->currentDir == 0){
		for(i=0;i<224;i++){
			if(fileinfo[i].name[0]==0x00){
				show_string_sheet(cons,"none",1);
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
	
	show_string_sheet(cons,"---------------",1);
	fs_showlist(cons,task->currentDir);
}

void cmd_type(sCONSOLEINFO *cons, char *cmdline, uint *fat)
{
	char *filename=&cmdline[5], *data;
	int i=0;	
	char str[2];
	char name[30];
	strcpy(name,filename);
	sFILEINFO *fileinfo = (sFILEINFO *)(ADR_DISKIMG+0x002600);
	sFILEINFO *info=find_file(fileinfo,filename,0);
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	sTASK *task = task_now();
	if(info!=0){
		
		task->flag_allocRes = 1;
		data = (char *)mm_alloc(memManager, info->size);
		sHandlerInfo *handler=allocHandler(task, memManager,(uint)data, HANDLER_CONS);
		task->flag_allocRes = 0;
		
		file_readdata(fat,data,info->size,info->clustno);
		str[1]='\0';
		for(i=0;i<info->size;i++){
			str[0]=data[i];
			show_string_sheet(cons,str,0);
		}
		//mm_free(memManager,data);
		releaseHandlerSingle(task, handler);
		
		cons->cursorX=8;
		console_newline(cons);
	}
	else{
		char str[20];
		sFileInfoS *info = index_fileinfo(task->currentDir);
		fs_type(cons, task->currentDir, name);
	}
}

void cmd_exit()
{
	sTASK *task = task_now();
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	struct CONSOLEINFO *cons = task->cons;
	
	releaseHandler(task, HANDLER_FILE);
	releaseHandler(task, HANDLER_CONS);
	releaseThreads(task);
	
	if (cons->sheet != 0) {
		releaseHandler(task, HANDLER_TIMER);
	}
	
	char *temp;
	if(task->gdt_data!=0){
		temp = get_segBaseAddr(task->gdt_data/8);
		mm_free(memManager, temp);
		free_gdt(task->gdt_data);
	}
	
	if(task->gdt_code!=0){
		temp = get_segBaseAddr(task->gdt_code/8);
		mm_free(memManager, temp);
		free_gdt(task->gdt_code);
	}
	
	io_cli();
	if (cons->sheet != 0) {
		//用于在harimain中exit
		fifo32_push(task->os_fifo, (cons->sheet - task->shtctl->sheets0) + 768);	/* 768～1023 */
	} 
	io_sti();
	
	for (;;) {
		task_sleep(task);
	}
}

void cmd_start(sCONSOLEINFO *cons, char *cmdline, uint memtotal)
{
	sTASK *task = task_now();
	sSHEET *sht = open_console(task->shtctl, task->os_fifo, memtotal, 400, 500);
	
	sht->task->currentDir = task->currentDir;
	memcpy(sht->task->dir,task->dir, 6);
	
	sheet_slide(sht,  50, 50);
	sheet_updown(sht, task->shtctl->top);
	
	int i=0;
	for(i=0;i<strlen(cmdline);i++){
		fifo32_push((sht->task->fifo),cmdline[i]+256);
	}
	fifo32_push((sht->task->fifo),10+256);//enter
	
	cons->cursorX=8;
	console_newline(cons);
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
	else if(strcmp_sub(cmdline,"start ")==0){
		cmd_start(cons,&cmdline[6],memtotal);
	}
	else if(strcmp(cmdline,"exit")==0){
		cmd_exit();
	}
	else if(strcmp_sub(cmdline,"rm ")==0){
		char *filename=&cmdline[3];
		sTASK *task = task_now();
		
		task->flag_allocRes = 1;
		int ret=fs_delete(task->currentDir,filename);
		task->flag_allocRes = 0;
		
		if(ret==-1){
			show_string_sheet(cons,"not found",1);
		}
		else if(ret==-2){
			show_string_sheet(cons,"a file is writing",1);
		}
		else if(ret==-3){
			show_string_sheet(cons,"a file is reading",1);
		}
		else if(ret==-4){
			show_string_sheet(cons,"dir is busying",1);
		}
		else{
			show_string_sheet(cons,"delete success",1);
		}
		cons->cursorX=8;
		console_newline(cons);
	}
	else if(strcmp_sub(cmdline,"mkdir ")==0){
		char *filename=&cmdline[6];
		
		sTASK *task = task_now();
		task->flag_allocRes = 1;
		fs_createDir(task->currentDir, filename);
		task->flag_allocRes = 0;
		
		cons->cursorX=8;
		console_newline(cons);
	}
	else if(strcmp_sub(cmdline,"cd ")==0){
		char *filename=&cmdline[3];
		sTASK *task = task_now();
		
		if(strcmp(filename,"..")==0){
			task->currentDir=fs_back_dir(task->currentDir);
		}
		else if(strcmp(filename,".")==0){
		}
		else if(strlen(filename)>0){
			task->currentDir=fs_find_dir(task->currentDir, filename);
		}
		fs_showdir(cons,task->currentDir);
		cons->cursorX=8;
		console_newline(cons);
		
		sFileInfoS *info = index_fileinfo(task->currentDir);
		strcpy(task->dir,"   .>");
		mm_copy(task->dir, info->filename, (strlen(info->filename)>4)?3:strlen(info->filename));
	}
	else if(strcmp(cmdline,"disk")==0){
		char str[50];
		sprintf(str,"sector used: %d", used_sector_num());
		show_string_sheet(cons,str,1);
	}
	else if(strcmp(cmdline,"fs")==0){
		char str[50];
		sprintf(str,"fileinfo used: %d", used_fileinfo());
		show_string_sheet(cons,str,1);
	}
	else if(strcmp(cmdline,"task")==0){
		char str[50];
		handlerNum();
	}
	else if(strcmp(cmdline,"gdt")==0){
		char str[50];
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
		int t2=cons->sheet->bxsize-8;
		//上移
		for(y=cons->sy;y<=cons->cursorY;y++){
			t=y*cons->sheet->bxsize;
			t1=(y+16)*cons->sheet->bxsize;
			for(x=cons->sx;x<=t2;x++){
				cons->sheet->buf[t+x]=cons->sheet->buf[t1+x];
			}
		}
		
		for(y=cons->cursorY;y<=cons->cursorY+16;y++){
			t=y*cons->sheet->bxsize;
			for(x=cons->sx;x<=t2;x++){
				cons->sheet->buf[t+x]=COL8_000000;
			}
		}
		sheet_refresh(cons->sheet,cons->sx,cons->sy,cons->sheet->bxsize-8,cons->cursorY+16);
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
		else if(str[i]==0x0d){	// 回车符
			cons->cursorX=8;
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
			if(cons->cursorX<=(cons->maxW)){
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
		cons->cursorX=8;
	}
	else if(s[0]==0x09){//tab符
		t=4-((cons->cursorX-8)/8)%4;
		for(j=0;j<t;j++){
			s[0]=' ';
			show_string_xy(cons->sheet,s,cons->cursorX,cons->cursorY,COL8_FFFFFF,COL8_000000);
			if(cons->cursorX<=(cons->maxW)){
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
			if(cons->cursorX<=(cons->maxW)){
				cons->cursorX+=8;
			}
			else{
				cons->cursorX=8;
				console_newline(cons);
			}
		}
	}
}

