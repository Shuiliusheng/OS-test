; haribote-os
; TAB=4

[FORMAT "WCOFF"]	;制作目标文件的模式
[INSTRSET "i486p"]	;32位汇编
[BITS 32]			;制作32位模式用的机器语言

;制作目标文件的信息

[FILE "naskfunc.nas"]	;源文件名称信息
	global _io_hlt,_write_mem8
	global _io_cli,_io_sti,_io_stihlt
	global _io_in8,_io_in16,_io_in32
	global _io_out8,_io_out16,_io_out32
	global _io_load_eflags,_io_store_eflags
	global _load_gdtr,_load_idtr
	global _asm_inthandler21, _asm_inthandler2c, _asm_inthandler20
	global _load_cr0, _store_cr0
	global _memtest_sub
	global _load_tr
	global _farjmp
	global _farcall
	
	global _asm_cons_putchar
	extern _cons_putchar
	
	extern _inthandler21,_inthandler2c,_inthandler20	;;int.c中定义的函数
	
;具体的函数实现
[section .text]

_io_hlt:	;void io_hlt(void)
	hlt
	ret
	
_write_mem8: ;void write_mem8(int addr,int data)
	mov ecx,[esp+4]	;获取addr
	mov al,[esp+8]	;获取data，尽量使用32位寄存器，此时使用16的速度会更慢
	mov [ecx],al
	ret
	
_io_cli:
	cli
	ret
	
_io_sti:
	sti
	ret

_io_stihlt:
	sti
	hlt
	ret
	
_io_in8:
	mov edx,[esp+4]
	mov eax,0
	in al,dx		;ax作为返回值传递
	ret
	
_io_in16:
	mov edx,[esp+4]
	mov eax,0
	in ax,dx		;ax作为返回值传递
	ret

_io_in32:
	mov edx,[esp+4]
	mov eax,0
	in eax,dx		;ax作为返回值传递
	ret
	
_io_out8:
	mov edx,[esp+4]
	mov eax,[esp+8]
	out dx,al
	ret
	
_io_out16:
	mov edx,[esp+4]
	mov eax,[esp+8]
	out dx,ax
	ret
	
_io_out32:
	mov edx,[esp+4]
	mov eax,[esp+8]
	out dx,eax
	ret
	
_io_load_eflags:
	pushfd	;push eflags
	pop eax
	ret
	
_io_store_eflags:
	mov eax,[esp+4]
	push eax
	popfd ; pop eflags
	ret
	
_load_gdtr: ;void load_idtr(int limit, int addr)
	mov ax,[esp+4]	;limit,16位
	mov [esp+6],ax	;将16位的limit放到esp+6开始的两个字节，之后的四个字节是addr
	lgdt [esp+6]	;mem to gdtr，将48位放入到寄存器中
	ret
	
_load_idtr:		; void load_idtr(int limit, int addr);
	MOV		AX,[ESP+4]		; limit
	MOV		[ESP+6],AX
	LIDT	[ESP+6]
	RET

_asm_inthandler20: ;
	push es		;中断处理前保存寄存器状态
	push ds	
	pushad		;将EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI压栈
	mov eax,esp
	push eax	;栈顶压栈
	mov ax,ss
	mov ds,ax
	mov es,ax
	call _inthandler20
	pop eax
	popad
	pop ds
	pop es
	iretd

_asm_inthandler21: ;
	push es		;中断处理前保存寄存器状态
	push ds	
	pushad		;将EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI压栈
	mov eax,esp
	push eax	;栈顶压栈
	mov ax,ss
	mov ds,ax
	mov es,ax
	call _inthandler21
	pop eax
	popad
	pop ds
	pop es
	iretd		;从堆栈弹出返回的偏移和段地址，再弹出标志寄存器内容
	
_asm_inthandler2c: ;
	push es		;中断处理前保存寄存器状态
	push ds	
	pushad		;将EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI压栈
	mov eax,esp
	push eax	;栈顶压栈
	mov ax,ss
	mov ds,ax
	mov es,ax
	call _inthandler2c
	pop eax
	popad
	pop ds
	pop es
	iretd
	
_load_cr0: ; int load_cr0();
	mov eax,cr0
	ret

_store_cr0: ; void store_cr0(int )
	mov eax,[esp+4]
	mov cr0,eax
	ret

_memtest_sub: ; unsigned int memtest_sub(uint place);
	push edi
	push esi
	push ebx
	mov ebx,[esp+12+4]		;push 3 registers
	mov esi,0xaa55aa55
	mov edi,0x55aa55aa
	
	mov eax,[ebx]	; old = *(place)
	mov [ebx],esi
	XOR	DWORD [EBX],0xffffffff	; XOR
	cmp edi,[ebx]		; if (!=)
	jne	mts_failed
	XOR	DWORD [EBX],0xffffffff	; XOR
	cmp esi,[ebx]		; if (!=)
	jne	mts_failed			

	;success:	return 1
	mov [ebx],eax
	mov eax,1
	POP	ebx
	POP	esi
	POP	edi
	ret
	
_load_tr: 	;load_tr(int tr)
	LTR [esp+4]
	ret
	
	;无法直接使用far jmp
_farjmp: ;farjmp(uint eip, short cs)
	jmp far [esp+4]	;jmp far 会首先将4个字节的内容放入eip，
					;再将之后的两个字节放入cs中
	ret		;任务切换后会回到这个地方

_farcall: ;farjmp(uint eip, short cs)
	call far [esp+4]	;jmp far 会首先将4个字节的内容放入eip，
					;再将之后的两个字节放入cs中
	ret		;任务切换后会回到这个地方
	
mts_failed:		; return 0
	mov [ebx],eax
	mov eax,0
	POP	ebx
	POP	esi
	POP	edi
	ret
	
	
_asm_cons_putchar: 
	push 1			;move
	and eax,0xff	;al
	push eax		;char c
	push dword [0x0fec]	;*cons
	call _cons_putchar
	add esp,12
	retf
	
	
	
