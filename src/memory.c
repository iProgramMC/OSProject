#include <memory.h>

extern uint32_t g_pageDirectory[];
extern uint32_t g_pageTableArray[];
extern uint32_t* e_frameBitsetVirt;
extern uint32_t e_frameBitsetSize;
extern uint32_t e_placement;

extern void Move(); //earlybird.asm
void KeFirstThingEver(unsigned long mbiAddr)
{
	int nKbExtRam = ((uint32_t*)mbiAddr)[2]; //TODO: use multiboot_info_t struct
	Move();
	e_frameBitsetSize = (nKbExtRam >> 2); //nBytesRAM >> 12 = (nKbExtRam << 10) >> 12 = nKbExtRam >> 2
	e_placement += e_frameBitsetSize;
}

// Can allocate up to 256 MB of RAM.  No need for more I think,
// but if there is a need, just increase this. Recommend a Power of 2
#define PAGE_ENTRY_TOTAL 65536

PageEntry g_pageEntries   [PAGE_ENTRY_TOTAL]; 
int       g_pageEntriesNum   = 0;

// Simple heap: The start PageEntry of a big memalloc also includes the number of pageEntries that also need to be freed
// For this reason freeing just one page from a larger allocation is _not_ enough, you need to use KeFree instead
// (unless you used KeAllocageSinglePage to allocate it)
int g_memoryAllocationSize[PAGE_ENTRY_TOTAL];

#define PAGE_BITS 0X3 //present and read/write
#define PAGE_ALLOCATION_BASE 0x800000

uint32_t g_memoryStart;

#define  INDEX_FROM_BIT(a) (a / 32)
#define OFFSET_FROM_BIT(a) (a % 32)
static void SetFrame (uint32_t frameAddr)
{
	uint32_t frame = frameAddr >> 12;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	e_frameBitsetVirt[idx] |= (0x1 << off);
}
/*
static void ClrFrame (uint32_t frameAddr)
{
	uint32_t frame = frameAddr >> 12;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	e_frameBitsetVirt[idx] &=~(0x1 << off);
}
static bool TestFrame(uint32_t frameAddr) 
{
	uint32_t frame = frameAddr >> 12;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	return (e_frameBitsetVirt[idx] & (1 << off)) != 0;
}*/
static uint32_t FindFreeFrame()
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
	LogMsg("WARNING: No more free memory?!\n");
	return 0xffffffffu/*ck you*/;
}

void InvalidateTLB() {
	__asm__("movl %cr3, %ecx\n\tmovl %ecx, %cr3\n\t");
}
void MapPages()
{
	for (uint32_t i=0; i<INDEX_FROM_BIT(e_frameBitsetSize); i++)
	{
		e_frameBitsetVirt[i] = 0;
	}
	uint32_t paddr = g_memoryStart >> 12;
	for (uint32_t i=0; i<PAGE_ENTRY_TOTAL; i++)
	{
		//Assign its address (TODO: but DON'T mark it as present.  We'll do that later)
		//g_pageEntries[i].m_pAddress = paddr;
		//g_pageEntries[i].m_bPresent = true;
		//g_pageEntries[i].m_bReadWrite = true;
		//paddr++;
		
		*((uint32_t*)(g_pageEntries + i)) = 0;
		g_memoryAllocationSize[i] = 0;
	}
	int index = 2;
	for (uint32_t i=0; i<65536; i += 1024)
	{
		PageTable* pPageTable = (PageTable*)&g_pageEntries[i];
		//0x00800000
		
		//TODO: Normally we'd need 2.. but here we need 1?!?
		g_pageDirectory[index] = ((uint32_t)pPageTable-BASE_ADDRESS) | PAGE_BITS;//present + readwrite
		index++;
	}
	InvalidateTLB();
}
int g_offset = 0;
void KeInitMemoryManager()
{
	e_placement += 0x1000;
	e_placement &= ~0xFFF;
	
	g_offset = e_placement >> 12;
	
	g_memoryStart = e_placement;
	
	MapPages();

	#define PROBE_ADDRESS 0x00800000
//	for (int PROBE_ADDRESS = 0x800000; PROBE_ADDRESS <= 0x10000000; PROBE_ADDRESS += 0x100000)
	/*{
		uint32_t* pointer = (uint32_t*)PROBE_ADDRESS;
		
		*pointer = 0xADADADAD;
		
		LogInt(PROBE_ADDRESS);
		LogMsg("= ");
		LogInt(*pointer);
		//LogMsg(" | ");
		//break;
	}*/
}

void* KeSetupPage(int i, uint32_t* pPhysOut)
{
	uint32_t frame = FindFreeFrame();
	if (frame == 0xffffffffu/*ck you*/)
		return NULL;

	SetFrame(frame << 12);
	
	// Yes. Let's mark this as present, and return a made-up address from the index.
	g_pageEntries[i].m_bPresent = true;
	g_pageEntries[i].m_bReadWrite = true;
	g_pageEntries[i].m_bUserSuper = true;
	g_pageEntries[i].m_pAddress = frame + g_offset;
	
	g_memoryAllocationSize[i] = 0;
	
	// if pPhysOut is not null, we would probably want that too:
	if (pPhysOut)
		*pPhysOut = g_pageEntries[i].m_pAddress << 12;
	
	InvalidateTLB();
	return (void*)(PAGE_ALLOCATION_BASE + (i << 12));
}
void* KeAllocateSinglePagePhy(uint32_t* pPhysOut)
{
	// find a free pageframe.
	// For 4096 bytes we can use ANY hole in the pageframes list, and we
	// really do not care.
	
	for (int i = 0; i < PAGE_ENTRY_TOTAL; i++)
	{
		if (!g_pageEntries[i].m_bPresent) // A non-allocated pageframe?
		{
			return KeSetupPage(i, pPhysOut);
		}
	}
	// No more page frames?!
	LogMsg("WARNING: No more page entries\n");
	return NULL;
}
void* KeAllocateSinglePage()
{
	return KeAllocateSinglePagePhy(NULL);
}
void KeFreePage(void* pAddr)
{
	if (!pAddr) return;
	// Turn this into a g_pageEntries index.
	uint32_t addr = (uint32_t)pAddr;
	addr -= PAGE_ALLOCATION_BASE;
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
void *KeAllocate (size_t size)
{
	if (size <= 0x1000) //worth one page:
		return KeAllocateSinglePage();
	else {
		//more than one page, take matters into our own hands:
		int numPagesNeeded = ((size - 1) >> 12) + 1;
		//ex: if we wanted 6100 bytes, we'd take 6100-1=6099, then divide that by 4096 (we get 1) and add 1
		//    if we wanted 8192 bytes, we'd take 8192-1=8191, then divide that by 4096 (we get 1) and add 1 to get 2 pages
		
		for (int i = 0; i < PAGE_ENTRY_TOTAL; i++)
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
				void* pointer = KeSetupPage(i, NULL);
				
				// Not to forget, set the memory allocation size below:
				g_memoryAllocationSize[i] = numPagesNeeded - 1;
				
				for (int j = i+1; j < jfinal; j++)
				{
					KeSetupPage (j, NULL);
				}
				return pointer;
			}
		_label_continue:;
		}
		return NULL; //no continuous addressed pages are left.
	}
}
void KeFree(void* pAddr)
{
	if (!pAddr) return; //handle (hopefully) accidental NULL freeing
	
	// Free the first page, but before we do, save its g_memoryAllocationSize.
	uint32_t addr = (uint32_t)pAddr;
	addr -= PAGE_ALLOCATION_BASE;
	addr >>= 12;
	if (addr >= PAGE_ENTRY_TOTAL) return;
	
	int nSubsequentAllocs = g_memoryAllocationSize[addr];
	
	KeFreePage(pAddr);
	pAddr = (void*)((uint8_t*)pAddr+0x1000);
	for (int i = 0; i<nSubsequentAllocs; i++)
	{
		KeFreePage(pAddr);
		pAddr = (void*)((uint8_t*)pAddr+0x1000);
	}
}


