[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api_timer.nas"]

		GLOBAL	_api_allocTimer, _api_initTimer,_api_setTimer,_api_freeTimer

[SECTION .text]

_api_allocTimer:	; int api_allocTimer(void);
	mov edx,21
	INT		0x40
	RET

_api_initTimer:		; void api_initTimer(int timer, int value);
	mov eax,[esp+4]
	mov ecx,[esp+8]
	mov edx,22
	int 0x40
	ret

_api_setTimer:			; void api_setTimer(int timer, int msec);
	mov eax,[esp+4]
	mov ecx,[esp+8]
	mov edx,23
	int 0x40
	ret
	
_api_freeTimer:			; void api_freeTimer(int timer);
	mov eax,[esp+4]
	mov edx,24
	int 0x40
	ret