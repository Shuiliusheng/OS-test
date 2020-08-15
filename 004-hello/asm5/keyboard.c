#include"bootpack.h"

/*
	使用鼠标的前提：控制电路，鼠标本身两个装置有效
	鼠标的控制电路包含在键盘的控制电路中，如果键盘控制电路
	初始化完成，则鼠标的也会完成
*/
	
/* 等待键盘控制器8042输入缓冲器寄存器空。*/
// kbc:keyboard controller
void wait_KBC_sendready(void)
{
	//等待键盘控制电路准备完毕
	for(;;){
		/* 8042状态控制器bit[1]=0时表示其输入缓冲寄存器
		为空即已可接受输入  0x02 */
		if((io_in8(PORT_KEYSTA)&KEYSTA_SEND_NOTREADY) == 0){
			break;
		}
	}
}

/* 设置键盘控制器8042运作方式:键盘以PC兼容方式运作,在未完成
 * 当前键盘数据的接收时不接收下一个键盘输入。*/
void init_keyboard(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return ;
}


