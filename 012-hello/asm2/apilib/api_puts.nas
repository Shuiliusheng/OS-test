[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api_puts.nas"]

		GLOBAL	_api_puts

[SECTION .text]

_api_puts:	; void api_puts(char *);
	mov ecx,[esp+4]
	mov ax, ds
	mov edx,2	;putstr
	int 0x40	;
	ret			;
	
