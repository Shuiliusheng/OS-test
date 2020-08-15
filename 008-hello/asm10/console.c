#include"bootpack.h"
#include<string.h>

#define BACKSPACE 8
#define ENTER 10
#define FAT_OFFSET 0x000200

//// img中文件信息的结构体，共224个
struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date;
	//代表文件从磁盘哪个扇区开始存放，按字节为单位
	//起始位置为0x003e00
	unsigned short clustno;
	unsigned int size;
};
#define sFILEINFO struct FILEINFO

int console_newline(sSHEET *sheet, int cursor_y, int startx, int starty, int maxwidth, int maxheight);
void show_string_sheet(sSHEET *sheet, char str[], int *cx1, int *cy1, int sx, int sy, int mw, int mh, int enter);
int strcmp_sub(char str[], const char substr[]);
sFILEINFO *find_file(sFILEINFO *fileinfo, char filename[]);
void file_readfat(uint *fat, uchar *img);
void file_readdata(uint *fat, char *buf, uint size, int clustno);

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
	//file alloc table,记录文件在磁盘中的位置，类似于链表
	//软盘共2880个扇区，因此为了记录下一个扇区号，需要1.5B
	//windows为了避免使用2B存储，将2*1.5B组合成3B
	uint *fat = (uchar *)mm_alloc(memManager, 2880*4);
	file_readfat(fat, (uchar *)(ADR_DISKIMG+FAT_OFFSET));
	
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
				if(key == BACKSPACE){	
					if(cursor_x>16){
						show_string_xy(sheet," ",cursor_x,cursor_y,COL8_FFFFFF,COL8_000000);
						cursor_x -= 8;	
					}
				}
				else if(key == ENTER){
					//先消除光标
					boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y+15);
					sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y+16);
					cursor_y=console_newline(sheet,cursor_y, start_x, start_y,maxwidth, maxheight);
					cmdline[(cursor_x-16)/8]='\0';
					show_string(cmdline,27);
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
					else if(strcmp_sub(cmdline,"type ")==0){
						cursor_x=8;
						char *filename=&cmdline[5], *data;
						int i=0;
						int len=strlen(filename), t='A'-'a';
						for(i=0;i<len;i++){
							if(filename[i]>='a'&&filename[i]<='z'){
								filename[i]=filename[i]+t;
							}
						}
						show_string(filename,25);
						
						sFILEINFO *info=find_file(fileinfo,filename);
						if(info!=0){
							data = (char *)mm_alloc(memManager, info->size);
							file_readdata(fat,data,info->size,info->clustno);
							str[1]='\0';
							for(i=0;i<info->size;i++){
								str[0]=data[i];
								show_string_sheet(sheet,str,&cursor_x,&cursor_y,start_x,start_y, maxwidth, maxheight,0);
							}
							mm_free(memManager,data);
							cursor_x=8;
							cursor_y=console_newline(sheet,cursor_y, start_x, start_y,maxwidth, maxheight);
						}
						else{
							show_string_sheet(sheet,"file is not exist\t1234567123456712345671234567\n1234567",&cursor_x,&cursor_y,start_x,start_y, maxwidth, maxheight,1);
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

sFILEINFO *find_file(sFILEINFO *fileinfo, char filename[])
{
	int i=0,j=0,place=0;
	char str[30];
	for(i=0;i<224;i++){
		if(fileinfo[i].name[0]==0x00){
			//表示这一段不包括文件信息
			break;
		}
		if(fileinfo[i].name[0]!=0xe5){
			//0xe5表明该文件已被删除
			//0x10: 目录；0x08:非文件信息
			place=0;
			if( (fileinfo[i].type&0x18) == 0){
				for (j = 0; j < 8; j++) {
					if(fileinfo[i].name[j]!=' '){
						str[place] = fileinfo[i].name[j];
						place++;
					}
				}
				str[place]='.';
				place++;
				for (j = 0; j < 3; j++) {
					if(fileinfo[i].ext[j]!=' '){
						str[place] = fileinfo[i].ext[j];
						place++;
					}
				}
				str[place] = '\0';
				
				//show_string(str,26+i);
				if(strcmp(filename,str)==0){
					return &fileinfo[i];
				}
			}
		}
	}
	return 0;
}

void file_readfat(uint *fat, uchar *img)
{
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}

void file_readdata(uint *fat, char *buf, uint size, int clustno)
{
	char *img = (char*)(ADR_DISKIMG + 0x003e00);
	int i=0,j=0,place=0;
	while(size>=512){
		for(i=0;i<512;i++){
			buf[place]=img[clustno*512+i];
			place++;
		}
		clustno=fat[clustno];
		size-=512;
	}
	for(i=0;i<size;i++){
		buf[place]=img[clustno*512+i];
		place++;
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
			for(x=startx;x<=(maxwidth+8);x++){
				sheet->buf[t+x]=sheet->buf[t1+x];
			}
		}
		
		for(y=cursor_y;y<=cursor_y+16;y++){
			t=y*sheet->bxsize;
			for(x=startx;x<=(maxwidth+8);x++){
				sheet->buf[t+x]=COL8_000000;
			}
		}
		sheet_refresh(sheet,startx,starty,(maxwidth+8),cursor_y+16);
	}
	return cursor_y;
}

void show_string_sheet(sSHEET *sheet, char str[], int *cx1, int *cy1, int sx, int sy, int mw, int mh, int enter)
{
	int i=0,j=0,cx=*cx1,cy=*cy1,t=0;
	int len=strlen(str);
	char s[2];
	s[1]='\0';
	for(i=0;i<len;i++){
		if(str[i]==0x0a){//换行符
			cx=8;
			cy=console_newline(sheet,cy, sx, sy, mw, mh);
		}
		else if(str[i]==0x0d){
		}
		else if(str[i]==0x09){//tab符
			t=4-((cx-8)/8)%4;
			for(j=0;j<t;j++){
				s[0]=' ';
				show_string_xy(sheet,s,cx,cy,COL8_FFFFFF,COL8_000000);
				if(cx<=(mw-8)){
					cx+=8;
				}
				else{
					cx=8;
					cy=console_newline(sheet,cy, sx, sy, mw, mh);
				}
			}
		}
		else{
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
	}
	if(cx>8&&enter==1){
		cy=console_newline(sheet,cy, sx, sy, mw, mh);
		cx=8;
	}
	*cx1=cx;
	*cy1=cy;
}

