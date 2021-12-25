#include <debug.h>

void DumpRegisters (Registers* pRegs)
{
	LogMsg("Registers:");
	LogMsg("EAX=%x CS=%x "    "EIP=%x EFLGS=%x", pRegs->eax, pRegs->cs, pRegs->eip, pRegs->eflags);
	LogMsg("EBX=%x             ESP=%x EBP=%x",   pRegs->ebx,            pRegs->esp, pRegs->ebp);
	LogMsg("EBX=%x             ESI=%x", pRegs->ecx,            pRegs->esi);
	LogMsg("EBX=%x             EDI=%x", pRegs->edx,            pRegs->edi);
}

const char* g_pBugCheckReasonText[] = {
	// first 32: x86 exceptions
	"Divide by zero",
	"Unknown debugging exception",
	"Non maskable interrupt",
	"Breakpoint",
	"Overflow",
	"Boundary Range Exceeded",
	"Invalid opcode",
	"Device not available",
	"Double-fault",
	"Not the case",//FPU segment overrun
	"Not the case",//Invalid TSS
	"Segment not present",
	"Stack segment fault",
	"General protection fault",
	"Page fault",
	"Reserved",
	"x87 FPU exception",
	"Alignment check",
	"Machine check",
	"Not the case",//SIMD FPU exception
	"Not the case",//Virtualization exception
	"Not the case",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Not the case",
	"Not the case",
	"Not the case",
	"Reserved",
	
	// miscellaneous failures you may encounter
};

/* Assume, as is often the case, that EBP is the first thing pushed. If not, we are in trouble. */
typedef struct StackFrame {
	struct StackFrame* ebp;
	uint32_t eip;
} StackFrame;

#define MAX_FRAMES 10
void KeBugCheck (BugCheckReason reason, Registers* pRegs)
{
	LogMsg("*** STOP: %x", reason);
	LogMsg("Type string: %s", g_pBugCheckReasonText[reason]);
	
	if (!pRegs)
		LogMsg("No register information was provided.");
	else
		DumpRegisters(pRegs);
	
	// navigate the stack:
	StackFrame* stk = (StackFrame*)(pRegs->ebp);
	//__asm__ volatile ("movl %%ebp, %0"::"r"(stk));
	LogMsg("Stack trace:");
	LogMsg("-> 0x%x", pRegs->eip);
	for (unsigned int frame = 0; stk && frame < MAX_FRAMES; frame++)
	{
		LogMsgNoCr(" * 0x%x ", stk->eip);
		// TODO: addr2line implementation?
		LogMsg("");
		stk = stk->ebp;
	}
	
	
	cli;
	while (1) hlt;
}
