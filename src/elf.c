#if 0
#include <elf.h>
#include <string.h>
#include <memory.h>

struct ElfProcess
{
	uint32_t* m_pageDirectory;
	uint32_t* m_pageTables[64];
	
}

extern char g_testingElfStart, g_testingElfEnd;
extern char g_testingElf;

bool ElfCheckHeader(ElfHeader* pHeader)
{
	if (!pHeader) return false;
	
	if (pHeader->m_ident[EI_MAG0] != ELFMAG0) return false;
	if (pHeader->m_ident[EI_MAG1] != ELFMAG1) return false;
	if (pHeader->m_ident[EI_MAG2] != ELFMAG2) return false;
	if (pHeader->m_ident[EI_MAG3] != ELFMAG3) return false;
	return true;
}
int ElfIsSupported(ElfHeader* pHeader)
{
	if (!ElfCheckHeader(pHeader)) return ELF_HEADER_INCORRECT;
	
	if (pHeader->m_ident[EI_CLASS] != ELF_CLASS_32BIT) 		return ELF_CLASS_INCORRECT; //The image file %s is for a different address-width, which we don't understand.
	if (pHeader->m_ident[EI_DATA ] != ELF_DATA_BYTEORDER)	return ELF_ENDIANNESS_INCORRECT; //The image file %s is valid, but is for a machine type other than the current machine.
	if (pHeader->m_ident[EI_DATA ] != ELF_VER_CURRENT)		return ELF_VERSION_INCORRECT; //The image file %s is ELF Version %d, currently unsupported.
	if (pHeader->m_machine != ELF_MACH_386)					return ELF_MACHINE_INCORRECT; //The image file %s is valid, but is for a machine type other than the current machine.
	if (pHeader->m_type != ELF_TYPE_RELOC &&
		pHeader->m_type != ELF_TYPE_EXEC)					return ELF_FILETYPE_INCORRECT; //The image file %s is a type of executable we don't understand.
	
	return true;
}

uint32_t* ElfMakeNewPageDirectory(uint32_t* pPhys)
{
	// Make a new page directory:
	uint32_t* pd = (uint32_t*)MmAllocate (4096);
	
	if (!pd) {
		LogMsg("ERROR: Could not allocate new page directory\n");
		return NULL; // Why didn't we manage to do this?
	}
	
	// Clone the kernel's page dir to this one:
	fast_memcpy (pd, MmGetKernelPageDir(), 4096);
	
	// Zero out all entries until 0xC0000000:
	ZeroMemory (pd, 0, sizeof(uint32_t) * 768);
	
	// Then return this page directory.
	return pd;
}

void ElfMapAddress(uint32_t* pageDir, void *virt, size_t size, void* data)
{
	uint32_t pdIndex = (uint32_t)virt >> 22;
	uint32_t size1 = size;
	
	// A page table maps 4 MB of memory, or (1 << 22) bytes.
	uint32_t pageTablesNecessary = ((size1 - 1) >> 22) + 1;
	
	while (pageTablesNecessary)
	{
		uint32_t phys = 0;
		void *pageVirt = KeAllocateSinglePagePhy(&phys);
		pageDir[pdIndex] = phys | 0x3; //present and read/write
		ZeroMemory (pageVirt, 0, 4096);
		
		pageTablesNecessary--; 
		pdIndex++;
	}
	
	
}

int ElfExecute (void *pElfFile, size_t size)
{
	uint8_t* pElfData = (uint8_t*)pElfFile; //to do arithmetic with this
	//check the header.
	ElfHeader* pHeader = (ElfHeader*)pElfFile;
	
	int errCode = ElfIsSupported(pHeader);
	if (errCode != 1) //not supported.
	{
		LogMsg("Got error %x while loading the elf.", errCode);
		return errCode;
	}
	
	ElfProgHeader* pProgHeader = (ElfProgHeader*)(pElfData + pHeader->m_phOffs);
	
	// Allocate a new page directory for the elf:
	uint32_t* newPageDir = ElfMakeNewPageDirectory();
	
	void *addr = (void*)pProgHeader->m_virtAddr;
	size_t size1 = pProgHeader->m_memSize;
	int offs = pProgHeader->m_offset;
	
	ElfMapAddress (newPageDir, addr, size1, &pElfData[offs]);
}
#endif
