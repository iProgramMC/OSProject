/*****************************************
		NanoShell Operating System
	      (C) 2022 iProgramInCpp

    Window Built-in Application module
******************************************/

#include <wbuiltin.h>
#include <wterm.h>

#define DebugLogMsg  SLogMsg

// Test program application.
#if 1
void CALLBACK TestProgramProc (Window* pWindow, int messageType, int parm1, int parm2)
{
	int npp = GetNumPhysPages(), nfpp = GetNumFreePhysPages();
	switch (messageType)
	{
		case EVENT_CREATE: {
			//add a predefined list of controls:
			Rectangle r = {108, 190, 208, 220};
			
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
#endif

// Icon test application.
#if 1
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
#endif

// Scribble application.
#if 1
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

// Main launcher application.
#if 1

void LaunchSystem()
{
	int errorCode = 0;
	Task* pTask = KeStartTask(TestProgramTask, 0, &errorCode);
	DebugLogMsg("Created System window. Pointer returned:%x, errorcode:%x", pTask, errorCode);
}
void LaunchNotepad()
{
	int errorCode = 0;
	Task* pTask = KeStartTask(TerminalHostTask, 0, &errorCode);
	DebugLogMsg("Created System window. Pointer returned:%x, errorcode:%x", pTask, errorCode);
}
void LaunchPaint()
{
	int errorCode = 0;
	Task* pTask = KeStartTask(PrgPaintTask, 0, &errorCode);
	DebugLogMsg("Created System window. Pointer returned:%x, errorcode:%x", pTask, errorCode);
}
void LaunchCabinet()
{
	int errorCode = 0;
	Task* pTask = KeStartTask(IconTestTask, 0, &errorCode);
	DebugLogMsg("Created System window. Pointer returned:%x, errorcode:%x", pTask, errorCode);
}

enum {
	LAUNCHER_SYSTEM = 0x10,
	LAUNCHER_NOTEPAD,
	LAUNCHER_PAINT,
	LAUNCHER_CABINET,
};

#define RECT(rect,x,y,w,h) do {\
	rect.left = x, rect.top = y, rect.right = x+w, rect.bottom = y+h;\
} while (0)
void CALLBACK LauncherProgramProc (Window* pWindow, int messageType, int parm1, int parm2)
{
	//int npp = GetNumPhysPages(), nfpp = GetNumFreePhysPages();
	switch (messageType)
	{
		case EVENT_CREATE: {
			#define START_X 20
			#define STEXT_X 60
			#define START_Y 30
			#define DIST_ITEMS 36
			// Add a label welcoming the user to NanoShell.
			Rectangle r;
			RECT(r, START_X, 20, 200, 20);
			AddControl (pWindow, CONTROL_TEXT, r, "Welcome to NanoShell!", 0, TRANSPARENT);
			
			// Add the system icon.
			RECT(r, START_X, START_Y+0*DIST_ITEMS, 32, 32);
			AddControl(pWindow, CONTROL_ICON, r, NULL, ICON_COMPUTER, 0);
			
			RECT(r, STEXT_X, START_Y+0*DIST_ITEMS, 200, 32);
			AddControl(pWindow, CONTROL_CLICKLABEL, r, "System", LAUNCHER_SYSTEM, 0);
			
			// Add the notepad icon.
			RECT(r, START_X, START_Y+1*DIST_ITEMS, 32, 32);
			AddControl(pWindow, CONTROL_ICON, r, NULL, ICON_CABINET, 0);
			
			RECT(r, STEXT_X, START_Y+1*DIST_ITEMS, 200, 32);
			AddControl(pWindow, CONTROL_CLICKLABEL, r, "File cabinet", LAUNCHER_CABINET, 0);
			
			// Add the notepad icon.
			RECT(r, START_X, START_Y+2*DIST_ITEMS, 32, 32);
			AddControl(pWindow, CONTROL_ICON, r, NULL, ICON_NOTES, 0);
			
			RECT(r, STEXT_X, START_Y+2*DIST_ITEMS, 200, 32);
			AddControl(pWindow, CONTROL_CLICKLABEL, r, "Notepad", LAUNCHER_NOTEPAD, 0);
			
			// Add the paint icon.
			RECT(r, START_X, START_Y+3*DIST_ITEMS, 32, 32);
			AddControl(pWindow, CONTROL_ICON, r, NULL, ICON_DRAW, 0);
			
			RECT(r, STEXT_X, START_Y+3*DIST_ITEMS, 200, 32);
			AddControl(pWindow, CONTROL_CLICKLABEL, r, "Scribble!", LAUNCHER_PAINT, 0);
			
			//DefaultWindowProc(pWindow, messageType, parm1, parm2);
			
			break;
		}
		case EVENT_PAINT: {
			/*char test[100];
			sprintf(test, "Hi!  Memory usage: %d KB / %d KB", (npp-nfpp)*4, npp*4);
			VidFillRect (0xFF00FF, 10, 40, 100, 120);
			VidTextOut (test, 10, 30, 0, TRANSPARENT);*/
			VidFillRect(0xFFFFFF,
						3, 4 + TITLE_BAR_HEIGHT, 
						GetScreenSizeX() - WINDOW_RIGHT_SIDE_THICKNESS - 4, 
						GetScreenSizeY() - WINDOW_RIGHT_SIDE_THICKNESS - 4);
			VidDrawRect(0x7F7F7F,
						3, 4 + TITLE_BAR_HEIGHT, 
						GetScreenSizeX() - WINDOW_RIGHT_SIDE_THICKNESS - 4, 
						GetScreenSizeY() - WINDOW_RIGHT_SIDE_THICKNESS - 4);
			break;
		}
		case EVENT_COMMAND: {
			switch (parm1)
			{
				case LAUNCHER_SYSTEM:
					LaunchSystem();
					break;
				case LAUNCHER_NOTEPAD:
					LaunchNotepad();
					break;
				case LAUNCHER_PAINT:
					LaunchPaint();
					break;
				case LAUNCHER_CABINET:
					LaunchCabinet();
					break;
				/*{
					//The only button:
					int randomX = GetRandom() % 320;
					int randomY = GetRandom() % 240;
					int randomColor = GetRandom();
					VidTextOut("*click*", randomX, randomY, randomColor, TRANSPARENT);
					break;
				}*/
			}
			break;
		}
		default:
			DefaultWindowProc(pWindow, messageType, parm1, parm2);
	}
}

void LauncherProgramTask(__attribute__((unused)) int arg)
{
	// create ourself a window:
	int ww = 300, wh = 200, sw = GetScreenSizeX(), sh = GetScreenSizeY();
	int wx = (sw - ww) / 2, wy = (sh - wh) / 2;
	
	Window* pWindow = CreateWindow ("Home", wx, wy, ww, wh, LauncherProgramProc);
	
	if (!pWindow)
		DebugLogMsg("Hey, the main launcher window couldn't be created");
	
	// setup:
	//ShowWindow(pWindow);
	
	// event loop:
#if THREADING_ENABLED
	while (HandleMessages (pWindow));
#endif
}
#endif
