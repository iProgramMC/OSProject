/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

          Task Scheduler  module
******************************************/
#include <task.h>
#include <memory.h>
#include <string.h>

// The kernel task is task 0.  Other tasks are 1-indexed.
// This means g_runningTasks[0] is unused.

Task g_runningTasks[C_MAX_TASKS];
static int s_currentRunningTask = -1;
static CPUSaveState g_kernelSaveState;
void KeTaskDebugDump()
{
	cli;
	bool any_tasks = false;
	LogMsg("Listing tasks.");
	for (int i = 0; i < C_MAX_TASKS; i++)
	{
		Task* t = g_runningTasks + i;
		if (!t->m_bExists) continue;
		any_tasks = true;
		LogMsg("- %d  F:%x  AL:%d AF:%s AS:%s", i, t->m_pFunction, t->m_authorLine, t->m_authorFunc, t->m_authorFile);
	}
	if (!any_tasks)
		LogMsg("No tasks currently running.");
	sti;
}

// This function (in asm/task.asm) prepares the initial task for
// execution.
extern void KeTaskStartup();

void KeConstructTask (Task* pTask)
{
	pTask->m_state.esp = ((int)pTask->m_pStack + C_STACK_BYTES_PER_TASK) & ~0xF; //Align to 4 bits
	pTask->m_state.ebp = 0;
	pTask->m_state.eip = (int)KeTaskStartup;
	pTask->m_state.eax = (int)pTask;
	// fill in the other registers with garbage, so we know that it's executed ok
	pTask->m_state.ebx = 0x01234567;
	pTask->m_state.ecx = 0x7A7053E1;
	pTask->m_state.edx = 0xCE999999;
	pTask->m_state.esi = 0xAAAAAAAA;
	pTask->m_state.edi = 0xBBBBBBBB;
	pTask->m_state.cs  = 0x8;//same as our CS
	pTask->m_state.eflags = 0x297; //same as our own EFL register
	
	// push the iretd worthy registers on the stack:
	pTask->m_state.esp -= sizeof(int) * 5;
	memcpy ((void*)(pTask->m_state.esp), &pTask->m_state.eip, sizeof(int)*3);
}

Task* KeStartTaskD(TaskedFunction function, int argument, int* pErrorCodeOut, const char* authorFile, const char* authorFunc, int authorLine)
{
	cli; //must do this, because otherwise we can expect an interrupt to come in and load our unfinished structure
	
	int i = 1;
	for (; i < C_MAX_TASKS; i++)
	{
		if (!g_runningTasks[i].m_bExists) break;
	}
	
	if (i == C_MAX_TASKS)
	{
		*pErrorCodeOut = TASK_ERROR_TOO_MANY_TASKS;
		sti;
		return NULL;
	}
	
	void *pStack = MmAllocate(C_STACK_BYTES_PER_TASK);
	if (pStack)
	{
		//Setup our new task here:
		Task* pTask = &g_runningTasks[i];
		pTask->m_bExists = true;
		pTask->m_pFunction = function;
		pTask->m_pStack = pStack;
		pTask->m_bFirstTime = true;
		pTask->m_authorFile = authorFile;
		pTask->m_authorFunc = authorFunc;
		pTask->m_authorLine = authorLine;
		pTask->m_argument   = argument;
		pTask->m_bMarkedForDeletion = false;
		
		KeConstructTask(pTask);
		
		if (pErrorCodeOut)
			*pErrorCodeOut = TASK_SUCCESS;
		sti;
		return pTask;
	}
	else
	{
		*pErrorCodeOut = TASK_ERROR_STACK_ALLOC_FAILED;
		sti;
		return NULL;
	}
}
static void KeResetTask(Task* pTask, bool killing)
{
	if (pTask == KeGetRunningTask())
	{
		SLogMsg("Marked current task for execution (KeResetTask)");
		pTask->m_bMarkedForDeletion = true;
		while (1) hlt;
	}
	else
	{
		SLogMsg("Deleting task %x for execution (KeResetTask, killing:%d)", pTask, killing);
		if (killing && pTask->m_pStack)
		{
			MmFree(pTask->m_pStack);
		}
		pTask->m_pStack = NULL;
		
		pTask->m_bFirstTime = false;
		pTask->m_bExists    = false;
		pTask->m_pFunction  = NULL;
		pTask->m_authorFile = NULL;
		pTask->m_authorFunc = NULL;
		pTask->m_authorLine = 0;
		pTask->m_argument   = 0;
		pTask->m_featuresArgs = false;
		pTask->m_bMarkedForDeletion = false;
	}
}
bool KeKillTask(Task* pTask)
{
	if (pTask == KeGetRunningTask())
		KeExit();
	
	cli;
	if (pTask == NULL)
	{
		sti; return false;
	}
	if (!pTask->m_bExists)
	{
		sti; return false;
	}
	KeResetTask(pTask, true);
	sti;
	return true;
}
Task* KeGetRunningTask()
{
	if (s_currentRunningTask == -1) return NULL;
	return &g_runningTasks[s_currentRunningTask];
} 
void KiTaskSystemInitialize()
{
	for (int i = 0; i < C_MAX_TASKS; i++)
		KeResetTask(g_runningTasks + i, false);
}

CPUSaveState* g_saveStateToRestore1 = NULL;
extern void KeStartedNewKernelTask();
extern void KeStartedNewTask();
void KeRestoreKernelTask()
{
	g_saveStateToRestore1 = &g_kernelSaveState;
	KeStartedNewKernelTask();
}
void KeRestoreStandardTask(Task* pTask)
{
	g_saveStateToRestore1 = &pTask->m_state;
	KeStartedNewTask();
}
void KeTaskStartupC(Task* pTask)
{
	//call the main function of the thread
	pTask->m_pFunction (pTask->m_argument);
	
	//after we're done, kill the task
	KeExit();
}
__attribute__((noreturn))
void KeExit()
{
	if (!KeGetRunningTask())
	{
		KeStopSystem();
	}
	
	SLogMsg("Marked current task for execution (KeExit)");
	KeGetRunningTask()->m_bMarkedForDeletion = true;
	while (1) hlt;
}
void KeSwitchTask(CPUSaveState* pSaveState)
{
	Task* pTask = KeGetRunningTask();
	if (pTask)
	{
		memcpy (& pTask -> m_state, pSaveState, sizeof(CPUSaveState));
	}
	else
	{
		memcpy (&g_kernelSaveState, pSaveState, sizeof(CPUSaveState));
	}
	
	// Acknowledge the interrupt:
	WritePort (0x20, 0x20);
	WritePort (0xA0, 0x20);
	
	// If using RTC, also flush register C
	//WritePort (0x70, 0x0C);
	//ReadPort  (0x71);
	
	int i = s_currentRunningTask + 1;
	for (; i < C_MAX_TASKS; i++)
	{
		if (g_runningTasks[i].m_bExists)
			break;
	}
	
	Task* pNewTask = NULL;
	if (i < C_MAX_TASKS) pNewTask = g_runningTasks + i;
	
	if (pNewTask)
	{
		s_currentRunningTask = i;
		
		//if old task was marked for deletion, remove it:
		if (pTask)
			if (pTask->m_bMarkedForDeletion) KeResetTask(pTask, true);
		
		KeRestoreStandardTask(pNewTask);
	}
	else
	{
		//Kernel task
		s_currentRunningTask = -1;
		
		//if old task was marked for deletion, remove it:
		if (pTask)
			if (pTask->m_bMarkedForDeletion) KeResetTask(pTask, true);
		
		KeRestoreKernelTask();
	}
}

