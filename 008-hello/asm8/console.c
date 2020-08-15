#include"bootpack.h"
#include<string.h>

#define BACKSPACE 8
#define ENTER 10

//// img中文件信息的结构体，共224个
struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};
#define sFILEINFO struct FILEINFO

int console_newline(sSHEET *sheet, int cursor_y, int startx, int starty, int maxwidth, int maxheight);
void show_string_sheet(sSHEET *sheet, char str[], int *cx1, int *cy1, int sx, int sy, int mw, int mh, int enter);

void console_task(sSHEET *sheet, sTASK *task, int memtotal)
{
	struct MEM_MAN *memManager;
	uint fifobuf[128];
	memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	fifo32_init(&task->fifo, 128, fifobuf,task);
	
	sTIMER *timer1;
	timer1=timer_alloc(memManager);
	timer_init(timer1, &task->fifo, 1);
	timer_settime(timer1, 50);
	
	////从这个地方开始放文件结构体信息
	sFILEINFO *fileinfo = (sFILEINFO *)(ADR_DISKIMG+0x002600);
	
	int key,i=0;
	char str[32], cmdline[128];
	int start_x = 8, start_y = 28;
	int cursor_x = 16, cursor_y = 28, cursor_color = COL8_000000;
	int maxheight = sheet->bysize - 10, maxwidth = sheet->bxsize - 16;
	show_string_xy(sheet,">",8,cursor_y,COL8_FFFFFF,COL8_000000);
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
					if(cursor_color!=-1){
						cursor_color = COL8_000000;
					}
				} else {
					timer_init(timer1, &task->fifo, 1); 
					if(cursor_color!=-1){
						cursor_color = COL8_FFFFFF;
					}
				}
				timer_settime(timer1, 50);
			}
			else if(key==3){//停止闪烁
				cursor_color=-1;
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y+15);
				sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y+16);
			}
			else if(key==4){//开始闪烁
				cursor_color=COL8_000000;
			}
			else if(key>=256&&key<=511){
				key-=256;
				if(key == BACKSPACE && cursor_x>16){		
					show_string_xy(sheet," ",cursor_x,cursor_y,COL8_FFFFFF,COL8_000000);
					cursor_x -= 8;	
				}
				else if(key == ENTER){
					//先消除光标
					boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y+15);
					sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y+16);
					cursor_y=console_newline(sheet,cursor_y, start_x, start_y,maxwidth, maxheight);
					cmdline[(cursor_x-16)/8]='\0';
					
					if(strcmp(cmdline,"mem")==0){
						cursor_x=8;
						sprintf(str,"total:%d MB",memtotal/1024/1024);
						show_string_sheet(sheet,str,&cursor_x,&cursor_y,start_x,start_y, maxwidth, maxheight,1);
						sprintf(str,"free:%d KB",memManager->freesize/1024);
						show_string_sheet(sheet,str,&cursor_x,&cursor_y,start_x,start_y, maxwidth, maxheight,1);
					}
					else if(strcmp(cmdline,"cls")==0){
						boxfill8(sheet->buf, sheet->bxsize, COL8_000000, start_x, start_y, maxwidth, maxheight);
						sheet_refresh(sheet, start_x, start_y, maxwidth, maxheight);
						cursor_y=start_y;
						cursor_x=16;
						show_string_xy(sheet,">",8,cursor_y,COL8_FFFFFF,COL8_000000);
					}
					else if(strcmp(cmdline,"dir")==0){
						cursor_x=8;
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
									show_string_sheet(sheet,str,&cursor_x,&cursor_y,start_x,start_y, maxwidth, maxheight,1);
								}
							}
						}
					}
					else if(strlen(cmdline)>0){
						cursor_x=8;
						show_string_xy(sheet,"bad command",cursor_x,cursor_y,COL8_FFFFFF,COL8_000000);
						cursor_y=console_newline(sheet,cursor_y, start_x, start_y,maxwidth, maxheight);
					}
					cursor_x=16;
					show_string_xy(sheet,">",8,cursor_y,COL8_FFFFFF,COL8_000000);
				}
				else{
					if(cursor_x<=(maxwidth-8)){
						str[0]=key;
						str[1]=0;
						cmdline[(cursor_x-16)/8]=key;
						show_string_xy(sheet,str,cursor_x,cursor_y,COL8_FFFFFF,COL8_000000);
						cursor_x += 8;
					}
				}
			}
			if(cursor_color!=-1){
				boxfill8(sheet->buf, sheet->bxsize, cursor_color, cursor_x, cursor_y, cursor_x + 7, cursor_y+15);
				sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y+16);
			}
		}
	}
}


int console_newline(sSHEET *sheet, int cursor_y, int startx, int starty, int maxwidth, int maxheight)
{
	if(cursor_y<=(maxheight-16)){
		cursor_y+=16;
	}
	else{//滚动
		int x,y,t,t1;
		
		//上移
		for(y=starty;y<=cursor_y;y++){
			t=y*sheet->bxsize;
			t1=(y+16)*sheet->bxsize;
			for(x=startx;x<=maxwidth;x++){
				sheet->buf[t+x]=sheet->buf[t1+x];
			}
		}
		
		for(y=cursor_y;y<=cursor_y+16;y++){
			t=y*sheet->bxsize;
			for(x=startx;x<=maxwidth;x++){
				sheet->buf[t+x]=COL8_000000;
			}
		}
		sheet_refresh(sheet,startx,starty,maxwidth,cursor_y+16);
	}
	return cursor_y;
}

void show_string_sheet(sSHEET *sheet, char str[], int *cx1, int *cy1, int sx, int sy, int mw, int mh, int enter)
{
	int i=0,cx=*cx1,cy=*cy1;
	int len=strlen(str);
	char s[2];
	s[1]='\0';
	for(i=0;i<len;i++){
		s[0]=str[i];
		show_string_xy(sheet,s,cx,cy,COL8_FFFFFF,COL8_000000);
		if(cx<=(mw-8)){
			cx+=8;
		}
		else{
			cx=8;
			cy=console_newline(sheet,cy, sx, sy, mw, mh);
		}
	}
	if(cx>8&&enter==1){
		cy=console_newline(sheet,cy, sx, sy, mw, mh);
		cx=8;
	}
	*cx1=cx;
	*cy1=cy;
}
