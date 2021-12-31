/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

           Misc - timing module
******************************************/
#include <main.h>
#include <misc.h>
#include <memory.h>
#include <video.h>

int g_nRtcTicks = 0;
void GetTimeStampCounter(int* high, int* low)
{
	if (!high && !low) return; //! What's the point?
	int edx, eax;
	__asm__ volatile ("rdtsc":"=a"(eax),"=d"(edx));
	if (high) *high = edx;
	if (low ) *low  = eax;
}

int GetTickCount()
{
	return g_nRtcTicks * 1000 / RTC_TICKS_PER_SECOND;
}
int GetRawTickCount()
{
	return g_nRtcTicks;
}

extern uint32_t g_cpuidLastLeaf;
extern char g_cpuidNameEBX[];
extern char g_cpuidBrandingInfo[];
extern CPUIDFeatureBits g_cpuidFeatureBits;

const char* GetCPUType()
{
	return g_cpuidNameEBX;
}
const char* GetCPUName()
{
	return g_cpuidBrandingInfo;
}
CPUIDFeatureBits GetCPUFeatureBits()
{
	return g_cpuidFeatureBits;
}

void KePrintSystemInfoAdvanced()
{
	//oldstyle:
	/*
	LogMsg("Information about the system:");
	LogMsg("CPU Type:        %s", g_cpuidNameEBX);
	LogMsg("CPU Branding:    %s", g_cpuidBrandingInfo);
	LogMsg("Feature bits:    %x", *((int*)&g_cpuidFeatureBits));
	LogMsgNoCr("x86 Family   %d ", g_cpuidFeatureBits.m_familyID);
	LogMsgNoCr("Model %d ", g_cpuidFeatureBits.m_model);
	LogMsg("Stepping %d", g_cpuidFeatureBits.m_steppingID);
	LogMsg("g_cpuidLastLeaf: %d", g_cpuidLastLeaf);*/
	
	//nativeshell style:
	LogMsg("\x01\x0BNanoShell Operating System " VersionString);
	LogMsg("\x01\x0CVersion Number: %d", VersionNumber);
	
	LogMsg("\x01\x0F-------------------------------------------------------------------------------");
	LogMsg("\x01\x09[CPU] Name: %s", GetCPUName());
	LogMsg("\x01\x09[CPU] x86 Family %d Model %d Stepping %d.  Feature bits: %d",
			g_cpuidFeatureBits.m_familyID, g_cpuidFeatureBits.m_model, g_cpuidFeatureBits.m_steppingID);
	LogMsg("\x01\x0A[RAM] PageSize: 4K. Physical pages: %d. Total physical RAM: %d Kb", GetNumPhysPages(), GetNumPhysPages()*4);
	LogMsg("\x01\x0A[VID] Screen resolution: %dx%d.  Textmode size: %dx%d characters, of type %d.", GetScreenSizeX(), GetScreenSizeY(), 
																						g_debugConsole.width, g_debugConsole.height, g_debugConsole.type);
	LogMsg("\x01\x0F");
}

void KePrintSystemInfo()
{
	//neofetch style:
	LogMsg("(note, the logo you see is temporary)");
	LogMsg("\x01\x0E N    N       \x01\x0C OS:       NanoShell Operating System");
	LogMsg("\x01\x0E NN   N       \x01\x0C Kernel:   %s (%d)", VersionString, VersionNumber);
	LogMsg("\x01\x0E N N  N       \x01\x0C Uptime:   ?");
	LogMsg("\x01\x0E N  N N       \x01\x0C CPU:      %s", GetCPUName());
	LogMsg("\x01\x0E N   NN       \x01\x0C CPU type: %s", GetCPUType());
	LogMsg("\x01\x0E N    N\x01\x0D SSSS  \x01\x0C Memory:   %d KB / %d KB", GetNumFreePhysPages()*4, GetNumPhysPages()*4);
	LogMsg("\x01\x0D       S    S \x01\x0C ");
	LogMsg("\x01\x0D       S      \x01\x0C ");
	LogMsg("\x01\x0D        SSSS  \x01\x0C ");
	LogMsg("\x01\x0D            S \x01\x0C ");
	LogMsg("\x01\x0D       S    S \x01\x0C ");
	LogMsg("\x01\x0D        SSSS  \x01\x0C ");
	LogMsg("\x01\x0F");
	//Last Line Of Code Written In 2021
}
