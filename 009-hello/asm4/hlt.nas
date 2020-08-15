[BITS 32]
	mov al,'A'
	call 2*8:0xDBC
fin:
	HLT
	JMP fin