#include <memory.h>

extern uint32_t g_pageDirectory[];
extern uint32_t g_pageTableArray[];
extern uint32_t e_frameBitsetVirt[];
extern uint32_t e_frameBitsetSize;
extern uint32_t e_placement;

// Can allocate up to 256 MB of RAM.  No need for more I think,
// but if there is a need, just increase this.
PageEntry g_pageEntries   [65536]; 
int       g_pageEntriesNum   = 0;

#define PAGE_BITS 0X3 //present and read/write

uint32_t g_memoryStart;
void InvalidateTLB() {
	__asm__("movl %cr3, %ecx\n\tmovl %ecx, %cr3\n\t");
}
void MapPages()
{
	uint32_t paddr = g_memoryStart >> 12;
	for (int i=0; i<65536; i++)
	{
		//Assign its address (TODO: but DON'T mark it as present.  We'll do that later)
		g_pageEntries[i].m_pAddress = paddr;
		g_pageEntries[i].m_bPresent = true;
		g_pageEntries[i].m_bReadWrite = true;
		paddr++;
	}
	for (int i=0; i<=65536; i += 1024)
	{
		PageTable* pPageTable = (PageTable*)&g_pageEntries[i];
		//0x00800000
		g_pageDirectory[2 + i>>10] = ((uint32_t)pPageTable-BASE_ADDRESS) | PAGE_BITS;//present + readwrite
	}
	InvalidateTLB();
}

void KeInitMemoryManager()
{
	e_placement += 0x1000;
	e_placement &= ~0xFFF;
	
	g_memoryStart = e_placement;
	
	MapPages();

	#define PROBE_ADDRESS 0x800000
	//for (int PROBE_ADDRESS = 0x800000; PROBE_ADDRESS <= 0x10000000; PROBE_ADDRESS += 0x100000)
	{
		uint32_t* pointer = (uint32_t*)PROBE_ADDRESS;
		
		*pointer = 0xADADADAD;
		
		LogInt(PROBE_ADDRESS);
		LogMsg("= ");
		LogInt(*pointer);
		//LogMsg(" | ");
	}
}

#define  INDEX_FROM_BIT(a) (a / 32)
#define OFFSET_FROM_BIT(a) (a % 32)

static void SetFrame (uint32_t frameAddr)
{
	uint32_t frame = frameAddr / 0x1000;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	e_frameBitsetVirt[idx] |= (0x1 << off);
}
static void ClrFrame (uint32_t frameAddr)
{
	uint32_t frame = frameAddr / 0x1000;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	e_frameBitsetVirt[idx] &=~(0x1 << off);
}
static bool TestFrame(uint32_t frameAddr) 
{
	uint32_t frame = frameAddr / 0x1000;
	uint32_t idx = INDEX_FROM_BIT (frame),
			 off =OFFSET_FROM_BIT (frame);
	return (e_frameBitsetVirt[idx] & (1 << off)) != 0;
}
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
	return 0xffffffffu/*ck you*/;
}

void *KeAllocateOnePageWorthOfMemory ()
{
	
}

