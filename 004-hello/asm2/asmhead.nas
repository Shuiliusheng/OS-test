; haribote-os
; TAB=4

BOTPAK  EQU 0x00280000	; bootpack的加载目标地址
DSKCAC  EQU 0x00100000	; 磁盘缓存位置
DSKCAC0 EQU 0x00008000	; ipl中加载10个柱面的位置0x8200

; boot_info的信息保存地址（目前仍未被使用的地址）
CYLS EQU 0x0ff0		; 指定扇区
LEDS EQU 0x0ff1		;
VMODE EQU 0x0ff2		;用于记录颜色的信息，颜色的位数
SCRNX EQU 0x0ff4		;分辨率X
SCRNY EQU 0x0ff6		;分辨率Y
VRAM EQU 0x0ff8		;图像缓冲区的起始地址,video ram
					;显存，用于显示画面


; set the showing picture
	ORG		0xc200	; 用于告知编译器要被放入内存的地方

	MOV		AL,0x13	; VGA图形模式，320*200*8
					; 0x12:640*480*4
	MOV		AH,0x00
	INT		0x10	;
	mov byte [VMODE],8	;保存颜色信息
	mov word [SCRNX],320
	mov word [SCRNY],200
	mov dword [VRAM],0x000a0000
	
; 用BIOS取得键盘上各个LED指示灯的状态
	mov ah,0x02	;2号功能?用
				;键盘上各特殊功能按键的状态。
				;状态放入AL寄存器中
	int 0x16	;keyboard bios
	mov [LEDS],al;
	
	
; PIC关闭一切中断
;	根据AT兼容机的规格，如果要初始化PIC
;	必须在CLI之前进行，否则有时会挂起
;	随后进行PIC的初始化(Programmable Interrupt Controller)

; 如果在切换模式的（指从实模式切换至保护模式）过程中发生了中断，
; CPU是不能停下来去处理中断的（因为处理的方式都没有切换过来），
; 所以，在这之前，需要关闭中断。
; 两句out指令用于屏蔽主PIC和从PIC的中断发送，
; CLI用于停止CPU级别的中断。
; NOP指令使得CPU空转一个时钟周期，这是
; 为了防止两句OUT连用存在的隐患（例如没有优化到位的竞争冒险）。
	mov al,0xff
	out 0x21,al ; 访问外设单独提供了in/out指令,0x21为PIC外设端口地址
	nop			;不能够立即执行下一条out指令
	out 0xa1,al ; 设置中断位（应该是的）
	cli		;CLI 禁止中断发,只能在内核模式下执行
	
	
; 为了让CPU能够访问1MB以上的内存空间，设定A20GATE
;;(增加一根地址线)
	call waitkbdout ;初始化键盘数据，启动键盘控制电路。
	
	mov al,0xd1
	out 0x64,al		;开启1M以上的内存空间
	;在x86（以及x86以后）的架构中，计算机刚启动时都必须
	;进入实模式，而为了使用这个模式，就不得不先把1MB以外
	;的内存屏蔽掉，使得此时的状态能够模拟出8086时的工作状态，
	;换句话说就是为了向下兼容，在软件级别上使得其与8086工作
	;模式相同。而这条out指令将会使得A20GATE信号线转为1状态
	;（也就是禁止其抑制状态），也就开启了大内存的支持
	
	
	call waitkbdout
	mov al,0xdf		; enable a20
	out 0x60,al		;写60h端口，写input buffer
	call waitkbdout
	
	
; 切换到保护模式
[INSTRSET "i486p"]	;; 要想使用486指令
; 之后的汇编指令将会被翻译成32位机器码

	;由于保护模式和实模式寻址方式不同，
	;要想使得段寄存器有效，就必须使用GDT
	lgdt [GDTR0]	;设定临时的GDT
	;设置了CR0之后，便正式进入保护模式
	mov eax,cr0
	and eax,0x7fffffff	; bit31设为0
	or eax,0x00000001	; bit0设为1，（为了切换到保护模式）
	mov cr0,eax
	
	;由于保护模式的机器语言会采用流水线机制，
	;会预先解释下面的指令，但是，由于刚刚转换成保护模式，
	;下面的一条语句还是按照以前的方式来解释的，
	;直接去执行会出错，所以专门加一条jmp指令是为了容错，
	;让计算机重新解释一遍后面的语句，防止奇怪的bug
	jmp pipelineflush
	
pipelineflush:
	mov ax,1*8	;
	mov ds,ax	;
	mov es,ax	;
	mov fs,ax	;80386起增加的两个辅助段寄存器,fs,gs
	mov gs,ax	;减轻ES寄存器的负担
	mov ss,ax
	
; bootpack的传送
; 把将来将要写的操作系统内核的部分加载到内存
	mov esi,bootpack ; 源地址
	mov edi,BOTPAK	 ; 传送那个目的地址
	mov ecx,512*1024/4	
	call memcpy

; 磁盘数据最终转送到它本来的位置去
; 首先从启动扇区开始
	mov esi,0x7c00
	mov edi,DSKCAC
	mov ecx,512/4
	call memcpy

; 所有剩下的
	mov esi,DSKCAC0+512
	mov edi,DSKCAC+512
	mov ecx,0
	mov cl,byte [CYLS]
	imul ecx,512*18*2/4	;从柱面数变换为字节数除以4
						; 512*扇区数*正反面
	sub ecx,512/4	;减去IPL
	call memcpy
	
; 必须由asmhead来完成的工作，至此全部完毕
;	以后交由bootpack完成

; bootpack的启动,解析系统文件头
; 对文件头进行分析，找出合适的执行切入点，
; 才可以把用C语言的入口函数加载到需要的地方
	mov ebx,BOTPAK
	mov ecx,[ebx+16]
	add ecx,3
	shr ecx,2	;ecx/=4;
	jz skip		;没有要传送的东西时
	
	mov esi,[ebx+20]
	add esi,ebx
	mov edi,[ebx+12]
	call memcpy
	
; 跳转到刚才已经加载合适的程序部分
skip:
	mov esp,[ebx+12]	;栈初始值
	jmp dword 2*8:0x0000001b
	
; 清除缓冲器数据，把键盘缓冲区中的数据转移出来，
; 并且清空，循环读取直至控制器数据为0，跳出函数体
waitkbdout:
	in al,0x64
	and al,0x02
	in al,0x60	; 空读（为了清空数据接收缓冲区中的垃圾数据）
	jnz waitkbdout ; AND的结果如果不是0，就跳到waitkbdout
	ret
	
; 将以ESI数据为首地址的内存数据传送到
; 以EDI数据为首地址的内存中，传送数据的大小是ECX中的值
memcpy:
	mov eax,[esi]
	add esi,4
	mov [edi],eax
	add edi,4
	sub ecx,1
	jnz memcpy
	ret
	

; GDT: global segment descriptor table
; 指定段的寄存器只有16位(低三位不能用)，但是在32位模式下，每一个段需要64位描述符
; 用于指明段大小，段起始地址，段权限等信息
; 为此在内存中设计一个描述表，共8192（2^13）个表项，每个表项64位
; IDT: interrupt descriptor table
; IDT中记录了0-255个中断号和中断处理程序之间的对应关系
	alignb 16
GDT0:						
	resb 8	;null selector
	DW	0xffff,0x0000,0x9200,0x00cf	; 可以读写的段（segment）32bit
	DW	0xffff,0x0000,0x9a28,0x0047	; 可以执行的段（segment）32bit（bootpack用）
	DW	0 
	
GDTR0:
	dw 8*3-1
	dd GDT0
	
	alignb 16
bootpack:
