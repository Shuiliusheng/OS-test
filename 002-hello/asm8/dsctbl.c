#include"bootpack.h"
	

void init_gdtidt(void)
{
	//BOTPAK  EQU 0x00280000
	//0x270000 - 0x270000+8192没有被使用，因此用来作为GDT
	struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR *) 0x00270000;
	//0x26f800-0x26ffff也是
	struct GATE_DESCRIPTOR *idt=(struct GATE_DESCRIPTOR *) 0x0026f800;
	int i;
	
	//GDT init
	for(i=0;i<8192;i++){
		set_segmdesc(gdt+i,0,0,0);
	}
	//段号1：0-4GB，用于管理全部内存
	set_segmdesc(gdt+1,0xffffffff,0x00000000,0x4092);
	//段号2：0x280000,512KB, bootpack的区域
	set_segmdesc(gdt+2,0x0007ffff,0x00280000,0x409a);
	//给GDTR赋值，需要使用汇编
	load_gdtr(0xffff, 0x00270000);
	
	//IDT init
	for(i=0;i<256;i++){
		set_gatedesc(idt+i,0,0,0);
	}
	//0x7ff:2048
	load_idtr(0x7ff,0x0026f800);
	
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

