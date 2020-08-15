void api_putchar(int c);
void api_return();
void api_putstr(char *str);

void api_initmalloc();
char *api_malloc(unsigned int size);
void api_free(unsigned int addr);
unsigned int api_freesize();

#include<string.h>
void HariMain(void)
{
	char str[100];
	
	//init
	api_initmalloc();
	sprintf(str,"start segsize: %d\n",api_freesize());
	api_putstr(str);
	
	char *src = api_malloc(1024);
	src[0]=1;
	src[1023]=2;
	
	sprintf(str,"alloc segsize 1024 : %d,0x%p\n",api_freesize(),src);
	api_putstr(str);
	
	//alloc	
	char *temp = api_malloc(10240);
	sprintf(str,"alloc segsize 10240: %d,0x%p\n",api_freesize(),temp);
	api_putstr(str);
	
	sprintf(str,"expand correct test: %d,%d\n",src[0],src[1023]);
	api_putstr(str);
	
	//free
	api_free(temp);
	sprintf(str,"free segsize 10240: %d\n",api_freesize());
	api_putstr(str);
	
	sprintf(str,"free correct test: %d,%d\n",src[0],src[1023]);
	api_putstr(str);
	
	api_free(src);
	sprintf(str,"free segsize 1024: %d\n",api_freesize());
	api_putstr(str);
	
	api_return();
}