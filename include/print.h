#ifndef _PRINT_H
#define _PRINT_H

#include <main.h>

extern uint16_t TermWidth,TermHeight;
extern int TrmColumn, TrmRow;

extern void InitTerminal();
extern void MoveCursor();
void vsprintf(char*a,const char*c, va_list arg);
void sprintf(char*a,const char*c, ...);
void TextModePlotChar(int x, int y, uint16_t chr);
uint16_t TextModeMakeChar(uint8_t fgbg, uint8_t chr);
void ClearTerminal();

#endif//_PRINT_H