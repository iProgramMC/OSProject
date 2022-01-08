/*****************************************
		NanoShell Operating System
	      (C) 2022 iProgramInCpp

        Console Window Host module
******************************************/
#include <wterm.h>

#define DebugLogMsg  SLogMsg

void CALLBACK TerminalHostProc (UNUSED Window* pWindow, UNUSED int messageType, UNUSED int parm1, UNUSED int parm2)
{
	Console* pConsole = (Console*)pWindow->m_data;
	switch (messageType)
	{
		case EVENT_CLICKCURSOR:
			//CLogMsgNoCr(pConsole, "Clicked! ");
			break;
		case EVENT_KEYPRESS:
		{
			CoPrintChar(pConsole, (char)parm1);
			break;
		}
		case EVENT_PAINT:
		{
			//re-draw every character.
			for (int j = 0; j < pConsole->height; j++)
			{
				for (int i = 0; i < pConsole->width; i++)
				{
					CoPlotChar(pConsole, i, j, pConsole->textBuffer[i + j * pConsole->width]);
				}
			}
			break;
		}
		default:
			DefaultWindowProc(pWindow, messageType, parm1, parm2);
			break;
	}
	hlt; hlt;
}
//! NOTE: arg is a pointer to an array of 4 ints.
void TerminalHostTask(int arg)
{
	int* array = (int*)arg;
	int arrayDefault[] = { 100, 100, 80, 25 };
	if (!array)
		array = arrayDefault;
	Window *pWindow = CreateWindow(
		"nsterm", 
		array[0], array[1], 
		array[2] * 6 + 6 + WINDOW_RIGHT_SIDE_THICKNESS, 
		array[3] * 8 + 8 + WINDOW_RIGHT_SIDE_THICKNESS + TITLE_BAR_HEIGHT, 
		TerminalHostProc);
	if (!pWindow)
	{
		DebugLogMsg("ERROR: Could not create window for nsterm");
		return;
	}
	
	Console basic_console;
	memset (&basic_console, 0, sizeof(basic_console));
	
	uint16_t* pBuffer = (uint16_t*)MmAllocate(sizeof(uint16_t) * array[2] * array[3]);
	
	basic_console.type = CONSOLE_TYPE_WINDOW;
	basic_console.m_vbeData = &pWindow->m_vbeData;
	basic_console.textBuffer = pBuffer;
	basic_console.width  = array[2];
	basic_console.height = array[3];
	basic_console.offX = 2;
	basic_console.offY = 4 + TITLE_BAR_HEIGHT;
	basic_console.color = 0x06;//green background
	basic_console.curX = basic_console.curY = 0;
	basic_console.pushOrWrap = 0; //wrap for now
	basic_console.cwidth = 6;
	basic_console.cheight = 8;
	
	pWindow->m_data = &basic_console;
	
	/*int confusion = 0;
	Task* pTask = KeStartTask(exec, (int)(&basic_console),  &confusion);
	
	if (!pTask)
	{
		DebugLogMsg("ERROR: Could not spawn task for nsterm (returned error code %x)", confusion);
		DestroyWindow(pWindow);
		ReadyToDestroyWindow(pWindow);
		return;
	}*/
	
	CoClearScreen(&basic_console);
	CLogMsg(&basic_console, "Select this window and type something.");
	
	while (HandleMessages (pWindow));
	
	//KeKillTask(pTask);
}

