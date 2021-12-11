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

#endif//_MEMORY_H