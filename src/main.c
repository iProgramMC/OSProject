#include <main.h>

uint16_t* g_pBufferBase = (uint16_t*)(KERNEL_MEM_START + 0xB8000);
uint16_t* g_pBuffer = (uint16_t*)(KERNEL_MEM_START + 0xB8000);

uint32_t VirtualToPhysical(void* pVirtual)
{
	uint32_t virtual = (uint32_t)pVirtual;
	if (virtual >= KERNEL_MEM_START && virtual < KERNEL_MEM_START+0x400000) //Size of 1 largepage
		return virtual - KERNEL_MEM_START;
	
	//don't know how to turn this to physical.
	return virtual;
}
void* PhysicalToVirtual(uint32_t physical)
{
	if (physical <= 0x400000)
		return (void*)(physical + KERNEL_MEM_START);
	else
		//don't know how to turn this to virtual.
		return (void*)(physical);
}

void LogMsg(const char* pText)
{
	while (*pText)
	{
		*g_pBuffer = 0x1F00 | *pText;
		pText++;
		g_pBuffer++;
		
		//Roll over
		if (g_pBuffer - g_pBufferBase >= 80*25)
			g_pBuffer -= 80*25;
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

void KeStartupSystem (unsigned long magic, unsigned long mbi)
{
	//print the hello text, to see if the os booted properly
	LogMsg("NanoShell Operating System " VersionString);
	LogInt(magic);
	LogInt(mbi);
	
	KeStopSystem();
}