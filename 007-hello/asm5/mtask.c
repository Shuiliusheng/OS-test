#include "bootpack.h"

#define TASK_RUNNING 3
#define TASK_SLEEP 2
#define TASK_IDLE 1

struct TASKCTL taskctl;
sTIMER *task_timer;

sTASK *task_init(struct MEM_MAN *memManager)
{
	//初始化ctl
	taskctl.running = 0;
	taskctl.now=NULL;
	taskctl.lastTask=NULL;
	taskctl.tasks=NULL;
	
	//判断是否有空闲的GDT
	uint gdtnum = alloc_gdt();
	if(gdtnum == 0){
		return 0;
	}
	
	//调用函数的程序变为task
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	sTASK *task;
	task=(sTASK *)mm_alloc(memManager, sizeof(sTASK));
	task->next=NULL;
	task->selector = gdtnum;
	task->flags=TASK_RUNNING;
	set_segmdesc(gdt + task->selector/8, 103, (int)&(task->tss), AR_TSS32);
	load_tr(task->selector);
	
	///将任务加入ctl
	taskctl.tasks=task;
	taskctl.now=task;
	taskctl.lastTask=task;
	taskctl.running++;
	
	//设定任务切换定时器
	task_timer=timer_alloc(memManager);
	timer_settime(task_timer, 2);
	
	return 0;
}

sTASK *task_alloc(struct MEM_MAN *memManager)
{
	uint gdtnum = alloc_gdt();
	if(gdtnum == 0){
		return 0;
	}
	
	//调用函数的程序变为task
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	sTASK *task;
	task=(sTASK *)mm_alloc(memManager, sizeof(sTASK));
	task->next=NULL;
	task->selector = gdtnum;
	task->flags=TASK_IDLE;
	set_segmdesc(gdt + task->selector/8, 103, (int)&(task->tss), AR_TSS32);
	
	task->tss.eflags = 0x00000202; 
	task->tss.eax = 0; 
	task->tss.ecx = 0;
	task->tss.edx = 0;
	task->tss.ebx = 0;
	task->tss.ebp = 0;
	task->tss.esi = 0;
	task->tss.edi = 0;
	task->tss.es = 0;
	task->tss.ds = 0;
	task->tss.fs = 0;
	task->tss.gs = 0;
	task->tss.ldtr = 0;
	task->tss.iomap = 0x40000000;
	
	return task;
}

void task_run(sTASK *task)
{
	task->flags=TASK_RUNNING;
	
	taskctl.running++;
	if(taskctl.lastTask!=NULL){
		sTASK *last=taskctl.lastTask;
		last->next=task;
		task->next=NULL;
		taskctl.lastTask=task;
	}
	else{
		taskctl.tasks=task;
		taskctl.now=task;
		taskctl.lastTask=task;
		task->next=NULL;
		farjmp(0, taskctl.now->selector);
	}
	return ;
}

void task_switch(void)
{
	timer_settime(task_timer, 2);
	if(taskctl.running>=2){
		taskctl.now=taskctl.now->next;
		if(taskctl.now==NULL){
			taskctl.now=taskctl.tasks;
		}
		farjmp(0, taskctl.now->selector);
	}
	return ;
}
















