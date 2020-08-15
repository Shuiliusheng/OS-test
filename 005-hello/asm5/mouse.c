#include"bootpack.h"


// 激活鼠标
void enable_mouse(sMOUSE_DEC *mouseDec)
{
	wait_KBC_sendready();
	//0xd4,下一个数据会自动发生给鼠标
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	
	//initial phase
	mouseDec->phase=0;
	return;
}

int mouse_decode(sMOUSE_DEC *mouseDec, uchar key)
{
	if(mouseDec->phase==0){
		if(key==0xfa){
			mouseDec->phase=1;
		}
		return 0;
	}
	else if(mouseDec->phase==1){
		//鼠标连线会存在接触不良的情况，导致数据丢失
		//通过该判断来防止数据丢失而导致的错位
		//0xc8: 11001000
		if((key&0xc8)==0x08){
			mouseDec->data[0]=key;
			mouseDec->phase=2;
		}
		return 0;
	}
	else if(mouseDec->phase==2){
		mouseDec->data[1]=key;
		mouseDec->phase=3;
		return 0;
	}
	else if(mouseDec->phase==3){
		mouseDec->data[2]=key;
		mouseDec->phase=1;
		
		//get mouse information
		//获取鼠标按键信息，低三位
		//低三位分别代表左键，中键，右键的信息，按下为1
		mouseDec->btn = mouseDec->data[0] & 0x07;
		
		//获取鼠标移动的信息
		mouseDec->x=mouseDec->data[1];
		mouseDec->y=mouseDec->data[2];
		
		//第一个字节的五和六位，表明是否要进行xy的处理
		if((mouseDec->data[0]&0x10)!=0){
			mouseDec->x |= 0xffffff00;
		}
		if((mouseDec->data[0]&0x20)!=0){
			mouseDec->y |= 0xffffff00;
		}
		
		
		//鼠标与屏幕的y方向相反
		mouseDec->y = - mouseDec->y;
		
		return 1;
	}
	return -1;
}

