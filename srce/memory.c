//
// Simple Memory manager.
// Copyright (c) 2020-2021 iProgramInCpp
//

// TODO: Read in the memmap passed in by grub instead?

#include <main.h>
#include <memory.h>
#include <string.h>

size_t g_memoryUsable = 0;

#ifdef SAFE_MODE
uint8_t  g_memory[DEFAULT_MEMORY];
#else
uint8_t* g_memory;
#endif

typedef struct AllocatedMemory
{
    void* start; size_t size;//, actualSize;
    const char* callerFile; int callerLine;
} 
AllocatedMemory;
typedef struct FreeMemory
{
    void* start; size_t size;
} FreeMemory;

AllocatedMemory g_blocks    [BLOCK_COUNT];
FreeMemory      g_freeBlocks[BLOCK_COUNT];

void PrintMemoryDebug()
{
    LogMsg("Allocations:");
    int i = 0;
    for (; i < BLOCK_COUNT; i++)
    {
        if (g_blocks[i].start == (void*)0xFFFFFFFF || g_blocks[i].start == NULL) break;
        LogMsg("[A]: start: 0x%x, size: 0x%x %d, calledby: %s:%d", (int)g_blocks[i].start, g_blocks[i].size, g_blocks[i].size, g_blocks[i].callerFile, g_blocks[i].callerLine);
    }
    if (i == 0) LogMsg("None.");
    i = 0;
    LogMsg("Free slots:");
    for (; i < BLOCK_COUNT; i++)
    {
        if (g_freeBlocks[i].start == (void*)0xFFFFFFFF || g_freeBlocks[i].start == NULL) break;
        LogMsg("[F]: start: 0x%x, size: 0x%x %d", (int)g_freeBlocks[i].start, g_freeBlocks[i].size, g_freeBlocks[i].size);
    }
    if (i == 0) LogMsg("None???");
    LogMsg("\n");
}

#define mm_min(a,b)((a)<(b)?(a):(b))
#define mm_max(a,b)((a)>(b)?(a):(b))

void ConnectNeighboringOrOverlappingFreeMemRegions()
{
    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        if (g_freeBlocks[i].start == (void*)0xFFFFFFFF) break;
        for (int j = 0; j < BLOCK_COUNT; j++)
        {
            if (g_freeBlocks[j].start == (void*)0xFFFFFFFF) break;
            if (i == j) continue;
            void* start1 = g_freeBlocks[i].start;
            void* start2 = g_freeBlocks[j].start;
            void* end1 = (uint8_t*)start1 + g_freeBlocks[i].size;
            void* end2 = (uint8_t*)start2 + g_freeBlocks[j].size;

            //do these regions overlap in anyway?
            if ((uintptr_t)start1 <= (uintptr_t)end2 && (uintptr_t)start2 <= (uintptr_t)end1)
            {
                uint8_t* minStart, * maxEnd;
                minStart = (uint8_t*)mm_min((int)start1, (int)start2);
                maxEnd   = (uint8_t*)mm_max((int)end1,   (int)end2);
                g_freeBlocks[i].start = minStart;
                g_freeBlocks[i].size  = maxEnd - minStart;
                g_freeBlocks[j].start = NULL;
                g_freeBlocks[j].size = 0;
            }
        }
    }
}


void AddFreeMemoryBlock(void* start, size_t size)
{
    //find free slot
    int freeSlot = -1;
    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        if (g_freeBlocks[i].start == 0 || g_freeBlocks[i].start == (void*)0xFFFFFFFF)
        {
            freeSlot = i; break;
        }
    }
    //set the free memory block
    if (freeSlot == -1)
    {
        LogMsg("[!!! FATAL ERROR !!!] %s:%d AddFreeMemoryBlock failed. Start: 0x%x, size: 0x%x %d", __FILE__, __LINE__, (int)start, size, size);
        return;
    }
    
    g_freeBlocks[freeSlot].start = start;
    g_freeBlocks[freeSlot].size  = size;

    ConnectNeighboringOrOverlappingFreeMemRegions();
}

int CeilTo4K(size_t size) {
	return (size & ~0xFFF) + 0x1000;
}

void* AllocateMemory1(size_t size, const char* calledBy, int callLine)
{
    int freeSlot = -1;
    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        if (g_blocks[i].start == NULL || g_blocks[i].start == (void*)0xFFFFFFFF)
        {
            freeSlot = i; break;
        }
    }
    //set the free memory block
    if (freeSlot == -1)
    {
        LogMsg("[!!! FATAL ERROR !!!] %s:%d AllocateMemory failed (too many blocks allocated). Size: 0x%x %d, called by %s:%d", __FILE__, __LINE__, size, size, calledBy, callLine);
        return NULL;
    }

    //look through all the free blocks
    int freeBlock = -1;
    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        if (g_freeBlocks[i].start != NULL && g_freeBlocks[i].start != (void*)0xFFFFFFFF)
        {
            //check if we can fit here
            if (g_freeBlocks[i].size >= size)
            {
                freeBlock = i;
                break;
            }
        }
    }
    if (freeBlock == -1)
    {
        LogMsg("[!!! FATAL ERROR !!!] %s:%d AllocateMemory failed (not enough free contiguous space). Size: 0x%x %d, called by %s:%d", __FILE__, __LINE__, size, size, calledBy, callLine);
        return NULL;
    }

    //do math
    FreeMemory* p = &g_freeBlocks[freeBlock];//p because I don't like typing that long thing again
    void* newMemory = p->start;
    p->start = (void*)((uint8_t*)p->start + size);
    p->size -= size;
    if (p->size == 0) p->start = NULL;

    AllocatedMemory* pMem = &g_blocks[freeSlot];
    pMem->start = newMemory;
    pMem->size  = size;
    pMem->callerFile = calledBy;
    pMem->callerLine = callLine;

    return newMemory;
}
void* AllocateMemory(size_t size, const char* calledBy, int callLine)
{
	return AllocateMemory1 (CeilTo4K(size), calledBy, callLine);
}
//#define kalloc(size) AllocateMemory(size, __FILE__, __LINE__)

void MmFreeMem2(void* mem, const char* pCalledBy, int calledLine)
{
    //find it in the allocations list
    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        if (g_blocks[i].start == (void*)0xFFFFFFFF) break;
        if (g_blocks[i].start == mem)
        {
            //add to free list
            AddFreeMemoryBlock(g_blocks[i].start, g_blocks[i].size);
            //memset to zero, for security
            memset(g_blocks[i].start, 0, g_blocks[i].size);
            //reset the block data
            g_blocks[i].start = NULL;
            g_blocks[i].size  = 0;
            g_blocks[i].callerFile = NULL;
            g_blocks[i].callerLine = 0;
            return;
        }
    }
    LogMsg("Attempted to free something not in the heap, address: 0x%x. Called by %s:%d", (int)mem, pCalledBy, calledLine);
}

void MmInit()
{
    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        //set to uninitialized.
        //this is different from just 0x00'ing, because it means we can end right here and there's no more blocks to count.
        g_blocks[i].start     = (void*)0xFFFFFFFF;
        g_blocks[i].size      =        0x00000000;
        g_freeBlocks[i].start = (void*)0xFFFFFFFF;
        g_freeBlocks[i].size  =        0x00000000;
    }
    AddFreeMemoryBlock(g_memory, g_memoryUsable);
}

size_t MmGetMemFreeOnHeap() {
	
	size_t mem = 0;
	for (int i = 0; i < BLOCK_COUNT; i++)
	{
		if (g_freeBlocks[i].size != 0) mem += g_freeBlocks[i].size;
	}
	
	return mem;
}

uint32_t MmGetMemAvailable() {
	return g_memoryUsable;
}

void* MmKAllocate(size_t size, char*callee, int calleeLine) {
	return AllocateMemory(size,callee,calleeLine);
}

// TODO: realloc

void MmLogDebug() {
	PrintMemoryDebug();
}

void *MmReAllocate(void* pToRealloc, size_t newSize, char* callee, int calleeLine) {
	
    size_t minSize = newSize;

    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        if (g_blocks[i].start == pToRealloc)
        {
            minSize = g_blocks[i].size < minSize ? g_blocks[i].size : minSize;
            break;
        }
    }

    void* pNew = AllocateMemory(newSize, callee, calleeLine);
    if (!pNew) return NULL;
    memcpy(pNew, pToRealloc, minSize);
    MmFreeMem2(pToRealloc, callee, calleeLine);
    return pNew;
}