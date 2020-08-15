; haribote-os
; TAB=4

	ORG		0xc200	; 用于告知??器要被?入内存的地方

	MOV		AL,0x13	; VGA?形模式，320*200*8
					; 0x12:640*480*4
	MOV		AH,0x00
	INT		0x10	;
fin:
	HLT
	JMP		fin
