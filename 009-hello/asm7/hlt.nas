[INSTRSET "i486p"]
[BITS 32]
	mov ecx,msg
putloop:
	mov al,[cs:ecx]
	cmp al,0
	je finish
	mov edx,1
	int 0x40	;中断0x40，自己注册的
	add ecx,1
	jmp putloop
	
finish:
	retf		;远程返回farcall
	
msg:
	db "hello",0