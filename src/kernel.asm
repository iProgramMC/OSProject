; Boot Code - meant to assemble with NASM
bits 32

; globals
global Entry
global Irq2EmptyHandler
global ReadPort
global WritePort
global ReadPortW
global WritePortW
global OnKeyboardInterrupt
global LoadIDT
;externs
extern StartupSystem
extern KeyboardHandlerMain

; multiboot header
; %define vbe
section .multiboot
	; multiboot V1 spec
    align 4
    dd 0x1BADB002              ; magic
	%ifdef vbe
    dd 0x06                    ; flags (enable vbe video and pass memory information)
    dd - (0x1BADB002 + 0x06)   ; checksum. m+f+c should be zero
	%else
    dd 0x02                    ; flags (enable vbe video and pass memory information)
    dd - (0x1BADB002 + 0x02)   ; checksum. m+f+c should be zero
	%endif
	dd 0
	dd 0
	dd 0
	dd 0
	dd 0
	%ifdef vbe
	dd 0
	dd 1024
	dd 768
	dd 32
	%else
	dd 0
	dd 0
	dd 0
	dd 0
	%endif

; Code
section .text

;;;;;; GDT Data

GDTStart:
GDTNull:
	dd 0x0
	dd 0x0
GDTCode:
	dw 0xffff
	dw 0x00
	db 0x00
	db 0x9a
	db 0xcf
	db 0x00
GDTData:
	dw 0xffff
	dw 0x00
	db 0x00
	db 0x92
	db 0xcf
	db 0x00
GDTEnd:
GDTDescription:
	dw GDTEnd - GDTStart - 1
	dd GDTStart
CODE_SEG equ GDTCode - GDTStart
DATA_SEG equ GDTData - GDTStart

;;;;;; GDT LOADING CODE
LoadGDT:
	pop eax
	lgdt [eax]
	jmp 8:.set_cs
.set_cs:
	mov eax, 16
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	mov ss, eax
	ret 				; return
; PORT I/O
ReadPort:	
	mov edx, [esp + 4]
	in al, dx
	ret
WritePort:
	mov edx, [esp + 4]
	mov al, [esp + 8]
	out dx, al
	ret
ReadPortW:
	mov edx, [esp + 4]
	in ax, dx
	ret
WritePortW:
	mov edx, [esp + 4]
	mov ax, [esp + 8]
	out dx, ax
	ret
LoadIDT:
	mov edx, [esp + 4]
	lidt [edx]
	sti
	ret

;does nothing
Irq2EmptyHandler:
	pusha
	cli
	push eax
	push edx
	xor edx, edx
	xor eax, eax
	mov al, 0x20
	mov dl, 0x20
	out dx, al
	mov dl, 0xA0
	out dx, al
	pop edx
	pop eax
	sti
	popa
	iretd
OnKeyboardInterrupt:
	pusha
	call KeyboardHandlerMain
	popa
	iretd
SetupGDT:
	lgdt [GDTDescription]
	jmp CODE_SEG:.set_cs
.set_cs:
	mov eax, DATA_SEG
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	mov ss, eax
	ret

;;;;;; ENTRY POINT
Entry:
	cli 				; block interrupts
	mov esp, g_stackSpace
	mov edx, eax 		; Make a backup of EAX, that InitGDT bastard uses and destroys it...
	call SetupGDT
	push ebx			; Push the pointer to the Multiboot information struct
	push edx			; Push the magic number, normally that'd be `eax` but since we've made
						; a backup of it in `edx`, we push that and save a few cycles :)
	call StartupSystem  ; StartupSystem restores interrupts
	
	cli                 ; clear all interrupts because if we don't then the code will get 
						; continuously called and jmp will be more frequent :)
						
mht:hlt 				; halt the CPU (aka wait for interrupt)
	jmp mht				; if it **SOMEHOW** escaped, make it jump back
	; I don't think we will ever reach below the jump, but if  we do, the OS will triple-fault anyway
	
	
;;;;;; STACK SPACE
section .bss
resb 32768; 32KB for stack
g_stackSpace:
