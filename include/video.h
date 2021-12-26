#ifndef _VIDEO_H
#define _VIDEO_H

#include <multiboot.h>

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

/**
 * Gets the screen width.
 */
int GetScreenSizeX();

/**
 * Gets the screen height.
 */
int GetScreenSizeY();

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
 * Checks if the video subsystem is available and has been initialized correctly.
 */
bool VidIsAvailable();


#endif//_VIDEO_H