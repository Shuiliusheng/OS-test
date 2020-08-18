[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

	global _api_putchar,_api_putstr, _api_return
	global _api_initmalloc, _api_malloc, _api_free,_api_freesize
	global _api_file_open, _api_file_close, _api_file_read, _api_file_write
	global _api_file_eof, _api_file_seek, _api_file_tell
	
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
	
_api_freesize:			; uint api_freesize();
	PUSH	EBX
	MOV		EDX,11
	MOV		EBX,[CS:0x0020]
	mov 	ax, ds				; 数据段段号
	INT		0x40
	POP		EBX
	RET
	
;_api_file, , , 
_api_file_open:  ;	int api_file_open(char filename[], int flag);
	push ebx
	push edi
	mov eax,[esp+12]
	mov ebx,[esp+16]
	mov di, ds
	mov edx,12	;putstr
	int 0x40	;
	pop edi
	pop ebx
	ret			;
	
_api_file_close: ; void api_file_close(int handler)
	mov edx,13
	mov eax,[esp+4]
	int 0x40
	ret
	
_api_file_read: ;int api_file_read(int handler, char *buf, int size)
	push ebx
	push edi
	mov edx,14
	mov eax,[esp+12]
	mov ebx,[esp+16]
	mov ecx,[esp+20]
	mov di,ds
	int 0x40
	pop edi
	pop ebx
	ret
	
_api_file_write: ;int api_file_write(int handler, char *buf, int size)
	push ebx
	push edi
	mov edx,15
	mov eax,[esp+12]
	mov ebx,[esp+16]
	mov ecx,[esp+20]
	mov di,ds
	int 0x40
	pop edi
	pop ebx
	ret
	
;_api_file_eof, _api_file_seek, _api_file_tell
_api_file_eof: ;int api_file_eof(int handler)
	mov edx,16
	mov eax,[esp+4]
	int 0x40
	ret
	
_api_file_seek:	;int api_file_seek(int handler, int offset, int flag)
	push ebx
	mov edx,17
	mov eax,[esp+8]
	mov ebx,[esp+12]
	mov ecx,[esp+16]
	int 0x40
	pop ebx
	ret

_api_file_tell: ; int api_file_tell(int handler)
	mov edx,18
	mov eax,[esp+4]
	int 0x40
	ret

