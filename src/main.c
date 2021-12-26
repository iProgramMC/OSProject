#include <main.h>
#include <memory.h>
#include <vga.h>
#include <video.h>
#include <print.h>
#include <idt.h>
#include <keyboard.h>
#include <elf.h>
#include <multiboot.h>
#include <shell.h>

__attribute__((noreturn))
void KeStopSystem()
{
	cli;
	while (1)
		hlt;
}

extern uint32_t e_placement;
int g_nKbExtRam = 0;

void KePrintSystemVersion()
{
	LogMsg("NanoShell (TM), December 2021 - " VersionString);
	LogMsg("[%d Kb System Memory]", g_nKbExtRam);
}
void TestAllocFunctions()
{
	void *pPage = MmAllocateSinglePage();
	LogMsg("pPage address: 0x%x", pPage);
	MmFreePage(pPage);
	
	// try allocating something:
	void *a = MmAllocate (8100); // 2 pages
	void *b = MmAllocate(12000); // 3 pages
	
	*((uint32_t*)a) = 0xAAAA;
	*((uint32_t*)b) = 0xBBBB;
	
	LogMsg("A: 0x%x, B: 0x%x, Av: 0x%x, Bv: 0x%x", a, b, *((uint32_t*)a), *((uint32_t*)b));
	
	MmFree(a);
	void *c = MmAllocate(12000); //3 pages, should not have same address as a
	void *d = MmAllocate (4000); //only one page, it should have the same addr as a
	*((uint32_t*)c) = 0xCCCC;
	*((uint32_t*)d) = 0xDDDD;
	LogMsg("C: 0x%x, D: 0x%x, Cv: 0x%x, Dv: 0x%x, Bv: 0x%x", a, b, *((uint32_t*)c), *((uint32_t*)d), *((uint32_t*)b));
	
	MmFree(a);
	MmFree(b);
	MmFree(c);
	MmFree(d);
	a = b = c = d = NULL;
}
void TestHeapFunctions()
{
	// print a hello string, so we know that the 0xC0000000 memory range (where kernel is located) is ok
	LogMsg("Testing out the heap functions!  Heap size: %d", GetHeapSize());
	
	// let's try allocating something:
	void* aa = MmAllocate (1024);
	LogMsg("   Got pointer: 0x%x", aa);
	
	// print out what we've allocated so far
	MmDebugDump();
	
	// try writing to there
	*((uint32_t*)aa + 50) = 0x12345678;
	
	// and reading
	LogMsg("   What we just wrote at 0x%x: 0x%x", aa, *((uint32_t*)aa + 50));
	
	// and finally, free the thing:
	MmFree(aa);
}
void TestHeap()
{
	Heap testHeap;
	AllocateHeap (&testHeap, 64);
	UseHeap (&testHeap);
	TestHeapFunctions();
	FreeHeap (&testHeap);
}
void FreeTypeThing()
{
	LogMsgNoCr("\nType something! >");
	
	char test[2];
	test[1] = '\0';
	while (1)
	{
		char k = KbWaitForKeyAndGet();
		test[0] = k;
		LogMsgNoCr(test);
		hlt;
	}
}

__attribute__((noreturn))
void KeStartupSystem (unsigned long magic, unsigned long mbaddr)
{
	CoInitAsE9Hack (&g_debugConsole);
	// Initialise the terminal.
	g_debugConsole.color = DefaultConsoleColor;//default
	//SwitchMode(0);
	//PrInitialize();
	
	// Check the multiboot stuff
	if (magic != 0x2badb002)
	{
		//LogMsg("Sorry, this ain't a compatible multiboot bootloader. %x", magic);
		KeStopSystem();
	}
	mbaddr += 0xc0000000; //turn it virtual straight away
	
	multiboot_info_t *mbi = (multiboot_info_t*)mbaddr;
	
	g_nKbExtRam = mbi->mem_upper;
	MmFirstThingEver(g_nKbExtRam);
	
	if (g_nKbExtRam < 8192)
	{
		//LogMsg("NanoShell has not found enough extended memory.  8Mb of extended memory is\nrequired to run NanoShell.  You may need to upgrade your computer.");
		KeStopSystem();
	}
	KeIdtInit();
	
	//print the hello text, to see if the os booted properly
	KePrintSystemVersion();
	
	MmInit();
	
	VidInitialize (mbi);
	
	//TestAllocFunctions();
	//ElfPerformTest();
	//KePrintSystemInfo();
	//TestHeap();
	
	//MmDebugDump();
	//FreeTypeThing();
	
	//ShellRun();
	LogMsg("Kernel ready to shutdown.");
	KeStopSystem();
}