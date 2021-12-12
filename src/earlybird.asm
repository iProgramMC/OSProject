BITS 32

; Setup kernel page directory and subsequent pages.  Maps the entire memory map from 0x00000000 to BASE_ADDRESS.
%define BASE_ADDRESS 0xC0000000
%define VIRT_TO_PHYS(k) ((k) - BASE_ADDRESS)
%define VGA_MEM 0xB8000
; Pages to identity map from the kernel. First 4MB are mapped.
; WORK: To change this to a value > 1024, you need to add support for more than one page directory entry being changed.
PagesToMap equ 2048

; main multiboot header
section .multibootdata
	;multiboot V1 spec
	align 4
	dd 0x1BADB002
	%ifdef vbe
		dd 0x06
		dd - (0x1BADB002 + 0x06)
	%else
		dd 0x02
		dd - (0x1BADB002 + 0x02)
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
	
section .data
	
extern g_pageDirectory
extern e_placement
extern e_frameBitsetSize
extern e_frameBitsetVirt

; WORK: Change this if necessary.  Paging is not setup at this stage
;       so this address is purely PHYSICAL.
e_placement dd 0x400000

section .bss

	align 4096
g_pageDirectory:
	resd 1024 ; reserve 1024 Page Directory Entries.
	
	; When the PG bit is set in CR0, any and all reads and writes from the CPU 
	; the addresses get turned into linear addresses via the TLB and MMU.
	;
	; The structure of a virtual address (on 32-Bit) is as follows:
	; [31        -       22] [21     -      12] [11            -           0]
	; [Page directory index] [Page table index] [Address inside the 4KB page]
	;
	; The page directory array's address is stored in CR3, along with some flags.
	
g_pageTableArray:
	resd 2048
	
e_temporary1 resd 1
e_temporary2 resd 1
e_frameBitsetVirt resd 1
e_frameBitsetSize resd 1

section .multiboottext
global KeEntry
extern KeStartupSystem

KeEntry:
	cli					; Block all interrupts.  We don't have them setup yet
	
	; We don't actually need a stack at this stage
	
	mov [VIRT_TO_PHYS(e_temporary1)], eax	; Make a backup of the multiboot parameters.
	mov [VIRT_TO_PHYS(e_temporary2)], ebx
	
	; let's not bother, we'll do it in C
	
	; offset 8 inside multiboot_info = mem_upper
	;add ebx, 8
	;mov ecx, dword [ebx]
	
	; Shift right 5 bits.  Has the effect of getting the number of pages available, divided by 8 (frame bitset size)
	;shl ecx, 5
	;mov [VIRT_TO_PHYS(e_frameBitsetSize)], ecx
	
	; Set the frame bitset to whatever
	;mov dword [VIRT_TO_PHYS(e_frameBitsetVirt)], e_placement
	
	; Then, increment e_placement
	;mov edx, [VIRT_TO_PHYS(e_placement)]
	;add edx, ecx
	;mov [VIRT_TO_PHYS(e_placement)], edx
	
	; First address to map is 0x00000000
	xor esi, esi

	; Map 2048 pages.
	mov ecx, PagesToMap
	
	; Get physical address of the boot page table.
	mov edi, VIRT_TO_PHYS (g_pageTableArray)
	
.label1:
	mov edx, esi
	or  edx, 0x03 ; Set present and R/W bits
	mov [edi], edx
	
	add esi, 0x1000 ; The size of a page is 4096 bytes
	add edi, 4
	;inc dword [VIRT_TO_PHYS(e_pageTableNum)]
	loop .label1
	
	; NOTE: If the kernel goes haywire just change PagesToMap
.label3:
	
	; Map the one and only pagetable (TODO) to both virtual addresses 0x0 and 0xC0000000
	mov dword [VIRT_TO_PHYS(g_pageDirectory) +   0*4], VIRT_TO_PHYS(g_pageTableArray+0000) + 0x03
	mov dword [VIRT_TO_PHYS(g_pageDirectory) +   1*4], VIRT_TO_PHYS(g_pageTableArray+4096) + 0x03
	mov dword [VIRT_TO_PHYS(g_pageDirectory) + 768*4], VIRT_TO_PHYS(g_pageTableArray+0000) + 0x03
	mov dword [VIRT_TO_PHYS(g_pageDirectory) + 769*4], VIRT_TO_PHYS(g_pageTableArray+4096) + 0x03
	
	; Set CR3 to the physical address of the g_pageDirectory
	mov ecx, VIRT_TO_PHYS(g_pageDirectory)
	mov cr3, ecx
	
	; Set PG and WP bit
	mov ecx, cr0
	or  ecx, 0x80010000
	mov cr0, ecx
	
	; Jump to higher half:
	mov ecx, (KeHigherHalfEntry)
	jmp ecx
	
section .text
KeHigherHalfEntry:
	; Unmap the identity mapping, we don't need it anymore
	mov dword [g_pageDirectory], 0
	
	; Reload CR3 to force a TLB flush (we updated the PDT but TLB isn't aware of that)
	; NOTE: you can probably also use invlpg.  We won't use that
	mov ecx, cr3
	mov cr3, ecx
	
	; Set up the stack
	mov esp, g_stackSpace
	
	; Restore the multiboot data we got earlier
	mov eax, [e_temporary1]
	mov ebx, [e_temporary2]
	
	push ebx
	push eax
	
	; Enter the high-level kernel now.
	call KeStartupSystem
	
	; Infinite loop, guess KeStartupSystem returned something:
	cli
loop: hlt
	jmp loop

global Move
Move:
	mov ecx, dword [e_placement]
	add ecx, 0xC0000000
	mov dword [e_frameBitsetVirt], ecx
	ret


section .bss
	resb 32768 ; 32KB for stack
g_stackSpace:



