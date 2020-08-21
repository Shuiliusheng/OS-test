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
	task->flag_allocRes=0;
	task->gdt_code=0;
	task->gdt_data=0;
	task->os_fifo=0;
	task->shtctl=0;
	
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
	task->flag_allocRes=0;
	task->gdt_code=0;
	task->gdt_data=0;
	task->os_fifo=0;
	task->shtctl=0;
	
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
	task->flag_allocRes=0;
	task->gdt_code=0;
	task->gdt_data=0;
	task->os_fifo=0;
	task->shtctl=0;
	
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
	if(task->flags==TASK_RUNNING){
		task_sleep(task);
	}
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

sHandlerInfo *allocHandler(sTASK *task, struct MEM_MAN * memMan, uint addr, int type)
{
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	task->flag_allocRes=1;
	sHandlerInfo *handler = mm_alloc(man, sizeof(sHandlerInfo));
	handler->next = task->handler;
	task->handler = handler;
	task->flag_allocRes=0;
	
	handler->addr = addr;
	handler->man = memMan;
	handler->type = type;
	return handler;
}	

void releaseHandlerSingle(sTASK *task, sHandlerInfo *handler)
{
	if(task->handler != handler){
		return ;
	}
	if(handler->type == HANDLER_TIMER){
		timer_free(handler->man, (sTIMER *)handler->addr);
	}
	else{
		mm_free(handler->man, (char*)handler->addr);
	}
	task->flag_allocRes = 1;
	task->handler = handler->next;
	mm_free((struct MEM_MAN *) MEMMAN_ADDR, (char*)handler);
	task->flag_allocRes = 0;
}

void releaseHandler(sTASK *task, int type)
{
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	sHandlerInfo *info = task->handler;
	sHandlerInfo *temp=NULL;
	sHandlerInfo *last=NULL;
	int mm=0,hf=0,hc=0,ht=0;
	while(info!=0){
		if(info->type == type){
			if(type == HANDLER_TIMER){
				timer_free(info->man, (sTIMER *)info->addr);
				ht++;
			}
			else if(info->type == HANDLER_FILE){
				file_close((sFileHandler *)info->addr);
				mm_free(info->man, (char*)info->addr);	
				hf++;
			}
			else if(info->type == HANDLER_CONS){
				mm_free(info->man, (char*)info->addr);
				hc++;
			}
			else{
				mm_free(info->man, (char*)info->addr);
				mm++;
			}
			task->flag_allocRes = 1;
			if(last!=NULL){
				last->next = info->next;
			}
			else{
				task->handler = info->next;
			}
			task->flag_allocRes = 0;
			
			temp = info;
			info = info->next;
			mm_free(man, (char*)temp);
		}
		else{
			last = info;
			info = info->next;
		}
	}
	
	sCONSOLEINFO * cons = task->cons;
	char str[20];
	sprintf(str,"Relese MM:%5d\n",mm);
	show_string_sheet(cons, str,1);
	sprintf(str,"Relese HF:%5d\n",hf);
	show_string_sheet(cons, str,1);
	sprintf(str,"Relese HC:%5d\n",hc);
	show_string_sheet(cons, str,1);
	sprintf(str,"Relese HT:%5d\n",ht);
	show_string_sheet(cons, str,1);
}


int handlerNum()
{
	sTASK *task=task_now();
	sHandlerInfo *info = task->handler;
	int mm=0,hf=0,hc=0,ht=0;
	while(info!=0){
		if(info->type == HANDLER_TIMER){
			ht++;
		}
		else if(info->type == HANDLER_MALLOC){
			mm++;
		}
		else if(info->type == HANDLER_CONS){
			hc++;
		}
		else if(info->type == HANDLER_FILE){
			hf++;
		}
		info = info->next;
	}
	sCONSOLEINFO * cons = task->cons;
	char str[20];
	sprintf(str,"MM:%5d\n",mm);
	show_string_sheet(cons, str,1);
	sprintf(str,"HF:%5d\n",hf);
	show_string_sheet(cons, str,1);
	sprintf(str,"HC:%5d\n",hc);
	show_string_sheet(cons, str,1);
	sprintf(str,"HT:%5d\n",ht);
	show_string_sheet(cons, str,1);
}









