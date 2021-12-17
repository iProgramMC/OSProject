#include <main.h>
#include <memory.h>
#include <print.h>
#include <idt.h>
#include <keyboard.h>
#include <elf.h>

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
	
	KeIdtInit();
	
	//print the hello text, to see if the os booted properly
	LogMsg("iProgramInCpp's Operating System " VersionString "\nmultiboot parms: %x %x", magic, mbi);
	
	MmInit();
	
	void *pPage = MmAllocateSinglePage();
	LogMsg("pPage address: 0x%x", pPage);
	MmFreePage(pPage);
	
	// try allocating something:
	void *a = MmAllocate (8100); // 2 pages
	void *b = MmAllocate(12000); // 3 pages
	
	*((uint32_t*)a) = 0xAAAA;
	*((uint32_t*)b) = 0xBBBB;
	
	LogMsg("A: 0x%x, B: 0x%x, Aval: 0x%x, Bval: 0x%x", a, b, *((uint32_t*)a), *((uint32_t*)b));
	
	MmFree(a);
	void *c = MmAllocate(12000); //3 pages, should not have same address as a
	void *d = MmAllocate (4000); //only one page, it should have the same addr as a
	*((uint32_t*)c) = 0xCCCC;
	*((uint32_t*)d) = 0xDDDD;
	LogMsg("C: 0x%x, D: 0x%x, Cval: 0x%x, Dval: 0x%x, Bval: 0x%x", a, b, *((uint32_t*)c), *((uint32_t*)d), *((uint32_t*)b));
	
	MmFree(a);
	MmFree(b);
	MmFree(c);
	MmFree(d);
	a = b = c = d = NULL;
	elf_test();
	
	KePrintSystemInfo();
	
	LogMsgNoCr("\nType something! >");
	
	char test[2];
	test[1] = '\0';
	while (1)
	{
		char k = KbWaitForKeyAndGet();
		test[0] = k;
		LogMsg(test);
		hlt;
	}
	
	//__asm__("int $0x80\n\t");
	
	
	KeStopSystem();
}