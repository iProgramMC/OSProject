/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

     Window Manager module header file
******************************************/
#ifndef _WINDOW_H
#define _WINDOW_H

#include <main.h>
#include <string.h>
#include <video.h>
#include <console.h>
#include <memory.h>

#define WINDOWS_MAX 64
#define WINDOW_TITLE_MAX 250
#define EVENT_QUEUE_MAX 256

#define BACKGROUND_COLOR 0xFF00AAAA

enum {
	EVENT_NULL,
	EVENT_CREATE,
	EVENT_DESTROY,
	EVENT_PAINT,
	EVENT_MOVE,
	EVENT_SIZE,
	EVENT_ACTIVATE,
	EVENT_SETFOCUS,
	EVENT_KILLFOCUS,
	EVENT_UPDATE,
	EVENT_MOVECURSOR,
	EVENT_CLICKCURSOR,
	EVENT_RELEASECURSOR,
	EVENT_MAX
};

struct WindowStruct;
typedef void (*WindowProc) (struct WindowStruct*, int);

typedef struct WindowStruct
{
	bool       m_used;
	bool       m_hidden;
	bool       m_isBeingDragged;
	bool       m_isSelected;
	
	char       m_title [250];
	
	WindowProc m_callback;
	Rectangle  m_rect;
	//uint32_t*  m_framebuffer;
	//int        m_fbWidth, m_fbHeight;
	VBEData    m_vbeData;
	
	bool       m_eventQueueLock;
	short      m_eventQueue[EVENT_QUEUE_MAX];
	int        m_eventQueueSize;
	
	bool       m_markedForDeletion;
} Window;

/**
 * Define that does nothing... yet.
 * Mark your callbacks with this anyway!
 */
#define CALLBACK 

/**
 * Entry point of the window manager.
 *
 * For utility this can directly be put inside a KeStartTask.
 */
void WindowManagerTask(__attribute__((unused)) int useless_argument);


#endif//_WINDOW_H