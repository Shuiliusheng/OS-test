[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

	global _api_putchar,_api_putstr, _api_return
	global _api_initmalloc, _api_malloc, _api_free,_api_freesize
	
[SECTION .text]	
_api_putchar:	; void api_putchar(int c);
	mov eax,[esp+4]
	mov edx,1	;putchar
	int 0x40	;中断0x40，自己注册的
	ret			; reft由c语言调用者写
	
_api_putstr:	; void api_putchar(char *);
	mov ecx,[esp+4]
	mov ax, ds
	mov edx,2	;putstr
	int 0x40	;
	ret			;
	
_api_return:	; void api_putchar(int c);
	mov edx,4	;putchar
	int 0x40	;中断0x40，自己注册的
	
_api_initmalloc:	; void api_initmalloc(void);
	PUSH	EBX
	MOV		EDX,8
	; malloc的开始可用的地址
	; 包括memManager的地址
	MOV		EBX,[CS:0x0020]		
	mov 	ax, ds				; 数据段段号
	INT		0x40
	POP		EBX
	RET

_api_malloc:		; char *api_malloc(int size);
	PUSH	EBX
	MOV		EDX,9
	MOV		EBX,[CS:0x0020]
	MOV		ECX,[ESP+8]			;分配的内存大小 
	mov 	ax, ds				; 数据段段号
	INT		0x40
	POP		EBX
	RET

_api_free:			; void api_free(char *addr);
	PUSH	EBX
	MOV		EDX,10
	MOV		EBX,[CS:0x0020]
	MOV		ECX,[ESP+ 8]		; 
	mov 	ax, ds				; 数据段段号
	INT		0x40
	POP		EBX
	RET
	
_api_freesize:			; uinnt api_freesize();
	PUSH	EBX
	MOV		EDX,11
	MOV		EBX,[CS:0x0020]
	mov 	ax, ds				; 数据段段号
	INT		0x40
	POP		EBX
	RET