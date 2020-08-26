#include "apilib.h"
#include <string.h>
//thread test
void testfuc1(int addr)
{
	int fd = api_file_open((char*)addr,'w');
	if(fd == 0){
		api_return();
	}
	char buf[30];
	int i=0;
	for(i=0;i<30;i++){
		buf[i]='a'+i%26;
	}
	buf[29]='\0';
	for(i=0;i<30;i++){
		api_file_write(fd,buf,30);
	}
	api_file_close(fd);
	api_puts((char*)addr);
	api_return();
}

void testfuc2(int addr)
{
	int fd = api_file_open((char*)addr,'w');
	if(fd == 0){
		api_return();
	}
	char buf[30];
	int i=0;
	for(i=0;i<30;i++){
		buf[i]='A'+i%26;
	}
	buf[29]='\0';
	for(i=0;i<30;i++){
		api_file_write(fd,buf,30);
	}
	api_file_close(fd);
	api_puts((char*)addr);
	api_return();
}

void HariMain(void)
{	
	int i;
	char buf[50];

	api_initmalloc();
	char *stack1 = api_malloc(1024);
	char *stack2 = api_malloc(1024);
	char *stack3 = api_malloc(1024);
	char *stack4 = api_malloc(1024);
	char str[50],str1[50],str2[50],str3[50];
	
	strcpy(str,"file1.txt");
	strcpy(str1,"file2.txt");
	strcpy(str2,"file3.txt");
	strcpy(str3,"file4.txt");
	int thread1 = api_thread_create((int)testfuc1, stack1+1024, str);
	int thread2 = api_thread_create((int)testfuc1, stack2+1024, str1);
	int thread3 = api_thread_create((int)testfuc2, stack3+1024, str2);
	int thread4 = api_thread_create((int)testfuc2, stack4+1024, str3);
	
	api_thread_run(thread1);
	api_thread_run(thread2);
	api_thread_run(thread3);
	api_thread_run(thread4);
	
	
	int fd = api_file_open("file5.txt",'w');
	if(fd == 0){
		api_return();
	}
	for(i=0;i<30;i++){
		buf[i]='0'+i%10;
	}
	buf[29]='\0';
	api_file_write(fd,buf,30);
	api_file_close(fd);
	
	
	while(1){
		if(api_thread_isover(thread1)==1
			&&api_thread_isover(thread2)==1
			&&api_thread_isover(thread3)==1
			&&api_thread_isover(thread4)==1){
			break;
		}
	}	
	api_return();
}