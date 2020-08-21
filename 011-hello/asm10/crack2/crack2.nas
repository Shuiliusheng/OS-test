[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "crack2.nas"]

		GLOBAL	_HariMain

[SECTION .text]

_HariMain:

	mov eax,1*8
	mov ds,ax
	mov byte [0x102600],0
	
	mov edx,4
	int 0x40