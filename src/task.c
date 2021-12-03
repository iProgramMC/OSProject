#include <main.h>
#include <memory.h>
#include <string.h>
#include <task.h>

// The kernel task is task 0. Other tasks are 1-indexed. 
// This means g_runningTasks[0] is unused.

Task g_runningTasks[C_MAX_TASKS];
static int s_currentRunningTask = -1;//not visible to any other file
CPUSaveState g_kernelSaveState;

extern void TaskInitialFunc();

void ConstructTask(Task* pTask)
{
	pTask->m_cpuState.esp = ((int)pTask->m_allocatedStack + C_STACK_BYTES_PER_TASK) & ~0xF;//align to 4 bits
	pTask->m_cpuState.ebp = ((int)pTask->m_allocatedStack + C_STACK_BYTES_PER_TASK) & ~0xF;//align to 4 bits
	pTask->m_cpuState.eip =  (int)TaskInitialFunc;
	pTask->m_cpuState.eax =  (int)pTask;
	pTask->m_cpuState.ebx =  0x01234567;
	pTask->m_cpuState.ecx =  0x89abcdef;
	pTask->m_cpuState.edx =  0xEEEEEEEE;
	pTask->m_cpuState.esi =  0xAAAAAAAA;
	pTask->m_cpuState.edi =  0xBBBBBBBB;
	pTask->m_cpuState.cs  = 8;//same as our CS
	pTask->m_cpuState.eflags = 0x297;//same as our own eflags
	
	//pTask->m_cpuState.esp -= sizeof (CPUSaveState);
	pTask->m_cpuState.esp -= sizeof (int) * 5;
	
	//memcpy ((void*)(pTask->m_cpuState.esp), &pTask->m_cpuState, sizeof(CPUSaveState));
	memcpy ((void*)(pTask->m_cpuState.esp), &pTask->m_cpuState.eip, sizeof(int) * 3);
	
	//Should point to EIP
	//pTask->m_cpuState.esp += sizeof(int) * offsetof(CPUSaveState, eip);
}
Task* StartTaskD(TaskedFunction function, VariantList* pVList, int* errorCodeOut,
				   const char* callerFile, const char* callerFunc, int callerLine)
{
	// Find a free slot, if you can't find any, write TASK_ERROR_TOO_MANY_TASKS.
	int i = 1;
	for (; i < C_MAX_TASKS; i++)
	{
		if (!g_runningTasks[i].m_exists) break;
	}
	if (i == C_MAX_TASKS)
	{
		*errorCodeOut = TASK_ERROR_TOO_MANY_TASKS;
		return NULL;
	}
	
	void* pStack = malloc (C_STACK_BYTES_PER_TASK);
	if (pStack)
	{
		//setup task here
		Task* pTask = &g_runningTasks[i];
		
		pTask->m_exists = true;
		pTask->m_function = function;
		pTask->m_allocatedStack = pStack;
		if (pVList)
		{
			pTask->m_featuresArgs = true;
			memcpy (&pTask->m_arguments, pVList, sizeof(VariantList));
		}
		else
		{
			pTask->m_featuresArgs = false;
		}
		pTask->m_firstTime = true;
		pTask->m_callerFile = callerFile;
		pTask->m_callerLine = callerLine;
		pTask->m_callerFunc = callerFunc;
		
		ConstructTask(pTask);
		
		*errorCodeOut = TASK_SUCCESS;
		return pTask;
	}
	else
	{
		*errorCodeOut = TASK_ERROR_STACK_ALLOC_FAILED;
		return NULL;
	}
}

static void ResetTask(Task* pTask, bool killing)
{
	if (killing && pTask->m_allocatedStack)
	{
		free(pTask->m_allocatedStack);
	}
	pTask->m_allocatedStack = NULL;
	
	pTask->m_firstTime = false;
	pTask->m_exists    = false;
	pTask->m_function  = NULL;
	pTask->m_callerFile = NULL;
	pTask->m_callerFunc = NULL;
	pTask->m_callerLine = 0;
	pTask->m_featuresArgs = false;
}

bool KillTask(Task* pTask)
{
	if (pTask == NULL) return false;
	if (!pTask->m_exists) return false;
	
	ResetTask(pTask, true);
	
	return true;
}
Task* GetRunningTask()
{
	if (s_currentRunningTask == -1) return NULL;
	return &g_runningTasks[s_currentRunningTask];
}
void InitializeTaskSystem()
{
	for (int i = 0; i < C_MAX_TASKS; i++)
	{
		ResetTask(&g_runningTasks[i], false);
	}
}

void DumpSaveState(CPUSaveState* pSaveState)
{
	LogMsg("Start register dump.");
	LogMsg("EAX=0x%x  EBX=0x%x  ECX=0x%x  EDX   =0x%x", pSaveState->eax, pSaveState->ebx, pSaveState->ecx, pSaveState->edx);
	LogMsg("ESI=0x%x  EDI=0x%x  EBP=0x%x", pSaveState->esi, pSaveState->edi, pSaveState->ebp);
	LogMsg("EIP=0x%x  ESP=0x%x   CS=0x%x  EFLAGS=0x%x", pSaveState->eip, pSaveState->esp, pSaveState->cs, pSaveState->eflags);
	LogMsg("End register dump.");
}

extern void OnStartedNewTask();
extern void OnStartedNewKernelTask();
CPUSaveState* g_saveStateToRestore = NULL;
void RestoreKernelTask()
{
	g_saveStateToRestore = &g_kernelSaveState;
	OnStartedNewKernelTask();
}
void RestoreStandardTask(Task* pTask)
{
	g_saveStateToRestore = &pTask->m_cpuState;
	//DumpSaveState(g_saveStateToRestore);
	OnStartedNewTask();
}

void TaskInitialFuncC(Task* pTask)
{
	//call the main function of the thread
	pTask->m_function (&pTask->m_arguments);
	
	
	//now kill the task
	KillTask(pTask);
	//and stall
	while (1) hlt;
}

void SwitchToNextTask (CPUSaveState* pSaveState)
{
	Task *pTask = GetRunningTask();
	if (pTask)
	{
		memcpy (&pTask->m_cpuState, pSaveState, sizeof(g_kernelSaveState));
	}
	else
	{
		//Switching from the kernel task
		memcpy (&g_kernelSaveState, pSaveState, sizeof(g_kernelSaveState));
	}
		
	// End Of Interrupt.
	WritePort (0x20, 0x20);
	WritePort (0xA0, 0x20);
	
	//read register C - use if RTC is used to switch tasks  rather than PIC
	//WritePort (0x70, 0x0C);
	//ReadPort  (0x71);
	
	// Get the next task, or use the kernel task if we don't have any more tasks running.
	int i = s_currentRunningTask + 1;
	for (; i < C_MAX_TASKS; i++)
	{
		if (g_runningTasks[i].m_exists) break;
	}
	
	Task* pNewTask = NULL;
	if (i < C_MAX_TASKS) pNewTask = &g_runningTasks[i];
	
	if (pNewTask) 
	{
		s_currentRunningTask = i;
		RestoreStandardTask(pNewTask);
	}
	else
	{
		//kernel task
		s_currentRunningTask = -1;
		RestoreKernelTask();
	}
}
