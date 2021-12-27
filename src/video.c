/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

             VBE Video module
******************************************/
#include <main.h>
#include <vga.h>
#include <video.h>
#include <memory.h>
#include <string.h>

//! TODO: perhaps merge vga.c with this?
#include "extra/fonts.h"

bool g_isVideoMode = false;
uint32_t g_framebufferPhysical = 0;

extern uint32_t *g_curPageDir;

//! TODO: if planning to extend this beyond 1920x1080x32, extend THIS variable.
#define MAX_VIDEO_PAGES 2048
#define FRAMEBUFFER_MAPPED_ADDR 0xE0000000
uint32_t g_vbePageEntries[MAX_VIDEO_PAGES] __attribute__((aligned(4096))); 
uint32_t* g_framebufferCopy = NULL;

typedef struct
{
	bool     m_available;			    //if the vbe display is available
	unsigned m_width, m_height, m_pitch;//bytes per row
	int      m_bitdepth;                //bits per pixel, only values we support: 0=8, 1=16, 2=32
	union {
		uint32_t* m_framebuffer32; //for ease of addressing
		uint16_t* m_framebuffer16;
		uint8_t * m_framebuffer8;
	};
	int m_pitch32, m_pitch16;      //uint32_t's and uint16_t's per row.
}
VBEData;

VBEData g_vbeData;

bool VidIsAvailable()
{
	return g_vbeData.m_available;
}
int GetScreenSizeX()
{
	return g_vbeData.m_width;
}
int GetScreenSizeY()
{
	return g_vbeData.m_height;
}
void VidInitializeVBEData(multiboot_info_t* pInfo)
{
	int bpp = pInfo->framebuffer_bpp;
	g_vbeData.m_available = true;
	g_vbeData.m_width     = pInfo->framebuffer_width;
	g_vbeData.m_height    = pInfo->framebuffer_height;
	if (bpp == 32)
		g_vbeData.m_bitdepth  = 2;
	else if (bpp == 16)
		g_vbeData.m_bitdepth  = 1;
	else
		g_vbeData.m_bitdepth  = 0;
	g_vbeData.m_pitch     = pInfo->framebuffer_pitch;
	g_vbeData.m_pitch32   = pInfo->framebuffer_pitch / sizeof (int);
	g_vbeData.m_pitch16   = pInfo->framebuffer_pitch / sizeof (short);
	
	g_vbeData.m_framebuffer32 = (uint32_t*)FRAMEBUFFER_MAPPED_ADDR;
	g_vbeData.m_framebuffer16 = (uint16_t*)FRAMEBUFFER_MAPPED_ADDR;
	g_vbeData.m_framebuffer8  = (uint8_t *)FRAMEBUFFER_MAPPED_ADDR;
}

typedef void (*PlotPixelFunction)(unsigned x, unsigned y, unsigned color);

//! Temporary placeholder functions:
char VidColor32to8(unsigned color)
{
	return color & 0xFF;
}
short VidColor32to16(unsigned color)
{
	return color & 0xFF;
}

void VidPlotPixelRaw8  (unsigned x, unsigned y, unsigned color)
{
	g_vbeData.m_framebuffer8 [x + y * g_vbeData.m_pitch  ] = VidColor32to8 (color);
}
void VidPlotPixelRaw16 (unsigned x, unsigned y, unsigned color)
{
	g_vbeData.m_framebuffer16[x + y * g_vbeData.m_pitch16] = VidColor32to16(color);
}
void VidPlotPixelRaw32 (unsigned x, unsigned y, unsigned color)
{
	g_vbeData.m_framebuffer32[x + y * g_vbeData.m_pitch32] = color;
}
void VidPlotPixel(unsigned x, unsigned y, unsigned color)
{
	if (x >= g_vbeData.m_width && y >= g_vbeData.m_height) return;
	switch (g_vbeData.m_bitdepth)
	{
		case 0: VidPlotPixelRaw8 (x, y, color); break;
		case 1: VidPlotPixelRaw16(x, y, color); break;
		case 2: VidPlotPixelRaw32(x, y, color); break;
	}
	g_framebufferCopy[x + y * g_vbeData.m_width] = color;
}
void VidPrintTestingPattern()
{
	for (int y = 0; y < GetScreenSizeY(); y++) 
	{
		for (int x = 0; x < GetScreenSizeX(); x++)
		{
			int pixel = (x + y) * 0x010101;
			VidPlotPixel(x, y, pixel);
		}
	}
}
void VidFillScreen(unsigned color)
{
	int color2 = color;
	switch (g_vbeData.m_bitdepth)
	{
		case 0:
			color2 = VidColor32to8(color);
			for (int y = 0; y < GetScreenSizeY(); y++) 
				for (int x = 0; x < GetScreenSizeX(); x++)
				{
					g_vbeData.m_framebuffer8 [x + y * g_vbeData.m_pitch  ] = color2;
					g_framebufferCopy[x + y * g_vbeData.m_width] = color;
				}
			break;
		case 1:
			color2 = VidColor32to16(color);
			for (int y = 0; y < GetScreenSizeY(); y++) 
				for (int x = 0; x < GetScreenSizeX(); x++)
				{
					g_vbeData.m_framebuffer16[x + y * g_vbeData.m_pitch16] = color2;
					g_framebufferCopy[x + y * g_vbeData.m_width] = color;
				}
			break;
		case 2:
			for (int y = 0; y < GetScreenSizeY(); y++) 
				for (int x = 0; x < GetScreenSizeX(); x++)
				{
					g_vbeData.m_framebuffer32[x + y * g_vbeData.m_pitch32] = color;
					g_framebufferCopy[x + y * g_vbeData.m_width] = color;
				}
			break;
	}
}
void VidFillRect(unsigned color, int left, int top, int right, int bottom)
{
	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right >= GetScreenSizeX()) right = GetScreenSizeX() - 1;
	if (bottom >= GetScreenSizeY()) bottom = GetScreenSizeY() - 1;
	
	for (int y = top; y <= bottom; y++)
	{
		for (int x = left; x <= right; x++)
			VidPlotPixel(x, y, color);
	}
}
// Font rendering
bool g_uses8by16Font = 0;

const unsigned char* g_fontIDToData[] = {
	g_FamiSans8x8,
	g_TamsynRegu8x16,
	g_TamsynBold8x16,
	g_PaperMFont8x16,
};
const unsigned char* g_pCurrentFont = NULL;
void VidSetFont(unsigned fontType)
{
	if (fontType >= FONT_LAST) 
	{
		LogMsg("Can't set the font to that! (%d)", fontType);
		return;
	}
	g_pCurrentFont  = g_fontIDToData[fontType];
	g_uses8by16Font = (g_pCurrentFont[1] == 16);
}
void VidPlotChar (char c, unsigned ox, unsigned oy, unsigned colorFg, unsigned colorBg /*=0xFFFFFFFF*/)
{
	if (!g_pCurrentFont) {
		SLogMsg("FUCK!");
		return;
	}
	int width = g_pCurrentFont[0], height = g_pCurrentFont[1];
	const unsigned char* test = g_pCurrentFont + 2;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0, bitmask = (1 << (width - 1)); x < width; x++, bitmask >>= 1)
		{
			if (test[c * height + y] & bitmask)
				VidPlotPixel(ox + x, oy + y, colorFg);
			else if (colorBg ^ 0xFFFFFFFF)
				VidPlotPixel(ox + x, oy + y, colorBg);
		}
	}
}
unsigned VidReadPixel (unsigned x, unsigned y)
{
	if (x >= (unsigned)GetScreenSizeX()) return 0;
	if (y >= (unsigned)GetScreenSizeY()) return 0;
	return g_framebufferCopy[x + y * GetScreenSizeX()];
}
void VidShiftScreen (int howMuch)
{
	if (howMuch >= GetScreenSizeY())
		return;
	/*for (int i = howMuch; i < GetScreenSizeY(); i++) {
		for (int k = 0; k < GetScreenSizeX(); k++) {
			VidPlotPixel (k, i-howMuch, VidReadPixel (k, i));
		}
	}*/
	
	if (g_vbeData.m_bitdepth == 2)
	{
		int a = g_vbeData.m_width * 4;
		for (int i = howMuch, j = 0, k = 0; i < GetScreenSizeY(); i++, j += g_vbeData.m_pitch, k += a)
		{
			fast_memcpy(((uint8_t*)g_vbeData.m_framebuffer32 + j), &g_framebufferCopy[i * g_vbeData.m_width], a);
			fast_memcpy(((uint8_t*)g_framebufferCopy         + k), &g_framebufferCopy[i * g_vbeData.m_width], a);
		}
	}
	else
	{
		;//unhandled
	}
}

//present, read/write, user/supervisor, writethrough
#define VBE_PAGE_BITS (1 | 2 | 4 | 8)
void VidInitialize(multiboot_info_t* pInfo)
{
	cli;
	g_vbeData.m_available = false;
	if (pInfo->flags & MULTIBOOT_INFO_VIDEO_INFO)
	{
		if (pInfo->framebuffer_type != 1)
		{
			LogMsg("Need direct RGB framebuffer!");
			sti;
			return;
		}
		// map shit to 0xE0000000
		int index = FRAMEBUFFER_MAPPED_ADDR >> 22;
		uint32_t pointer = pInfo->framebuffer_addr;
		uint32_t final_address = 0xE0000000;
		final_address += pointer & 0xFFF;
		pointer &= ~0xFFF;
		
		//LogMsg("VBE Pointer: %x", pointer);
		//LogMsg("Bitdepth: %d", pInfo->framebuffer_bpp);
		for (int i = 0; i < MAX_VIDEO_PAGES; i++)
		{
			g_vbePageEntries[i] = pointer | VBE_PAGE_BITS;
			pointer += 0x1000;
		}
		for (int i = 0; i < MAX_VIDEO_PAGES; i += 1024)
		{
			//LogMsg("Mapping %d-%d to %dth page table pointer", i,i+1023, index);
			uint32_t pageTable = ((uint32_t)&g_vbePageEntries[i]) - BASE_ADDRESS;
			
			g_curPageDir[index] = pageTable | VBE_PAGE_BITS;
			index++;
		}
		
		g_framebufferCopy = MmAllocate (pInfo->framebuffer_width * pInfo->framebuffer_height * 4);
		
		MmTlbInvalidate();
		
		// initialize the VBE data:
		VidInitializeVBEData (pInfo);
		VidPrintTestingPattern();
		
		// initialize the console:
		//LogMsg("Setting font.");
		VidSetFont (FONT_TAMSYN_BOLD);
		//LogMsg("Re-initializing debug console with graphics");
		CoInitAsGraphics(&g_debugConsole);
		sti;
	}
	else
	{
		SwitchMode (0);
		CoInitAsText(&g_debugConsole);
		LogMsg("Warning: no VBE mode specified.");
		sti;
	}
}
