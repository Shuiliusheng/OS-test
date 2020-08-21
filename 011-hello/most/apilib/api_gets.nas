[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api_gets.nas"]

		GLOBAL	_api_gets

[SECTION .text]

_api_gets:	; int api_gets(char *str,int len)
	push edi
	mov eax,[esp+8]
	mov ecx,[esp+12]
	mov di,ds
	mov edx,20
	int 0x40
	pop edi
	ret
	
