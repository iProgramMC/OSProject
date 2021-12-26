#include <main.h>
#include <vga.h>
#include <video.h>
#include <memory.h>

//! TODO: perhaps merge vga.c with this?

bool g_isVideoMode = false;
uint32_t g_framebufferPhysical = 0;

extern uint32_t *g_curPageDir;

//! TODO: if planning to extend this beyond 1920x1080x32, extend THIS variable.
#define MAX_VIDEO_PAGES 2048
uint32_t g_vbePageEntries[MAX_VIDEO_PAGES] __attribute__((aligned(4096))); 

void VidInitialize(multiboot_info_t* pInfo)
{
	if (pInfo->flags & MULTIBOOT_INFO_VIDEO_INFO)
	{
		// map shit to 0xE0000000
		int index = 0x380;
		uint32_t pointer = pInfo->framebuffer_addr;
		LogMsg("VBE Pointer: %x", pointer);
		for (int i = 0; i < MAX_VIDEO_PAGES; i++)
		{
			g_vbePageEntries[i] = pointer | 0X3;
			pointer += 0x1000;
		}
		for (int i = 0; i < MAX_VIDEO_PAGES; i += 1024)
		{
			LogMsg("Mapping %d-%d to %dth page table pointer", i,i+1023, index);
			uint32_t pageTable = ((uint32_t)&g_vbePageEntries[i]) - BASE_ADDRESS;
			
			g_curPageDir[index] = pageTable | 0X3;
			index++;
		}
		
		MmTlbInvalidate();
		
		// plot a single white pixel, to test that everything works ok
		*((uint32_t*)0xE0000000) = 0xFFFFFFFF;
		
		// then freeze, as we're done for now	
	}
	else
	{
		//SwitchMode (0);
		//CoInitAsText(&g_debugConsole);
		LogMsg("Warning: no VBE mode specified.");
	}
}
