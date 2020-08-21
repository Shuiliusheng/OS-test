[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api_putchar.nas"]

		GLOBAL	_api_putchar

[SECTION .text]

_api_putchar:	; void api_putchar(int c);
	mov eax,[esp+4]
	mov edx,1	;putchar
	int 0x40	;中断0x40，自己注册的
	ret			; reft由c语言调用者写
	
