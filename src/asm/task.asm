BITS 32

extern KeSwitchTask
global IrqTaskA
IrqTaskA:
	cli
	push esp
	push ebp
	push edi
	push esi
	push edx
	push ecx
	push ebx
	push eax
	
	push esp
	call KeSwitchTask
	add esp, 4 ; get rid of what we had on the stack
	
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp
	pop esp
	
	; There is actually no need to call "sti", iretd 
	; reloads the EFLAGS, which automatically does that.
	iretd
	
extern g_saveStateToRestore1
global KeStartedNewTask
global KeStartedNewKernelTask
KeStartedNewTask:
KeStartedNewKernelTask:
	mov esp, [g_saveStateToRestore1]
	
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp
	pop esp
	
	iretd

extern KeTaskStartupC
global KeTaskStartup
KeTaskStartup:
	; Sets up the task, calls its function, and then
	; kills it.  The task's pointer is in eax.
	
	push eax
	call KeTaskStartupC
	
	; freeze if we got to this point!?
.halt:
	hlt
	jmp .halt