#include"bootpack.h"
#include<string.h>

#define THREAD_INVALID 0xffe

#define THREAD_FLAG_RUNNING 1
#define THREAD_FLAG_SLEEP 2
#define THREAD_FLAG_OVER 3

sThread *thread_create(sTASK *ftask, int app_stack, int app_eip)
{
	uint task_b_esp;
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	ftask->flag_allocRes = 1;
	sThread *thread = (sThread *)mm_alloc(memManager, sizeof(sThread));
	thread->stask = 0;
	ftask->flag_allocRes = 0;

	ftask->flag_allocRes = 1;
	thread->stask = task_alloc(memManager);
	ftask->flag_allocRes = 0;
	
	thread->ftask = ftask;
	thread->flag =  THREAD_FLAG_SLEEP;
	
	sTASK *self = thread->stask;
	ftask->flag_allocRes = 1;
	self->stack_addr = mm_alloc(memManager, 1024);
	ftask->flag_allocRes = 0;
	
	task_b_esp = self->stack_addr + 1024;
	*(int *)(task_b_esp-4)=app_eip;
	*(int *)(task_b_esp-8)=app_stack;
	*(int *)(task_b_esp-12)=(int)thread;
	self->tss.esp = task_b_esp - 16;
	self->tss.es = 1 * 8;	
	self->tss.cs = 2 * 8;
	self->tss.ss = 1 * 8;
	self->tss.ds = 1 * 8;
	self->tss.fs = 1 * 8;
	self->tss.gs = 1 * 8;
	self->tss.eip = (int) &thread_app;
	self->cons = ftask->cons;
	self->fifo = ftask->fifo;
	
	ftask->flag_allocRes = 1;
	thread->next = ftask->thread;
	ftask->thread = thread;
	ftask->flag_allocRes = 0;
	
	return thread;
}

void thread_app(sThread *thread, uint app_stack, uint app_eip)
{
	int t=0;
	sTASK *ftask = thread->ftask;
	sTASK *stask = thread->stask;
	start_app(app_eip, ftask->gdt_code, app_stack , ftask->gdt_data, &(stask->tss.esp0));
	
	thread->flag =  THREAD_FLAG_OVER;
	
	ftask->flag_allocRes = 1;
	releaseHandler(stask, HANDLER_FILE);
	releaseHandler(stask, HANDLER_TIMER_APP);
	task_sleep(stask);
	ftask->flag_allocRes = 0 ;
}


void thread_run(sThread *thread)
{
	if(thread->stask!=THREAD_INVALID){
		thread->ftask->flag_allocRes = 1;
		thread->flag =  THREAD_FLAG_RUNNING;
		task_run(thread->stask, 2, 2);
		thread->ftask->flag_allocRes = 0;
	}
}

void thread_sleep(sThread *thread)
{
	if(thread->stask!=THREAD_INVALID && thread->stask==THREAD_FLAG_RUNNING){
		thread->ftask->flag_allocRes = 1;
		thread->flag =  THREAD_FLAG_SLEEP;
		task_sleep(thread->stask);	
		thread->ftask->flag_allocRes = 0;
	}
}

int thread_isover(sThread *thread)
{
	if(thread->flag == THREAD_FLAG_OVER || 
	    thread->flag == THREAD_FLAG_SLEEP){
		return 1;
	}
	else{
		return 0;
	}
}


void thread_delete(sThread *thread)
{
	if(thread->stask==THREAD_INVALID){
		return ;
	}
	struct MEM_MAN *memManager = (struct MEM_MAN *) MEMMAN_ADDR;
	thread->ftask->flag_allocRes = 1;
	task_sleep(thread->stask);
	releaseHandler(thread->stask, HANDLER_FILE);
	releaseHandler(thread->stask, HANDLER_TIMER_APP);
	thread->ftask->flag_allocRes = 0;
	
	mm_free(memManager, thread->stask->stack_addr);
	
	thread->ftask->flag_allocRes = 1;
	thread->flag =  THREAD_FLAG_OVER;
	task_delete(memManager, thread->stask);
	thread->stask = THREAD_INVALID;
	thread->ftask->flag_allocRes = 0;
}

void releaseThreads(sTASK *task)
{
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	sThread *temp;
	while(task->thread!=0){
		task->flag_allocRes = 1;
		temp = task->thread->next;
		thread_delete(task->thread);
		
		task->flag_allocRes = 1;
		mm_free(man,task->thread);
		task->thread = temp;
		task->flag_allocRes = 0;
	}
}