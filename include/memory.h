#ifndef _MEMORY_H
#define _MEMORY_H

#define BLOCK_COUNT 32768
//#define SAFE_MODE
#define DEFAULT_MEMORY 2*1024*1024 

void* MmKAllocate(size_t size, char*callee, int calleeLine);
void *MmReAllocate(void* pToRealloc, size_t newSize, char* callee, int calleeLine);
void MmFreeMem2(void* ptr, const char*calledBy,int callLine);
void MmLogDebug();
void MmInit();
uint32_t MmGetMemAvailable();
size_t MmGetMemFreeOnHeap();
#define malloc(size) MmKAllocate(size,__FILE__,__LINE__)
#define realloc(ptr, size) MmReAllocate(ptr,size,__FILE__,__LINE__)
#define MmFreeMem(ptr) MmFreeMem2(ptr,__FILE__,__LINE__)
#define free(ptr)      MmFreeMem2(ptr,__FILE__,__LINE__)
extern size_t g_memoryUsable;

#ifdef SAFE_MODE
extern uint8_t g_memory[];
#else
extern uint8_t* g_memory;
#endif

#endif//_MEMORY_H