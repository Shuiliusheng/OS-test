#include "bootpack.h"

#define FLAGS_OVERRUN 0x0001
void fifo8_init(sFIFO8 *fifo, int size, uchar *buf)
{
	fifo->buf=buf;
	fifo->size=size;
	fifo->flags=0;
	fifo->free=size;
	fifo->wp=0;
	fifo->rp=0;
	return ;
}	

int fifo8_push(sFIFO8 *fifo, uchar data)
{
	if(fifo->free == 0){
		fifo->flags=FLAGS_OVERRUN;
		return -1;//overflow
	}
	
	fifo->buf[fifo->wp]=data;
	fifo->wp++;
	fifo->free--;
	if(fifo->wp == fifo->size){
		fifo->wp=0;
	}
	return 0;
}

int fifo8_pop(sFIFO8 *fifo)
{
	int data;
	if(fifo->free == fifo->size){
		return -1;
	}
	data=fifo->buf[fifo->rp];
	fifo->rp++;
	fifo->free++;
	if(fifo->rp == fifo->size){
		fifo->rp=0;
	}
	return data;
}

int fifo8_status(sFIFO8 *fifo)
{
	//used size;
	return fifo->size-fifo->free;
}





