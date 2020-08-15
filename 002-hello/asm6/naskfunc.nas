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

