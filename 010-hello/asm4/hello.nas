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
	mov edx,4		;远程返回farcall
	int 0x40
	
msg:
	db "hello",0