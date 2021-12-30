/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

         Video module header file
******************************************/
#ifndef _VIDEO_H
#define _VIDEO_H

#include <multiboot.h>
#include <mouse.h>

/**
 * An enum with the font types available.
 */
enum {
	FONT_TAMSYN_REGULAR,
	FONT_TAMSYN_BOLD,
	FONT_PAPERM,
	FONT_FAMISANS,
	FONT_LAST,
};

#define CLICK_INFO_MAX 256
typedef struct
{
	uint16_t width, height;
	int16_t leftOffs, topOffs;
	int32_t* bitmap;
}
Cursor;

enum
{
	CLICK_LEFT,
	CLICK_RIGHT,
	CLICK_LEFTD,
	CLICK_LEFTR,
};

typedef struct
{
	int clickType, clickedAtX, clickedAtY;
}
ClickInfo;

typedef struct
{
	int left, top, right, bottom;
}
Rectangle;

typedef struct
{
	int x, y;
}
Point;

typedef struct
{
	bool     m_available;			    //if the vbe display is available
	unsigned m_width, m_height, m_pitch;//bytes per row
	int      m_bitdepth;                //bits per pixel, only values we support: 0=8, 1=16, 2=32
	bool     m_dirty;					//useful if the framebuffer won't directly be pushed to the screen
	union {
		uint32_t* m_framebuffer32; //for ease of addressing
		uint16_t* m_framebuffer16;
		uint8_t * m_framebuffer8;
	};
	int m_pitch32, m_pitch16;      //uint32_t's and uint16_t's per row.
}
VBEData;

/**
 * Sets the current VBE data, or NULL for the mainscreen.
 */
void VidSetVBEData(VBEData* pData);

/**
 * Gets the screen width.
 */
int GetScreenSizeX();

/**
 * Gets the screen height.
 */
int GetScreenSizeY();

/**
 * Gets the width (distance between right and left) of the rectangle.
 */
int GetWidth(Rectangle* rect);

/**
 * Gets the height (distance between bottom and top) of the rectangle.
 */
int GetHeight(Rectangle* rect);

/**
 * Initializes the graphics API based on the multiboot info.
 */
void VidInitialize (multiboot_info_t* pInfo);

/**
 * Plots a single pixel on the screen.
 */
void VidPlotPixel(unsigned x, unsigned y, unsigned color);

/**
 * Fills the screen with a certain color.
 */
void VidFillScreen(unsigned color);

/**
 * Sets the current screen font.
 */
void VidSetFont(unsigned fontType);

/**
 * Draws a character in "colorFg" with an optional colorBg (if it's 0xFFFFFFFF we don't draw any).
 */
void VidPlotChar (char c, unsigned ox, unsigned oy, unsigned colorFg, unsigned colorBg /*=0xFFFFFFFF*/);

/**
 * Shifts the screen up, by a certain amount of pixels.  Anything larger than the screen height will
 * effectively clear the screen.
 */
void VidShiftScreen (int amount);

/**
 * Fills a rectangle on the screen.  The ranges of pixels are all inclusive, so
 * pixels[right][bottom] is also getting drawn.
 */
void VidFillRect(unsigned color, int left, int top, int right, int bottom);

/**
 * Fills a rectangle on the screen.  The ranges of pixels are all inclusive, so
 * pixels[rect.right][rect.bottom] is also getting drawn.
 */
void VidFillRectangle(unsigned color, Rectangle rect);

/**
 * Checks if the video subsystem is available and has been initialized correctly.
 */
bool VidIsAvailable();

/**
 * Sets the cursor to be visible.
 */
void SetMouseVisible(bool cursor);

/**
 * Resets the current cursor to the default.
 */
void SetDefaultCursor();

/**
 * Sets the cursor to a custom one.
 */
void SetCursor(Cursor* pCursor);

/**
 * Gets the current cursor.
 */
Cursor* GetCurrentCursor();

/**
 * Forces the mouse position to go somewhere.
 */
void SetMousePos (unsigned pX, unsigned pY);

/**
 * Handler routine to update mouse data.
 */
void OnUpdateMouse (uint8_t flags, uint8_t dx, uint8_t dy, uint8_t dz);


#endif//_VIDEO_H