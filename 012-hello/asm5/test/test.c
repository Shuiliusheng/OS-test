#include "apilib.h"
#include <string.h>
//thread test
void testfuc1(int addr)
{
	char *buf = (char *)addr;
	int i=0;
	for(i=0;i<50;i++){
		buf[i]='0'+i%5;
	}
	buf[49]='\0';
	api_return();
}

void testfuc2(int addr)
{
	char *buf = (char *)addr;
	int i=0;
	for(i=0;i<50;i++){
		buf[i]='5'+i%5;
	}
	buf[49]='\0';
	api_return();
}

void HariMain(void)
{	
	int i;
	char buf[50];

	api_initmalloc();
	char *stack1 = api_malloc(1024);
	char *stack2 = api_malloc(1024);
	char str[50],str1[50];
	
	int thread1 = api_thread_create((int)testfuc1, stack1+1024, &str);
	int thread2 = api_thread_create((int)testfuc2, stack2+1024, &str1);
	
	api_thread_run(thread1);
	api_thread_run(thread2);
	
	for(i=0;i<50;i++){
		buf[i]='0'+i%10;
	}
	buf[49]='\0';
	
	
	while(1){
		if(api_thread_isover(thread1)==1
			&&api_thread_isover(thread2)==1){
			break;
		}
	}
	
	api_puts(buf);
	api_puts("\n");
	api_puts(str);
	api_puts("\n");
	api_puts(str1);
	api_puts("\n");
	
	api_return();
}