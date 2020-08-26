[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api_file.nas"]

		GLOBAL	_api_file_open, _api_file_close, _api_file_read, _api_file_write
		global  _api_file_eof, _api_file_seek, _api_file_tell

[SECTION .text]

_api_file_open:  ;	int api_file_open(char filename[], int flag);
	push ebx
	push edi
	mov eax,[esp+12]
	mov ebx,[esp+16]
	mov di, ds
	mov edx,12	;putstr
	int 0x40	;
	pop edi
	pop ebx
	ret			;
	
_api_file_close: ; void api_file_close(int handler)
	mov edx,13
	mov eax,[esp+4]
	int 0x40
	ret
	
_api_file_read: ;int api_file_read(int handler, char *buf, int size)
	push ebx
	push edi
	mov edx,14
	mov eax,[esp+12]
	mov ebx,[esp+16]
	mov ecx,[esp+20]
	mov di,ds
	int 0x40
	pop edi
	pop ebx
	ret
	
_api_file_write: ;int api_file_write(int handler, char *buf, int size)
	push ebx
	push edi
	mov edx,15
	mov eax,[esp+12]
	mov ebx,[esp+16]
	mov ecx,[esp+20]
	mov di,ds
	int 0x40
	pop edi
	pop ebx
	ret
	
;_api_file_eof, _api_file_seek, _api_file_tell
_api_file_eof: ;int api_file_eof(int handler)
	mov edx,16
	mov eax,[esp+4]
	int 0x40
	ret
	
_api_file_seek:	;int api_file_seek(int handler, int offset, int flag)
	push ebx
	mov edx,17
	mov eax,[esp+8]
	mov ebx,[esp+12]
	mov ecx,[esp+16]
	int 0x40
	pop ebx
	ret

_api_file_tell: ; int api_file_tell(int handler)
	mov edx,18
	mov eax,[esp+4]
	int 0x40
	ret