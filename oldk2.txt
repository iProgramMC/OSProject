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
	;call KeyboardHandlerMain
	popa
	iretd
