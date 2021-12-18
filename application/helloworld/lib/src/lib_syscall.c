#include "lib_header.h"

void PutString(const char* pText)
{
	DoSyscall(LOGMSG, (int)pText, 0, 0, 0);
}