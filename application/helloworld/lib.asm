bits 32

global DoSyscall
DoSyscall:
	mov eax, [esp + 4]
	mov esi, [esp + 8]
	mov ecx, [esp + 12]
	mov edx, [esp + 16]
	mov ebx, [esp + 20]
	
	;int 0x80

	ret
