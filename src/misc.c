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

// basic garbage rand():
int GetRandom()
{
	//read the tsc:
	int hi, lo;
	GetTimeStampCounter(&hi, &lo);
	//combine the high and low numbers:
	
	hi ^= lo;
	
	//then mask it out so it wont look obvious:
	hi ^= 0xe671c4b4;
	
	//then make it positive:
	hi &= 2147483647;
	
	//lastly, return.
	return hi;
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
	int npp = GetNumPhysPages(), nfpp = GetNumFreePhysPages();
	LogMsg("(note, the logo you see is temporary)");
	//below are the first lines modified in 2022 :)\x01\x10  /\x01\x10     \x01\x08 \x01\x0C
	/*
	LogMsgNoCr("  \x01\x10/                   ");                                                   LogMsg("\x01\x0C OS:       \x01\x0FNanoShell Operating System");
	LogMsgNoCr(" \x01\x10/                \x01\x1F#   ");                                           LogMsg("\x01\x0C Kernel:   \x01\x0F%s (%d)", VersionString, VersionNumber);
	LogMsgNoCr("\x01\x1F#     \x01\x10.\x01\x14+\x01\x1C#####\x01\x14:\x01\x10.    \x01\x10\\  ");  LogMsg("\x01\x0C Uptime:   \x01\x0F?");
	LogMsgNoCr("    \x01\x10+\x01\x14s\x01\x1C#########\x01\x14:\x01\x10.     ");                   LogMsg("\x01\x0C CPU:      \x01\x0F%s", GetCPUName());
	LogMsgNoCr("   \x01\x10+\x01\x14s\x01\x1C###########\x01\x14:     \x01\x10|");                  LogMsg("\x01\x0C CPU type: \x01\x0F%s", GetCPUType());
	LogMsgNoCr("  \x01\x1FmMMMMMMm\x01\x14:\x01\x1C######:.   \x01\x10|");                          LogMsg("\x01\x0C Memory:   \x01\x0F%d KB / %d KB", (npp-nfpp)*4, npp*4);
	LogMsgNoCr("  \x01\x1FW\x01\x10###\x01\x1E##\x01\x1FWWWwwwwwwW#   ");                           LogMsg("\x01\x0C ");
	LogMsgNoCr("    \x01\x1F\x01\x10##\x01\x1E####\x01\x10##\x01\x1E#####    \x01\x10/");           LogMsg("\x01\x0C ");
	LogMsgNoCr("\x01\x10\\    \x01\x1E\\####\x01\x10#\x01\x1E####/    \x01\x10/ ");                 LogMsg("\x01\x0C ");
	LogMsgNoCr(" \x01\x10\\                 \x01\x10/");                                            LogMsg("\x01\x0C ");
	LogMsgNoCr("  \x01\x1F#       #       #");                                                      LogMsg("\x01\x0C ");
	LogMsgNoCr("         /\x01\x0F");                                                               LogMsg("\x01\x0C ");
	LogMsg("\x01\x0F");
	
	LogMsgNoCr("  \x01\x10/                    ");                                                   							LogMsg("\x01\x0C OS:       \x01\x0FNanoShell Operating System");
	LogMsgNoCr(" \x01\x10/                \x01\x1F#    ");                                           							LogMsg("\x01\x0C Kernel:   \x01\x0F%s (%d)", VersionString, VersionNumber);
	LogMsgNoCr("\x01\x1F#     \x01\x10.\x01\x14+\x01\x1C\x02\x02\x02\x02\x02\x01\x14:\x01\x10.    \x01\x10\\   ");  			LogMsg("\x01\x0C Uptime:   \x01\x0F?");
	LogMsgNoCr("    \x01\x10+\x01\x14s\x01\x1C\x02\x02\x02\x02\x02\x02\x02\x02\x02\x01\x14:\x01\x10.      ");					LogMsg("\x01\x0C CPU:      \x01\x0F%s", GetCPUName());
	LogMsgNoCr("   \x01\x10+\x01\x14s\x01\x1C\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x01\x14:     \x01\x10|");				LogMsg("\x01\x0C CPU type: \x01\x0F%s", GetCPUType());
	LogMsgNoCr("  \x01\x1FmMMMMMMm\x01\x14:\x01\x1C\x02\x02\x02\x02\x02\x02:.   \x01\x10|");									LogMsg("\x01\x0C Memory:   \x01\x0F%d KB / %d KB", (npp-nfpp)*4, npp*4);
	LogMsgNoCr("  \x01\x1FW\x01\x10\x02\x02\x02\x01\x1E\x02\x02\x01\x1FWWWwwwwwwW#   ");										LogMsg("\x01\x0C ");
	LogMsgNoCr("    \x01\x1F\x01\x10\x02\x02\x01\x1E\x02\x02\x02\x02\x01\x10\x02\x02\x01\x1E\x02\x02\x02\x02\x02    \x01\x10/");LogMsg("\x01\x0C ");
	LogMsgNoCr("\x01\x10\\    \x01\x1E\\\x02\x02\x02\x02\x01\x10\x02\x01\x1E\x02\x02\x02\x02/    \x01\x10/ ");					LogMsg("\x01\x0C ");
	LogMsgNoCr(" \x01\x10\\                 \x01\x10/");																		LogMsg("\x01\x0C ");
	LogMsgNoCr("  \x01\x1F#       #       #");																					LogMsg("\x01\x0C ");
	LogMsgNoCr("         \x01\x10/\x01\x0F");																					LogMsg("\x01\x0C ");
	LogMsg("\x01\x0F");*/
	
	LogMsg("\x01\x0E N    N       "      "\x01\x0C OS:       \x01\x0FNanoShell Operating System");
	LogMsg("\x01\x0E NN   N       "      "\x01\x0C Kernel:   \x01\x0F%s (%d)", VersionString, VersionNumber);
	LogMsg("\x01\x0E N N  N       "      "\x01\x0C Uptime:   \x01\x0F?");
	LogMsg("\x01\x0E N  N N       "      "\x01\x0C CPU:      \x01\x0F%s", GetCPUName());
	LogMsg("\x01\x0E N   NN       "      "\x01\x0C CPU type: \x01\x0F%s", GetCPUType());
	LogMsg("\x01\x0E N    N\x01\x0D SSSS  \x01\x0C Memory:   \x01\x0F%d KB / %d KB", (npp-nfpp)*4, npp*4);
	LogMsg("\x01\x0D       S    S "      "\x01\x0C ");
	LogMsg("\x01\x0D       S      "      "\x01\x0C ");
	LogMsg("\x01\x0D        SSSS  "      "\x01\x0C ");
	LogMsg("\x01\x0D            S "      "\x01\x0C ");
	LogMsg("\x01\x0D       S    S "      "\x01\x0C ");
	LogMsg("\x01\x0D        SSSS  "      "\x01\x0C ");
	LogMsg("\x01\x0F");
	/**/
	//Last Line Of Code Written In 2021
}
