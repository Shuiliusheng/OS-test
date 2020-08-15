; haribote-os
; TAB=4

; boot_info�I�M���ۑ��n���i�ڑO������g�p�I�n���j
CYLS EQU 0x0ff0		;	?��??��
LEDS EQU 0x0ff1		;
VMODE EQU 0x0ff2		;?��?�F�I�M���C?�F�I�ʐ�
SCRNX EQU 0x0ff4		;������X
SCRNY EQU 0x0ff6		;������Y
VRAM EQU 0x0ff8		;?��?�t��I?�n�n��,video ram
					;?���C�p��?�����



	ORG		0xc200	; �p�����m??��v��?�������I�n��

	MOV		AL,0x13	; VGA?�`�͎��C320*200*8
					; 0x12:640*480*4
	MOV		AH,0x00
	INT		0x10	;
	
	mov byte [VMODE],8	;???�F�M��
	mov word [SCRNX],320
	mov word [SCRNY],200
	mov dword [VRAM],0x000a0000
	
; �pBIOS�擾??��e?LED�w�����I��?
	mov ah,0x02	;2�����\?�p
				;????��e������\?�I��?�B
				;?�s�@�C�e?������\?�I��?����AL�񑶊풆
	int 0x16	;keyboard bios
	mov [LEDS],al;
	
	
fin:
	HLT
	JMP		fin
