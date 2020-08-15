#include "bootpack.h"
/*
PIT: programmable interval timer
通过设定PIT可以让定时器每隔多长时间产生一次中断
PIT连接着IRQ的0号，因此通过设定PIT就可以设定IRQ0的中断
间隔

IRQ0的中断周期变更：
al=0x34: out(0x43, al);
al=中断周期的低8位;out(0x40,al)
al=中断周期的高8位：out(0x40,al);

如果指定中断周期为0，会被看作设定为65536
实际中断产生频率为单位时间周期数（主频）/设定的数值
CLOCK_TICK_RATE产生的值为1193182，是8254芯片的内部振荡器频率

如果设定值为1000，则中断产生频率为1.193182KHz
如果设定值为11932（0x2e9c)则，频率为100Hz
*/

//timer
#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

#define TIMER_FLAGS_IDLE		1	
#define TIMER_FLAGS_USING		2	



struct TIMERCTL timerctl;

void init_PIT(void)
{
	io_out8(PIT_CTRL,0x34);
	io_out8(PIT_CNT0,0x9c);
	io_out8(PIT_CNT0,0x2e);
	timerctl.count=0;
	timerctl.next = 0xffffffff;//用于确定下一个时间点
	timerctl.using = 0;	//使用了多少个定时器
	timerctl.timer=NULL;
	return;
}

void timer_free(struct MEM_MAN *man, sTIMER *timer)
{
	if(timer->flags == TIMER_FLAGS_USING){
		if(timer->last==NULL){
			timerctl.timer=timer->next;
			timer->next->last=NULL;
			if(timer->next==NULL){
				timerctl.next=0xffffffff;
			}
			else{
				timerctl.next = timerctl.timer->timeout;
			}
		}
		else{
			timer->last->next=timer->next;
			timer->next->last=timer->last;
		}
	}	
	mm_free(man,timer);
}

sTIMER *timer_alloc(struct MEM_MAN *man)
{
	sTIMER *timer;
	timer=(sTIMER *)mm_alloc(man, sizeof(sTIMER));
	timer->flags = TIMER_FLAGS_IDLE;
	return timer;
}

void timer_init(sTIMER *timer, sFIFO32 *fifo, uint data)
{
	timer->fifo=fifo;
	timer->data=data;
}

void timer_settime(sTIMER *timer, uint timeout)
{
	uint e,i,j;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	
	e = io_load_eflags();
	io_cli();
	
	sTIMER *firstTimer=timerctl.timer;
	if(firstTimer==NULL){
		timerctl.timer=timer;
		timer->last=NULL;
		timer->next=NULL;
	}
	else{
		while(firstTimer!=NULL){
			if(timer->timeout < firstTimer->timeout){
				if(firstTimer->last==NULL){
					timerctl.timer=timer;
					timer->last=NULL;
					timer->next=firstTimer;
					firstTimer->last=timer;
				}
				else{
					timer->last=firstTimer->last;
					timer->next=firstTimer;
					firstTimer->last->next=timer;
					firstTimer->last=timer;
					
				}
				break;
			}
			
			if(firstTimer->next==NULL){
				timer->last=firstTimer;
				timer->next=NULL;
				firstTimer->next=timer;
				break;
			}
			firstTimer=firstTimer->next;
		}
	}
	timerctl.next = timerctl.timer->timeout;
	
	io_store_eflags(e);
	return ;
}

extern sTIMER *task_timer;

void inthandler20(int *esp)
{
	//通知PIC已经接收到该IRQ0的中断，可以继续接收中断
	io_out8(PIC0_OCW2, 0x60);//0x61=0x60+0(IRQ0)
	timerctl.count++;
	
	if(timerctl.next > timerctl.count){
		return ;
	}
	
	sTIMER *firstTimer=timerctl.timer;
	uint switchFlag=0;
	while(firstTimer!=NULL){
		if(firstTimer->timeout > timerctl.count){
			break;
		}
		
		firstTimer->flags=TIMER_FLAGS_IDLE;
		if(firstTimer!=task_timer){
			fifo32_push(firstTimer->fifo, firstTimer->data);
		}
		else{
			switchFlag=1;
		}
		firstTimer=firstTimer->next;
	}
	
	timerctl.timer=firstTimer;
	if(firstTimer==NULL){
		timerctl.next = 0xffffffff;
	}
	else{
		timerctl.next = firstTimer->timeout;
		firstTimer->last=NULL;
	}
	
	//需要在定时器都处理完成之后再切换，
	//避免切换后修改定时器导致出错
	if(switchFlag==1){
		task_switch();
	}
	
	return ;
}

int timer_number()
{
	int n=0;
	sTIMER *firstTimer=timerctl.timer;
	while(firstTimer!=NULL){
		n++;
		firstTimer=firstTimer->next;
	}
	return n;
}
