[INSTRSET "i486p"]
[BITS 32]
	mov ebx,msg
	mov edx,2
	mov eax,1
	int 0x40
	retf		;远程返回farcall
	
msg:
	db "hello2",0