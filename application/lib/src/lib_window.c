#include "../include/window.h"
#include "lib_header.h"

Window* CreateWindow(char* title, int xPos, int yPos, int xSize, int ySize, WindowProc proc)
{
	//perform a syscall
	
	//note that some properties need to be combined
	
	return (Window*)DoSyscall(WINDOW_CREATE, (int)title, (int)(xPos<<16|yPos), (int)(xSize<<16|ySize), (int)WindowProc, 0);
}
