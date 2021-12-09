#include <paging.h>
#include <memory.h>
#include <string.h>

uint32_t* g_pFrames, g_nFrames;

#define  INDEX_FROM_BIT(a) (a / 32)
#define OFFSET_FROM_BIT(a) (a % 32)

static void SetFrame (uint32_t frameAddr)
{
	uint32_t frame = frameAddr / 0x1000;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	g_pFrames[idx] |= (0x1 << off);
}
static void ClrFrame (uint32_t frameAddr)
{
	uint32_t frame = frameAddr / 0x1000;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	g_pFrames[idx] &=~(0x1 << off);
}
static bool TestFrame(uint32_t frameAddr) 
{
	uint32_t frame = frameAddr / 0x1000;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	return (g_pFrames[idx] & (1 << off)) != 0;
}
static uint32_t FindFreeFrame()
{
	for (uint32_t i=0; i<INDEX_FROM_BIT(g_nFrames); i++)
	{
		//Any bit free?
		if (g_pFrames[i] != 0xFFFFFFFF) {
			//yes, which?
			for (int j=0; j<32; j++)
			{
				if (!(g_pFrames[i] & (1<<j)))
					return i*32 + j;
			}
		}
		//no, continue
	}
	//what
	return 0xffffffffu;
}


void KeAllocFrame (PageEntry* pPage, bool bSuperUser, bool bWritable)
{
	//if frame was already allocated, return right away
	if (pPage->m_pAddress) return;
	
	uint32_t idx = FindFreeFrame();
	ASSERT (idx != 0xffffffffu && "No free frames!");
	SetFrame(idx * 0x1000);
	
	//now this is assigned to something
	pPage->m_bPresent = true;
	pPage->m_bReadWrite = bWritable;
	pPage->m_bUserSuper = !bSuperUser;
	pPage->m_pAddress = idx;
}
void KeFreeFrame (PageEntry *pPage) 
{
	uint32_t frame;
	if (!(frame = pPage->m_pAddress))
	{
		//the page didn't actually have an allocated frame.
		return;
	}
	
	ClrFrame (frame);
	pPage->m_pAddress = 0;
}


PageDirectory* g_kernelDirectory, *g_currentDirectory;
void KeSetupPaging (int memoryEnd)
{
	uint32_t memEndPage = memoryEnd & ~0xFFF; 
	
	g_nFrames = memEndPage / 0x1000;
	g_pFrames = (uint32_t*)malloc (INDEX_FROM_BIT(g_nFrames));
	
	ZeroMemory (g_pFrames, g_nFrames / 8);
	
	// Make a page directory
			   g_kernelDirectory = (PageDirectory*)malloc(sizeof (*g_kernelDirectory));
	ZeroMemory(g_kernelDirectory, 						  sizeof (*g_kernelDirectory));
	
	// We need to identity map our own code (1:1 mapping between virt and phys)
	uint32_t i=0;
	while (i < (int)g_memory) {
		//kernel code is readable but not writable from userspace
		KeAllocFrame(KeGetPage (i, 1, g_kernelDirectory), 0, 0);
		i += 0x1000;
	}
	
	//TODO: register pagefault handler.
	KeSwitchPageDir (g_kernelDirectory);
}

#define BIT_PG 0x80000000 //if for whatever reason we need to change this
void KeSwitchPageDir (PageDirectory* pNew)
{
	g_currentDirectory = pNew;
	uint32_t cr0;
	__asm__ volatile ("mov %0, %%cr3"::"r"(&pNew->m_pRealTables));
	__asm__ volatile ("mov %%cr0, %0": "=r"(cr0));
	cr0 |= BIT_PG;
	__asm__ volatile ("mov %0, %%cr0"::"r"(cr0));
}

PageEntry* KeGetPage(uintptr_t address, bool make, PageDirectory* pDir)
{
	// turn address to index
	address /= 0x1000;
	uint32_t tableIdx = address / 1024;
	
	if (pDir->m_pTables[tableIdx])
		// this table is already assigned
		return &pDir->m_pTables[tableIdx]->m_entries[address % 1024];
	
	if (!make) return 0;
	
	void* pPageTable = malloc(sizeof(PageTable));
	pDir->m_pTables[tableIdx] = (PageTable*)pPageTable;
	pDir->m_pRealTables[tableIdx] = (int)pPageTable | 0x7; //present, read/write, superuser
	
	ZeroMemory(pDir->m_pTables[tableIdx], 0x1000);
	
	return &pDir->m_pTables[tableIdx]->m_entries[address%1024];
}


PageDirectory* KeCloneDirectory (PageDirectory* pSrc)
{
	// Allocate a new page directory
	PageDirectory* pNew = malloc (sizeof (PageDirectory));
	uint32_t physAddress = (uint32_t)pNew; //! TODO: add VirtualToPhysical support?
	
	// Ensure that it is blank
	memset (pNew, 0, sizeof (*pNew));
	
	
	
	
	
	return pNew;
}


