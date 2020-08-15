/* int.c, 可编程中断控制器(PIC)8259A初始化程序接口 */

#include "bootpack.h"
#include <stdio.h>

/* 两片8259A中断控制器级联时,
 * 8259A-2(从片，PIC1)连在8259A-1(主片，PIC0)的IR2引脚上,
 * 8259A-1与8259A-2共用IR0-IR15分别接收与其相
 * 连设备的中断输入。
 * IRQ0 - 日时钟(8254)
 * IRQ1 - 键盘(8042)	第二个
 * IRQ2 - 连接8258A-2
 * IRQ3 - 串口2(8250 etc.)
 * IRQ4 - 串口1
 * IRQ5 - 并口2
 * IRQ6 - 软盘
 * IRQ7 - 并口1
 * 
 * IRQ8 - 实时钟(CMOS RAM)
 * IRQ9 - 顶替IRQ2
 * IRQ12 - PS2鼠标			第五个，
 * IRQ13 - 协处理器
 * IRQ14 - 硬盘
 * IRQ10,11,15 - 暂保留未用
 *
 * 经init_pic配置后,
 * [IR0, IRQ15]中断向量号[0x20, 0x2f]对应。
 * 即在开启8259A中断和设置IDT后,在允许CPU处理当
 * 前任务中断前提下,与IR0-IRQ15相连设备分别向引
 * 脚IR0-IR15发出中断时,配置在IDT[20h..2fh]中的
 * 程序将会被CPU执行。*/
 
/* init_pic,
 * 初始化配置可编程中断控制器(PIC),
 * 为PIC分配中断向量[0x20, 0x2f],暂屏蔽所有中断。*/
void init_pic(void)
{
    /* 往8259A-1和8259A-2写OCW1,先让8259A屏蔽所有中断输入。
     * 同 asmhead.nas 中语句,由于目前还未对8259A下发过ICW
     * 命令组,此语句应该无效。*/
	 
	 //IMR：interrupt mask register
	 //如果某个IRQ没有连接设备，需要屏蔽该路信号，防止静电干扰等问题
	 //对中断进行设置时，也需要屏蔽中断，防止接收到中断引发混乱
	io_out8(PIC0_IMR,  0xff  );	//禁止所有中断
    io_out8(PIC1_IMR,  0xff  );	//禁止所有中断

    /* 向8259A-1下发
     * ICW1,边缘触发中断,多片级联,会下发ICW4;
     * ICW2,8259A-1中断号为[0x20,0x27];
     * ICW3,IR2连接从片;
     * ICW4,16位微机处理方式。*/
	 //ICW：initial control word
	 //ICW3用于设定主-从连接
    io_out8(PIC0_ICW1, 0x11  );
	//中断发生后，PIC直接根据该信息发送中断指令和中断号
    io_out8(PIC0_ICW2, 0x20  );	//IRQ0-7由INT20-27接收
    io_out8(PIC0_ICW3, 1 << 2);	//PIC1由IRQ2连接
    io_out8(PIC0_ICW4, 0x01  ); //无缓冲区模式

    /* 向8259A-2下发
     * ICW1,边缘触发中断,多片级联,会下发ICW4;
     * ICW2,8259A-1中断号为[0x28,0x2f];
     * ICW3,从片中断级为2;
     * ICW4,16位微机处理方式。*/
    io_out8(PIC1_ICW1, 0x11  );
    io_out8(PIC1_ICW2, 0x28  );
    io_out8(PIC1_ICW3, 2     );
    io_out8(PIC1_ICW4, 0x01  );

    /* 向8258A-1写OCW1=0xfb允许IRQ2中断即接收从片中断,
     * 向8259A-2写OCW1=0xff即屏蔽从片所有中断。*/
    io_out8(PIC0_IMR,  0xfb  );
    io_out8(PIC1_IMR,  0xff  );

    return;
}	

