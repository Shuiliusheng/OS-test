; haribote-os
; TAB=4

; boot_infoIM§Û¶n¬iÚO¹¢ígpIn¬j
CYLS EQU 0x0ff0		;	?è??æ
LEDS EQU 0x0ff1		;
VMODE EQU 0x0ff2		;?°?FIM§C?FIÊ
SCRNX EQU 0x0ff4		;ª¦X
SCRNY EQU 0x0ff6		;ª¦Y
VRAM EQU 0x0ff8		;??tæI?nn¬,video ram
					;?¶Cp°?¦æÊ



	ORG		0xc200	; p°m??íví?üà¶Inû

	MOV		AL,0x13	; VGA?`Í®C320*200*8
					; 0x12:640*480*4
	MOV		AH,0x00
	INT		0x10	;
	
	mov byte [VMODE],8	;???FM§
	mov word [SCRNX],320
	mov word [SCRNY],200
	mov dword [VRAM],0x000a0000
	
; pBIOSæ¾??ãe?LEDw¦Ió?
	mov ah,0x02	;2÷\?p
				;????ãeÁê÷\?Ió?B
				;?s@Ce?Áê÷\?Ió?úüALñ¶í
	int 0x16	;keyboard bios
	mov [LEDS],al;
	
	
fin:
	HLT
	JMP		fin
