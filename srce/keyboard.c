#include <keyboard.h>
#include <string.h>


#define inb(a) ReadPort(a)
#define outb(a,b) WritePort(a,b)
const unsigned char KeyboardMap[256] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter/Return key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,  0, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};	
const unsigned char PrintableChars[256] =
{
    0,  0, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,  0, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};	

KeyState keyboardState[128];

#define ENTER_KEY_CODE 0x1c

#define SCANCODE_RELEASE 0x80
#define SCANCODE_NOTREL 0x7f

#define PIC1_DATA 0x21
#define PIC2_DATA 0xa1

#define KB_BUF_SIZE 32
char KeyboardBuffer[KB_BUF_SIZE] = {0x0, };
int KeyboardBufferPos = 0;

void IRQClearMask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);        
}
bool typing = false;
char charToType = 0;

bool ShiftPressed()
{
	bool ls = (keyboardState[KEY_LSHIFT] == KEY_PRESSED);
	bool rs = (keyboardState[KEY_RSHIFT] == KEY_PRESSED);
	return ls || rs;
}

bool IsPrintable(unsigned char keycode)
{
	if (PrintableChars[keycode] > 0) 
	{
		return true;
	}
	return false;
}

bool KbIsBufferEmpty()
{
	if (KeyboardBufferPos < 0) KeyboardBufferPos = 0;
	return KeyboardBufferPos == 0;
}

void KbAddKeyToBuffer(char key)
{
	if (key == 0) return;
	if (KeyboardBufferPos > 31) return;
	KeyboardBuffer[KeyboardBufferPos++] = key;
}

int KbNumKeystrokes = 0;
uint8_t KbEncoderReadBuffer()
{
	return ReadPort(KEYBOARD_DATA_PORT);
}

void KbEncoderSendCommand(uint8_t command)
{
	while (1)
		if ((ReadPort(KEYBOARD_STATUS_PORT) & 2) == 0) break;
	
	WritePort(KEYBOARD_STATUS_PORT, command);
}

void KbSetStatusBits(bool num, bool caps, bool scroll)
{
	uint8_t flags = caps << 2 | num << 1 | scroll;
	//uint8_t flags = KbNumKeystrokes % 8;
	
	KbEncoderSendCommand(0xED);
	KbEncoderSendCommand(flags);
}
extern int g_pcOnKbPress;
extern int g_d1OnKbPress,g_d2OnKbPress,g_d3OnKbPress,g_d4OnKbPress;
bool getchflag = false;
void KeyboardHandlerMain()
{
	unsigned char status;
	char keycode;
	/* acknowledge interrupt */
	WritePort(0x20, 0x20);
	WritePort(0xA0, 0x20);

	status = ReadPort(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = ReadPort(KEYBOARD_DATA_PORT);
		
		if((keycode & SCANCODE_RELEASE) != 0) // the key was released
		{
			keyboardState[(keycode & SCANCODE_NOTREL)] = KEY_RELEASED;
		}
		else
		{
			KbNumKeystrokes++;
			
			keyboardState[(keycode & SCANCODE_NOTREL)] = KEY_PRESSED;
			KbAddKeyToBuffer(KeyboardMap[(keycode & SCANCODE_NOTREL) + (ShiftPressed() ? 0x80 : 0x00)]);
		}
		
		getchflag = false;
	}
}

char KbGetKeyFromBuffer()
{
	if (KeyboardBufferPos <= 0) return 0;
	char key = KeyboardBuffer[KeyboardBufferPos - 1];
	KeyboardBufferPos--;
	return key;
}

void WaitForKeyPress()
{
	while(KbIsBufferEmpty()) hlt;
	KbGetKeyFromBuffer();
}

uint16_t term_x = 0;
uint16_t term_y = 0;

void KeyboardFlush()
{
	KeyboardBufferPos = 0;
	memset(KeyboardBuffer, 0, KB_BUF_SIZE);
}
