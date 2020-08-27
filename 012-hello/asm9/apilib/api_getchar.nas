[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api_getchar.nas"]

		GLOBAL	_api_getchar

[SECTION .text]

_api_getchar:	; char api_getchar(char mode)
	mov eax,[esp+4]
	and eax,0x01
	mov edx,19
	int 0x40
	ret
	
