#include "apilib.h"
#include <string.h>

#define MOUSE_EVENT_LEFTDOWN 1
#define MOUSE_EVENT_LEFTUP 2
#define MOUSE_EVENT_RIGHTDOWN 3
#define MOUSE_EVENT_RIGHTUP 4
#define MOUSE_EVENT_MOVE 5

struct MOUSE_EVENT{
	short x, y;
	int type;
};

char *buf;
char *map;
char input[20];
int index;
int sht;
int bufw;
int bufh;

int show_x1, show_x2;
int result;
int caltype;

int str2int(char str[])
{
	int len = strlen(str);
	int i=0;
	int num=0,flag=1;
	for(i=0;i<len;i++){
		if(str[i]=='-'){
			flag=-1;
		}
		else if(str[i]<='9'&&str[i]>='0'){
			num = num*10+(str[i]-'0');
		}
		else{
			break;
		}
	}
	return num*flag;
}

void cal()
{
	int value = str2int(input);
	if(caltype == 11||caltype == 0){
		result+=value;
	}
	else if(caltype == 12){
		result-=value;
	}
	else if(caltype == 13){
		result*=value;
	}
	else if(caltype == 14){
		result/=value;
	}
	sprintf(input,"%d ",result);
	index  = strlen(input);
}

void show_input(char str[], int line)
{
	int len = strlen(str);
	int x = 170-len*8;
	if(line == 1){
		api_draw_block(sht, show_x1, 50, 171-show_x1, 20, 0);
		show_x1 = x;
		api_draw_string(sht, str, x, 50, 1, 0);
	}
	else{
		api_draw_block(sht, show_x2, 70, 171-show_x2, 20, 0);
		show_x2 = x;
		api_draw_string(sht, str, x, 70, 1, 0);
	}
}

void listen_event(struct MOUSE_EVENT event)
{
	char str[10];
	int t1=0;
	if(event.type == MOUSE_EVENT_LEFTDOWN){
		if(event.x>=0&&event.y>=0&&event.x<bufw&&event.y<bufh){
			t1 = event.y*bufw+event.x;
			if(map[t1]>=1&&map[t1]<=10){
				input[index]=(map[t1]%10)+'0';
				input[index+1]='\0';
				index ++ ;
				show_input(input,2);
			}
			else if(map[t1] == 11){
				cal(input);
				caltype = map[t1];
				input[index]='+';
				input[index+1]='\0';
				show_input(input,1);
				index = 0;
				show_input(" ",2);
			}
			else if(map[t1] == 12){
				cal(input);
				caltype = map[t1];
				input[index]='-';
				input[index+1]='\0';
				show_input(input,1);
				index = 0;
				show_input(" ",2);
			}
			else if(map[t1] == 13){
				cal(input);
				caltype = map[t1];
				input[index]='*';
				input[index+1]='\0';
				show_input(input,1);
				index = 0;
				show_input(" ",2);
			}
			else if(map[t1] == 14){
				cal(input);
				caltype = map[t1];
				input[index]='/';
				input[index+1]='\0';
				show_input(input,1);
				index = 0;
				show_input(" ",2);
			}
			else if(map[t1] == 15){
				index -= 1;
				if(index<0){index = 0;}
				input[index]='\0';
				show_input(input,2);
			}
			else if(map[t1] == 16){
				cal(input);
				caltype = map[t1];
				input[index]='=';
				input[index+1]='\0';
				show_input(input,1);
				index = 0;
				show_input(" ",2);
			}
		}
	}
	api_return();
}

void createButton(int x, int y, int w, int h, char *text, int value)
{
	api_draw_block(sht, x, y, w, h, 14);
	
	int len = w/8;
	if(strlen(text)>len){
		text[len]='\0';
	}
	api_draw_string(sht, text, x, y, 0, 14);
	
	int i=0,j=0;
	for(i=y;i<y+h;i++){
		for(j=x;j<x+w;j++){
			map[i*bufw+j] = value;
		}
	}
}


void HariMain(void)
{	
	api_initmalloc();
	bufw = 200;
	bufh = 270;
	index = 0;
	show_x1 = 20;
	show_x2 = 20;
	result = 0;
	caltype = 0;
	buf = api_malloc(bufw*bufh);
	map = api_malloc(bufw*bufh);
	sht = api_initwindow(buf,bufw,bufh,-1,"test");
	int i=0,j=0;
	for(i=0;i<bufh;i++){
		for(j=0;j<bufw;j++){
			map[i*bufw+j] = 0;
		}
	}
	createButton(20, 100, 30, 30, "1", 1);
	createButton(60, 100, 30, 30, "2", 2);
	createButton(100, 100, 30, 30, "3", 3);
	createButton(140, 100, 30, 30, "+", 11);
	
	createButton(20, 140, 30, 30, "4", 4);
	createButton(60, 140, 30, 30, "5", 5);
	createButton(100, 140, 30, 30, "6", 6);
	createButton(140, 140, 30, 30, "-", 12);
	
	createButton(20, 180, 30, 30, "7", 7);
	createButton(60, 180, 30, 30, "8", 8);
	createButton(100, 180, 30, 30, "9", 9);
	createButton(140, 180, 30, 30, "*", 13);
	
	createButton(20, 220, 30, 30, "0", 10);
	createButton(60, 220, 30, 30, "CE", 15);
	createButton(100, 220, 30, 30, "=", 16);
	createButton(140, 220, 30, 30, "/", 14);
	
	
	char *stack = api_malloc(1024);
	int te = api_thread_listen((int)&listen_event, stack+1024);
	
	while(api_getchar(1)!='c');
	api_thread_delete(te);
	api_return();
}