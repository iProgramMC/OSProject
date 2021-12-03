#ifndef _MAIN_H
#define _MAIN_H

#include<stddef.h>
#include<stdarg.h>
#include<stdint.h>

typedef char bool;
typedef char byte;
typedef char BYTE;
typedef unsigned uint;

#define false 0
#define true 1

#define hlt __asm__("hlt\n\t")  
#define cli __asm__("cli\n\t")
#define sti __asm__("sti\n\t")

#define Version 10
#define VersionString "V0.10"

#define crash __asm__("int $0x10\n\t") // Int 0x10 doesn't work in pmode! Might as well make use of it.

extern void WritePort(unsigned short port, unsigned char data);
extern unsigned char ReadPort(unsigned short port);
extern void WritePortW(unsigned short port, unsigned short data);
extern unsigned short ReadPortW(unsigned short port);

void LogMsg (const char* str, ...);

void OnPanicTriggered (const char* pFile, int nLine, const char* pReason, bool isAssert);
#define PANIC OnPanicTriggered(__FILE__, __LINE__, "No reason specified.", false)
#define PANICR(message) OnPanicTriggered(__FILE__, __LINE__, message, false)
#define ASSERT(cond) do { if (!(cond)) OnPanicTriggered(__FILE__, __LINE__, "ASSERT: " #cond, true); } while (0)

#endif//_MAIN_H