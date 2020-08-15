#include"bootpack.h"

#define BACKSPACE 8
#define ENTER 10
void console_task(sSHEET *sheet, sTASK *task)
{
	struct MEM_MAN *memManager;
	uint fifobuf[128];
	memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	fifo32_init(&task->fifo, 128, fifobuf,task);
	
	sTIMER *timer1;
	timer1=timer_alloc(memManager);
	timer_init(timer1, &task->fifo, 1);
	timer_settime(timer1, 50);
	
	int key;
	char str[20];
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
					if(cursor_y<=(maxheight-16)){
						cursor_y+=16;
					}
					else{//滚动
						int x,y,t,t1;
						
						//上移
						for(y=start_y;y<=cursor_y;y++){
							t=y*sheet->bxsize;
							t1=(y+16)*sheet->bxsize;
							for(x=start_x;x<=maxwidth;x++){
								sheet->buf[t+x]=sheet->buf[t1+x];
							}
						}
						
						for(y=cursor_y;y<=cursor_y+16;y++){
							t=y*sheet->bxsize;
							for(x=start_x;x<=maxwidth;x++){
								sheet->buf[t+x]=COL8_000000;
							}
						}
						sheet_refresh(sheet,start_x,start_y,maxwidth,cursor_y+16);
					}
					sprintf(str,"cursor_y:%5d,%5d",cursor_y,maxheight);
					show_string(str,23);
					cursor_x=16;
					show_string_xy(sheet,">",8,cursor_y,COL8_FFFFFF,COL8_000000);
				}
				else{
					if(cursor_x<=(maxwidth-8)){
						str[0]=key;
						str[1]=0;
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
