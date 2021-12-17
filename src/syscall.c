#include <syscall.h>
#include <debug.h>
#include <userspace/syscalls.h>

void OnSyscallReceived (Registers* pRegs)
{
	LogMsg("Got syscall!");
	DumpRegisters(pRegs);
	LogMsg("Syscall ID: %d", pRegs->esi);
	switch (pRegs->esi)
	{
		case LOGMSG:
			LogMsg("LOGMSG syscall!");
			LogMsg((char*)pRegs->eax);
			break;
		default:
			LogMsg("UND syscall!");
			LogMsg("warning: undefined syscall");
			break;
	}
}