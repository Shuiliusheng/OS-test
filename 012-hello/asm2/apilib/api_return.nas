[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api_return.nas"]

		GLOBAL	_api_return

[SECTION .text]

_api_return:	; ;
	mov edx,4	;putchar
	int 0x40	;中断0x40，自己注册的
	
	
