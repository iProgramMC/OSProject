#include <debug.h>

void DumpRegisters (Registers* pRegs)
{
	LogMsg("Registers:\n");
	LogMsg(  "EAX="); LogInt(pRegs->eax); LogMsg("CS="); LogInt(pRegs->cs); LogMsg("EIP="); LogInt(pRegs->eip); LogMsg("EFLGS="); LogInt(pRegs->eflags);
	LogMsg("\nEBX="); LogInt(pRegs->ebx); LogMsg("              ");         LogMsg("ESP="); LogInt(pRegs->esp); LogMsg("EBP="); LogInt(pRegs->ebp);
	LogMsg("\nEBX="); LogInt(pRegs->ecx); LogMsg("              ");         LogMsg("ESI="); LogInt(pRegs->esi);
	LogMsg("\nEBX="); LogInt(pRegs->edx); LogMsg("              ");         LogMsg("EDI="); LogInt(pRegs->edi);
	LogMsg("\n");
}
