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
	
	global _asm_hrb_api, _start_app
	global _asm_inthandler0d		;异常处理
	
	extern _hrb_api
	
	extern _inthandler21,_inthandler2c,_inthandler20	;;int.c中定义的函数
	extern _inthandler0d
	
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
	mov ax,ss
	cmp ax,1*8	;判断是否是os进入中断
	jne .from_app	;不是，意味着从用户进入中断
	;os
	mov eax,esp
	push eax	;栈顶压栈，传参数
	mov ax,ss
	mov ds,ax
	mov es,ax
	call _inthandler20
	pop eax
	popad
	pop ds
	pop es
	iretd
.from_app:
	mov eax,1*8
	mov ecx,[0xfe4]	;获取os的esp
	add ecx,-8
	mov [ecx+4], ss	;将用户的ss,esp压入os的栈中
	mov [ecx], esp
	mov ds,ax		;切换ds
	mov ss,ax
	mov es,ax
	mov esp,ecx
	call _inthandler20
	pop ecx			;切换回用户程序
	pop eax			
	mov ss,ax
	mov esp,ecx
	popad
	pop ds
	pop es
	iretd
	

_asm_inthandler21: ;
	push es		;中断处理前保存寄存器状态
	push ds	
	pushad		;将EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI压栈
	mov ax,ss
	cmp ax,1*8	;判断是否是os进入中断
	jne .from_app	;不是，意味着从用户进入中断
	;os
	mov eax,esp
	push eax	;栈顶压栈，传参数
	mov ax,ss
	mov ds,ax
	mov es,ax
	call _inthandler21
	pop eax
	popad
	pop ds
	pop es
	iretd
.from_app:
	mov eax,1*8
	mov ecx,[0xfe4]	;获取os的esp
	add ecx,-8
	mov [ecx+4], ss	;将用户的ss,esp压入os的栈中
	mov [ecx], esp
	mov ds,ax		;切换ds
	mov ss,ax
	mov es,ax
	mov esp,ecx
	call _inthandler21
	pop ecx			;切换回用户程序
	pop eax			
	mov ss,ax
	mov esp,ecx
	popad
	pop ds
	pop es
	iretd	;从堆栈弹出返回的偏移和段地址，再弹出标志寄存器内容
	
_asm_inthandler2c: ;
	push es		;中断处理前保存寄存器状态
	push ds	
	pushad		;将EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI压栈
	mov ax,ss
	cmp ax,1*8	;判断是否是os进入中断
	jne .from_app	;不是，意味着从用户进入中断
	;os
	mov eax,esp
	push eax	;栈顶压栈，传参数
	mov ax,ss
	mov ds,ax
	mov es,ax
	call _inthandler2c
	pop eax
	popad
	pop ds
	pop es
	iretd
.from_app:
	mov eax,1*8
	mov ecx,[0xfe4]	;获取os的esp
	add ecx,-8
	mov [ecx+4], ss	;将用户的ss,esp压入os的栈中
	mov [ecx], esp
	mov ds,ax		;切换ds
	mov ss,ax
	mov es,ax
	mov esp,ecx
	call _inthandler2c
	pop ecx			;切换回用户程序
	pop eax			
	mov ss,ax
	mov esp,ecx
	popad
	pop ds
	pop es
	iretd
	
_asm_inthandler0d: ;
	;允许CPU处理中断(CPU在栈异常现场保护时设置了TF=IF=0即禁止了CPU处理中断)
	sti			
	push es		;中断处理前保存寄存器状态
	push ds	
	pushad		;将EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI压栈
	mov ax,ss
	cmp ax,1*8	;判断是否是os进入中断
	jne .from_app	;不是，意味着从用户进入中断
	;os
	mov eax,esp
	push eax	;栈顶压栈，传参数
	mov ax,ss
	mov ds,ax
	mov es,ax
	call _inthandler0d
	pop eax
	popad
	pop ds
	pop es
	add esp,4	;跳过栈中的异常错误码 er
	iretd
.from_app:
	mov eax,1*8
	mov ecx,[0xfe4]	;获取os的esp
	add ecx,-8
	mov [ecx+4], ss	;将用户的ss,esp压入os的栈中
	mov [ecx], esp
	mov ds,ax		;切换ds
	mov ss,ax
	mov es,ax
	mov esp,ecx
	sti
	call _inthandler0d
	cli
	cmp eax,0	;判断inthandler0d返回值
	jne .kill
	pop ecx			;切换回用户程序
	pop eax			
	mov ss,ax
	mov esp,ecx
	popad
	pop ds
	pop es
	add esp,4	;跳过栈中的异常错误码 er
	iretd
;不通过用户返回，而是直接进入os返回
;这一段start_app最后一段一样
.kill:			
	mov eax,1*8
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov fs,ax
	mov gs,ax
	mov esp,[0xfe4]
	sti
	popad		;恢复os进入应用程序时的pushad
	ret
	
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
	
	
_asm_hrb_api: 
	;最开始就禁止中断?,是不是该加cti
	push ds		;保存用户进程的ds,es
	push es
	pushad
	
	mov eax,1*8	;os
	mov ds,ax
	;需要先设定ds，才能够正常访问0xfe4
	mov ecx,[0xfe4]	;进入用户程序时，保存着os的esp
	add ecx,-40		;8个传参数的寄存器和用户的esp,ss
	mov [ecx+32], esp
	mov [ecx+36], ss
	
	;传参的pushad
	;edi, esi, ebp, esp, ebx, edx, ecx, eax
	mov edx,[esp]
	mov ebx,[esp+4]
	mov [ecx],edx	;push edi
	mov [ecx+4],ebx
	mov edx,[esp+8]
	mov ebx,[esp+12]
	mov [ecx+8],edx
	mov [ecx+12],ebx
	mov edx,[esp+16]
	mov ebx,[esp+20]
	mov [ecx+16],edx
	mov [ecx+20],ebx
	mov edx,[esp+24]
	mov ebx,[esp+28]
	mov [ecx+24],edx
	mov [ecx+28],ebx

	mov esp,ecx
	mov es,ax
	mov ss,ax
	mov fs,ax
	mov gs,ax
	;应用程序利用int中断调用时，已经将cs设置为操作系统cs了
	sti
	
	call _hrb_api
	mov ecx,[esp+32]
	mov eax,[esp+36]
	cli
	mov ss,ax
	mov esp,ecx		;切换回用户的esp
	popad
	pop es
	pop ds
	iretd		;中断返回，自动开中断
	
	
_start_app:	;void start_app(int eip, int cs, int esp, int ds);
	pushad			;保存寄存器，之后会被修改,8*4=32,esp-=32
	mov eax,[esp+36]	;eip
	mov ecx,[esp+40]	;cs
	mov edx,[esp+44]	;esp
	mov ebx,[esp+48]	;ds/es
	mov [0xfe4],esp		;保存操作系统此时的栈基址
	
	cli					;关中断，切换到用户段s
	mov es,bx
	mov ds,bx
	mov ss,bx			;stack segment
	mov fs,bx			;fs类似于es
	mov gs,bx			;gs类似于es
	mov esp,edx			;用户的栈基址
	sti
	
	;进入用户程序
	push ecx
	push eax
	call far [esp]		;call far 会esp的将4个字节的内容放入eip，
	
	mov eax,1*8			;os的数据段
	cli
	mov es,ax
	mov ds,ax
	mov ss,ax
	mov fs,ax
	mov gs,ax
	mov esp,[0xfe4]
	sti
	popad
	ret					;返回操作系统调用的地方，cmd_app
	
