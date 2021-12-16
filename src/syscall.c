#include <syscall.h>
#include <debug.h>
#include <userspace/syscalls.h>

void OnSyscallReceived (Registers* pRegs)
{
	LogMsg("Got syscall!\n");
	DumpRegisters(pRegs);
	LogMsg("Syscall ID: "); LogInt(pRegs->esi); LogMsg("\n");
	switch (pRegs->esi)
	{
		case LOGMSG:
			LogMsg("LOGMSG syscall!\n");
			LogMsg((char*)pRegs->eax);
			break;
		default:
			LogMsg("UND syscall!\n");
			LogMsg("warning: undefined syscall\n");
			break;
	}
}