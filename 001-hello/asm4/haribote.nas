; haribote-os
; TAB=4

	ORG		0xc200	; �p�����m??��v��?�������I�n��

	MOV		AL,0x13	; VGA?�`�͎��C320*200*8
					; 0x12:640*480*4
	MOV		AH,0x00
	INT		0x10	;
fin:
	HLT
	JMP		fin
