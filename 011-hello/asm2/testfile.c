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

#include<string.h>
void HariMain(void)
{	
	//init
	api_initmalloc();
	char str[100];
	char *buf = (char *)api_malloc(100);	//1kb
	int i=0,j=0;
	
	int fh=0;
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
	
	fh = api_file_open("test0.txt", 'r');
	api_file_read(fh,buf,100);
	api_putstr(buf);
	api_file_close(fh);
	
	api_return();
}