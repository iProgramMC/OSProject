#include <elf.h>
#include <string.h>
#include <memory.h>

struct ElfProcess
{
	uint32_t* m_pageDirectory;
	uint32_t* m_pageTables[64];
	
};

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
	uint32_t* pd = (uint32_t*)MmAllocateSinglePagePhy (pPhys);
	
	if (!pd) {
		LogMsg("ERROR: Could not allocate new page directory\n");
		return NULL; // Why didn't we manage to do this?
	}
	
	// Clone the kernel's page dir to this one:
	memcpy (pd, MmGetKernelPageDir(), 4096);
	// Zero out all entries until 0xC0000000:
	ZeroMemory (pd, sizeof(uint32_t) * 768);
	
	// Then return this page directory.
	return pd;
}
#define TODO_Please_Make_It_So_We_Can_Clean_Up_After_Ourselves
void ElfMapAddress(uint32_t* pageDir, void *virt, size_t size, void* data)
{
	TODO_Please_Make_It_So_We_Can_Clean_Up_After_Ourselves /*!!!*/
	
	uint32_t pdIndex =  (uint32_t)virt >> 22;
	uint32_t ptIndex = ((uint32_t)virt >> 12) & 0x3FF;
	uint32_t size1 = size;
	
	// A page table maps 4 MB of memory, or (1 << 22) bytes.
	uint32_t pageTablesNecessary = ((size1 - 1) >> 22) + 1;
	uint32_t pagesNecessary = ((size1 - 1) >> 12) + 1;
	
	uint32_t* pointer = (uint32_t*)data;
	LogInt(pdIndex);
	LogInt(ptIndex);
	
	while (pageTablesNecessary)
	{
		uint32_t phys = 0;
		PageEntry *pageTVirt = (PageEntry*)MmAllocateSinglePagePhy(&phys);
		pageDir[pdIndex] = phys | 0x3; //present and read/write
		ZeroMemory (pageTVirt, 4096);
		
		uint32_t min = 4096;
		if (min > pagesNecessary)
			min = pagesNecessary;
		for (uint32_t i=ptIndex; i<min; i++)
		{
			uint32_t phys2 = 0;
			void *pageVirt = MmAllocateSinglePagePhy(&phys2);
			ZeroMemory (pageVirt, 4096);
			pageTVirt[i].m_pAddress = phys2 >> 12;
			pageTVirt[i].m_bPresent = true;
			pageTVirt[i].m_bUserSuper = true;
			pageTVirt[i].m_bReadWrite = true;
			
			memcpy (pageVirt, pointer, 4096);
			pointer += 4096;
		}
		pageTablesNecessary--; 
		pagesNecessary -= 4096-ptIndex;
		ptIndex = 0;
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
		LogMsg("Got error ");LogInt(errCode);LogMsg("while loading the elf.\n");
		return errCode;
	}
	
	ElfProgHeader* pProgHeader = (ElfProgHeader*)(pElfData + pHeader->m_phOffs);
	
	// Allocate a new page directory for the elf:
	uint32_t  newPageDirP;
	uint32_t* newPageDir = ElfMakeNewPageDirectory(&newPageDirP);
	
	void *addr = (void*)pProgHeader->m_virtAddr;
	size_t size1 = pProgHeader->m_memSize;
	int offs = pProgHeader->m_offset;
	
	LogInt(addr);
	ElfMapAddress (newPageDir, addr, size1, &pElfData[offs]);
	
	MmUsePageDirectory(newPageDir, newPageDirP);
	
	//now that we have switched, call the entry func:
	ElfEntry entry = (ElfEntry)pHeader->m_entry;
	
	LogMsg("Loaded ELF successfully! Executing it now.");
	int e = entry();
	
	LogMsg("Did we do it?! (TODO: Add freeing functions.)");
	MmRevertToKernelPageDir();
	
	return ELF_ERROR_NONE;
}

void elf_test()
{
	int sz = &g_testingElfEnd - &g_testingElfStart;
	ElfExecute (&g_testingElf, sz);
}
