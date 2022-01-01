/*****************************************
		NanoShell Operating System
	      (C) 2022 iProgramInCpp

      Icon listing module headerfile
******************************************/
#ifndef _ICON_H
#define _ICON_H

#include <main.h>
#include <video.h>

//WORK: Add in your icons here in this enum:
enum 
{
	ICON_NULL,
	ICON_CABINET,
	ICON_CHIP,
	ICON_CHIP_SQ,
	ICON_COMPUTER,
	ICON_COMPUTER_SHUTDOWN,
	ICON_DESKTOP,
	ICON_DRAW,
	ICON_EARTH,
	ICON_ERROR,
	ICON_EXECUTE_FILE,
	ICON_FILE,
	ICON_FILES,
	ICON_FOLDER,
	ICON_FOLDER_BLANK,
	ICON_FOLDER_MOVE,
	ICON_FOLDER_PARENT,
	ICON_FOLDER16_CLOSED,
	ICON_FOLDER16_OPEN,
	ICON_GLOBE,
	ICON_GO,
	ICON_HAND,
	ICON_HELP,
	ICON_INFO,
	ICON_KEYBOARD,
	ICON_KEYBOARD2,
	ICON_LAPTOP,
	ICON_NOTES,
	ICON_PAINT,
	ICON_SERIAL,
	ICON_STOP,
	ICON_TEXT_FILE,
	ICON_WARNING,
	ICON_NANOSHELL,
	ICON_NANOSHELL16,
	ICON_COUNT
};

typedef int IconType;

/**
 * Renders an icon to the screen.
 */
void RenderIcon(IconType type, int x, int y);

#endif//_ICON_H