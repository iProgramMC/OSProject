/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

              Console module
******************************************/
#include <stdint.h>
#include <main.h>
#include <string.h>
#include <print.h>
#include <console.h>
#include <video.h>

#undef cli
#define cli __asm__("cli\n\t")

//note: important system calls are preceded by cli and succeeded by sti

uint16_t TextModeMakeChar(uint8_t fgbg, uint8_t chr) {
	uint8_t comb = fgbg;
	uint16_t comb2 = comb << 8 | chr;
	return comb2;
}
uint32_t g_vgaColorsToRGB[] = {
	0xFF000000,
	0xFF0000AA,
	0xFF00AA00,
	0xFF00AAAA,
	0xFFAA0000,
	0xFFAA00AA,
	0xFFAAAA00,
	0xFFAAAAAA,
	0xFF555555,
	0xFF5555FF,
	0xFF55FF55,
	0xFF55FFFF,
	0xFFFF5555,
	0xFFFF55FF,
	0xFFFFFF55,
	0xFFFFFFFF,
};

extern bool g_uses8by16Font;
Console g_debugConsole; // for LogMsg
Console g_debugSerialConsole; // for SLogMsg

Console* g_currentConsole = &g_debugConsole;

uint16_t* g_pBufferBase = (uint16_t*)(KERNEL_MEM_START + 0xB8000);
int g_textWidth = 80, g_textHeight = 25;

void SetConsole(Console* pConsole) {
	g_currentConsole = pConsole;
}
void ResetConsole() {
	g_currentConsole = &g_debugConsole;
}
void CoClearScreen(Console *this) {
	if (this->type == CONSOLE_TYPE_TEXT) {
		uint16_t lolo = TextModeMakeChar(this->color, ' ');
		for (int i = 0; i < this->width*this->height; i++) this->textBuffer[i] = lolo;
	}
	else if (this->type == CONSOLE_TYPE_FRAMEBUFFER)
	{
		VidFillScreen (g_vgaColorsToRGB[this->color >> 4]);
	}
}
void CoInitAsText(Console *this) {
	this->curX = this->curY = 0;
	this->width  = g_textWidth;
	this->height = g_textHeight;
	this->type = CONSOLE_TYPE_TEXT;
	this->textBuffer = g_pBufferBase;
	this->pushOrWrap = 0;//push
	uint16_t lolo = TextModeMakeChar(this->color, ' ');
	for (int i = 0; i < this->width*this->height; i++) this->textBuffer[i] = lolo;
}
void CoInitAsE9Hack(Console *this) {
	this->curX = this->curY = 0;
	this->width  = 0;
	this->height = 0;
	this->type = CONSOLE_TYPE_E9HACK;
	this->textBuffer = NULL;
	this->pushOrWrap = 0;//push
	uint16_t lolo = TextModeMakeChar(this->color, ' ');
	for (int i = 0; i < this->width*this->height; i++) this->textBuffer[i] = lolo;
}
void CoInitAsGraphics(Console *this) {
	this->curX = this->curY = 0;
	this->width = GetScreenSizeX() / 8;
	this->height = GetScreenSizeY() / (g_uses8by16Font ? 16 : 8);
	this->type = CONSOLE_TYPE_FRAMEBUFFER;
	this->color = DefaultConsoleColor;//default
	this->pushOrWrap = 0;//push
	CoClearScreen (this);
}
void CoMoveCursor(Console* this) {
	if (this->type != CONSOLE_TYPE_TEXT) return; // Not Initialized
	if (this->textBuffer == g_pBufferBase) {
		uint16_t cursorLocation = this->curY * this->width + this->curX;
		WritePort(0x3d4, 14);
		WritePort(0x3d5, cursorLocation >> 8);
		WritePort(0x3d4, 15);
		WritePort(0x3d5, cursorLocation);
	}
}
void CoPlotChar (Console *this, int x, int y, char c) {
	if (this->type == CONSOLE_TYPE_TEXT) {
		uint16_t chara = TextModeMakeChar (this->color, c);
		// TODO: add bounds check
		this->textBuffer [x + y * this->width] = chara;
	}
	else if (this->type == CONSOLE_TYPE_FRAMEBUFFER)
	{
		VidPlotChar (c, x << 3, y << (3 + (g_uses8by16Font)), g_vgaColorsToRGB[this->color & 0xF], g_vgaColorsToRGB[this->color >> 4]);
	}
}
void CoScrollUpByOne(Console *this) {
	if (this->type == CONSOLE_TYPE_TEXT) {
		if (this->pushOrWrap) {
			//CoClearScreen(this);
			this->curX = this->curY = 0;
			return;
		}
		memcpy (this->textBuffer, &this->textBuffer[this->width], this->width * (this->height - 1) * sizeof(short));
		//uint16_t* p = &this->textBuffer[this->width * (this->height - 1) * sizeof(short)];
		for (int i=0; i<this->width; i++)
		{
			CoPlotChar (this, i, this->height - 1, 0);
		}
	}
	else if (this->type != CONSOLE_TYPE_E9HACK)
	{
		if (this->pushOrWrap)
		{
			CoClearScreen(this);
			this->curX = this->curY = 0;
		}
		else
		{
			int htChar = 1 << (3 + g_uses8by16Font);
			VidShiftScreen (htChar);
			VidFillRect (g_vgaColorsToRGB[this->color >> 4], 0, (this->height - 1) * htChar, GetScreenSizeX() - 1, GetScreenSizeY() - 1);
		}
	}
}
bool g_shouldntUpdateCursor = false;
//returns a bool, if it's a true, we need to skip the next character.
bool CoPrintCharInternal (Console* this, char c, char next) {
	if (this->type == CONSOLE_TYPE_E9HACK) 
	{
		WritePort(0xE9, c);
		//return; // Not Initialized
		return false;
	}
	if (this->type == CONSOLE_TYPE_NONE) return false; // Not Initialized
	switch (c) {
		case '\x01':
			//allow foreground color switching.
			//To use this, just type `\x01\x0B`, for example, to switch to bright cyan
			//Typing \x00 will end the parsing, so you can use \x01\x10, or \x01\x30.
			
			if (!next) break;
			char color = next & 0xF;
			this->color = (this->color & 0xF0) | color;
			return true;
		case '\b':
			if (--this->curX < 0) {
				this->curX = this->width - 1;
				if (--this->curY < 0) this->curY = 0;
			}
			CoPlotChar(this, this->curX, this->curY, 0);
			break;
		case '\r': 
			this->curX = 0;
			break;
		case '\n': 
			this->curX = 0;
			this->curY++;
			while (this->curY >= this->height) {
				CoScrollUpByOne(this);
				this->curY--;
			}
			break;
		case '\t': 
			this->curX = (this->curX + 4) & ~3;
			if (this->curX >= this->width) {
				this->curX = 0;
				this->curY++;
			}
			while (this->curY >= this->height) {
				CoScrollUpByOne(this);
				this->curY--;
			}
			break;
		
		default: {
			CoPlotChar(this, this->curX, this->curY, c);
			// advance cursor
			if (++this->curX >= this->width) {
				this->curX = 0;
				this->curY++;
			}
			while (this->curY >= this->height) {
				CoScrollUpByOne(this);
				this->curY--;
			}
			break;
		}
	}
	if (!g_shouldntUpdateCursor) CoMoveCursor(this);
	return false;
}
void CoPrintChar (Console* this, char c)
{
	CoPrintCharInternal(this, c, 0);
}
void CoPrintString (Console* this, const char *c) {
	if (this->type == CONSOLE_TYPE_NONE) return; // Not Initialized
	g_shouldntUpdateCursor = true;
	while (*c) {
		if (CoPrintCharInternal(this, *c, *(c+1))) c++;
		c++;
	}
	g_shouldntUpdateCursor = false;
	CoMoveCursor(this);
}

void LogMsg (const char* fmt, ...) {
	////allocate a buffer well sized
	char cr[8192];
	va_list list;
	va_start(list, fmt);
	vsprintf(cr, fmt, list);
	
	sprintf (cr + strlen(cr), "\n");
	CoPrintString(g_currentConsole, cr);
	
	va_end(list);
}
void LogMsgNoCr (const char* fmt, ...) {
	////allocate a buffer well sized
	char cr[8192];
	va_list list;
	va_start(list, fmt);
	vsprintf(cr, fmt, list);
	CoPrintString(g_currentConsole, cr);
	
	va_end(list);
}
void SLogMsg (const char* fmt, ...) {
	////allocate a buffer well sized
	char cr[8192];
	va_list list;
	va_start(list, fmt);
	vsprintf(cr, fmt, list);
	
	sprintf (cr + strlen(cr), "\n");
	CoPrintString(&g_debugSerialConsole, cr);
	
	va_end(list);
}
void SLogMsgNoCr (const char* fmt, ...) {
	////allocate a buffer well sized
	char cr[8192];
	va_list list;
	va_start(list, fmt);
	vsprintf(cr, fmt, list);
	CoPrintString(&g_debugSerialConsole, cr);
	
	va_end(list);
}
const char* g_uppercaseHex = "0123456789ABCDEF";

void LogHexDumpData (void* pData, int size) {
	uint8_t* pDataAsNums = (uint8_t*)pData, *pDataAsText = (uint8_t*)pData;
	char c[7], d[4];
	c[5] = 0;   d[2] = ' ';
	c[6] = ' '; d[3] = 0;
	c[4] = ':';
	
	#define BYTES_PER_ROW 16
	for (int i = 0; i < size; i += BYTES_PER_ROW) {
		// print the offset
		c[0] = g_uppercaseHex[(i & 0xF000) >> 12];
		c[1] = g_uppercaseHex[(i & 0x0F00) >>  8];
		c[2] = g_uppercaseHex[(i & 0x00F0) >>  4];
		c[3] = g_uppercaseHex[(i & 0x000F) >>  0];
		LogMsgNoCr("%s", c);
		
		for (int j = 0; j < BYTES_PER_ROW; j++) {
			uint8_t p = *pDataAsNums++;
			d[0] = g_uppercaseHex[(p & 0xF0) >> 4];
			d[1] = g_uppercaseHex[(p & 0x0F) >> 0];
			LogMsgNoCr("%s", d);
		}
		LogMsgNoCr("   ");
		for (int j = 0; j < BYTES_PER_ROW; j++) {
			char c = *pDataAsText++;
			if (c < ' ') c = '.';
			LogMsgNoCr("%c",c);
		}
		LogMsg("");
	}
}
