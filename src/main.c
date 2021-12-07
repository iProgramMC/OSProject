#include <main.h>
#include <multiboot.h>
#include <console.h>
#include <memory.h>
#include <task.h>
#include <idt.h>

bool g_verboseBoot = true; //false;

void VerboseLog (const char* pText)
{
	if (g_verboseBoot) LogMsgNoCr(pText);
}

void StopSystem()
{
	cli;
	while (1)
		hlt;
}
extern size_t g_memoryUsable;

void TaskA(VariantList* unused)
{
	unused += 0;
	
	while (1) 
	{
		//LogMsg("Hello from task A!");
		LogMsgNoCr("A");
		//useless delay
		for (int i = 0; i < 100000; i++)
			i += 0;
	}
}
void TaskB(VariantList* unused)
{
	unused += 0;
	
	while (1) 
	{
		//LogMsg("Hello from task B!");
		LogMsgNoCr("B");
		//useless delay
		for (int i = 0; i < 100000; i++)
			i += 0;
	}
}
void TaskK()
{
	while (1) 
	{
		//LogMsg("Hello from task K!");
		LogMsgNoCr("K");
		//useless delay
		for (int i = 0; i < 100000; i++)
			i += 0;
	}
}


void OnPanicTriggered (const char* pFile, int nLine, const char* pReason, bool bIsAssert)
{
	//change color to red
	g_debugConsole.color = 0x1F;//not 0xC as background cuz its the same as 4 but flashing
	//then print out the details of the panic
	
	LogMsg("[!] KERNEL %s [!]", bIsAssert ? "ASSERT" : "PANIC");
	LogMsg("Requested by: %s:%d", pFile, nLine);
	LogMsg("\n%s%s", bIsAssert ? "Condition: " : "Reason: ", pReason);
	
	StopSystem();
}

void StartupSystem (unsigned long magic, multiboot_info_t *mbi)
{
	//initialise console
	CoInitAsText (&g_debugConsole);
	ASSERT (magic == MULTIBOOT_BOOTLOADER_MAGIC && "Invalid multiboot header");
	
	ResetConsole();
	
	//print the hello text, to see if the os booted properly
	LogMsg("NanoShell Operating System " VersionString);
	
	//initialize memory subsystem
	VerboseLog ("1>Initializing memory subsystem");
	
	//take away the first few megs of kernel memory, we'll use that for our own rodata/etc.
	//the other meg is taken out of the other end of memory, it's just to be safe.
	
	size_t memoryStart = 0x380000;
	
	g_memoryUsable = (mbi->mem_upper) * 1024 - memoryStart;
	g_memory = (uint8_t*)memoryStart;//start at 3 megs. TODO: make memoryStart automatically adjust itself.
	
	//report if we can't initialize memory
	bool memoryCheck = g_memoryUsable < 0x500000;
	
	//maybe this check did succeed, so let's also check for potential overflows
	if (mbi->mem_upper < g_memoryUsable/1024) memoryCheck |= true;
	
	if (memoryCheck) //need 8 meg of ram to run
	{
		VerboseLog("[FAIL]\n\n");
		LogMsg ("The OS needs 8 MB of extended memory to run.");
		StopSystem();
	}
	
	LogMsg("[%d Kb System Memory, %d Kb Usable Memory]\n", mbi->mem_upper, g_memoryUsable/1024);
	
	MmInit();
	
	VerboseLog ("[OK]\n");
	
	//initialize task subsystem
	VerboseLog ("1>Initializing task subsystem");
	InitializeTaskSystem();
	VerboseLog ("[OK]\n");
	
	//initialize our IDT
	VerboseLog ("1>Setting up our IDT");
	InitIDT();
	InitPIT();
	VerboseLog ("[OK]\n");
	
	sti;
	
	//LogMsg("Spawning some tasks.");
	
	//test();
	
	// Initialize the Programmable Interval Timer (PIT) too
	
	//int errorCode = 0;
	//StartTask(TaskA, NULL, &errorCode);
	//LogMsg("Task A started with error code: %d", errorCode);
	//StartTask(TaskB, NULL, &errorCode);
	//LogMsg("Task B started with error code: %d", errorCode);
	
	//LogMsg("Starting mainthread task");
	//TaskK();
	
	LogMsg("Hello, world! ");
	
	StopSystem();
}