[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api_thread.nas"]

		GLOBAL	_api_thread_create, _api_thread_run,_api_thread_sleep
		global _api_thread_delete,_api_thread_isover

[SECTION .text]

_api_thread_create:	; uint api_thread_create(int func_addr, int stack_addr, int paddr);
	mov eax,[esp+4]
	mov ecx,[esp+8]
	mov edx,[esp+12]
	sub ecx,4
	mov [ecx],edx
	sub ecx,4
	mov edx,25	;
	int 0x40	;
	ret			;
	
_api_thread_run:	; void api_thread_run(uint thread_id);
	mov eax,[esp+4]
	mov edx,26	;
	int 0x40	;
	ret			;
	
_api_thread_sleep:	; void api_thread_sleep(uint thread_id);
	mov eax,[esp+4]
	mov edx,27	;
	int 0x40	;
	ret			;
	
_api_thread_delete:	; void api_thread_delete(uint thread_id);
	mov eax,[esp+4]
	mov edx,28	;
	int 0x40	;
	ret			;
	
_api_thread_isover:	; int api_thread_isover(uint thread_id);
	mov eax,[esp+4]
	mov edx,29	;
	int 0x40	;
	ret			;
	