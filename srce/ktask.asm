;resb 300000

extern SwitchToNextTask

OnTaskInterrupt:
	cli
	push esp
	push ebp
	push edi
	push esi
	push edx
	push ecx
	push ebx
	push eax
	mov  eax, cr3 ; Store CR3 inside eax.  I can't push cr3 directly.
	push eax
	
	; in the end, the esp points to a block of memory oddly familiar...
	; yes, it's the CPUSaveState
	
	; Since ESP grows from higher to lower, at ESP we effectively
	; have a CPUSaveState.
	push esp 
	call SwitchToNextTask
	add  esp, 4            ; FIX: Need to pop the ESP that we pushed onto the stack, 
						   ; the SwitchToNextTask call does not get rid of it for us.
	
	; restore the registers to task if something went wrong
	pop eax
	mov cr3, eax
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp
	pop esp
	;sti
	; and do a regular return
	iretd
; Function ends here. No more
extern g_saveStateToRestore1

OnStartedNewTask:
OnStartedNewKernelTask:
	;cli
	; restore the registers now
	mov esp, [g_saveStateToRestore1]  ; This should not be a problem, because the first 3 MB of kernel memory or so should always be identity mapped
	pop eax                           ; Same problem as before.  We can't pop CR3 directly, need to use EAX to do it.
	mov cr3, eax
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp
	pop esp
	
	;sti
	; and do a regular return
	iretd
	
	; Some nops to preceed the KeTaskInitialFunc. 
	; For some reason it's set 1 byte before 	
	nop
	nop
	nop
	nop
	nop
	
extern TaskInitialFuncC
TaskInitialFunc:
	; Sets up the task, calls its function, then kills it.
	; The task's pointer is now in EDX
	
	; Since everything has been setup, we can push it to the C function
	push eax
	call TaskInitialFuncC
	
	; freeze if we got to this point?!
.halt:
	hlt
	jmp .halt

global OnStartedNewTask
global OnStartedNewKernelTask
global TaskInitialFunc
global OnTaskInterrupt
