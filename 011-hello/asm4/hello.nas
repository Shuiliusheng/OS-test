[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "hello.nas"]

	GLOBAL	_HariMain
	
[SECTION .text]	

_HariMain:

	mov ecx,msg
putloop:
	mov al,[ds:ecx]
	cmp al,0
	je finish
	mov edx,1
	int 0x40	;中断0x40，自己注册的
	add ecx,1
	jmp putloop
	
finish:
	mov edx,4		;远程返回farcall
	int 0x40
	

[SECTION .data]
msg:
	db "hello,world",0