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

// Basic definitions for video
#if 1
bool g_isVideoMode = false;
uint32_t g_framebufferPhysical = 0;

extern uint32_t *g_curPageDir;

//! TODO: if planning to extend this beyond 1920x1080x32, extend THIS variable.
#define MAX_VIDEO_PAGES 2048
#define FRAMEBUFFER_MAPPED_ADDR 0xE0000000
uint32_t g_vbePageEntries[MAX_VIDEO_PAGES] __attribute__((aligned(4096))); 
uint32_t* g_framebufferCopy = NULL;

VBEData g_mainScreenVBEData;

VBEData* g_vbeData = NULL;
#endif

// Mouse graphics stuff
#if 1

int g_mouseX = 0, g_mouseY = 0;


#define X 0XFFFFFFFF,
#define B 0XFF000000,
#define o 0X00FFFFFF,

uint32_t g_cursorColors[] = 
	{
		/*B o o o o o o o o o o o
		B B o o o o o o o o o o
		B X B o o o o o o o o o
		B X X B o o o o o o o o
		B X X X B o o o o o o o
		B X X X X B o o o o o o
		B X X X X X B o o o o o
		B X X X X X X B o o o o
		B X X X X X X X B o o o
		B X X X X X X X X B o o
		B X X X X X X X X X B o
		B X X X X X X B B B B B
		B X X X B X X B o o o o
		B X X B B X X B o o o o
		B X B o o B X X B o o o
		B B o o o B X X B o o o
		B o o o o o B X X B o o
		o o o o o o B X X B o o
		o o o o o o o B X X B o
		o o o o o o o B X X B o
		o o o o o o o o B B o o*/
		4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294903296, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294902784, 4294904576, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294905856, 4294906624, 4294908672, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294908672, 4294909440, 4294910976, 4294913024, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294911744, 4294912256, 4294913280, 4294915072, 4294917120, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294914816, 4294915072, 4294916096, 4294917376, 4294919424, 4294921472, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294917888, 4294918144, 4294918912, 4294920192, 4294921728, 4294923776, 4294925824, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294920960, 4294921216, 4294921728, 4294923008, 4294924288, 4294925824, 4294927872, 4294929920, 4278190080,TRANSPARENT,TRANSPARENT, 4278190080, 4294924032, 4294924288, 4294924544, 4294925824, 4294927104, 4294928640, 4294930176, 4294932224, 4294934272, 4278190080,TRANSPARENT, 4278190080, 4294927104, 4294927104, 4294927616, 4294928640, 4294929664, 4294931200, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4294929920, 4294930176, 4294930688, 4278190080, 4294932480, 4294933760, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294932992, 4294933248, 4278190080, 4278190080, 4294935296, 4294936320, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294936064, 4278190080,TRANSPARENT,TRANSPARENT, 4278190080, 4294939136, 4294940416, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294941952, 4294943232, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294945792, 4294947072, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294948608, 4294949888, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294952704, 4294953984, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294955520, 4294956800, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT, 

	};
uint32_t g_waitCursorColors[] = 
	{
		4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080,TRANSPARENT, 4278190080, 4278190080, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4278190080, 4278190080,TRANSPARENT, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT, 4278190080, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294967295, 4294967295, 4278190080, 4294967295, 4278190080, 4294967295, 4278190080, 4294967295, 4294967295, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294967295, 4294967295, 4294967295, 4278190080, 4294967295, 4278190080, 4294967295, 4294967295, 4294967295, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080, 4294967295, 4294967295, 4294967295, 4278190080, 4294967295, 4294967295, 4294967295, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080, 4294967295, 4278190080, 4294967295, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080, 4294967295, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080, 4294967295, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080, 4294967295, 4294967295, 4294967295, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080, 4294967295, 4294967295, 4278190080, 4294967295, 4294967295, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4278190080, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294967295, 4294967295, 4294967295, 4294967295, 4278190080, 4294967295, 4294967295, 4294967295, 4294967295, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294967295, 4294967295, 4294967295, 4278190080, 4294967295, 4278190080, 4294967295, 4294967295, 4294967295, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294967295, 4294967295, 4278190080, 4294967295, 4278190080, 4294967295, 4278190080, 4294967295, 4294967295, 4278190080,TRANSPARENT,TRANSPARENT,TRANSPARENT, 4278190080, 4294967295, 4278190080, 4294967295, 4278190080, 4294967295, 4278190080, 4294967295, 4278190080, 4294967295, 4278190080,TRANSPARENT,TRANSPARENT, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080,TRANSPARENT, 4278190080, 4278190080, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4278190080, 4278190080,TRANSPARENT, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080, 4278190080,TRANSPARENT, 
	};

Cursor g_defaultCursor = {
	12, 21, 0, 0, 
	g_cursorColors
};
Cursor g_waitCursor = {
	14, 22, 0, 0, 
	g_waitCursorColors
};
#undef X
#undef B
#undef o
Cursor* g_currentCursor = NULL;
Cursor* g_pDefaultCursor = NULL; // pointer for easy checking
Cursor* g_pWaitCursor = NULL; // pointer for easy checking

bool g_isMouseVisible = false;

//forward decls to stuff
unsigned VidReadPixel (unsigned x, unsigned y);
static void VidPlotPixelIgnoreCursorChecksChecked(unsigned x, unsigned y, unsigned color);

//Click queue to handle clicks later (window manager)
ClickInfo g_clickQueue [CLICK_INFO_MAX];
int       g_clickQueueSize = 0;
bool      g_clickQueueLock = false;

void AddClickInfoToQueue(const ClickInfo* info)
{
	ACQUIRE_LOCK (g_clickQueueLock);
	
	if (g_clickQueueSize >= CLICK_INFO_MAX)
	{
		//only handle the next clicks now
		g_clickQueueSize = 0;
	}
	g_clickQueue[g_clickQueueSize++] = *info;
	
	FREE_LOCK (g_clickQueueLock);
}
void OnLeftClick()
{
	ClickInfo info;
	info.clickType = CLICK_LEFT;
	info.clickedAtX = g_mouseX;
	info.clickedAtY = g_mouseY;
	AddClickInfoToQueue (&info);
}
void OnLeftClickRelease()
{
	ClickInfo info;
	info.clickType = CLICK_LEFTR;
	info.clickedAtX = g_mouseX;
	info.clickedAtY = g_mouseY;
	AddClickInfoToQueue (&info);
}
void OnLeftClickDrag()
{
	ClickInfo info;
	info.clickType = CLICK_LEFTD;
	info.clickedAtX = g_mouseX;
	info.clickedAtY = g_mouseY;
	AddClickInfoToQueue (&info);
}
void OnRightClick()
{
	ClickInfo info;
	info.clickType = CLICK_RIGHT;
	info.clickedAtX = g_mouseX;
	info.clickedAtY = g_mouseY;
	AddClickInfoToQueue (&info);
}

uint8_t g_previousFlags = 0;
void OnUpdateMouse(uint8_t flags, uint8_t Dx, uint8_t Dy, __attribute__((unused)) uint8_t Dz)
{
	int dx, dy;
	dx = (flags & (1 << 4)) ? (int8_t)Dx : Dx;
	dy = (flags & (1 << 5)) ? (int8_t)Dy : Dy;
	
	//move the cursor:
	int newX = g_mouseX + dx;
	int newY = g_mouseY - dy;
	if (newX < 0) newX = 0;
	if (newY < 0) newY = 0;
	SetMousePos (newX, newY);
	
	if (flags & MOUSE_FLAG_R_BUTTON)
	{
		if (!(g_previousFlags & MOUSE_FLAG_R_BUTTON))
			OnRightClick();
	}
	if (flags & MOUSE_FLAG_L_BUTTON)
	{
		if (!(g_previousFlags & MOUSE_FLAG_L_BUTTON))
			OnLeftClick();
		else
			OnLeftClickDrag();
	}
	else if (g_previousFlags & MOUSE_FLAG_L_BUTTON)
		OnLeftClickRelease();
	
	g_previousFlags = flags & 7;
}

Cursor* GetCurrentCursor()
{
	return g_currentCursor;
}

void SetCursor(Cursor* pCursor)
{
	if (!pCursor) pCursor = g_pDefaultCursor;
	if (g_currentCursor == pCursor) return;
	
	VBEData* backup = g_vbeData;
	g_vbeData = &g_mainScreenVBEData;
	
	//undraw the old cursor:
	if (g_currentCursor)
	{
		for (int i = -2; i <= g_currentCursor->height + 1; i++)
		{
			for (int j = -2; j <= g_currentCursor->width + 1; j++)
			{
				int x = g_mouseX + j - g_currentCursor->leftOffs;
				int y = g_mouseY + i - g_currentCursor->topOffs;
				VidPlotPixelIgnoreCursorChecksChecked (x, y, VidReadPixel(x, y));
			}
		}
	}
	
	//draw the new cursor:
	g_currentCursor = pCursor;
	for (int i = 0; i < g_currentCursor->height; i++)
	{
		for (int j = 0; j < g_currentCursor->width; j++)
		{
			int id = i * g_currentCursor->width + j;
			if (g_currentCursor->bitmap[id] != TRANSPARENT)
			{
				VidPlotPixelIgnoreCursorChecksChecked(
					j + g_mouseX - g_currentCursor->leftOffs,
					i + g_mouseY - g_currentCursor->topOffs,
					g_currentCursor->bitmap[id]
				);
			}
		}
	}
	
	
	
	g_vbeData = backup;
}

void SetMouseVisible (bool b)
{
	g_isMouseVisible = b;
	if (!g_isMouseVisible)
	{
		for (int i = 0; i < g_currentCursor->height; i++)
		{
			if (i + g_mouseY - g_currentCursor->topOffs >= GetScreenSizeY()) break;
			for (int j = 0; j < g_currentCursor->width; j++)
			{
				if (j + g_mouseX - g_currentCursor->leftOffs >= GetScreenSizeX()) break;
				int id = i * g_currentCursor->width + j;
				if (g_currentCursor->bitmap[id] != TRANSPARENT)
				{
					int kx = j + g_mouseX - g_currentCursor->leftOffs,
						ky = i + g_mouseY - g_currentCursor->topOffs;
					VidPlotPixelIgnoreCursorChecksChecked (
						kx, ky, VidReadPixel (kx, ky)
					);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < g_currentCursor->height; i++)
		{
			if (i + g_mouseY - g_currentCursor->topOffs >= GetScreenSizeY()) break;
			for (int j = 0; j < g_currentCursor->width; j++)
			{
				if (j + g_mouseX - g_currentCursor->leftOffs >= GetScreenSizeX()) break;
				int id = i * g_currentCursor->width + j;
				if (g_currentCursor->bitmap[id] != TRANSPARENT)
				{
					int kx = j + g_mouseX - g_currentCursor->leftOffs,
						ky = i + g_mouseY - g_currentCursor->topOffs;
					VidPlotPixelIgnoreCursorChecksChecked (
						kx, ky, g_currentCursor->bitmap[id]
					);
				}
			}
		}
	}
}

void SetMousePos (unsigned newX, unsigned newY)
{
	//NOTE: As this is called in an interrupt too, a call here might end up coming right
	//while we we're drawing a window or something.  Keep a backup of the previous settings.
	
	VBEData* backup = g_vbeData;
	g_vbeData = &g_mainScreenVBEData;
	
	int oldX = g_mouseX, oldY = g_mouseY;
	
	if (newX >= (unsigned)GetScreenSizeX()) newX = GetScreenSizeX() - 1;
	if (newY >= (unsigned)GetScreenSizeY()) newY = GetScreenSizeY() - 1;
	
	g_mouseX = newX, g_mouseY = newY;
	
	//--uncomment if you want one pixel cursor (This is very useless and hard to use)
	//VidPlotPixelIgnoreCursorChecks (g_mouseX, g_mouseY, 0xFF);
	//VidPlotPixel (oldX, oldY, VidReadPixel(oldX, oldY));
	
	//Draw the cursor image at the new position:
	for (int i = 0; i < g_currentCursor->height; i++)
	{
		for (int j = 0; j < g_currentCursor->width; j++)
		{
			int id = i * g_currentCursor->width + j;
			if (g_currentCursor->bitmap[id] != TRANSPARENT)
			{
				int kx = j + g_mouseX - g_currentCursor->leftOffs,
					ky = i + g_mouseY - g_currentCursor->topOffs;
				if (kx < 0 || ky < 0 || kx >= GetScreenSizeX() || ky >= GetScreenSizeY()) continue;
				VidPlotPixelIgnoreCursorChecksChecked (
					kx,
					ky,
					g_currentCursor->bitmap[id]
				);
			}
		}
	}
	
	//Then, redraw all the pixels under where the cursor was previously:
	for (int i = 0; i < g_currentCursor->height; i++)
	{
		for (int j = 0; j < g_currentCursor->width; j++)
		{
			int id = i * g_currentCursor->width + j;
			if (g_currentCursor->bitmap[id] != TRANSPARENT)
			{
				int kx = j + oldX - g_currentCursor->leftOffs,
					ky = i + oldY - g_currentCursor->topOffs;
				if (kx < 0 || ky < 0 || kx >= GetScreenSizeX() || ky >= GetScreenSizeY()) continue;
				VidPlotPixel (
					kx, ky, VidReadPixel (kx, ky)
				);
			}
		}
	}
	
	//TODO: check flags here
	
	g_vbeData = backup;
}

void SetDefaultCursor ()
{
	g_currentCursor = &g_defaultCursor;
	g_pDefaultCursor = &g_defaultCursor;
	g_pWaitCursor = &g_waitCursor;
}
#endif

// Getters
#if 1
bool VidIsAvailable()
{
	return g_vbeData->m_available;
}
int GetScreenSizeX()
{
	return g_vbeData->m_width;
}
int GetScreenSizeY()
{
	return g_vbeData->m_height;
}
int GetWidth (Rectangle* rect)
{
	return rect->right - rect->left;
}
int GetHeight (Rectangle* rect)
{
	return rect->bottom - rect->top;
}
#endif

// Graphical drawing routines
#if 1
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
	g_vbeData->m_dirty = 1;
	g_vbeData->m_framebuffer8 [x + y * g_vbeData->m_pitch  ] = VidColor32to8 (color);
}
void VidPlotPixelRaw16 (unsigned x, unsigned y, unsigned color)
{
	g_vbeData->m_dirty = 1;
	g_vbeData->m_framebuffer16[x + y * g_vbeData->m_pitch16] = VidColor32to16(color);
}
void VidPlotPixelRaw32 (unsigned x, unsigned y, unsigned color)
{
	g_vbeData->m_dirty = 1;
	g_vbeData->m_framebuffer32[x + y * g_vbeData->m_pitch32] = color;
}
static void VidPlotPixelIgnoreCursorChecks(unsigned x, unsigned y, unsigned color)
{
	switch (g_vbeData->m_bitdepth)
	{
		case 0: VidPlotPixelRaw8 (x, y, color); break;
		case 1: VidPlotPixelRaw16(x, y, color); break;
		case 2: VidPlotPixelRaw32(x, y, color); break;
	}
}
static void VidPlotPixelIgnoreCursorChecksChecked(unsigned x, unsigned y, unsigned color)
{
	if ((int)x < 0 || (int)y < 0 || (int)x >= GetScreenSizeX() || (int)y >= GetScreenSizeY()) return;
	switch (g_vbeData->m_bitdepth)
	{
		case 0: VidPlotPixelRaw8 (x, y, color); break;
		case 1: VidPlotPixelRaw16(x, y, color); break;
		case 2: VidPlotPixelRaw32(x, y, color); break;
	}
}
static void VidPlotPixelToCopy(unsigned x, unsigned y, unsigned color)
{
	if ((int)x < 0 || (int)y < 0 || (int)x >= GetScreenSizeX() || (int)y >= GetScreenSizeY()) return;
	if (g_vbeData == &g_mainScreenVBEData)
		g_framebufferCopy[x + y * g_vbeData->m_width] = color;
}
void VidPlotPixel(unsigned x, unsigned y, unsigned color)
{
	if ((int)x < 0 || (int)y < 0 || (int)x >= GetScreenSizeX() || (int)y >= GetScreenSizeY()) return;
	VidPlotPixelToCopy(x, y, color);
	
	// if inside the cursor area, don't display this pixel on the screen:
	if (g_vbeData == &g_mainScreenVBEData)
	{
		if (g_currentCursor && g_isMouseVisible)
		{
			if ((int)x >= g_mouseX - g_currentCursor->leftOffs &&
				(int)y >= g_mouseY - g_currentCursor->topOffs  &&
				(int)x <  g_mouseX + g_currentCursor->width  - g_currentCursor->leftOffs &&
				(int)y <  g_mouseY + g_currentCursor->height - g_currentCursor->topOffs)
			{
				int mx = x - g_mouseX + g_currentCursor->leftOffs;
				int my = y - g_mouseY + g_currentCursor->topOffs;
				int index = my * g_currentCursor->width + mx;
				if (g_currentCursor->bitmap[index] != TRANSPARENT)
				{
					return;
				}
			}
		}
	}
	
	
	VidPlotPixelIgnoreCursorChecks (x, y, color);
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
	g_vbeData->m_dirty = 1;
	int color2 = color;
	bool alsoToTheCopy = (g_vbeData == &g_mainScreenVBEData);
	switch (g_vbeData->m_bitdepth)
	{
		case 0:
			color2 = VidColor32to8(color);
			for (int y = 0; y < GetScreenSizeY(); y++) 
				for (int x = 0; x < GetScreenSizeX(); x++)
				{
					g_vbeData->m_framebuffer8 [x + y * g_vbeData->m_pitch  ] = color2;
					if (alsoToTheCopy) g_framebufferCopy[x + y * g_vbeData->m_width] = color;
				}
			break;
		case 1:
			color2 = VidColor32to16(color);
			for (int y = 0; y < GetScreenSizeY(); y++) 
				for (int x = 0; x < GetScreenSizeX(); x++)
				{
					g_vbeData->m_framebuffer16[x + y * g_vbeData->m_pitch16] = color2;
					if (alsoToTheCopy) g_framebufferCopy[x + y * g_vbeData->m_width] = color;
				}
			break;
		case 2:
			for (int y = 0; y < GetScreenSizeY(); y++) 
				for (int x = 0; x < GetScreenSizeX(); x++)
				{
					g_vbeData->m_framebuffer32[x + y * g_vbeData->m_pitch32] = color;
					if (alsoToTheCopy) g_framebufferCopy[x + y * g_vbeData->m_width] = color;
				}
			break;
	}
}
void VidFillRect(unsigned color, int left, int top, int right, int bottom)
{
	//basic clipping:
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
void VidDrawHLine(unsigned color, int left, int right, int y)
{
	//basic clipping:
	if (left < 0) left = 0;
	if (y < 0) y = 0;
	if (right >= GetScreenSizeX()) right = GetScreenSizeX() - 1;
	if (y >= GetScreenSizeY()) y = GetScreenSizeY() - 1;
	
	for (int x = left; x <= right; x++)
	{
		VidPlotPixel(x, y, color);
		VidPlotPixel(x, y, color);
	}
}
void VidDrawVLine(unsigned color, int top, int bottom, int x)
{
	//basic clipping:
	if (x < 0) x = 0;
	if (top < 0) top = 0;
	if (x >= GetScreenSizeX()) x = GetScreenSizeX() - 1;
	if (bottom >= GetScreenSizeY()) bottom = GetScreenSizeY() - 1;
	
	for (int y = top; y <= bottom; y++)
	{
		VidPlotPixel(x, y, color);
		VidPlotPixel(x, y, color);
	}
}
int absinl(int i)
{
	if (i < 0) return -i; 
	return i;
}

//shamelessly stolen from https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/olcPixelGameEngine.h#L1866
void VidDrawLine(unsigned p, int x1, int y1, int x2, int y2)
{
	//is line vertical?
	int aux = 0;
	if (x1 == x2)
	{
		//we have a more optimized version, use that
		if (y1 > y2)
		{
			aux = y1; y1 = y2; y2 = aux;
		}
		VidDrawVLine(p, y1, y2, x1);
		return;
	}
	//is line horizontal?
	if (y1 == y2)
	{
		if (x1 > x2)
		{
			aux = x1; x1 = x2; x2 = aux;
		}
		//we have a more optimized version, use that
		VidDrawHLine(p, x1, x2, y1);
		return;
	}
	
	int dx = x2 - x1, dy = y2 - y1;
	int dx1 = absinl(dx), dy1 = absinl(dy), xe, ye, x, y;
	int px = 2 * dy1 - dx1, py = 2 * dx1 - dy1;
	
	if (dy1 <= dx1)
	{
		if (dx >= 0)
		{
			x = x1, y = y1, xe = x2;
		}
		else
		{
			x = x2, y = y2, xe = x1;
		}
		
		VidPlotPixel(x, y, p);
		
		for (int i = 0; x < xe; i++)
		{
			x++;
			if (px < 0)
				px += 2 * dy1;
			else
			{
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) y++; else y--;
				px += 2 * (dy1 - dx1);
			}
			VidPlotPixel(x, y, p);
		}
	}
	else
	{
		if (dy >= 0)
		{
			x = x1, y = y1, ye = y2;
		}
		else
		{
			x = x2, y = y2, ye = y1;
		}
		
		VidPlotPixel(x, y, p);
		
		for (int i = 0; y < ye; i++)
		{
			y++;
			if (py <= 0)
				py += 2 * dx1;
			else
			{
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x++; else x--;
				py += 2 * (dx1 - dy1);
			}
			VidPlotPixel(x, y, p);
		}
	}
}
void VidDrawRect(unsigned color, int left, int top, int right, int bottom)
{
	//basic clipping:
	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right >= GetScreenSizeX()) right = GetScreenSizeX() - 1;
	if (bottom >= GetScreenSizeY()) bottom = GetScreenSizeY() - 1;
	
	for (int x = left; x <= right; x++)
	{
		VidPlotPixel(x, top,    color);
		VidPlotPixel(x, bottom, color);
	}
	for (int y = top; y <= bottom; y++)
	{
		VidPlotPixel(left,  y, color);
		VidPlotPixel(right, y, color);
	}
}
void VidFillRectangle(unsigned color, Rectangle rect)
{
	VidFillRect (color, rect.left, rect.top, rect.right, rect.bottom);
}
void VidDrawRectangle(unsigned color, Rectangle rect)
{
	VidDrawRect (color, rect.left, rect.top, rect.right, rect.bottom);
}

void VidSetVBEData(VBEData* pData)
{
	if (pData)
		g_vbeData = pData;
	else
		g_vbeData = &g_mainScreenVBEData;
}


// Font rendering
bool g_uses8by16Font = 0;

const unsigned char* g_fontIDToData[] = {
	g_FamiSans8x8,
	g_TamsynRegu8x16,
	g_TamsynBold8x16,
	g_PaperMFont8x16,
	g_BasicFontData,
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
	g_uses8by16Font = (g_pCurrentFont[1] != 8);
}
void VidPlotChar (char c, unsigned ox, unsigned oy, unsigned colorFg, unsigned colorBg /*=0xFFFFFFFF*/)
{
	if (!g_pCurrentFont) {
		SLogMsg("FUCK!");
		return;
	}
	int width = g_pCurrentFont[0], height = g_pCurrentFont[1];
	const unsigned char* test = g_pCurrentFont + 3;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0, bitmask = (1 << (width - 1)); x < width; x++, bitmask >>= 1)
		{
			if (test[c * height + y] & bitmask)
				VidPlotPixel(ox + x, oy + y, colorFg);
			else if (colorBg != TRANSPARENT)
				VidPlotPixel(ox + x, oy + y, colorBg);
		}
	}
}
void VidTextOut(const char* pText, unsigned ox, unsigned oy, unsigned colorFg, unsigned colorBg)
{
	int x = ox, y = oy;
	int lineHeight = g_pCurrentFont[1], charWidth = g_pCurrentFont[0];
	bool hasVariableCharWidth = g_pCurrentFont[2];
	
	while (*pText)
	{
		//print this character:
		char c = *pText;
		if (c == '\n')
		{
			y += lineHeight;
			x = ox;
		}
		else
		{
			int cw = charWidth;
			if (hasVariableCharWidth) cw = g_pCurrentFont[3 + 256 * lineHeight + c];
			VidPlotChar(c, x, y, colorFg, colorBg);
			x += cw;
		}
		pText++;
	}
}

//! DO NOT use this on non-main-screen framebuffers!
unsigned VidReadPixel (unsigned x, unsigned y)
{
	if (x >= (unsigned)GetScreenSizeX()) return 0;
	if (y >= (unsigned)GetScreenSizeY()) return 0;
	return g_framebufferCopy[x + y * GetScreenSizeX()];
}

//! DO NOT use this on non-main-screen framebuffers!
void VidShiftScreen (int howMuch)
{
	if (howMuch >= GetScreenSizeY())
		return;
	/*for (int i = howMuch; i < GetScreenSizeY(); i++) {
		for (int k = 0; k < GetScreenSizeX(); k++) {
			VidPlotPixel (k, i-howMuch, VidReadPixel (k, i));
		}
	}*/
	
	if (g_vbeData->m_bitdepth == 2)
	{
		int a = g_vbeData->m_width * 4;
		for (int i = howMuch, j = 0, k = 0; i < GetScreenSizeY(); i++, j += g_vbeData->m_pitch, k += a)
		{
			fast_memcpy(((uint8_t*)g_vbeData->m_framebuffer32 + j), &g_framebufferCopy[i * g_vbeData->m_width], a);
			fast_memcpy(((uint8_t*)g_framebufferCopy         + k), &g_framebufferCopy[i * g_vbeData->m_width], a);
		}
	}
	else
	{
		;//unhandled
	}
}
#endif

// Video initialization
#if 1
void VidInitializeVBEData(multiboot_info_t* pInfo)
{
	int bpp = pInfo->framebuffer_bpp;
	g_vbeData->m_available = true;
	g_vbeData->m_width     = pInfo->framebuffer_width;
	g_vbeData->m_height    = pInfo->framebuffer_height;
	if (bpp == 32)
		g_vbeData->m_bitdepth  = 2;
	else if (bpp == 16)
		g_vbeData->m_bitdepth  = 1;
	else
		g_vbeData->m_bitdepth  = 0;
	g_vbeData->m_pitch     = pInfo->framebuffer_pitch;
	g_vbeData->m_pitch32   = pInfo->framebuffer_pitch / sizeof (int);
	g_vbeData->m_pitch16   = pInfo->framebuffer_pitch / sizeof (short);
	
	g_vbeData->m_framebuffer32 = (uint32_t*)FRAMEBUFFER_MAPPED_ADDR;
	g_vbeData->m_framebuffer16 = (uint16_t*)FRAMEBUFFER_MAPPED_ADDR;
	g_vbeData->m_framebuffer8  = (uint8_t *)FRAMEBUFFER_MAPPED_ADDR;
}
//present, read/write, user/supervisor, writethrough
#define VBE_PAGE_BITS (1 | 2 | 4 | 8)
void VidInitialize(multiboot_info_t* pInfo)
{
	cli;
	g_vbeData = &g_mainScreenVBEData;
	
	g_vbeData->m_available = false;
	if (pInfo->flags & MULTIBOOT_INFO_VIDEO_INFO)
	{
		if (pInfo->framebuffer_type != 1)
		{
			LogMsg("Need direct RGB framebuffer!");
			sti;
			return;
		}
		// map shit to 0xE0000000 or above
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
		//VidSetFont (FONT_BASIC);
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
#endif
