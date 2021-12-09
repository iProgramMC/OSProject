BITS 32

; Setup kernel page directory and subsequent pages.  Maps the entire memory map from 0x00000000 to BASE_ADDRESS.
%define BASE_ADDRESS 0xC0000000
%define VIRT_TO_PHYS(k) ((k) - BASE_ADDRESS)
%define VGA_MEM 0xB8000
; Pages to identity map from the kernel. First 4MB are mapped.
; WORK: To change this to a value > 1024, you need to add support for more than one page directory entry being changed.
PagesToMap equ 1024

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
	resd PagesToMap
	
g_temporary1:
	resd 1
g_temporary2:
	resd 1

section .multiboottext
global KeEntry
extern KeStartupSystem

KeEntry:
	cli					; Block all interrupts.  We don't have them setup yet
	
	; We don't actually need a stack at this stage
	
	mov [VIRT_TO_PHYS(g_temporary1)], eax	; Make a backup of the multiboot parameters.
	mov [VIRT_TO_PHYS(g_temporary2)], ebx
	
	; First address to map is 0x00000000
	xor esi, esi

	; Map 1022 pages, The 1023rd and 1024th will be 8192 bytes of the VGA text buffer.
	; 8192 because what if we want to switch to 80x50 text mode?!
	mov ecx, PagesToMap
	
	; Get physical address of the boot page table.
	mov edi, VIRT_TO_PHYS (g_pageTableArray)
	
.label1:
	mov edx, esi
	or  edx, 0x03 ; Set present and R/W bits
	mov [edi], edx
	
	add esi, 0x1000 ; The size of a page is 4096 bytes
	add edi, 4
	loop .label1
	
	; NOTE: If the kernel goes haywire just change PagesToMap
.label3:

	; Map the one and only pagetable (TODO) to both virtual addresses 0x0 and 0xC0000000
	mov dword [VIRT_TO_PHYS(g_pageDirectory) +   0*4], VIRT_TO_PHYS(g_pageTableArray) + 0x03
	mov dword [VIRT_TO_PHYS(g_pageDirectory) + 768*4], VIRT_TO_PHYS(g_pageTableArray) + 0x03
	
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
	mov eax, [g_temporary1]
	mov ebx, [g_temporary2]
	
	push ebx
	push eax
	
	; Enter the high-level kernel now.
	call KeStartupSystem
	
	; Infinite loop, guess KeStartupSystem returned something:
	cli
loop: hlt
	jmp loop




section .bss


	resb 32768 ; 32KB for stack
g_stackSpace:



