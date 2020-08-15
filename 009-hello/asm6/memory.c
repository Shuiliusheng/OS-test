#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE 	0x60000000
	
#define BLOCK_USED 0x11
#define BLOCK_FREE 0x33

void addFreeBlock(struct MEM_MAN *man, sMEM_BLOCK *block);
void tryMergeFreeBlock(sMEM_BLOCK *block);

void initMemManger(struct MEM_MAN *man)
{
	man->freesize=0;
	man->lostsize=0;
	man->losts=0;
	man->freeblocks=NULL;
}

void setFreeRange(struct MEM_MAN *man, int addr, int size)
{
	//太小了，暂时先放弃
	//之后可以先将其和之前的block进行合并，如果合并不了再放弃
	if(size<=sizeof(sMEM_BLOCK))
		return ;
	sMEM_BLOCK *memblock=(sMEM_BLOCK *)addr;
	//used size: size 4B;flags 4B; sMEM_BLOCK*在分配时会被用作data使用, 	
	memblock->size=size - sizeof(uint)*2;
	
	//作为一个tag，当分配后设定为某个值，表示被分配了
	memblock->flags=BLOCK_FREE;	
	memblock->lastBlock=NULL;
	memblock->nextBlock=NULL;

	addFreeBlock(man, memblock);
}

void tryMergeFreeBlock(sMEM_BLOCK *block)
{
	sMEM_BLOCK *lastBlock = block->lastBlock;
	sMEM_BLOCK *nextBlock = block->nextBlock;
	uint size_self= sizeof(uint)*2;;
	
	// 向前合并
	if(lastBlock!=NULL && (uint)lastBlock+lastBlock->size+size_self == (uint)block){
		//合并到前一个
		lastBlock->size=block->size + lastBlock->size + size_self;
		lastBlock->nextBlock=block->nextBlock;
		block->nextBlock->lastBlock=lastBlock;
		
		block=lastBlock;
	}
	
	if(nextBlock!=NULL && (uint)block+block->size+size_self == (uint)nextBlock){
		
		//将后一个合并到自己
		block->size=block->size + nextBlock->size + size_self;
		block->nextBlock=nextBlock->nextBlock;
		nextBlock->nextBlock->lastBlock=block;
	}
}

void addFreeBlock(struct MEM_MAN *man, sMEM_BLOCK *block)
{
	sMEM_BLOCK *freeblock=man->freeblocks;
	man->freesize+=block->size;
	if(freeblock==NULL){
		man->freeblocks=block;
		return ;
	}
	while(freeblock!=NULL)
	{
		if((uint)block < (uint)freeblock){
			sMEM_BLOCK *temp = freeblock->lastBlock;
			
			//第一块
			if(temp == NULL){
				block->nextBlock=freeblock;
				block->lastBlock=NULL;
				man->freeblocks=block;
				freeblock->lastBlock=block;
			}
			else{
				block->lastBlock=temp;
				block->nextBlock=freeblock;
				temp->nextBlock=block;
				freeblock->lastBlock=block;
			}
			
			tryMergeFreeBlock(block);
			break;
		}
		else{
			//freeblock->next=NULL;
			if(freeblock->nextBlock==NULL){
				block->lastBlock=freeblock;
				block->nextBlock=NULL;
				freeblock->nextBlock=block;
				
				tryMergeFreeBlock(block);
				break;
			}
			else{
				freeblock=freeblock->nextBlock;
			}
		}
	}
	return ;
}


uint mm_alloc(struct MEM_MAN *man, uint bytes)
{
	if(man->freesize<bytes){
		man->losts++;
		man->lostsize+=(bytes-man->freesize);
		return 0;
	}
	
	sMEM_BLOCK *freeblock=man->freeblocks;
	sMEM_BLOCK *subopBlock = NULL;
	while(freeblock!=NULL){
		//如果当前块的大小不够，next
		if(freeblock->size<bytes){
			freeblock=freeblock->nextBlock;
		}
		//正好够分的
		else if(freeblock->size == bytes){
			uint addr = (uint)freeblock + sizeof(uint)*2;
			freeblock->flags=BLOCK_USED;//用于指明该块是被分配的块
			man->freesize-=bytes;
			return addr;
		}
		//剩余的不够再次建立一个block，不选择该block
		else if((freeblock->size - bytes) < sizeof(sMEM_BLOCK)){
			//记录，当找不到合适的块，用于分配
			if(subopBlock==NULL){
				subopBlock=freeblock;
			}
			freeblock=freeblock->nextBlock;
		}
		//剩余的大小仍旧够建立一个结构体
		else{
			//(uint)freeblock + sizeof(uint)*2 + freeblock->size - bytes - sizeof(uint)*2;
			uint addr = (uint)freeblock + freeblock->size - bytes;
			sMEM_BLOCK *allocBlock = (sMEM_BLOCK *)addr;
			allocBlock->size = bytes;
			allocBlock->flags = BLOCK_USED;
			
			man->freesize-=(freeblock->size);
			freeblock->size=freeblock->size-bytes-sizeof(uint)*2;
			man->freesize+=freeblock->size;
			return addr+sizeof(uint)*2;
		}
	}
	
	if(subopBlock!=NULL){
		uint addr = (uint)subopBlock + sizeof(uint)*2;
		uint restSize = subopBlock->size - bytes;
		subopBlock->size = bytes;
		subopBlock->flags=BLOCK_USED|(restSize<<24);//用于指明该块是被分配的块
		man->freesize-=(subopBlock->size);
		return addr;
	}
	//无法分配
	return 0;
}

#define FREE_SUCCESS 0
#define FREE_ERROR_INVALIDADDR  1

uchar mm_free(struct MEM_MAN *man, uint addr)
{
	sMEM_BLOCK *block = (sMEM_BLOCK *)(addr-sizeof(uint)*2);
	uint flags = block->flags%256;
	uint size = block->size + (block->flags>>24);
	
	if(flags!=BLOCK_USED){
		return FREE_ERROR_INVALIDADDR;
	}
	
	block->size = size;
	block->flags=BLOCK_FREE;
	char str[100];
	sprintf(str,"free: 0x%p,%d",block,size);
	show_string(str,8);
	addFreeBlock(man, block);
	return FREE_SUCCESS;
}

void freeBlockNum(struct MEM_MAN *man)
{
	uint num=0;
	sMEM_BLOCK *freeblock=man->freeblocks;
	while(freeblock!=NULL){
		num++;
		freeblock=freeblock->nextBlock;
	}
	
	char str[100];
	sprintf(str,"free blocks:%d",num);
	show_string(str, 1);
	
	freeblock=man->freeblocks;
	num=2;
	uint temp  = sizeof(uint)*2;
	while(freeblock!=NULL){
		sprintf(str,"%d: 0x%p-0x%p",num-1,freeblock,(uint)freeblock+temp+freeblock->size);
		show_string(str,num);
		num++;
		freeblock=freeblock->nextBlock;
	}
}
















//// 利用遍历的方式，判断每个内存位置是否可用，从而确定内存大小
//// 加快速度：检查每个4KB页的最后4B是否可用
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




