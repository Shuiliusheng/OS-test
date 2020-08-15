#include"bootpack.h"

#define BACKSPACE 8

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
	int cursor_x = 16, cursor_color = COL8_000000;
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
					cursor_color = COL8_000000;
				} else {
					timer_init(timer1, &task->fifo, 1); 
					cursor_color = COL8_FFFFFF;
				}
				timer_settime(timer1, 50);
			}
			
			if(key>=256&&key<=511){
				key-=256;
				if(key == BACKSPACE && cursor_x>16){		
					show_string_xy(sheet," ",cursor_x,28,COL8_FFFFFF,COL8_000000);
					cursor_x -= 8;	
				}
				else{
					str[0]=key-256;
					str[1]=0;
					show_string_xy(sheet,str,cursor_x,28,COL8_FFFFFF,COL8_000000);
					cursor_x += 8;
				}
			}
			boxfill8(sheet->buf, sheet->bxsize, cursor_color, cursor_x, 28, cursor_x + 7, 43);
			sheet_refresh(sheet, cursor_x, 28, cursor_x + 8, 44);
		}
	}
}
