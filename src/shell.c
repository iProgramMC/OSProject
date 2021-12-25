#include <main.h>
#include <shell.h>
#include <keyboard.h>
#include <string.h>
#include <console.h>
#include <vga.h>
#include <print.h>
#include <memory.h>

void ShellExecuteCommand(char* p)
{
	TokenState state;
	state.m_bInitted = 0;
	char* token = Tokenize (&state, p, " ");
	
	if (strcmp (token, "help") == 0)
	{
		LogMsg("NanoShell Shell Help");
		LogMsg("cls        - clear screen");
		LogMsg("crash      - attempt to crash the kernel");
		LogMsg("color XX   - change the screen color");
		LogMsg("help       - shows this list");
		LogMsg("lm         - list memory allocations");
		LogMsg("mode X     - change the screen mode");
		LogMsg("sysinfo    - dump system information");
		LogMsg("ver        - print system version");
	}
	else if (strcmp (token, "cls") == 0)
	{
		PrInitialize();
		//CoClearScreen (&g_debugConsole);
	}
	else if (strcmp (token, "ver") == 0)
	{
		KePrintSystemVersion();
	}
	else if (strcmp (token, "lm") == 0)
	{
		MmDebugDump();
	}
	else if (strcmp (token, "crash") == 0)
	{
		LogMsg("OK");
		*((uint32_t*)0xFFFFFFFF) = 0;
	}
	else if (strcmp (token, "mode") == 0)
	{
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
			PrInitialize();
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
		LogMsgNoCr("\nshell>");
		char buffer[256];
		KbGetString (buffer, 256);
		
		ShellExecuteCommand (buffer);
	}
}
