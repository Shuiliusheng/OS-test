[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api_draw.nas"]

		GLOBAL	_api_initwindow,_api_draw_string
		global _api_draw_block,_api_draw_refresh

[SECTION .text]

; void api_initwindow(char *buf,int w, int h, int col_inv,char *title);
_api_initwindow:	
	push ebx
	push esi
	push edi
	mov ebx,[esp+16]
	mov eax,[esp+20]
	mov ecx,[esp+24]
	mov edi,[esp+28]
	mov esi,[esp+32]
	mov edx,30	;
	int 0x40	;
	pop edi
	pop esi
	pop ebx
	ret			;
	
; void api_draw_string(int sht, char *str, int x, int y, int color,int bcolor);
_api_draw_string:	
	push ebx
	push esi
	push edi
	push ebp
	mov ebx,[esp+20]
	mov edi,[esp+24]
	mov eax,[esp+28]
	mov ecx,[esp+32]
	mov esi,[esp+36]
	mov ebp,[esp+40]
	mov edx,31	;
	int 0x40	;
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret			;
	
; void api_draw_block(int sht, int x, int y, int w, int h, int color);
_api_draw_block:	
	push ebx
	push esi
	push edi
	push ebp
	mov ebx,[esp+20]
	mov eax,[esp+24]
	mov ecx,[esp+28]
	mov esi,[esp+32]
	mov edi,[esp+36]
	mov ebp,[esp+40]
	mov edx,32	;
	int 0x40	;
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret			;
	
; void api_draw_refresh(int sht, int x, int y, int w, int h);
_api_draw_refresh:	
	push ebx
	push esi
	push edi
	mov ebx,[esp+16]
	mov eax,[esp+20]
	mov ecx,[esp+24]
	mov esi,[esp+28]
	mov edi,[esp+32]
	mov edx,33	;
	int 0x40	;
	pop edi
	pop esi
	pop ebx
	ret			;
	
