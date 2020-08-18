#include "bootpack.h"

#define TASK_LEVEL_NO_CHANGE 100000

struct TASKCTL taskctl;
sTIMER *task_timer;
void task_findlevel();

void task_idle(void)
{
	for(;;){
		io_hlt();
	}
}

void task_add_IdleTask(struct MEM_MAN *memManager)
{
	//判断是否有空闲的GDT
	uint gdtnum = alloc_gdt();
	if(gdtnum == 0){
		return 0;
	}
	
	//调用函数的程序变为task
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	sTASK *task;
	task=(sTASK *)mm_alloc(memManager, sizeof(sTASK));
	
	task = task_alloc(memManager);
	task->tss.esp = mm_alloc(memManager, 64 * 1024) + 64 * 1024;
	task->tss.es = 1 * 8;	
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	task->tss.eip = (int) &task_idle;
	task->tss.ss0 = 0;
	
	task->next=NULL;
	task->last=NULL;
	task->handler=0;
	task->currentDir=0;
	task->cons=0;
	
	task->selector = gdtnum;
	task->flags=TASK_RUNNING;
	task->level=9;
	task->priority=2;//切换时间为0.02
	set_segmdesc(gdt + task->selector/8, 103, (int)&(task->tss), AR_TSS32);
	
	
	///将任务加入level
	taskctl.levels[task->level].tasks=task;
	taskctl.levels[task->level].now=task;
	taskctl.levels[task->level].lastTask=task;
	taskctl.levels[task->level].running++;
}

sTASK *task_init(struct MEM_MAN *memManager)
{
	//初始化ctl
	taskctl.now_level = 0;
	taskctl.level_change=TASK_LEVEL_NO_CHANGE;
	
	int i=0;
	for(i=0;i<TASK_MAX_LEVELS;i++){
		taskctl.levels[i].running=0;
		taskctl.levels[i].now=NULL;
		taskctl.levels[i].lastTask=NULL;
		taskctl.levels[i].tasks=NULL;
	}
	
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
	task->last=NULL;
	task->handler=0;
	task->currentDir=0;
	task->cons=0;
	task->selector = gdtnum;
	task->flags=TASK_RUNNING;
	task->level=0;
	task->priority=2;//切换时间为0.02
	set_segmdesc(gdt + task->selector/8, 103, (int)&(task->tss), AR_TSS32);
	load_tr(task->selector);
	
	///将任务加入level
	taskctl.levels[task->level].tasks=task;
	taskctl.levels[task->level].now=task;
	taskctl.levels[task->level].lastTask=task;
	taskctl.levels[task->level].running++;
	
	task_add_IdleTask(memManager);
	//设定任务切换定时器
	task_timer=timer_alloc(memManager);
	timer_settime(task_timer, task->priority);
	
	return task;
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
	task->last=NULL;
	task->handler=0;
	task->currentDir=0;
	task->cons=0;
	task->level=9;
	task->priority=10;
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
	task->tss.ss0 = 0;
	
	return task;
}

int test1=0;

void task_run(sTASK *task, int level, int priority)
{
	if(level<0){
		level=task->level;
	}
	if(priority>0){
		task->priority=priority;
	}
	if(task->flags==TASK_RUNNING){
		if(task->level==level){
			return ;
		}
		else{
			task_sleep(task);
		}
	}
	
	task->flags=TASK_RUNNING;
	task->level=level;
	
	taskctl.levels[level].running++;
	sTASKLEVEL *tasklevel = &taskctl.levels[level];
	if(tasklevel->lastTask!=NULL){
		sTASK *last=tasklevel->lastTask;
		last->next=task;
		task->next=NULL;
		task->last=last;
		tasklevel->lastTask=task;
	}
	else{
		tasklevel->tasks=task;
		tasklevel->now=task;
		tasklevel->lastTask=task;
		task->next=NULL;
		task->last=NULL;
		//farjmp(0, tasklevel->now->selector);
	}
	if(taskctl.now_level>level&&taskctl.level_change>level){
		taskctl.level_change=level;
	}
	return ;
}

void task_findlevel()
{
	int i=0;
	for(i=0;i<TASK_MAX_LEVELS;i++){
		if(taskctl.levels[i].running>0){
			taskctl.now_level=i;
			taskctl.level_change=TASK_LEVEL_NO_CHANGE;
			return ;
		}
	}
}

void task_sleep(sTASK *task)
{
	uchar switch_flag=0;
	sTASKLEVEL *tasklevel=&taskctl.levels[task->level]; 
	if(task->flags==TASK_RUNNING){
		tasklevel->running--;
		//本层最后一个，需要切换到其它层
		if(tasklevel->running==0){
			task->flags=TASK_SLEEP;
			task->next=NULL;
			task->last=NULL;
			tasklevel->tasks=NULL;
			tasklevel->now=NULL;
			tasklevel->lastTask=NULL;
			
			//如果当前正在运行的层空了，则需要调整
			if(task->level==taskctl.now_level)
			{
				task_findlevel();
				int taskgdt=taskctl.levels[taskctl.now_level].now->selector;
				farjmp(0, taskgdt);
			}
			return ;
		}
		if(tasklevel->tasks==task){
			tasklevel->tasks=task->next;
			task->next->last=NULL;
			if(tasklevel->now==task){
				switch_flag=1;
				tasklevel->now=task->next;
			}
		}
		else if(tasklevel->lastTask==task){
			tasklevel->lastTask=task->last;
			task->last->next=NULL;
			if(tasklevel->now==task){
				switch_flag=1;
				tasklevel->now=tasklevel->tasks;
			}
		}
		else{
			task->last->next=task->next;
			task->next->last=task->last;
			if(tasklevel->now==task){
				switch_flag=1;
				tasklevel->now=task->next;
			}
		}
	}
	task->flags=TASK_SLEEP;
	task->next=NULL;
	task->last=NULL;
	if(switch_flag==1&&task->level==taskctl.now_level){
		farjmp(0, tasklevel->now->selector);
	}
}

void task_delete(struct MEM_MAN *man, sTASK *task)
{
	task_sleep(task);
	free_gdt(task->selector);
	mm_free(man,task);
}

void task_switch(void)
{
	sTASKLEVEL *nowlevel = &taskctl.levels[taskctl.now_level];
	sTASK *lastTask = nowlevel->now;
	if(nowlevel->now!=NULL){
		nowlevel->now=nowlevel->now->next;
		if(nowlevel->now==NULL){
			nowlevel->now=nowlevel->tasks;
		}
	}
	
	if(taskctl.level_change!=TASK_LEVEL_NO_CHANGE){
		taskctl.now_level=taskctl.level_change;
		nowlevel = &taskctl.levels[taskctl.now_level];
		taskctl.level_change=TASK_LEVEL_NO_CHANGE;
	}
	
	//防止出错
	if(nowlevel->now==NULL){
		task_findlevel();
		nowlevel = &taskctl.levels[taskctl.now_level];
	}
	
	timer_settime(task_timer, nowlevel->now->priority);
	if(lastTask!=nowlevel->now){
		farjmp(0, nowlevel->now->selector);
	}
	return ;
}


sTASK *task_now()
{
	return (taskctl.levels[taskctl.now_level]).now;
}


void releaseHandler(sTASK *task)
{
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	sHandlerInfo *info = task->handler;
	sHandlerInfo *temp;
	while(info!=0){
		if(info->type == HANDLER_MALLOC){
			mm_free(info->man, (char*)info->addr);
		}
		else if(info->type == HANDLER_FILE){
			file_close((sFileHandler *)info->addr);
			mm_free(info->man, (char*)info->addr);		}
		else{
			mm_free(info->man, (char*)info->addr);
		}
		temp = info;
		info = info->next;
		mm_free(man, (char*)temp);
	}
	task->handler=0;
}










