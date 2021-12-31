/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

           Window Manager module
******************************************/
#define THREADING_ENABLED 1 //0

#if THREADING_ENABLED
#define MULTITASKED_WINDOW_MANAGER
#endif

#define DebugLogMsg  SLogMsg

#include <window.h>
#include <task.h>


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

void TestProgramTask (int argument);
void TestProgramTask1(int argument);
void PrgPaintTask    (int argument);
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
					//WindowRegisterEvent(&g_windows[i], EVENT_PAINT, 0, 0);
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

bool HandleMessages(Window* pWindow);
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
	TestProgramTask1(0);
	PrgPaintTask(0);
#else
	int errorCode = 0;
	Task* pTask = KeStartTask(TestProgramTask, 0, &errorCode);
	DebugLogMsg("Created test task 1. pointer returned:%x, errorcode:%x", pTask, errorCode);
	errorCode = 0;
	pTask = KeStartTask(TestProgramTask1, 0, &errorCode);
	DebugLogMsg("Created test task 2. pointer returned:%x, errorcode:%x", pTask, errorCode);
	errorCode = 0;
	pTask = KeStartTask(PrgPaintTask, 0, &errorCode);
	DebugLogMsg("Created test task 3. pointer returned:%x, errorcode:%x", pTask, errorCode);
#endif
	
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
		
		for (int i =0; i<2; i++)
			hlt;
	}
	KillWindowDepthBuffer();
}
#endif

// Event processors called by user processes.
#if 1
void RenderWindow (Window* pWindow)
{
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
}

void PaintWindowBackgroundAndBorder(Window* pWindow)
{
	VidFillScreen(TRANSPARENT);
	
	Rectangle recta = pWindow->m_rect;
	recta.right  -= recta.left; recta.left = 0;
	recta.bottom -= recta.top;  recta.top  = 0;
	
	//! X adjusts the size of the dropshadow on the window.
	recta.right  -= WINDOW_RIGHT_SIDE_THICKNESS+1;
	recta.bottom -= WINDOW_RIGHT_SIDE_THICKNESS+1;
	
	Rectangle rectb = recta;
	
	VidFillRectangle(0xAAAAAA, recta);
	VidDrawRectangle(0x000000, recta);
	
	for (int i = 0; i < WINDOW_RIGHT_SIDE_THICKNESS; i++) {
		recta.left++; recta.right++; recta.bottom++; recta.top++;
		VidDrawHLine(0x000000, recta.left, recta.right, recta.bottom);
		VidDrawVLine(0x000000, recta.top, recta.bottom, recta.right);
	}
	
	//draw the window title:
	rectb.left++;
	rectb.top ++;
	rectb.right--;
	rectb.bottom = rectb.top + TITLE_BAR_HEIGHT;
	
	//todo: gradients?
	VidFillRectangle(0x00007F, rectb);
	
	VidTextOut(pWindow->m_title, rectb.left + 1, rectb.top + 1, 0xFFFFFF, 0x00007F);
	
#undef X
}

bool HandleMessages(Window* pWindow)
{
	// grab the lock
	ACQUIRE_LOCK (g_screenLock);
	ACQUIRE_LOCK (g_windowLock);
	ACQUIRE_LOCK (pWindow->m_eventQueueLock);
	
	int size = pWindow->m_eventQueueSize;
	pWindow->m_eventQueueSize = 0;
	for (int i = 0; i < size; i++)
	{
		//setup paint stuff so the window can only paint in their little box
		VidSetVBEData (&pWindow->m_vbeData);
		if (pWindow->m_eventQueue[i] == EVENT_CREATE || pWindow->m_eventQueue[i] == EVENT_PAINT)
			PaintWindowBackgroundAndBorder(pWindow);
		
		pWindow->m_callback(pWindow, pWindow->m_eventQueue[i], pWindow->m_eventQueueParm1[i], pWindow->m_eventQueueParm2[i]);
		
		//reset to main screen
		VidSetVBEData (NULL);
		
		//if the contents of this window have been modified, redraw them:
		if (pWindow->m_vbeData.m_dirty && !pWindow->m_hidden)
			RenderWindow(pWindow);
		
		if (pWindow->m_eventQueue[i] == EVENT_DESTROY) return false;
	}
	
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
	switch (messageType)
	{
		case EVENT_PAINT:
			VidFillRect (0xFF00FF, 10, 40, 100, 120);
			VidTextOut ("Hey, it's the window :)", 50, 50, TRANSPARENT, 0xe0e0e0);
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
void TestProgramTask1 (__attribute__((unused)) int argument)
{
	// create ourself a window:
	Window* pWindow = CreateWindow ("Hello World", 300, 200, 320, 240, TestProgramProc);
	
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
	Window* pWindow = CreateWindow ("Scribble!", 200, 500, 500, 400, PrgPaintProc);
	
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
