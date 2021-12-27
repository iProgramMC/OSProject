/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

     Misc - timing module header file
******************************************/
#ifndef _MISC_H
#define _MISC_H

#define RTC_TICKS_PER_SECOND 2048

/**
 * Gets the TSC from the CPU.  It returns the number of cycles the CPU went
 * through after the last reset, in "high" and "low".
 * The "high" or "low" pointers can be NULL, but if you pass in both as NULL, this does not use rdtsc.
 */
void GetTimeStampCounter(int* high, int* low);

/**
 * Gets the number of milliseconds since boot.
 */
int GetTickCount();

/**
 * Gets the number of times the RTC interrupt handler was called.
 */
int GetRawTickCount();

#endif//_MISC_H