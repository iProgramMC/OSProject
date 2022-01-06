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

#define BACKGROUND_COLOR 0xFF007F7F
#define BUTTON_MIDDLE_COLOR 0xFFCCCCCC
#define WINDOW_BACKGD_COLOR 0xFFAAAAAA
#define WINDOW_EDGE_COLOR 0xFF000000
#define WINDOW_TITLE_ACTIVE_COLOR 0xFF00007F
#define WINDOW_TITLE_INACTIVE_COLOR 0xFF7F7F7F
#define WINDOW_TITLE_TEXT_COLOR_SHADOW 0xFF00003F
#define WINDOW_TITLE_TEXT_COLOR 0x00FFFFFF

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
	EVENT_COMMAND,
	EVENT_MAX
};

enum {
	CONTROL_NONE,
	CONTROL_TEXT,
	CONTROL_ICON,
	CONTROL_BUTTON,
	CONTROL_TEXTINPUT,
	CONTROL_CHECKBOX,
	
	CONTROL_COUNT
};

struct WindowStruct;
struct ControlStruct;
typedef void (*WidgetEventHandler) (struct ControlStruct*, int eventType, int parm1, int parm2, struct WindowStruct* parentWindow);
typedef void (*WindowProc)         (struct WindowStruct*, int, int, int);

typedef struct ControlStruct
{
	bool      m_active;
	int       m_type;//CONTROL_XXX
	int       m_parm1, m_parm2;
	char      m_text[128];
	void*     m_dataPtr;
	Rectangle m_rect;
	bool      m_bMarkedForDeletion;
	
	//event handler
	WidgetEventHandler OnEvent;
}
Control;

#define TITLE_BAR_HEIGHT 18
#define WINDOW_RIGHT_SIDE_THICKNESS 3

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
	int        m_eventQueueParm1[EVENT_QUEUE_MAX];
	int        m_eventQueueParm2[EVENT_QUEUE_MAX];
	int        m_eventQueueSize;
	
	bool       m_markedForDeletion;
	
	Control*   m_pControlArray;
	int        m_controlArrayLen;
	
	void*      m_data; //user data
} Window;

/**
 * Define that does nothing... yet.
 * Mark your callbacks with this anyway!
 */
#define CALLBACK 

#define MAKE_MOUSE_PARM(x, y) ((x)<<16|(y))
#define GET_X_PARM(parm1)  (parm1>>16)
#define GET_Y_PARM(parm2)  (parm1&0xFFFF)

/**
 * Check if a rectangle contains a point.
 */
bool RectangleContains(Rectangle*r, Point*p) ;

/**
 * Register an event to a certain window.
 */
void WindowRegisterEvent (Window* pWindow, short eventType, int parm1, int parm2);

/**
 * Entry point of the window manager.
 *
 * For utility this can directly be put inside a KeStartTask.
 */
void WindowManagerTask(__attribute__((unused)) int useless_argument);


//Windowing API

/**
 * Creates a window, with its top left corner at (xPos, yPos), and its
 * bottom right corner at (xPos + xSize, yPos + ySize).
 *
 * WindowProc is the main event handler of the program, but it isn't called.
 * spontaneously. Instead, you use it like:
 *
 * while (HandleMessages(pWindow));
 */
Window* CreateWindow (char* title, int xPos, int yPos, int xSize, int ySize, WindowProc proc);

/**
 * Updates the window, and handles its messages.
 */
bool HandleMessages(Window* pWindow);

/**
 * The default window event procedure.  Call this when you don't know
 * how to handle an event properly.
 */
void DefaultWindowProc (Window* pWindow, int messageType, UNUSED int parm1, UNUSED int parm2);


#endif//_WINDOW_H