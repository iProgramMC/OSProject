#include <main.h>
#include <memory.h>
#include <print.h>

void KeStopSystem()
{
	cli;
	while (1)
		hlt;
}

extern uint32_t e_placement;
void KeStartupSystem (unsigned long magic, unsigned long mbi)
{
	mbi += 0xc0000000;
	MmFirstThingEver(mbi);
	
	PrInitialize();
	
	//print the hello text, to see if the os booted properly
	LogMsg("iProgramInCpp's Operating System " VersionString "\nmultiboot parms:");
	LogInt(magic);
	LogInt(mbi);
	LogMsg("\nHello world!\n\n");
	
	MmInitMemoryManager();
	LogInt (e_placement);
	LogMsg("\n");
	
	void *pPage = MmAllocateSinglePage();
	LogInt((int)pPage);
	MmFreePage(pPage);
	LogMsg("\n");
	
	// try allocating something:
	void *a = MmAllocate (8100); // 2 pages
	void *b = MmAllocate(12000); // 3 pages
	
	*((uint32_t*)a) = 0xAAAA;
	*((uint32_t*)b) = 0xBBBB;
	
	LogInt ((int)a);
	LogInt ((int)b);
	LogMsg("   ----   ");
	LogInt (*((uint32_t*)a));
	LogInt (*((uint32_t*)b));
	LogMsg("\n");
	
	MmFree(a);
	void *c = MmAllocate(12000); //3 pages, should not have same address as a
	void *d = MmAllocate (4000); //only one page, it should have the same addr as a
	*((uint32_t*)c) = 0xCCCC;
	*((uint32_t*)d) = 0xDDDD;
	LogInt ((int)c);
	LogInt ((int)d);
	LogMsg("   ----   ");
	LogInt (*((uint32_t*)c));
	LogInt (*((uint32_t*)d));
	LogInt (*((uint32_t*)b));
	LogMsg("\n");
	
	MmFree(a);
	MmFree(b);
	MmFree(c);
	MmFree(d);
	a = b = c = d = NULL;
	
	KePrintSystemInfo();
	
	KeStopSystem();
}