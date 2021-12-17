#ifndef _MEMORY_H
#define _MEMORY_H

#define BASE_ADDRESS 0xC0000000
#include <main.h>

typedef struct {
	bool m_bPresent:1;
	bool m_bReadWrite:1;
	bool m_bUserSuper:1;
	bool m_bAccessed:1;
	bool m_bDirty:1;
	int m_unused:7;
	uint32_t m_pAddress:20;
} 
__attribute__((packed))
PageEntry;

typedef struct {
	PageEntry m_entries[1024];
} PageTable;

typedef struct {
	// page tables (virtual address)
	PageTable *m_pTables[1024];
	
	// page tables (physical address)
	uint32_t   m_pRealTables[1024];
	
	// this table's real address
	uint32_t   m_pThisPhysical;
} PageDirectory;

void MmFirstThingEver(unsigned long mbiAddr);


/**
 * Initializes the memory manager and heap.
 */
void MmInit();

/**
 * Allocates a single page (4096 bytes).
 * 
 * This returns the address of the new page, or NULL if we ran out of memory.
 * pPhysOut may be NULL or not, in the case where it's not, the physical address 
 * of the page is returned.
 */
void* MmAllocateSinglePagePhyD(uint32_t* pPhysOut, const char* callFile, int callLine);
#define MmAllocateSinglePagePhy(physOut) MmAllocateSinglePagePhyD(physOut, __FILE__, __LINE__)

/**
 * Allocates a single page (4096 bytes).
 * 
 * This returns the address of the new page, or NULL if we ran out of memory.
 * Use MmAllocateSinglePagePhy if you also want the physical address of the page.
 */
void* MmAllocateSinglePageD(const char* callFile, int callLine);
#define MmAllocateSinglePage() MmAllocateSinglePageD(__FILE__, __LINE__)

/**
 * Frees a single memory page. A NULL pointer is carefully ignored.
 */
void MmFreePage(void* pAddr);

/**
 * Allocates a memory range of size `size`.  Actually allocates several pages'
 * worth of memory, so that the `size` always fits inside.
 * 
 * For example, MmAllocate(8000) behaves exactly the same as MmAllocate(4193) and MmAllocate(8192).
 * 
 * This kind of thing where we're allocating 1 byte's worth of memory but using several is not recommended,
 * because the specification can always change this later so that memory accesses beyond the size throw an error.
 *
 * This returns the starting address of the memory range we obtained.  NULL can also be returned, if we have
 * no more address space to actually allocate anything, or there are no more memory ranges available.
 * 
 * You must call MmFree with the *EXACT* address MmAllocate returned.
 *
 * See: MmFree
 */
void* MmAllocateD(size_t size, const char* callFile, int callLine);
#define MmAllocate(size) MmAllocateD(size, __FILE__, __LINE__)

/**
 * Frees a memory range allocated with MmAllocate. A NULL pointer is carefully ignored.
 * 
 * Note that the last 12 bits of pAddr are ignored, but the other 20 bits MUST represent
 * the first page.
 *
 * So for example MmFree(MmAllocate(400)) behaves exactly the same as MmFree(MmAllocate(400)+500),
 * but not as MmFree(MmAllocate(400)+4100).
 */
void MmFree(void* pAddr);

/**
 * Uses a certain page directory address (and its physical one) as the current page directory.
 * 
 * This is an _internal_ kernel function and should not be used by any non-memory code.
 */
void MmUsePageDirectory(uint32_t* curPageDir, uint32_t phys);

/**
 * Reverts to the kernel's default page directory.  Useful if you're returning from an ELF executable.
 */
void MmRevertToKernelPageDir();

/**
 * Gets the kernel's default page directory's virtual address.
 */
uint32_t* MmGetKernelPageDir();

/**
 * Gets the kernel's default page directory's physical address.
 */
uint32_t MmGetKernelPageDirP();


#endif//_MEMORY_H