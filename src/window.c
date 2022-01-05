/*****************************************
		NanoShell Operating System
	   (C) 2021-2022 iProgramInCpp

           Window Manager module
******************************************/
#define THREADING_ENABLED 1 //0

#if THREADING_ENABLED
#define MULTITASKED_WINDOW_MANAGER
#endif

#define DebugLogMsg  SLogMsg

#include <window.h>
#include <icon.h>
#include <print.h>
#include <task.h>
#include <widget.h>
#include <misc.h>

//util:
#if 1
bool RectangleContains(Rectangle*r, Point*p) 
{
	return (r->left <= p->x && r->right >= p->x && r->top <= p->y && r->bottom >= p->y);
}

Window g_windows [WINDOWS_MAX];

Window* GetWindowFromIndex(int i)
{
	if (i >= 0x1000) i -= 0x1000;
	return &g_windows[i];
}

bool g_windowManagerRunning = false;

extern ClickInfo g_clickQueue [CLICK_INFO_MAX];
extern int       g_clickQueueSize;
extern bool      g_clickQueueLock;

bool g_windowLock = false;
bool g_screenLock = false;

void TestProgramTask(int argument);
void IconTestTask   (int argument);
void PrgPaintTask   (int argument);
#endif

// Window depth buffer
#if 1
short* g_windowDepthBuffer = NULL; //must be allocated
size_t g_windowDepthBufferSzBytes = 0;
void KillWindowDepthBuffer ()
{
	if (g_windowDepthBuffer)
	{
		MmFree(g_windowDepthBuffer);
		g_windowDepthBuffer = NULL;
		g_windowDepthBufferSzBytes = 0;
	}
}
void InitWindowDepthBuffer ()
{
	KillWindowDepthBuffer();
	
	g_windowDepthBufferSzBytes = sizeof (short) * GetScreenSizeX() * GetScreenSizeY();
	g_windowDepthBuffer = MmAllocate(g_windowDepthBufferSzBytes);
}
void SetWindowDepthBuffer (int windowIndex, int x, int y)
{
	if (x < 0 || y < 0 || x >= GetScreenSizeX() || y >= GetScreenSizeY()) return;
	g_windowDepthBuffer[GetScreenSizeX() * y + x] = windowIndex;
}
short GetWindowIndexInDepthBuffer (int x, int y)
{
	if (x < 0 || y < 0 || x >= GetScreenSizeX() || y >= GetScreenSizeY()) return -1;
	short test = g_windowDepthBuffer[GetScreenSizeX() * y + x];
	return test;
}
void FillDepthBufferWithWindowIndex (Rectangle r, uint32_t* framebuffer, int index)
{
	if (!framebuffer) return;
	int hx = GetScreenSizeX(), hy = GetScreenSizeY();
	for (int y = r.top; y < r.bottom; y++) {
		if (y >= hy) break;//no point.
		if (y < 0) continue;
		for (int x = r.left; x < r.right; x++) {
			int idx = GetScreenSizeX() * y + x;
			if (x < 0) continue;
			if (x >= hx) break;//no point.
			
			if (*framebuffer != TRANSPARENT)
				g_windowDepthBuffer[idx] = index;
			framebuffer++;
		}		
	}
}
void UpdateDepthBuffer ()
{
	cli;
	memset (g_windowDepthBuffer, 0xFF, g_windowDepthBufferSzBytes);
	
	for (int i = 0; i < WINDOWS_MAX; i++)
	{
		if (g_windows[i].m_used)
			if (!g_windows[i].m_hidden)
			{
				if (!g_windows[i].m_isSelected)
					FillDepthBufferWithWindowIndex (g_windows[i].m_rect, g_windows[i].m_vbeData.m_framebuffer32, i);
			}
	}
	for (int i = 0; i < WINDOWS_MAX; i++)
	{
		if (g_windows[i].m_used)
			if (!g_windows[i].m_hidden)
			{
				if (g_windows[i].m_isSelected)
					FillDepthBufferWithWindowIndex (g_windows[i].m_rect, g_windows[i].m_vbeData.m_framebuffer32, i);
			}
	}
	sti;
}
#endif

// Window event processor
#if 1
void WindowRegisterEvent (Window* pWindow, short eventType, int parm1, int parm2)
{
	ACQUIRE_LOCK (pWindow->m_eventQueueLock);
	
	if (pWindow->m_eventQueueSize < EVENT_QUEUE_MAX - 1)
	{
		pWindow->m_eventQueue[pWindow->m_eventQueueSize] = eventType;
		pWindow->m_eventQueueParm1[pWindow->m_eventQueueSize] = parm1;
		pWindow->m_eventQueueParm2[pWindow->m_eventQueueSize] = parm2;
		
		pWindow->m_eventQueueSize++;
	}
	else
		DebugLogMsg("Could not register event %d for window %x", eventType, pWindow);
	
	FREE_LOCK (pWindow->m_eventQueueLock);
}
#endif

// Background code
#if 1
void RedrawBackground (Rectangle rect)
{
	//simple background:
	VidFillRectangle (BACKGROUND_COLOR, rect);
}
#endif

// Window utilitary functions:
#if 1
void HideWindow (Window* pWindow)
{
	pWindow->m_hidden = true;
	UpdateDepthBuffer();
	
	//redraw the background and all the things underneath:
	RedrawBackground(pWindow->m_rect);
	
	// draw the windows below it
	int sz=0; Window* windowDrawList[WINDOWS_MAX];
	
	//higher = faster, but may miss some smaller windows
	//a precision of 10 is a-ok if the window will never go beyond 11x11 in size
	#define PRECISION 2
	for (int y = pWindow->m_rect.top; y != pWindow->m_rect.bottom; y += PRECISION) {
		for (int x = pWindow->m_rect.left; x != pWindow->m_rect.right; x += PRECISION) {
			short h = GetWindowIndexInDepthBuffer(x,y);
			if (h == -1) continue;
			//check if it's present in the windowDrawList
			Window* pWindowToCheck = GetWindowFromIndex(h);
			bool exists = false;
			for (int i = 0; i < sz; i++) {
				if (windowDrawList[i] == pWindowToCheck) {
					exists = true; break;
				}
			}
			if (!exists) 
			{
				windowDrawList[sz++] = pWindowToCheck;
			}
		}
	}
	
	// We've added the windows to the list, so draw them. We don't need to worry
	// about windows above them, as the way we're drawing them makes it so pixels
	// over the window aren't overwritten.
	//DebugLogMsg("Drawing %d windows below this one", sz);
	for (int i=0; i<sz; i++) 
		WindowRegisterEvent (windowDrawList[i], EVENT_PAINT, 0, 0);
	
	WindowRegisterEvent (pWindow, EVENT_PAINT, 0, 0);
}

void ShowWindow (Window* pWindow)
{
	pWindow->m_hidden = false;
	UpdateDepthBuffer();
	WindowRegisterEvent (pWindow, EVENT_PAINT, 0, 0);
}

void ReadyToDestroyWindow (Window* pWindow)
{
	HideWindow (pWindow);
	
	if (pWindow->m_vbeData.m_framebuffer32)
	{
		MmFree (pWindow->m_vbeData.m_framebuffer32);
		pWindow->m_vbeData.m_available     = 0;
		pWindow->m_vbeData.m_framebuffer32 = NULL;
	}
	if (pWindow->m_pControlArray)
	{
		MmFree(pWindow->m_pControlArray);
		pWindow->m_controlArrayLen = 0;
	}
	pWindow->m_used = false;
	pWindow->m_eventQueueSize = 0;
}

void DestroyWindow (Window* pWindow)
{
	WindowRegisterEvent (pWindow, EVENT_DESTROY, 0, 0);
	// the task's last WindowCheckMessages call will see this and go
	// "ah yeah they want my window gone", then the WindowCallback will reply and say
	// "yeah you're good to go" and call ReadyToDestroyWindow().
}

void SelectThisWindowAndUnselectOthers(Window* pWindow)
{
	bool wasSelectedBefore = pWindow->m_isSelected;
	if (!wasSelectedBefore) {
		for (int i = 0; i < WINDOWS_MAX; i++) {
			if (g_windows[i].m_used) {
				if (g_windows[i].m_isSelected)
				{
					g_windows[i].m_isSelected = false;
					WindowRegisterEvent(&g_windows[i], EVENT_KILLFOCUS, 0, 0);
					WindowRegisterEvent(&g_windows[i], EVENT_PAINT, 0, 0);
				}
			}
		}
		pWindow->m_isSelected = true;
		UpdateDepthBuffer();
		WindowRegisterEvent(pWindow, EVENT_SETFOCUS, 0, 0);
		WindowRegisterEvent(pWindow, EVENT_PAINT, 0, 0);
	}
}
#endif

// Window creation
#if 1
Window* CreateWindow (char* title, int xPos, int yPos, int xSize, int ySize, WindowProc proc)
{
	int freeArea = -1;
	for (int i = 0; i < WINDOWS_MAX; i++)
	{
		if (!g_windows[i].m_used)
		{
			freeArea = i; break;
		}
	}
	if (freeArea == -1) return NULL;//can't create the window.
	
	Window* pWnd = &g_windows[freeArea];
	
	pWnd->m_used = true;
	pWnd->m_hidden = false;
	pWnd->m_isBeingDragged = false;
	pWnd->m_isSelected = false;
	
	int strl = strlen (title) + 1;
	if (strl >= WINDOW_TITLE_MAX) strl = WINDOW_TITLE_MAX - 1;
	memcpy (pWnd->m_title, title, strl + 1);
	
	pWnd->m_rect.left = xPos;
	pWnd->m_rect.top  = yPos;
	pWnd->m_rect.right  = xPos + xSize;
	pWnd->m_rect.bottom = yPos + ySize;
	pWnd->m_eventQueueSize = 0;
	pWnd->m_eventQueueLock = false;
	pWnd->m_markedForDeletion = false;
	pWnd->m_callback = proc; 
	
	pWnd->m_vbeData.m_available     = true;
	pWnd->m_vbeData.m_framebuffer32 = MmAllocate (sizeof (uint32_t) * xSize * ySize);
	pWnd->m_vbeData.m_width         = xSize;
	pWnd->m_vbeData.m_height        = ySize;
	pWnd->m_vbeData.m_pitch32       = xSize;
	pWnd->m_vbeData.m_bitdepth      = 2;     // 32 bit :)
	
	//give the window a starting point of 10 controls:
	pWnd->m_controlArrayLen = 10;
	size_t controlArraySize = sizeof(Control) * pWnd->m_controlArrayLen;
	pWnd->m_pControlArray   = (Control*)MmAllocate(controlArraySize);
	memset(pWnd->m_pControlArray, 0, controlArraySize);
	
	UpdateDepthBuffer();
	
	WindowRegisterEvent(pWnd, EVENT_CREATE, 0, 0);
	SelectThisWindowAndUnselectOthers(pWnd);
	
	return pWnd;
}
#endif 

// Mouse event handlers
#if 1
int g_currentlyClickedWindow = -1;
int g_prevMouseX, g_prevMouseY;

void OnUILeftClick (int mouseX, int mouseY)
{
	if (!g_windowManagerRunning) return;
	g_prevMouseX = (int)mouseX;
	g_prevMouseY = (int)mouseY;
	
	ACQUIRE_LOCK (g_windowLock);
	
	short idx = GetWindowIndexInDepthBuffer(mouseX, mouseY);
	
	if (idx > -1)
	{
		Window* window = GetWindowFromIndex(idx);
		
		SelectThisWindowAndUnselectOthers (window);
		
		g_currentlyClickedWindow = idx;
		
		int x = mouseX - window->m_rect.left;
		int y = mouseY - window->m_rect.top;
		WindowRegisterEvent (window, EVENT_CLICKCURSOR, MAKE_MOUSE_PARM (x, y), 0);
	}
	else
		g_currentlyClickedWindow = -1;
	
	FREE_LOCK(g_windowLock);
}
Cursor g_windowDragCursor;
#define TITLE_BAR_HEIGHT 12
#define WINDOW_RIGHT_SIDE_THICKNESS 4
void OnUILeftClickDrag (int mouseX, int mouseY)
{
	if (!g_windowManagerRunning) return;
	if (g_currentlyClickedWindow == -1) return;
	
	ACQUIRE_LOCK (g_windowLock);
	g_prevMouseX = (int)mouseX;
	g_prevMouseY = (int)mouseY;
	
	Window* window = GetWindowFromIndex(g_currentlyClickedWindow);
	
	if (!window->m_isBeingDragged)
	{
		//are we in the title bar region? TODO
		Rectangle recta = window->m_rect;
		recta.right  -= recta.left; recta.left = 0;
		recta.bottom -= recta.top;  recta.top  = 0;
		recta.right  -= WINDOW_RIGHT_SIDE_THICKNESS;
		recta.bottom -= WINDOW_RIGHT_SIDE_THICKNESS;
		recta.left++; recta.right--; recta.top++; recta.bottom = recta.top + TITLE_BAR_HEIGHT;
		
		int x = mouseX - window->m_rect.left;
		int y = mouseY - window->m_rect.top;
		Point mousePoint = {x, y};
		
		if (RectangleContains(&recta, &mousePoint))
		{
			window->m_isBeingDragged = true;
			
			HideWindow(window);
			
			//change cursor:
			g_windowDragCursor.width    = window->m_vbeData.m_width;
			g_windowDragCursor.height   = window->m_vbeData.m_height;
			g_windowDragCursor.leftOffs = mouseX - window->m_rect.left;
			g_windowDragCursor.topOffs  = mouseY - window->m_rect.top;
			g_windowDragCursor.bitmap   = window->m_vbeData.m_framebuffer32;//cast to fix warning
			
			SetCursor (&g_windowDragCursor);
		}
		else
		{
			WindowRegisterEvent (window, EVENT_CLICKCURSOR, MAKE_MOUSE_PARM (x, y), 0);
		}
	}
	FREE_LOCK(g_windowLock);
}
void OnUILeftClickRelease (int mouseX, int mouseY)
{
	if (!g_windowManagerRunning) return;
	if (g_currentlyClickedWindow == -1) return;
	
	ACQUIRE_LOCK (g_windowLock);
	
	g_prevMouseX = (int)mouseX;
	g_prevMouseY = (int)mouseY;
	
//	short idx = GetWindowIndexInDepthBuffer(mouseX, mouseY);
	
	Window* window = GetWindowFromIndex(g_currentlyClickedWindow);
	if (window->m_isBeingDragged)
	{
		Rectangle newWndRect;
		newWndRect.left   = mouseX - g_windowDragCursor.leftOffs;
		newWndRect.top    = mouseY - g_windowDragCursor.topOffs;
		newWndRect.right  = newWndRect.left + GetWidth(&window->m_rect);
		newWndRect.bottom = newWndRect.top  + GetHeight(&window->m_rect);
		window->m_rect = newWndRect;
		
		if (GetCurrentCursor() == &g_windowDragCursor)
		{
			SetCursor(NULL);
		}
		
		ShowWindow(window);
		WindowRegisterEvent(window, EVENT_PAINT, 0, 0);
		window->m_isBeingDragged = false;
	}
	int x = mouseX - window->m_rect.left;
	int y = mouseY - window->m_rect.top;
	WindowRegisterEvent (window, EVENT_RELEASECURSOR, MAKE_MOUSE_PARM (x, y), 0);
	
	FREE_LOCK(g_windowLock);
}
void OnUIRightClick (int mouseX, int mouseY)
{
	if (!g_windowManagerRunning) return;
	g_prevMouseX = (int)mouseX;
	g_prevMouseY = (int)mouseY;
	
	short idx = GetWindowIndexInDepthBuffer(mouseX, mouseY);
	
	if (idx > -1)
	{
		Window* window = GetWindowFromIndex(idx);
		
		//hide this window:
		HideWindow(window);
	}
}

#endif

// Main loop thread.
#if 1

void RedrawEverything()
{
	cli;
//	Rectangle r = {0, 0, GetScreenSizeX(), GetScreenSizeY() };
	/*VidFillScreen(BACKGROUND_COLOR);
	
	//wait for apps to fully setup their windows:
	sti;
	for (int i = 0; i < 50000; i++)
		hlt;
	cli;*/
	
	UpdateDepthBuffer();
	
	//for each window, send it a EVENT_PAINT:
	for (int p = 0; p < WINDOWS_MAX; p++)
	{
		Window* pWindow = &g_windows [p];
		if (!pWindow->m_used) continue;
		
		WindowRegisterEvent (pWindow, EVENT_PAINT, 0, 0);
	}
	
	sti;
}

bool HandleMessages(Window* pWindow);
void RenderWindow (Window* pWindow);
void WindowManagerTask(__attribute__((unused)) int useless_argument)
{
	g_clickQueueSize = 0;
	// load background?
	memset (&g_windows, 0, sizeof (g_windows));
	InitWindowDepthBuffer();
	CoClearScreen (&g_debugConsole);
	g_debugConsole.curX = g_debugConsole.curY = 0;
	g_debugConsole.pushOrWrap = 1;
	
	g_windowManagerRunning = true;
	
	UpdateDepthBuffer();
	VidFillScreen(BACKGROUND_COLOR);
	
	//redraw background?
	__attribute__((unused)) Rectangle r = {0, 0, GetScreenSizeX(), GetScreenSizeY() };
	//RedrawBackground (r);
	
	//CreateTestWindows();
	UpdateDepthBuffer();
	
	VidSetFont(FONT_BASIC);
	
	//test:
#if !THREADING_ENABLED
	TestProgramTask (0);
	IconTestTask(0);
	PrgPaintTask(0);
#else
	int errorCode = 0;
	Task* pTask = KeStartTask(TestProgramTask, 0, &errorCode);
	DebugLogMsg("Created test task 1. pointer returned:%x, errorcode:%x", pTask, errorCode);
	errorCode = 0;
	pTask = KeStartTask(IconTestTask, 0, &errorCode);
	DebugLogMsg("Created test task 2. pointer returned:%x, errorcode:%x", pTask, errorCode);
	errorCode = 0;
	pTask = KeStartTask(PrgPaintTask, 0, &errorCode);
	DebugLogMsg("Created test task 3. pointer returned:%x, errorcode:%x", pTask, errorCode);
#endif
	
	ACQUIRE_LOCK (g_clickQueueLock);
	ACQUIRE_LOCK (g_screenLock);
	//wait a bit
	
	for (int i = 0; i < 500; i++)
		hlt;
	//we're done.  Redraw everything.
	//UpdateDepthBuffer();
	RedrawEverything();
	
	for (int i = 0; i < 500; i++)
		hlt;
	
	FREE_LOCK (g_screenLock);
	FREE_LOCK (g_clickQueueLock);
	
	int timeout = 10;
	
	while (g_windowManagerRunning)
	{
		for (int p = 0; p < WINDOWS_MAX; p++)
		{
			Window* pWindow = &g_windows [p];
			if (!pWindow->m_used) continue;
			
			WindowRegisterEvent (pWindow, EVENT_UPDATE, 0, 0);
			
		#if !THREADING_ENABLED
			if (!HandleMessages (pWindow))
			{
				ReadyToDestroyWindow(pWindow);
				continue;
			}
		#endif
			if (pWindow->m_vbeData.m_dirty && !pWindow->m_hidden)
				RenderWindow(pWindow);
			
			if (pWindow->m_markedForDeletion)
			{
				//turn it off, because DestroyWindow sends an event here, 
				//and we don't want it to stack overflow. Stack overflows
				//go pretty ugly in this OS, so we need to be careful.
				pWindow->m_markedForDeletion = false;
				DestroyWindow (pWindow);
			}
		}
		
		ACQUIRE_LOCK (g_clickQueueLock);
		ACQUIRE_LOCK (g_screenLock);
		for (int i = 0; i < g_clickQueueSize; i++)
		{
			switch (g_clickQueue[i].clickType)
			{
				case CLICK_LEFT:   OnUILeftClick       (g_clickQueue[i].clickedAtX, g_clickQueue[i].clickedAtY); break;
				case CLICK_LEFTD:  OnUILeftClickDrag   (g_clickQueue[i].clickedAtX, g_clickQueue[i].clickedAtY); break;
				case CLICK_LEFTR:  OnUILeftClickRelease(g_clickQueue[i].clickedAtX, g_clickQueue[i].clickedAtY); break;
				case CLICK_RIGHT:  OnUIRightClick      (g_clickQueue[i].clickedAtX, g_clickQueue[i].clickedAtY); break;
			}
		}
		g_clickQueueSize = 0;
		FREE_LOCK (g_screenLock);
		FREE_LOCK (g_clickQueueLock);
		
		timeout--;
		
		for (int i = 0; i < 2; i++)
			hlt;
	}
	KillWindowDepthBuffer();
}
#endif

// Control creation and management
#if 1

//Returns an index, because we might want to relocate the m_pControlArray later.
int AddControl(Window* pWindow, int type, Rectangle rect, const char* text, int p1, int p2)
{
	if (!pWindow->m_pControlArray)
	{
		VidSetVBEData(NULL);
		LogMsg("No pControlArray!?");
		KeStopSystem();
		return -1;
	}
	int index = -1;
	for (int i = 0; i < pWindow->m_controlArrayLen; i++)
	{
		if (!pWindow->m_pControlArray[i].m_active)
		{
			index = i;
			break;
		}
	}
	if (index <= -1)
	{
		//Couldn't find a spot in the currently allocated thing.
		//Perhaps we need to expand the array.
		int cal = pWindow->m_controlArrayLen;
		if (cal < 2) cal = 2;
		
		cal += cal / 2;
		//series: 2, 3, 4, 6, 9, 13, 19, 28, 42, ...
		
		size_t newSize = sizeof(Control) * cal;
		Control* newCtlArray = (Control*)MmAllocate(newSize);
		memset(newCtlArray, 0, newSize);
		
		// copy stuff into the new control array:
		memcpy(newCtlArray, pWindow->m_pControlArray, sizeof(Control) * pWindow->m_controlArrayLen);
		
		// free the previous array:
		MmFree(pWindow->m_pControlArray);
		
		// then assign the new one
		pWindow->m_pControlArray   = newCtlArray;
		pWindow->m_controlArrayLen = cal;
		
		// last, re-search the thing
		index = -1;
		for (int i = 0; i < pWindow->m_controlArrayLen; i++)
		{
			if (!pWindow->m_pControlArray[i].m_active)
			{
				index = i;
				break;
			}
		}
	}
	if (index <= -1)
	{
		return -1;
	}
	
	// add the control itself:
	Control *pControl = &pWindow->m_pControlArray[index];
	pControl->m_active  = true;
	pControl->m_type    = type;
	pControl->m_dataPtr = NULL;
	pControl->m_rect    = rect;
	pControl->m_parm1   = p1;
	pControl->m_parm2   = p2;
	pControl->m_bMarkedForDeletion = false;
	
	if (text)
		strcpy (pControl->m_text, text);
	else
		pControl->m_text[0] = '\0';
	
	pControl->OnEvent = GetWidgetOnEventFunction(type);
	
	//register an event for the window:
	//WindowRegisterEvent(pWindow, EVENT_PAINT, 0, 0);
	
	return index;
}

void RemoveControl (Window* pWindow, int controlIndex)
{
	if (controlIndex >= pWindow->m_controlArrayLen || controlIndex < 0) return;
	
	cli;
	Control* pControl = &pWindow->m_pControlArray[controlIndex];
	if (pControl->m_dataPtr)
	{
		//TODO
	}
	pControl->m_active = false;
	pControl->m_bMarkedForDeletion = false;
	pControl->OnEvent = NULL;
	
	sti;
}

void ControlProcessEvent (Window* pWindow, int eventType, int parm1, int parm2)
{
	for (int i = 0; i != pWindow->m_controlArrayLen; i++)
	{
		if (pWindow->m_pControlArray[i].m_active)
		{
			Control* p = &pWindow->m_pControlArray[i];
			if (p->OnEvent)
				p->OnEvent(p, eventType, parm1, parm2, pWindow);
		}
	}
}

#endif

// Event processors called by user processes.
#if 1
extern VBEData* g_vbeData, g_mainScreenVBEData;
void RenderWindow (Window* pWindow)
{
	ACQUIRE_LOCK(g_screenLock);
	VBEData* backup = g_vbeData;
	g_vbeData = &g_mainScreenVBEData;
	
	int windIndex = pWindow - g_windows;
	int x = pWindow->m_rect.left,  y = pWindow->m_rect.top;
	int tw = pWindow->m_vbeData.m_width, th = pWindow->m_vbeData.m_height;
	uint32_t *texture = pWindow->m_vbeData.m_framebuffer32;
	
	int o = 0;
	int x2 = x + tw, y2 = y + th;
	
	for (int j = y; j != y2; j++)
	{
		for (int i = x; i != x2; i++)
		{
			short n = GetWindowIndexInDepthBuffer (i, j);
			if (n == windIndex || n == -1)
			{
				if (texture[o] != TRANSPARENT)
					VidPlotPixel (i, j, texture[o]);
			}
			o++;
		}
	}
	
	g_vbeData = backup;
	FREE_LOCK(g_screenLock);
}

void PaintWindowBorder(Window* pWindow)
{
	Rectangle recta = pWindow->m_rect;
	recta.right  -= recta.left; recta.left = 0;
	recta.bottom -= recta.top;  recta.top  = 0;
	
	//! X adjusts the size of the dropshadow on the window.
	recta.right  -= WINDOW_RIGHT_SIDE_THICKNESS+1;
	recta.bottom -= WINDOW_RIGHT_SIDE_THICKNESS+1;
	
	Rectangle rectb = recta;
	
	VidFillRectangle(WINDOW_BACKGD_COLOR, recta);
	VidDrawRectangle(WINDOW_EDGE_COLOR, recta);
	
	for (int i = 0; i < WINDOW_RIGHT_SIDE_THICKNESS; i++) {
		recta.left++; recta.right++; recta.bottom++; recta.top++;
		VidDrawHLine(WINDOW_EDGE_COLOR, recta.left, recta.right, recta.bottom);
		VidDrawVLine(WINDOW_EDGE_COLOR, recta.top, recta.bottom, recta.right);
	}
	
	//draw a white border thing:
	rectb.left++;
	rectb.top ++;
	rectb.right--;
	rectb.bottom--;
	//VidDrawRectangle(WINDOW_TITLE_TEXT_COLOR, rectb);
	VidDrawHLine (WINDOW_TITLE_TEXT_COLOR,     rectb.left, rectb.right, rectb.top);
	VidDrawHLine (WINDOW_TITLE_INACTIVE_COLOR, rectb.left, rectb.right, rectb.bottom);
	VidDrawVLine (WINDOW_TITLE_TEXT_COLOR,     rectb.top, rectb.bottom, rectb.left);
	VidDrawVLine (WINDOW_TITLE_INACTIVE_COLOR, rectb.top, rectb.bottom, rectb.right);
	
	//draw the window title:
	rectb.left++;
	rectb.top ++;
	rectb.right--;
	rectb.bottom = rectb.top + TITLE_BAR_HEIGHT;
	
	//todo: gradients?
	VidFillRectangle(pWindow->m_isSelected ? WINDOW_TITLE_ACTIVE_COLOR : WINDOW_TITLE_INACTIVE_COLOR, rectb);
	
	VidTextOut(pWindow->m_title, rectb.left + 2, rectb.top + 2, WINDOW_TITLE_TEXT_COLOR_SHADOW, TRANSPARENT);
	VidTextOut(pWindow->m_title, rectb.left + 1, rectb.top + 1, WINDOW_TITLE_TEXT_COLOR, TRANSPARENT);
	
#undef X
}
void PaintWindowBackgroundAndBorder(Window* pWindow)
{
	VidFillScreen(TRANSPARENT);
	PaintWindowBorder(pWindow);
}
bool IsEventDestinedForControlsToo(int type)
{
	switch (type)
	{
		case EVENT_PAINT:
		case EVENT_MOVECURSOR:
		case EVENT_CLICKCURSOR:
		case EVENT_RELEASECURSOR:
			return true;
	}
	return false;
}
bool HandleMessages(Window* pWindow)
{
	// grab the lock
	ACQUIRE_LOCK (g_screenLock);
	ACQUIRE_LOCK (g_windowLock);
	ACQUIRE_LOCK (pWindow->m_eventQueueLock);
	
	for (int i = 0; i < pWindow->m_eventQueueSize; i++)
	{
		//setup paint stuff so the window can only paint in their little box
		VidSetVBEData (&pWindow->m_vbeData);
		if (pWindow->m_eventQueue[i] == EVENT_CREATE || pWindow->m_eventQueue[i] == EVENT_PAINT)
			PaintWindowBackgroundAndBorder(pWindow);
		
		pWindow->m_callback(pWindow, pWindow->m_eventQueue[i], pWindow->m_eventQueueParm1[i], pWindow->m_eventQueueParm2[i]);
		
		if (IsEventDestinedForControlsToo(pWindow->m_eventQueue[i]))
			ControlProcessEvent(pWindow, pWindow->m_eventQueue[i], pWindow->m_eventQueueParm1[i], pWindow->m_eventQueueParm2[i]);
		
		//reset to main screen
		VidSetVBEData (NULL);
		
		//if the contents of this window have been modified, redraw them:
		//if (pWindow->m_vbeData.m_dirty && !pWindow->m_hidden)
		//	RenderWindow(pWindow);
		
		if (pWindow->m_eventQueue[i] == EVENT_DESTROY) return false;
	}
	pWindow->m_eventQueueSize = 0;
	
	FREE_LOCK (pWindow->m_eventQueueLock);
	FREE_LOCK (g_windowLock);
	FREE_LOCK (g_screenLock);
	hlt; //give it a good halt
	return true;
}

void PostQuitMessage (Window* pWindow)
{
	ReadyToDestroyWindow(pWindow);
	
	#if THREADING_ENABLED
	KeExit();
	#endif
}

void DefaultWindowProc (Window* pWindow, int messageType, UNUSED int parm1, UNUSED int parm2)
{
	switch (messageType)
	{
		case EVENT_CREATE:
			//VidFillScreen(0xFFAAAAAA);
			
			//paint window border:
			//also call an EVENT_PAINT
			pWindow->m_callback(pWindow, EVENT_PAINT, 0, 0);
			break;
		case EVENT_PAINT:
			//nope, user should handle this themselves
			//Actually EVENT_PAINT just requests a paint event,
			//so just mark this as dirty
			pWindow->m_vbeData.m_dirty = 1;
			break;
		case EVENT_SETFOCUS:
		case EVENT_KILLFOCUS:
			PaintWindowBorder(pWindow);
			break;
		case EVENT_DESTROY:
			PostQuitMessage(pWindow);//exits
			break;
		default:
			break;
	}
}
#endif

// Test programs
#if 1
void CALLBACK TestProgramProc (Window* pWindow, int messageType, int parm1, int parm2)
{
	int npp = GetNumPhysPages(), nfpp = GetNumFreePhysPages();
	switch (messageType)
	{
		case EVENT_CREATE: {
			//add a predefined list of controls:
			Rectangle r = {108, 200, 208, 220};
			
			//parm1 is the button number that we're being fed in EVENT_COMMAND
			AddControl (pWindow, CONTROL_BUTTON, r, "Click Me!", 1, 0);
			
			Rectangle r1 = {250,108,320,120};
			AddControl (pWindow, CONTROL_TEXT, r1, "Hello", 0xFFFFFF, TRANSPARENT);
			
			Rectangle r2 = {200,100,232,120};
			AddControl (pWindow, CONTROL_ICON, r2, NULL, ICON_GLOBE, 0);
			
			break;
		}
		case EVENT_PAINT: {
			char test[100];
			sprintf(test, "Hi!  Memory usage: %d KB / %d KB", (npp-nfpp)*4, npp*4);
			VidFillRect (0xFF00FF, 10, 40, 100, 120);
			VidTextOut (test, 10, 30, 0, TRANSPARENT);
			break;
		}
		case EVENT_COMMAND: {
			if (parm1 == 1)
			{
				//The only button:
				int randomX = GetRandom() % 320;
				int randomY = GetRandom() % 240;
				int randomColor = GetRandom();
				VidTextOut("*click*", randomX, randomY, randomColor, TRANSPARENT);
			}
			break;
		}
		default:
			DefaultWindowProc(pWindow, messageType, parm1, parm2);
	}
}
void CALLBACK IconTestProc (Window* pWindow, int messageType, int parm1, int parm2)
{
	switch (messageType)
	{
		case EVENT_PAINT:
			//draw until ICON_COUNT:
			for (int i = ICON_NULL+1; i < ICON_COUNT; i++)
			{
				int x = i & 7, y = i >> 3;
				RenderIcon((IconType)i, x*32 + 10, y*32 + 15);
			}
			/*RenderIcon(ICON_CABINET, 10, 20);*/
			break;
		default:
			DefaultWindowProc(pWindow, messageType, parm1, parm2);
	}
}
int g_paint1X = -1, g_paint1Y = -1;
void CALLBACK PrgPaintProc (Window* pWindow, int messageType, int parm1, int parm2)
{
	switch (messageType)
	{
		case EVENT_CREATE:
			g_paint1X = g_paint1Y = -1;
			DefaultWindowProc(pWindow, messageType, parm1, parm2);
			break;
		case EVENT_PAINT:
			//VidFillRect (0xFF00FF, 10, 40, 100, 120);
			//VidTextOut ("Hey, it's the window :)", 50, 50, TRANSPARENT, 0xe0e0e0);
			break;
		case EVENT_CLICKCURSOR:
		case EVENT_MOVECURSOR:
			if (g_paint1X == -1)
			{
				VidPlotPixel(GET_X_PARM(parm1), GET_Y_PARM(parm1), parm1);
			}
			else
			{
				VidDrawLine(parm1, g_paint1X, g_paint1Y, GET_X_PARM(parm1), GET_Y_PARM(parm1));
			}
			g_paint1X = GET_X_PARM(parm1);
			g_paint1Y = GET_Y_PARM(parm1);
			break;
		case EVENT_RELEASECURSOR:
			g_paint1X = g_paint1Y = -1;
			break;
		default:
			DefaultWindowProc(pWindow, messageType, parm1, parm2);
	}
}

void TestProgramTask (__attribute__((unused)) int argument)
{
	// create ourself a window:
	Window* pWindow = CreateWindow ("Hello World", 100, 100, 320, 240, TestProgramProc);
	
	if (!pWindow)
		DebugLogMsg("Hey, the window couldn't be created");
	
	// setup:
	//ShowWindow(pWindow);
	
	// event loop:
#if THREADING_ENABLED
	while (HandleMessages (pWindow));
#endif
}
void IconTestTask (__attribute__((unused)) int argument)
{
	// create ourself a window:
	Window* pWindow = CreateWindow ("Icon Test", 300, 200, 320, 240, IconTestProc);
	
	if (!pWindow)
		DebugLogMsg("Hey, the window couldn't be created");
	
	// setup:
	//ShowWindow(pWindow);
	
	// event loop:
#if THREADING_ENABLED
	while (HandleMessages (pWindow));
#endif
}

void PrgPaintTask (__attribute__((unused)) int argument)
{
	// create ourself a window:
	Window* pWindow = CreateWindow ("Scribble!", 200, 300, 500, 400, PrgPaintProc);
	
	if (!pWindow)
		DebugLogMsg("Hey, the window couldn't be created");
	
	// setup:
	//ShowWindow(pWindow);
	
	// event loop:
#if THREADING_ENABLED
	while (HandleMessages (pWindow));
#endif
}


#endif
