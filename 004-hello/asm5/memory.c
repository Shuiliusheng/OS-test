#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE 	0x60000000


//// 利用遍历的方式，判断每个内存位置是否可用，从而确定内存大小
/// 加快速度：检查每个4KB页的最后4B是否可用
// 可用的标准：写入再读出是一样的
// 在进行这样的检查前，需要保证cache不可用，数据直接放入mem中
unsigned int memtest(unsigned int start, unsigned int end)
{
	unsigned int eflags,cr0,size=0;
	char flag486=0;
	
	/* 确认CPU是386还是486，因为386没有cache*/
	// eflags的18位为AC位，如果是386，该位为零，写入1后再读出也是0
	eflags = io_load_eflags();
	eflags |= EFLAGS_AC_BIT;
	io_store_eflags(eflags);
	eflags = io_load_eflags();
	if((eflags&EFLAGS_AC_BIT)==1){
		flag486=1;
	}
	
	if(flag486==1){
		cr0=load_cr0();
		cr0|=CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	
	for(size=start;size<=end;size+=4096)//0x1000
	{
		if(memtest_sub(size+4092)==0)//0xffc
			break;
	}
	
	//size=memtest_sub(start,end);
	
	if(flag486==1){
		cr0=load_cr0();
		cr0&=~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	return size;
}


/*
编译的时候会把循环中的代码优化掉，因为都是在做没有意义的操作
因此选择直接使用汇编语言编写
unsigned int memtest_sub(unsigned int start, unsigned int end)
{
	unsigned int i=0,old=0,pat0=0xaa55aa55,pat1=0x55aa55aa;
	unsigned int *p;
	
	for(i=start;i<=end;i+=4096)//0x1000
	{
		//确定大小边界，因此选择最后4B
		p=(unsigned int *)(i+4092);//0xffc
		old=p;
		
		*p=pat0;
		*p ^= 0xffffffff;	//反转
		if(*p != pat1){
			*p=old;
			break;
		}
		
		*p ^= 0xffffffff;
		if(*p != pat0){
			*p=old;
			break;
		}
		*p=old;
	}
	return ;
}

*/
