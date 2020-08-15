[INSTRSET "i486p"]
[BITS 32]
	mov ebx,msg
	mov edx,2
	mov eax,1
	int 0x40
	mov edx,4		;远程返回farcall
	int 0x40
msg:
	db "hello2",0