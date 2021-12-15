#include <stdint.h>

enum {
	LOGMSG = 1,
};

void LogMsg(const char* pText)
{
	DoSyscall(LOGMSG, pText, 0, 0, 0);
}
