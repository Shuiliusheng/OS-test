#include "apilib.h"
#include <string.h>

int n1=0,n2=0;

void testfuc1()
{
	int a=0;
	char buf[10];
	while(n1<100000){
		a=n2-1000;
		sprintf(buf,"%d",a);
		n1++;
	}
	api_return();
}

void testfuc2()
{
	int a=0;
	char buf[10];
	while(n2<100000){
		a=n1-1000;
		sprintf(buf,"%d",a);
		n2++;
	}
	api_return();
}

void HariMain(void)
{	
	int i;
	char buf[20];

	api_initmalloc();
	char *stack1 = api_malloc(1024);
	char *stack2 = api_malloc(1024);
	
	int thread1 = api_thread_create((int)testfuc1, stack1+1023);
	int thread2 = api_thread_create((int)testfuc2, stack2+1023);
	
	api_thread_run(thread1);
	api_thread_run(thread2);
	
	for(i=0;i<40;i++){
		sprintf(buf,"%d:%d,%d\n",i,n1,n2);
		api_puts(buf);
	}
	
	
	while(1){
		if(api_thread_isover(thread1)==1
			&&api_thread_isover(thread2)==1){
			break;
		}
	}
	
	sprintf(buf,"over:%d:%d,%d\n",i,n1,n2);
		api_puts(buf);
	
	api_return();
}