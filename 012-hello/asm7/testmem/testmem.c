#include "apilib.h"

#include<string.h>
void HariMain(void)
{
	char str[100];
	
	//init
	api_initmalloc();
	sprintf(str,"start segsize: %d\n",api_freesize());
	api_puts(str);
	
	char *src = api_malloc(1024);
	src[0]=1;
	src[1023]=2;
	
	sprintf(str,"alloc segsize 1024 : %d,0x%p\n",api_freesize(),src);
	api_puts(str);
	
	//alloc	
	char *temp = api_malloc(10240);
	sprintf(str,"alloc segsize 10240: %d,0x%p\n",api_freesize(),temp);
	api_puts(str);
	
	sprintf(str,"expand correct test: %d,%d\n",src[0],src[1023]);
	api_puts(str);
	
	//free
	api_free(temp);
	sprintf(str,"free segsize 10240: %d\n",api_freesize());
	api_puts(str);
	
	sprintf(str,"free correct test: %d,%d\n",src[0],src[1023]);
	api_puts(str);
	
	api_free(src);
	sprintf(str,"free segsize 1024: %d\n",api_freesize());
	api_puts(str);
	
	api_return();
}