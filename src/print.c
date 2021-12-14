#include <print.h>
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
void LogIntDec(int toPrint)
{
	if (toPrint < 0)
	{
		LogMsg("-");
		toPrint = -toPrint;
	}
	if (!toPrint) {
		LogMsg("0 ");
		return;
	}
	uint32_t new = 0;
	while (toPrint)
	{
		new = new * 10 + toPrint % 10;
		toPrint /= 10;
	}
	
	char test [11];
	int index = 0;
	while (new)
	{
		test[index++] = '0' + (new % 10);
		new /= 10;
	}
	test[index++] = ' ';
	test[index++] = '\0';
	
	LogMsg(test);
}

typedef struct
{
	char m_steppingID : 4;
	char m_model : 4;
	char m_familyID : 4;
	char m_processorType : 2;
	char m_reserved : 2;
	char m_extModelID: 4;
	char m_extendedFamilyID;
	char m_reserved1 : 4;
}
__attribute__((packed))
CPUIDFeatureBits;

extern void KeCPUID();
extern uint32_t g_cpuidLastLeaf;
extern char g_cpuidNameEBX[];
extern CPUIDFeatureBits g_cpuidFeatureBits;

void KePrintSystemInfo()
{
	KeCPUID();
	LogMsg("Information about the system:\n");
	LogMsg("CPU Type: "); LogMsg(g_cpuidNameEBX);
	LogMsg("\nFeature bits: "); LogInt(*((int*)&g_cpuidFeatureBits));
	LogMsg("\nx86 Family "); LogIntDec(g_cpuidFeatureBits.m_familyID);
	LogMsg("Model "); LogIntDec(g_cpuidFeatureBits.m_model);
	LogMsg("Stepping "); LogIntDec(g_cpuidFeatureBits.m_steppingID);
	LogMsg("\ng_cpuidLastLeaf: "); LogIntDec(g_cpuidLastLeaf);
	LogMsg("\n\n");
}

void PrInitialize()
{
	g_textX = g_textY = 0;
	SwitchMode(0);
}
