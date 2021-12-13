#include <main.h>
#include <memory.h>
#include <vga.h>

uint16_t* g_pBufferBase = (uint16_t*)(KERNEL_MEM_START + 0xB8000);
uint16_t* g_pBuffer = (uint16_t*)(KERNEL_MEM_START + 0xB8000);

int g_textX, g_textY;
int g_textWidth = 80, g_textHeight = 25;

void LogMsg(const char* pText)
{
	while (*pText)
	{
		if (*pText == '\n')
		{
			g_textX = 0;
			g_textY++;
			if (g_textY == g_textHeight)
			{
				g_textY = 0;
			}
			pText++;
			continue;
		}
		g_pBuffer[g_textY * g_textWidth + g_textX] = *pText | 0x1F00;
		pText++;
		g_textX++;
		if (g_textX == g_textWidth)
		{
			g_textX = 0;
			g_textY++;
			if (g_textY == g_textHeight)
			{
				g_textY = 0;
			}
		}
	}
}
void LogInt(uint32_t toPrint)
{
	char test[12];
	test[0] = '0', test[1] = 'x';
	char* memory = test + 2;
	
	uint32_t power = (15 << 28), pt = 28;
	for (; power != 0; power >>= 4, pt -= 4) {
		uint32_t p = toPrint & power;
		p >>= pt;
		*memory = "0123456789abcdef"[p];
		memory++;
	}
	*memory++ = ' ';
	*memory++ = '\0';
	
	LogMsg(test);
}

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
	KeFirstThingEver(mbi);
	
	SwitchMode (1);
	g_textX = g_textY = 0;
	
	//print the hello text, to see if the os booted properly
	LogMsg("NanoShell Operating System " VersionString "\n");
	LogInt(magic);
	LogInt(mbi);
	LogMsg("\n");
	//#define PROBE_ADDRESS 0xC07ffffc
	//*((uint32_t*)PROBE_ADDRESS) = 0xF00DF00D;
	//LogMsg("= ");
	//LogInt (*((uint32_t*)PROBE_ADDRESS));
	LogMsg("\nhaha\n\n");
	
	KeInitMemoryManager();
	LogInt (e_placement);
	LogMsg("\n");
	
	void *pPage = KeAllocateSinglePage();
	LogInt((int)pPage);
	KeFreePage(pPage);
	LogMsg("\n");
	
	// try allocating something:
	void *a = KeAllocate (8100); // 2 pages
	void *b = KeAllocate(12000); // 3 pages
	
	InvalidateTLB();
	*((uint32_t*)a) = 0xFECC;
	
	LogInt ((int)a);
	//LogInt (*((uint32_t*)a));
	LogInt ((int)b);
	LogMsg("\n");
	
	KeFree(a);
	void *c = KeAllocate(12000); //3 pages, should not have same address as a
	void *d = KeAllocate (4000); //only one page, it should have the same addr as a
	LogInt ((int)c);
	LogInt ((int)d);
	LogMsg("\n");
	
	KeFree(a);
	KeFree(b);
	KeFree(c);
	KeFree(d);
	a = b = c = d = NULL;
	
	KeStopSystem();
}