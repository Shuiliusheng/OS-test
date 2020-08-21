void api_putchar(int c);
void api_return();
void api_putstr(char *str);

void api_initmalloc();
char *api_malloc(unsigned int size);
void api_free(unsigned int addr);
unsigned int api_freesize();

//file
int api_file_open(char *filename, int flag);
void api_file_close(int handler);
int api_file_read(int handler, char *buf, int size);
int api_file_write(int handler, char *buf, int size);
int api_file_eof(int handler);
int api_file_seek(int handler, int offset, int flag);
int api_file_tell(int handler);

//input
char api_getchar(int flag);//0:not sleep; 1:sleep
int api_gets(char *str, int len);

#include<string.h>
void HariMain(void)
{	
	//init
	api_initmalloc();
	char str[100];
	char *buf = (char *)api_malloc(100);	//1kb
	int i=0,j=0;
	
	int fh=0;
	char c=0;
	for(i=0;i<5;i++){	//16K,21K,12K
		sprintf(str,"test%d.txt",i);
		fh = api_file_open(str, 'w');
		
		for(j=0;j<100;j++){
			buf[j]=i+'0';
		}
		buf[9]='\0';
		api_file_write(fh,buf,10);
		api_file_close(fh);
	}
	
	while(1){
		c=api_getchar(1);
		if(c==10){
			break;
		}
		else{
			api_putchar(c);
		}
	}
	api_putstr("\n@@@@input1 over!\n");
	
	int t=api_gets(buf,100);
	api_putstr(buf);
	
	api_return();
}