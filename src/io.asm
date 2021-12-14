bits 32
section .text

global ReadPort
global WritePort
global ReadPortW
global WritePortW
global WriteFont8px
global WriteFont16px

extern e_placement
extern e_frameBitsetVirt

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

; Functions for DRIVERS/VGA.H
; https://wiki.osdev.org/VGA_Fonts#Get_from_VGA_RAM_directly
WriteFont8px:
	mov esi, [esp + 4]
	mov edi, 0xC00A0000
	;in: edi=4k buffer
	;out: buffer filled with font
	;clear even/odd mode
	mov	dx, 03ceh
	mov	ax, 5
	out	dx, ax
	;map VGA memory to 0A0000h
	mov	ax, 0406h
	out	dx, ax
	;set bitplane 2
	mov	dx, 03c4h
	mov	ax, 0402h
	out	dx, ax
	;clear even/odd mode (the other way, don't ask why)
	mov	ax, 0604h
	out	dx, ax
	;copy charmap
	mov	ecx, 256
	;copy 8 bytes to bitmap
.b:	
	movsd
	movsd
	;skip another 24 bytes
	add	edi, 24
	loop .b
	;restore VGA state to normal operation
	mov	ax, 0302h
	out	dx, ax
	mov	ax, 0204h
	out	dx, ax
	mov	dx, 03ceh
	mov	ax, 1005h
	out	dx, ax
	mov	ax, 0E06h
	out	dx, ax
	ret
	
WriteFont16px:
	mov esi, [esp + 4]
	mov edi, 0xC00A0000
	;in: edi=4k buffer
	;out: buffer filled with font
	;clear even/odd mode
	mov	dx, 03ceh
	mov	ax, 5
	out	dx, ax
	;map VGA memory to 0A0000h
	mov	ax, 0406h
	out	dx, ax
	;set bitplane 2
	mov	dx, 03c4h
	mov	ax, 0402h
	out	dx, ax
	;clear even/odd mode (the other way, don't ask why)
	mov	ax, 0604h
	out	dx, ax
	;copy charmap
	mov	ecx, 256
	;copy 16 bytes to bitmap
.b:	movsd
	movsd
    movsd
	movsd
	;skip another 16 bytes
	add	edi, 16
	loop .b
	;restore VGA state to normal operation
	mov	ax, 0302h
	out	dx, ax
	mov	ax, 0204h
	out	dx, ax
	mov	dx, 03ceh
	mov	ax, 1005h
	out	dx, ax
	mov	ax, 0E06h
	out	dx, ax
	ret

global KeStartupStuff
KeStartupStuff:
	mov ecx, dword [e_placement]
	add ecx, 0xC0000000
	mov dword [e_frameBitsetVirt], ecx
	ret
	