#ifndef _LIB_HEADER_H
#define _LIB_HEADER_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef uint8_t BYTE;
typedef uint8_t bool;
#define false 0
#define true 1

//returns the value in eax
int DoSyscall(int type_esi, int eax, int ebx, int ecx, int edx);

// syscalls:
enum {
	LOGMSG = 1,
	MALLOC = 2,
	FREE   = 3,
	DUMPMEM= 4,
};

void PutString(const char* text);

int memcmp(const void* ap, const void* bp, size_t size);
void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size);
void fmemcpy32 (void* restrict dest, const void* restrict src, size_t size);
void* memmove(void* restrict dstptr, const void* restrict srcptr, size_t size);
void* memset(void* bufptr, BYTE val, size_t size);
void* fast_memset(void* bufptr, BYTE val, size_t size);
void ZeroMemory (void* bufptr1, size_t size);
size_t strgetlento(const char* str, char chr);
int atoi(const char* str);
size_t strlen(const char* str);
void* strcpy(const char* ds, const char* ss);
void strtolower(char* as);
void strtoupper(char* as);
void memtolower(char* as, int w);
void memtoupper(char* as, int w);
int strcmp(const char* as, const char* bs);
void strcat(char* dest, char* after);


#endif