#include "bootpack.h"

//task切换定时器
sTIMER *mt_timer;
//task register
uint mt_tr;

void mt_init()
{
	struct MEM_MAN *memManager;
	memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	mt_timer = timer_alloc(memManager);
	timer_settime(mt_timer,2);
	mt_tr=3*8;
}

void mt_taskswitch(void)
{
	if(mt_tr==3*8){
		mt_tr=4*8;
	}
	else{
		mt_tr=3*8;
	}
	//先设置下一次的切换，然后再跳转
	//这是因为任务切换时独立出来的，和之前还有一点差别
	timer_settime(mt_timer,2);
	farjmp(0,mt_tr);
}