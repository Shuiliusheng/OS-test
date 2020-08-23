[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api_memory.nas"]

		GLOBAL	_api_initmalloc, _api_malloc,_api_free,_api_freesize

[SECTION .text]

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
	
_api_freesize:			; uint api_freesize();
	PUSH	EBX
	MOV		EDX,11
	MOV		EBX,[CS:0x0020]
	mov 	ax, ds				; 数据段段号
	INT		0x40
	POP		EBX
	RET