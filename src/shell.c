/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

     Command line kernel shell module
******************************************/
#include <main.h>
#include <shell.h>
#include <keyboard.h>
#include <string.h>
#include <console.h>
#include <vga.h>
#include <video.h>
#include <print.h>
#include <memory.h>
#include <misc.h>
#include <task.h>
#include <storabs.h>
#include <window.h>

char g_lastCommandExecuted[256] = {0};

void ShellTaskTest(int arg)
{
	while (1)	
	{
		SLogMsg("Task %d!", arg);
		for (int i = 0; i < 100; i++)
			hlt;
	}
}

void ShellTaskTest2(int arg)
{
	//This is the entry point of the new thread.
	//You can pass any 32-bit parm in the StartTask call. `arg` is one of them.
	//var represents the next color to set
	int var = 0;
	while (1)	
	{
		// set a 100 pixel tall column at that position:
		for (int y = 100; y < 200; y++)
			VidPlotPixel (arg%GetScreenSizeX(), y+arg/GetScreenSizeX(), var);
		
		//increment color by 32
		var += 32;
		
		//wait for 5 interrupts
		for (int i = 0; i < 5; i++)
			hlt;
	}
}

void GraphicsTest()
{
	CoClearScreen(&g_debugConsole);
	
	//demonstrate some of the apis that the kernel provides:
	VidFillRect(0xFF0000, 10, 150, 210, 310);
	VidDrawRect(0x00FF00, 100, 150, 250, 250);
	
	//lines, triangles, polygons, circles perhaps?
	VidDrawHLine (0xEE00FF, 100, 500, 400);
	VidDrawVLine (0xEE00FF, 150, 550, 15);
	
	VidPlotChar('A', 200, 100, 0xFF, 0xFF00);
	VidTextOut("Hello, opaque background world.\nI support newlines too!", 300, 150, 0xFFE, 0xAEBAEB);
	VidTextOut("Hello, transparent background world.\nI support newlines too!", 300, 182, 0xFFFFFF, TRANSPARENT);
	VidShiftScreen(10);
	
	LogMsg("Test complete.  Strike a key to exit.");
	KbWaitForKeyAndGet();
}

int g_nextTaskNum = 0;
bool g_ramDiskMounted = 0;
int g_ramDiskID = 0;
void ShellExecuteCommand(char* p)
{
	TokenState state;
	state.m_bInitted = 0;
	char* token = Tokenize (&state, p, " ");
	if (!token)
		return;
	if (*token == 0)
		return;
	if (strcmp (token, "help") == 0)
	{
		LogMsg("NanoShell Shell Help");
		LogMsg("cls        - clear screen");
		LogMsg("crash      - attempt to crash the kernel");
		LogMsg("color XX   - change the screen color");
		LogMsg("help       - shows this list");
		LogMsg("gt         - run a graphical test");
		LogMsg("lm         - list memory allocations");
		LogMsg("lt         - list currently running threads (pauses them during the print)");
		LogMsg("mode X     - change the screen mode");
		LogMsg("mrd        - mounts a testing RAM Disk");
		LogMsg("rd         - reads and dumps a sector from the RAM Disk");
		LogMsg("sysinfo    - dump system information");
		LogMsg("sysinfoa   - dump advanced system information");
		LogMsg("time       - get timing information");
		LogMsg("st         - spawns a single thread that makes a random line forever");
		LogMsg("tt         - spawns 64 threads that makes random lines forever");
		LogMsg("tte        - spawns 1024 threads that makes random lines forever");
		LogMsg("ver        - print system version");
		LogMsg("w          - start desktop manager");
	}
	else if (strcmp (token, "gt") == 0)
	{
		GraphicsTest();
	}
	else if (strcmp (token, "w") == 0)
	{
		WindowManagerTask(0);
	}
	else if (strcmp (token, "mrd") == 0)
	{
		if (g_ramDiskMounted)
		{
			LogMsg("Have a ramdisk mounted already.");
			return;
		}
		g_ramDiskID = StMountTestRamDisk();
		g_ramDiskMounted = true;
	}
	else if (strcmp (token, "rd") == 0)
	{
		if (!g_ramDiskMounted)
		{
			LogMsg("Must mount a ramdisk first.  Please use \"mrd\".");
			return;
		}
		char* secNum = Tokenize (&state, NULL, " ");
		if (!secNum)
		{
			LogMsg("Expected sector number");
		}
		else if (*secNum == 0)
		{
			LogMsg("Expected sector number");
		}
		else
		{
			int e = atoi (secNum);
			
			char sector[512];
			DriveStatus f = StDeviceRead(e, sector, g_ramDiskID, 1);
			LogMsg("Printing sector number %d.  The returned %x.", e, f);
			
			for (int i = 0; i < SECTOR_SIZE; i += 16)
			{
				LogMsgNoCr(" %x:", i);
				for (int j = 0; j < 16; j++)
				{
					LogMsgNoCr(" %B", sector[i+j]);
				}
				LogMsgNoCr("   ");
				for (int j = 0; j < 16; j++)
				{
					char e = sector[i+j];
					/**/ if (e == 0x00) e = ' ';
					else if (e <  0x20) e = '.';
					else if (e >= 0x7F) e = '.';
					LogMsgNoCr("%c", e);
				}
				LogMsg("");
			}
			
			//PrInitialize();
		}
	}
	else if (strcmp (token, "cls") == 0)
	{
		CoClearScreen (&g_debugConsole);
		g_debugConsole.curX = g_debugConsole.curY = 0;
	}
	else if (strcmp (token, "ver") == 0)
	{
		KePrintSystemVersion();
	}
	else if (strcmp (token, "lm") == 0)
	{
		MmDebugDump();
	}
	else if (strcmp (token, "lt") == 0)
	{
		KeTaskDebugDump();
	}
	else if (strcmp (token, "st") == 0)
	{
		int errorCode = 0;
		Task* task = KeStartTask(ShellTaskTest2, g_nextTaskNum++, &errorCode);
		LogMsg("Task %d (%x) spawned.  Error code: %x", g_nextTaskNum - 1, task, errorCode);
	}
	else if (strcmp (token, "tt") == 0)
	{
		int errorCode = 0;
		for (int i = 0; i < 64; i++)
		{
			KeStartTask(ShellTaskTest2, g_nextTaskNum++, &errorCode);
		}
		LogMsg("Tasks have been spawned.");
		//LogMsg("Task %d (%x) spawned.  Error code: %x", g_nextTaskNum - 1, task, errorCode);
	}
	else if (strcmp (token, "tte") == 0)
	{
		int errorCode = 0;
		for (int i = 0; i < 1024; i++)
		{
			KeStartTask(ShellTaskTest2, g_nextTaskNum++, &errorCode);
		}
		LogMsg("Tasks have been spawned.");
	}
	else if (strcmp (token, "crash") == 0)
	{
		LogMsg("OK");
		*((uint32_t*)0xFFFFFFFF) = 0;
	}
	else if (strcmp (token, "time") == 0)
	{
		int hi, lo;
		GetTimeStampCounter(&hi, &lo);
		LogMsg("Timestamp counter: %x%x (%d, %d)", hi, lo, hi, lo);
		
		int tkc = GetTickCount(), rtkc = GetRawTickCount();
		LogMsg("Tick count: %d, Raw tick count: %d", tkc, rtkc);
	}
	else if (strcmp (token, "mode") == 0)
	{
		if (VidIsAvailable())
		{
			LogMsg("Must use emergency text-mode shell to change mode.");
			return;
		}
		char* modeNum = Tokenize (&state, NULL, " ");
		if (!modeNum)
		{
			LogMsg("Expected mode number");
		}
		else if (*modeNum == 0)
		{
			LogMsg("Expected mode number");
		}
		else
		{
			SwitchMode (*modeNum - '0');
			//PrInitialize();
		}
	}
	else if (strcmp (token, "font") == 0)
	{
		char* fontNum = Tokenize (&state, NULL, " ");
		if (!fontNum)
		{
			LogMsg("Expected mode number");
		}
		else if (*fontNum == 0)
		{
			LogMsg("Expected font number");
		}
		else
		{
			VidSetFont (*fontNum - '0');
			LogMsg("the quick brown fox jumps over the lazy dog");
			LogMsg("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
			LogMsg("Font testing done.");
			//PrInitialize();
		}
	}
	else if (strcmp (token, "color") == 0)
	{
		char* colorNum = Tokenize (&state, NULL, " ");
		if (!colorNum)
		{
			LogMsg("Expected color hex");
		}
		else if (*colorNum == 0 || *(colorNum + 1) == 0)
		{
			LogMsg("Expected color hex");
		}
		else
		{
			//SwitchMode (*modeNum - '0');
			char c1 = colorNum[0], c2 = colorNum[1];
			
			/**/ if (c1 >= '0' && c1 <= '9') c1 -= '0';
			else if (c1 >= 'A' && c1 <= 'F') c1 -= 'A'-0xA;
			else if (c1 >= 'a' && c1 <= 'f') c1 -= 'a'-0xA;
			
			/**/ if (c2 >= '0' && c2 <= '9') c2 -= '0';
			else if (c2 >= 'A' && c2 <= 'F') c2 -= 'A'-0xA;
			else if (c2 >= 'a' && c2 <= 'f') c2 -= 'a'-0xA;
			
			g_debugConsole.color = c1 << 4 | c2;
		}
	}
	else if (strcmp (token, "sysinfo") == 0)
	{
		KePrintSystemInfo();
	}
	else if (strcmp (token, "sysinfoa") == 0)
	{
		KePrintSystemInfoAdvanced();
	}
	else
	{
		LogMsg("Unknown command.  Please type 'help'.");
	}
	
	//LogMsg("You typed: '%s'", p);
}

void ShellRun()
{
	while (1) 
	{
		LogMsgNoCr("shell>");
		char buffer[256];
		KbGetString (buffer, 256);
		memcpy (g_lastCommandExecuted, buffer, 256);
		
		ShellExecuteCommand (buffer);
		
		hlt; hlt; hlt; hlt;
	}
}
