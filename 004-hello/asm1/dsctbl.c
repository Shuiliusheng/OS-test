#include"bootpack.h"
	

void init_gdtidt(void)
{
	//BOTPAK  EQU 0x00280000
	//0x270000 - 0x270000+8192没有被使用，因此用来作为GDT
	struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	//0x26f800-0x26ffff也是
	struct GATE_DESCRIPTOR *idt=(struct GATE_DESCRIPTOR *) ADR_IDT;
	int i;
	
	//GDT init
	for(i=0;i<8192;i++){
		set_segmdesc(gdt+i,0,0,0);
	}
	//段号1：0-4GB，用于管理全部内存
	set_segmdesc(gdt+1,0xffffffff,0x00000000,AR_DATA32_RW);
	//段号2：0x280000,512KB, bootpack的区域
	set_segmdesc(gdt+2,LIMIT_BOTPAK,ADR_BOTPAK,AR_CODE32_ER);
	//给GDTR赋值，需要使用汇编
	load_gdtr(LIMIT_GDT, ADR_GDT);
	
	//IDT init
	for(i=0;i<256;i++){
		set_gatedesc(idt+i,0,0,0);
	}
	//0x7ff:2048
	load_idtr(LIMIT_IDT,ADR_IDT);
	
	
	//将中断处理程序注册到IDT中
	//2*8表示asm的程序属于段号2的那一段，*8因为低三位有其他含义
	//段号为2的段在上面定义了，是bootpack的段
	//AR_INTGATE32： 0x8e,表示这是用于中断处理的有效设定
	set_gatedesc(idt+0x21,(int) asm_inthandler21, 2*8, AR_INTGATE32);
	set_gatedesc(idt+0x2c,(int) asm_inthandler2c, 2*8, AR_INTGATE32);
	
	
	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	// > 1GB
	
	//当Gbit为1时，limit的单位是page，否则是B
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;	//4KB
	}
	
	//分成三段是为了兼容80286的程序
	sd->base_low     = base & 0xffff;	//基址低16位
	sd->base_mid     = (base >> 16) & 0xff;//基址17-24位
	sd->base_high    = (base >> 24) & 0xff;//基址25-32位

	sd->limit_low    = limit & 0xffff;	//大小低16位
	//limit的高16位中的低8位
	//((ar >> 8) & 0xf0): ar的9-16位，并且9-12设置为0
	//高四位作为段属性使用和剩余四位组成12位段属性
	//高四位代表的属性：GD00,Gbit；D指段模式，为1表示32位模式，为0则为16位模式
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	
	//0x92：系统专用，可读写，不执行
	//0x9a：系统专用，可读不可写，可执行
	//0xf2：应用程序
	//0xfa：应用程序
	sd->access_right = ar & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	
	return;
}

