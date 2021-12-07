#include <elf.h>
#include <string.h>

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




//! TODO: Size might not be used
int ElfLoad (void* pElfFile, size_t size)
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
	
	//Really simple for now.
	void *addr = (void*)pProgHeader->m_virtAddr;
	int size1 = pProgHeader->m_memSize;
	int offs = pProgHeader->m_offset;
	
	memcpy (addr, &pElfData[offs], size1);
	
	//! Also execute the thing.
	ElfEntry entry = (ElfEntry)pHeader->m_entry;
	
	LogMsg("Loaded ELF successfully! Executing it now.");
	entry();
	LogMsg("Did we do it?!");
	
	return ELF_ERROR_NONE;
}

void test()
{
	int sz = &g_testingElfEnd - &g_testingElfStart;
	ElfLoad (&g_testingElf, sz);
}