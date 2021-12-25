#include <memory.h>
#include <print.h>
#include <string.h>

extern uint32_t g_kernelPageDirectory[];
extern uint32_t g_pageTableArray[];
extern uint32_t* e_frameBitsetVirt;
extern uint32_t e_frameBitsetSize;
extern uint32_t e_placement;

uint32_t* g_pageDirectory = NULL;
uint32_t* g_curPageDir = NULL;

void MmTlbInvalidate() {
	__asm__("movl %cr3, %ecx\n\tmovl %ecx, %cr3\n\t");
}
void MmUsePageDirectory(uint32_t* curPageDir, uint32_t phys)
{
	g_curPageDir = curPageDir;
	__asm__ volatile ("mov %0, %%cr3"::"r"((uint32_t*)phys));
}
/*
void MmRevertToKernelPageDir()
{
	MmUsePageDirectory(g_pageDirectory, (uint32_t)g_pageDirectory - BASE_ADDRESS);
}*/

extern void MmStartupStuff(); //io.asm
void MmFirstThingEver(unsigned long memorySizeKb)
{
	MmStartupStuff();
	e_frameBitsetSize = (memorySizeKb >> 2); //nBytesRAM >> 12 = (nKbExtRam << 10) >> 12 = nKbExtRam >> 2
	e_placement += e_frameBitsetSize;
}

/**
 * Kernel heap.
 */
 
#if 1
// Can allocate up to 256 MB of RAM.  No need for more I think,
// but if there is a need, just increase this. Recommend a Power of 2
#define PAGE_ENTRY_TOTAL 65536

PageEntry g_kernelPageEntries   [PAGE_ENTRY_TOTAL] __attribute__((aligned(4096)));

// Simple heap: The start PageEntry of a big memalloc also includes the number of pageEntries that also need to be freed
// For this reason freeing just one page from a larger allocation is _not_ enough, you need to use MmFree instead
// (unless you used MmAllocageSinglePage to allocate it)
int g_kernelMemoryAllocationSize[PAGE_ENTRY_TOTAL];

//if debug only
const char* g_kernelMemoryAllocationAuthor[PAGE_ENTRY_TOTAL];
int  g_kernelMemoryAllocationAuthorLine[PAGE_ENTRY_TOTAL];

#define PAGE_BITS 0X3 //present and read/write
#define PAGE_ALLOCATION_BASE 0x80000000

#endif

//forward declaration because we need this function before it is declared 
void *MmAllocatePhyD (size_t size, const char* callFile, int callLine, uint32_t* physAddresses);

/**
 * Heap variables (can switch in and out of the kernel heap)
 */
#if 1
PageEntry  * g_pageEntries = NULL;
int        * g_memoryAllocationSize = NULL;
const char** g_memoryAllocationAuthor = NULL;
int        * g_memoryAllocationAuthorLine = NULL;
int          g_heapSize = 0;
Heap       * g_pHeap = NULL;
uint32_t     g_pageAllocationBase = PAGE_ALLOCATION_BASE;
#endif

uint32_t g_memoryStart;

/**
 * Physical memory manager.  This hands out 4KB chunks of physical memory so that they can be mapped
 * by the virtual memory manager below this.
 */
#if 1

#define  INDEX_FROM_BIT(a) (a / 32)
#define OFFSET_FROM_BIT(a) (a % 32)
static void MmSetFrame (uint32_t frameAddr)
{
	uint32_t frame = frameAddr >> 12;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	e_frameBitsetVirt[idx] |= (0x1 << off);
}
/*
static void MmClrFrame (uint32_t frameAddr)
{
	uint32_t frame = frameAddr >> 12;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	e_frameBitsetVirt[idx] &=~(0x1 << off);
}
static bool MmTestFrame(uint32_t frameAddr) 
{
	uint32_t frame = frameAddr >> 12;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	return (e_frameBitsetVirt[idx] & (1 << off)) != 0;
}*/
static uint32_t MmFindFreeFrame()
{
	for (uint32_t i=0; i<INDEX_FROM_BIT(e_frameBitsetSize); i++)
	{
		//Any bit free?
		if (e_frameBitsetVirt[i] != 0xFFFFFFFF) {
			//yes, which?
			for (int j=0; j<32; j++)
			{
				if (!(e_frameBitsetVirt[i] & (1<<j)))
					return i*32 + j;
			}
		}
		//no, continue
	}
	//what
	LogMsg("WARNING: No more free memory?!");
	return 0xffffffffu/*ck you*/;
}

void MmInitializePMM()
{
	for (uint32_t i=0; i<INDEX_FROM_BIT(e_frameBitsetSize); i++)
	{
		e_frameBitsetVirt[i] = 0;
	}
}

#endif


/**
 * Heap management code.  What comes after 0x80000000 is always mapped regardless of which
 * user heap we're currently using - it's just that we can't allocate to the kernel heap
 * while we are using the user heap.
 */
#if 1
int GetHeapSize()
{
	return g_heapSize;
}

void ResetToKernelHeap()
{
	g_pageAllocationBase = PAGE_ALLOCATION_BASE;
	g_heapSize = PAGE_ENTRY_TOTAL;
	g_pageEntries = g_kernelPageEntries;
	g_pageDirectory = g_kernelPageDirectory;
	g_memoryAllocationAuthor = g_kernelMemoryAllocationAuthor;
	g_memoryAllocationAuthorLine = g_kernelMemoryAllocationAuthorLine;
	g_memoryAllocationSize = g_kernelMemoryAllocationSize;
	g_pHeap = NULL;
	MmUsePageDirectory(g_kernelPageDirectory, (uint32_t)g_kernelPageDirectory - BASE_ADDRESS);
}

void UseHeap (Heap* pHeap)
{
	if (!pHeap) {
		ResetToKernelHeap();
		return;
	}
	g_pHeap = pHeap;
	g_heapSize                   = pHeap->m_pageEntrySize;
	g_pageEntries                = pHeap->m_pageEntries;
	g_pageDirectory              = pHeap->m_pageDirectory;
	g_memoryAllocationAuthor     = pHeap->m_memoryAllocAuthor;
	g_memoryAllocationAuthorLine = pHeap->m_memoryAllocAuthorLine;
	g_memoryAllocationSize       = pHeap->m_memoryAllocSize;
	
	//all user heap allocations start at 0x40000000
	g_pageAllocationBase = 0x40000000;
	
	
	MmUsePageDirectory(pHeap->m_pageDirectory, pHeap->m_pageDirectoryPhys);
}

// Frees a heap that was allocated on the kernel heap.
void FreeHeap (Heap* pHeap)
{
	ResetToKernelHeap();
	MmFree(pHeap->m_pageEntries);
	MmFree(pHeap->m_pageDirectory);
	MmFree(pHeap->m_memoryAllocAuthor);
	MmFree(pHeap->m_memoryAllocAuthorLine);
	MmFree(pHeap->m_memoryAllocSize);
	pHeap->m_pageEntries           = NULL;
	pHeap->m_pageDirectory         = NULL;
	pHeap->m_memoryAllocAuthor     = NULL;
	pHeap->m_memoryAllocAuthorLine = NULL;
	pHeap->m_memoryAllocSize       = NULL;
	pHeap->m_pageEntrySize         = 0;
}

void MmRevertToKernelPageDir()
{
	//MmUsePageDirectory(g_pageDirectory, (uint32_t)g_pageDirectory - BASE_ADDRESS);
	ResetToKernelHeap ();
}

//! ONLY call this for the kernel heap!
void MmSetupKernelHeapPages()
{
	int heapSize = GetHeapSize();
	for (int i = 0; i < heapSize; i++)
	{
		*((uint32_t*)(g_pageEntries + i)) = 0;
		g_memoryAllocationSize[i] = 0;
	}
	int index = 0x200;//2;
	for (int i = 0; i < heapSize; i += 1024)
	{
		uint32_t pPageTable = (uint32_t)&g_pageEntries[i];
		
		g_curPageDir[index] = (pPageTable-BASE_ADDRESS) | PAGE_BITS;//present + readwrite
		index++;
	}
	
	MmTlbInvalidate();
}

void MmSetupUserHeapPages(Heap* pHeap)
{
	int heapSize = pHeap->m_pageEntrySize;
	for (int i = 0; i < heapSize; i++)
	{
		*((uint32_t*)(pHeap->m_pageEntries + i)) = 0;
		pHeap->m_memoryAllocSize[i] = 0;
	}
	int index = 0x100, jindex = 0;//start at 0x40000000
	for (int i = 0; i < heapSize; i += 1024)
	{
		uint32_t pPageTable = pHeap->m_pageEntriesPhysical[jindex];
		
		pHeap->m_pageDirectory[index] = pPageTable | PAGE_BITS;//present + readwrite
		
		index++;
		jindex++;
	}
	
	//dump the first 100 entries in the PD
	/*for (int i=0; i<300; i+=8)
	{
		LogMsgNoCr("%x ",pHeap->m_pageDirectory[i+0]);
		LogMsgNoCr("%x ",pHeap->m_pageDirectory[i+1]);
		LogMsgNoCr("%x ",pHeap->m_pageDirectory[i+2]);
		LogMsgNoCr("%x ",pHeap->m_pageDirectory[i+3]);
		LogMsgNoCr("%x ",pHeap->m_pageDirectory[i+4]);
		LogMsgNoCr("%x ",pHeap->m_pageDirectory[i+5]);
		LogMsgNoCr("%x ",pHeap->m_pageDirectory[i+6]);
		LogMsg    ("%x ",pHeap->m_pageDirectory[i+7]);
	}*/
}

bool AllocateHeapD (Heap* pHeap, int size, const char* callerFile, int callerLine)
{
	//PAGE_ENTRIES_PHYS_MAX_SIZE represents how many pagedirectories can we create at one time
	if (size > PAGE_ENTRIES_PHYS_MAX_SIZE * 4096)
	{
		//can't:
		LogMsg("Can't allocate a heap bigger than %d pages big.  That may change in a future update.", PAGE_ENTRIES_PHYS_MAX_SIZE * 4096);
		return false;
	}
	
	//initialize stuff to null, this way we can FreeHeap already
	pHeap->m_pageEntrySize         = 0;
	pHeap->m_pageEntries           = NULL;
	pHeap->m_pageDirectory         = NULL;
	pHeap->m_memoryAllocAuthor     = NULL;
	pHeap->m_memoryAllocAuthorLine = NULL;
	pHeap->m_memoryAllocSize       = NULL;
	pHeap->m_pageDirectoryPhys     = 0;
	
	ResetToKernelHeap();
	
	uint32_t phys;
	pHeap->m_pageEntrySize         = size;
	pHeap->m_pageEntries           = MmAllocatePhyD(sizeof (int) * size, callerFile, callerLine, pHeap->m_pageEntriesPhysical);
	pHeap->m_pageDirectory         = MmAllocateSinglePagePhyD(&phys, callerFile, callerLine);
	pHeap->m_memoryAllocAuthor     = MmAllocateD(sizeof (int) * size, callerFile, callerLine);
	pHeap->m_memoryAllocAuthorLine = MmAllocateD(sizeof (int) * size, callerFile, callerLine);
	pHeap->m_memoryAllocSize       = MmAllocateD(sizeof (int) * size, callerFile, callerLine);
	pHeap->m_pageDirectoryPhys     = phys;
	
	// copy everything from the kernel heap:
	memcpy (pHeap->m_pageDirectory, g_kernelPageDirectory, 4096);
	
	// initialize this heap's pageentries:
	MmSetupUserHeapPages(pHeap);
	
	ResetToKernelHeap();
	
	return true;
}

#endif


int g_offset = 0;
void MmInit()
{
	e_placement += 0x1000;
	e_placement &= ~0xFFF;
	
	g_offset = e_placement >> 12;
	
	g_memoryStart = e_placement;
	
	MmInitializePMM();
	MmRevertToKernelPageDir();
	MmSetupKernelHeapPages();
}

void MmInvalidateSinglePage(uintptr_t add)
{
	//__asm__ volatile ("invlpg (%0)\n\t"::"r"(add));
	add += 0;
	MmTlbInvalidate();
}

void* MmSetupPage(int i, uint32_t* pPhysOut, const char* callFile, int callLine)
{
	uint32_t frame = MmFindFreeFrame();
	if (frame == 0xffffffffu/*ck you*/)
		return NULL;

	MmSetFrame(frame << 12);
	
	// Yes. Let's mark this as present, and return a made-up address from the index.
	g_pageEntries[i].m_bPresent = true;
	g_pageEntries[i].m_bReadWrite = true;
	g_pageEntries[i].m_bUserSuper = true;
	g_pageEntries[i].m_pAddress = frame + g_offset;
	
	g_memoryAllocationSize[i] = 0;
	g_memoryAllocationAuthor[i] = callFile;
	g_memoryAllocationAuthorLine[i] = callLine;
	
	// if pPhysOut is not null, we would probably want that too:
	if (pPhysOut)
		*pPhysOut = g_pageEntries[i].m_pAddress << 12;
	
	uint32_t retaddr = (g_pageAllocationBase + (i << 12));
	MmInvalidateSinglePage(retaddr);
	
	return (void*)retaddr;
}
void* MmAllocateSinglePagePhyD(uint32_t* pPhysOut, const char* callFile, int callLine)
{
	// find a free pageframe.
	// For 4096 bytes we can use ANY hole in the pageframes list, and we
	// really do not care.
	
	int heapSize = GetHeapSize();
	for (int i = 0; i < heapSize; i++)
	{
		if (!g_pageEntries[i].m_bPresent) // A non-allocated pageframe?
		{
			return MmSetupPage(i, pPhysOut, callFile, callLine);
		}
	}
	// No more page frames?!
	LogMsg("WARNING: No more page entries");
	return NULL;
}
void* MmAllocateSinglePageD(const char* callFile, int callLine)
{
	return MmAllocateSinglePagePhyD(NULL, callFile, callLine);
}
void MmFreePage(void* pAddr)
{
	if (!pAddr) return;
	// Turn this into a g_pageEntries index.
	uint32_t addr = (uint32_t)pAddr;
	
	//safety measures:
	if (addr >= 0xC0000000)
	{
		LogMsg("Can't free kernel memory!");
		return;
	}
	if (g_pHeap && addr >= 0x80000000)
	{
		LogMsg("Can't free from the kernel heap while you're using a user heap!");
		return;
	}
	if (!g_pHeap && addr < 0x80000000)
	{
		LogMsg("You aren't using a user heap!");
		return;
	}
	
	addr -= g_pageAllocationBase;
	addr >>= 12;
	
	if (!g_pageEntries[addr].m_bPresent)
	{
		//lol?
		return;
	}
	// Yes. Let's mark this as present, and return a made-up address from the index.
	g_pageEntries[addr].m_bPresent = false;
	g_memoryAllocationSize[addr] = 0;
}
//be sure to provide a decently sized physAddresses, or NULL
void *MmAllocatePhyD (size_t size, const char* callFile, int callLine, uint32_t* physAddresses)
{
	if (size <= 0x1000) //worth one page:
		return MmAllocateSinglePagePhyD(physAddresses, callFile, callLine);
	else {
		//more than one page, take matters into our own hands:
		int numPagesNeeded = ((size - 1) >> 12) + 1;
		//ex: if we wanted 6100 bytes, we'd take 6100-1=6099, then divide that by 4096 (we get 1) and add 1
		//    if we wanted 8192 bytes, we'd take 8192-1=8191, then divide that by 4096 (we get 1) and add 1 to get 2 pages
		
		int heapSize = GetHeapSize();
		for (int i = 0; i < heapSize; i++)
		{
			// A non-allocated pageframe?
			if (!g_pageEntries[i].m_bPresent)
			{
				// Yes.  Are there at least numPagesNeeded holes?
				int jfinal = i + numPagesNeeded;
				for (int j = i; j < jfinal; j++)
				{
					//Are there any already taken pages before we reach the end.
					if (g_pageEntries[j].m_bPresent)
					{
						//Yes.  This hole isn't large enough.
						//NOTE that THIS is why we need more levels of break than just "break". 
						//I'll just use a goto then.
						i = j;
						goto _label_continue;
					}
				}
				// Nope! We have space here!  Let's map all the pages, and return the address of the first one.
				uint32_t* pPhysOut = physAddresses;
				
				void* pointer = MmSetupPage(i, pPhysOut, callFile, callLine);
				// if not null, increment by 4.
				// if it WERE null and we DID increment by four, it would throw a pagefault
				if (pPhysOut)
					pPhysOut++;
				
				// Not to forget, set the memory allocation size below:
				g_memoryAllocationSize[i] = numPagesNeeded - 1;
				
				for (int j = i+1; j < jfinal; j++)
				{
					MmSetupPage (j, pPhysOut, callFile, callLine);
					// if not null, increment by 4.
					if (pPhysOut)
						pPhysOut++;
				}
				return pointer;
			}
		_label_continue:;
		}
		return NULL; //no continuous addressed pages are left.
	}
}
void *MmAllocateD (size_t size, const char* callFile, int callLine)
{
	return MmAllocatePhyD(size, callFile, callLine, NULL);
}
void MmFree(void* pAddr)
{
	if (!pAddr) return; //handle (hopefully) accidental NULL freeing
	
	// Free the first page, but before we do, save its g_memoryAllocationSize.
	uint32_t addr = (uint32_t)pAddr;
	addr -= g_pageAllocationBase;
	addr >>= 12;
	if (addr >= (uint32_t)GetHeapSize()) return;
	
	int nSubsequentAllocs = g_memoryAllocationSize[addr];
	
	MmFreePage(pAddr);
	pAddr = (void*)((uint8_t*)pAddr+0x1000);
	for (int i = 0; i<nSubsequentAllocs; i++)
	{
		MmFreePage(pAddr);
		pAddr = (void*)((uint8_t*)pAddr+0x1000);
	}
}

uint32_t* MmGetKernelPageDir()
{
	return g_pageDirectory;
}
uint32_t MmGetKernelPageDirP()
{
	return (uint32_t)g_pageDirectory - BASE_ADDRESS;
}
void MmDebugDump()
{
	int entryCount = 0;
	LogMsg("MmDebugDump: dumping memory allocations on heap 0x%x:", g_pHeap);
	for(int i = 0; i < GetHeapSize(); i++)
	{
		if (g_pageEntries[i].m_bPresent) {
			//S: subsequent mempages, A: author file, AL: author line
			LogMsg("%x:  phys address:%x  P%s%s%s%s S:%d A:%s AL:%d",
				i,
				g_pageEntries[i].m_pAddress<<12,
				&" \0W\0"[g_pageEntries[i].m_bReadWrite<<1],
				&" \0S\0"[g_pageEntries[i].m_bUserSuper<<1],
				&" \0A\0"[g_pageEntries[i].m_bAccessed<<1],
				&" \0D\0"[g_pageEntries[i].m_bDirty<<1],
				g_memoryAllocationSize[i],
				g_memoryAllocationAuthor[i],
				g_memoryAllocationAuthorLine[i]
			);
			entryCount++;
		}
	}
	LogMsgNoCr("There are %d unfreed allocations.", entryCount);
	if (entryCount)
		LogMsg("");
	else
		LogMsg(" Either you're leak-free or you haven't actually allocated anything.");
}
