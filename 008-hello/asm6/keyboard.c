#include"bootpack.h"


sFIFO32 *keyfifo;
uint keydata0;
	
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
void init_keyboard(sFIFO32 *fifo, uint data0)
{
	keyfifo=fifo;
	keydata0 = data0;
	
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return ;
}

void inthandler21(int *esp)//IRQ1-int21：键盘中断处理
{
	uint data;
	//通知PIC已经接收到该IRQ1的中断，可以继续接收中断
	//0x61=0x60+1(IRQ1)
	//OCW操作命令字组,OCW2,IMR(OCW1)
	io_out8(PIC0_OCW2,0x61);
	
	//获取键盘输入
	data = io_in8(PORT_KEYDAT);
	
	fifo32_push(keyfifo,data+keydata0);
	
	return ;
}



