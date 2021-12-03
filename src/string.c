//
// Memory and string processing
// Copyright (C) 2020 iProgramInCpp.
//
#include <string.h>

int memcmp(const void* ap, const void* bp, size_t size)
{
	const BYTE* a = (const BYTE*) ap;
	const BYTE* b = (const BYTE*) bp;
	for(size_t i = 0; i < size; i++)
	{
		if(a[i] < b[i])
			return -1;
		else if(b[i] < a[i])
			return 1;
	}
	return 0;
}
// What's the point of the "restrict"?
void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size)
{
	BYTE* dst = (BYTE*) dstptr;
	const BYTE* src = (const BYTE*) srcptr;
	for(size_t i = 0; i < size; i++)
	{
		dst[i] = src[i];
	}
	return dstptr;
}

// Moves in increments of four
void fmemcpy32 (void* restrict dest, const void* restrict src, size_t size) {
	uint32_t* dst = (uint32_t*)dest;
	uint32_t* ser = (uint32_t*)src;
	
	while (size > 0) {
		//! Should be somewhat fast. Needs to copy 3 145 728 bytes for a 1024x768x32 screen
		*(dst++) = *(ser++); 
	}
}

void* memmove(void* restrict dstptr, const void* restrict srcptr, size_t size)
{
	BYTE* dst = (BYTE*) dstptr;
	const BYTE* src = (const BYTE*) srcptr;
	if (dst < src) {
		for (size_t i = 0; i < size; i++)
			dst[i] = src[i];
	} else {
		for (size_t i = size; i > 0; i--)
			dst[i-1] = src[i-1];
	}
	return dstptr;
}

void* memset(void* bufptr, BYTE val, size_t size)
{
	BYTE* buf = (BYTE*) bufptr;
	for(size_t i = 0; i < size; i++)
	{
		buf[i] = val;
	}
	return bufptr;
}

void* fast_memset(void* bufptr, BYTE val, size_t size)
{
	uint32_t* buf = (uint32_t*) bufptr;
	uint32_t val2 = val<<24|val<<16|val<<8|val;
	for(size_t i = 0; i < size; i += 4)
	{
		buf[i] = val2;
	}
	return bufptr;
}

//NOTE: size must be 4 byte aligned!!
void ZeroMemory (void* bufptr1, size_t size)
{
	size &= ~3;
	uint32_t* bufptr = (uint32_t*)bufptr1;
	
	while (size) {
		*bufptr = 0;
		size -= 4;
		bufptr++;
	}
}

/*
void MemDumpSerial(void* ptr, size_t size)
{
	uint8_t* ptr_ = ptr;
	for(size_t i = 0; i < size; i++)
	{
		putch(HexNumbers[ptr_[i] >> 4]);
		putch(HexNumbers[ptr_[i] & 15]);
		putch(' ');
	}
}
*/
size_t strgetlento(const char* str, char chr) 
{
	size_t len = 0;
	while (str[len] != chr)
	{
		len++;
	}
	return len;
}

int atoi(const char* str) 
{
	int f = 0;
	int s = 1;
	int i = 0;
	if (str[0] == '-')
	{
		i++;
		s = -1;
	}
	for (; str[i] != '\0'; i++)
		f = f * 10 + str[i] - '0';
	
	return s * f;
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
	{
		len++;
	}
	return len;
}
//! NOTE: I changed this to also include the null terminator,
//  my patch might break some stuff, though I doubt it could.
//  28/06/2021
void* strcpy(const char* ds, const char* ss)
{
	return memcpy((void*)ds, (void*)ss, strlen(ss) + 1);//also include nullterm
}
void strtolower(char* as)
{
	while(*as != 0)
	{
		if(*as >= 'A' && *as <= 'Z')
			*as += ('a' - 'A');
		as++;
	}
}
void strtoupper(char* as)
{
	while(*as != 0)
	{
		if(*as >= 'a' && *as <= 'z')
			*as -= ('a' - 'A');
		as++;
	}
}
void memtolower(char* as, int w)
{
	int a = 0;
	while(a <= w)
	{
		if(*as >= 'A' && *as <= 'Z')
			*as += ('a' - 'A');
		as++;
		a++;
	}
}
void memtoupper(char* as, int w)
{
	int a = 0;
	while(a <= w)
	{
		if(*as >= 'a' && *as <= 'z')
			*as -= ('a' - 'A');
		as++;
		a++;
	}
}

int strcmp(const char* as, const char* bs)
{
	size_t al = strlen(as);
	size_t bl = strlen(bs);
	if(al < bl)
	{
		return -1;
	}
	else if(al > bl)
	{
		return 1;
	}
	else if(al == bl)
	{
		return memcmp((void*)as, (void*)bs, al + 1);//Also compare null term
	}
	return 0;
}
/*
int atoi(const char* s)
{
	int val = 0;
	while(*s != 0)
	{
		if(*s >= '0' && *s <= '9')
		{
			val += (*s - '0');
			val *= 10;
		}
		else
		{
			return 0;
		}
		s++;
	}
	return 0;
}
*/

// example: x = x + y
void strcat(char* dest, char* after)
{
	char* end = strlen(dest) + dest;
	memcpy(end, after, strlen(after) + 1);
}