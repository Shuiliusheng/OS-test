; haribote-os
; TAB=4

; boot_info的信息保存地址（目前仍未被使用的地址）
CYLS EQU 0x0ff0		;	?定??区
LEDS EQU 0x0ff1		;
VMODE EQU 0x0ff2		;?于?色的信息，?色的位数
SCRNX EQU 0x0ff4		;分辨率X
SCRNY EQU 0x0ff6		;分辨率Y
VRAM EQU 0x0ff8		;?像?冲区的?始地址,video ram
					;?存，用于?示画面



	ORG		0xc200	; 用于告知??器要被?入内存的地方

	MOV		AL,0x13	; VGA?形模式，320*200*8
					; 0x12:640*480*4
	MOV		AH,0x00
	INT		0x10	;
	
	mov byte [VMODE],8	;???色信息
	mov word [SCRNX],320
	mov word [SCRNY],200
	mov dword [VRAM],0x000a0000
	
; 用BIOS取得??上各?LED指示灯的状?
	mov ah,0x02	;2号功能?用
				;????上各特殊功能?的状?。
				;?行后，各?特殊功能?的状?放入AL寄存器中
	int 0x16	;keyboard bios
	mov [LEDS],al;
	
	
fin:
	HLT
	JMP		fin
