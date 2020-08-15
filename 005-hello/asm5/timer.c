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

struct TIMERCTL timerctl;

void init_PIT(void)
{
	io_out8(PIT_CTRL,0x34);
	io_out8(PIT_CNT0,0x9c);
	io_out8(PIT_CNT0,0x2e);
	timerctl.count=0;
	return;
}

void inthandler20(int *esp)
{
	//通知PIC已经接收到该IRQ1的中断，可以继续接收中断
	//0x61=0x60+0(IRQ0)
	//OCW操作命令字组,OCW2,IMR(OCW1)
	io_out8(PIC0_OCW2, 0x60);
	timerctl.count++;
	return ;
}

