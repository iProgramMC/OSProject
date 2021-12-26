#include <print.h>
#include <string.h>
#include <vga.h>

void ___swap(char*a, char*b) {
	char e;e=*a;*a=*b;*b=e;
}

void vsprintf(char* memory, const char* format, va_list list) {
	while (*format) {
		char m = *format;
		format++;
		if (m == '%') {
			m = *format++;
			// parse the m
			if (m == 0) return;
			switch (m) {
				case 's': {
					char* stringToPrint = va_arg(list, char*);
					if (stringToPrint == NULL) stringToPrint = "(null)";
					int length = strlen(stringToPrint);
					memcpy(memory, stringToPrint, length);
					memory += length;
					continue;
				}
				case 'c': {
					int chrToPrint = va_arg(list, int);
					*memory++ = chrToPrint;
					continue;
				}
				case 'd':case'i': {
					int32_t num = va_arg(list, int32_t);
					char str[14] = {0, };
					int i = 0;
					bool isNegative = false;
					if (num == 0) {
						str[i++] = '0'; 
						str[i] = '\0'; 
						goto skip1;
					}
					if (num == -2147483648) {
						char* e = "-2147483648";
						int length = strlen(e);
						memcpy(memory, e, length);
						memory += length;
						goto skip3;
					}
					
					// In standard itoa(), negative numbers are handled only with  
					// base 10. Otherwise numbers are considered unsigned. 
					if (num < 0) {
						isNegative = true;
						num = -num;
					}
					int base = 10;
					// Process individual digits 
					while (num != 0) 
					{ 
						int rem = num % base; 
						str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0'; 
						num = num/base; 
					} 
					// If number is negative, append '-' 
					if (isNegative) 
						str[i++] = '-'; 
					str[i] = '\0'; // Append string terminator 
					int start = 0; 
					int end = i -1; 
					while (start < end) 
					{ 
						___swap((str+start), (str+end)); 
						start++; 
						end--; 
					} 
				skip1:;
					int length = i;
					memcpy(memory, str, length);
					memory += length;
				skip3:;
					continue;
				}
				case 'u': {
					uint32_t num = va_arg(list, uint32_t);
					char str[14] = {0, };
					int i = 0;
					if (num == 0) {
						str[i++] = '0'; 
						str[i] = '\0'; 
						goto skip2;
					}
					int base = 10;
					// Process individual digits 
					while (num != 0) 
					{ 
						int rem = num % base; 
						str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0'; 
						num = num/base; 
					} 
					str[i] = '\0'; // Append string terminator 
					int start = 0; 
					int end = i -1; 
					while (start < end) 
					{ 
						___swap((str+start), (str+end)); 
						start++; 
						end--; 
					} 
				skip2:;
					int length = i;
					memcpy(memory, str, length);
					memory += length;
					continue;
				}
				case 'x': {
					uint32_t toPrint = va_arg(list, uint32_t);
					uint32_t power = (15 << 28), pt = 28;
					for (; power != 0; power >>= 4, pt -= 4) {
						uint32_t p = toPrint & power;
						p >>= pt;
						*memory = "0123456789abcdef"[p];
						memory++;
					}
					continue;
				}
				case 'X': {
					uint32_t toPrint = va_arg(list, uint32_t);
					uint32_t power = (15 << 28), pt = 28;
					for (; power != 0; power >>= 4, pt -= 4) {
						uint32_t p = toPrint & power;
						p >>= pt;
						*memory = "0123456789ABCDEF"[p];
						memory++;
					}
					continue;
				}
			}
		} else {
			*memory++ = m;
		}
	}
	*memory = '\0';
	return;
}
void sprintf(char*a, const char*c, ...) {
	va_list list;
	va_start(list, c);
	vsprintf(a, c, list);
	va_end(list);
}

typedef struct
{
	char m_steppingID : 4;
	char m_model : 4;
	char m_familyID : 4;
	char m_processorType : 2;
	char m_reserved : 2;
	char m_extModelID: 4;
	char m_extendedFamilyID;
	char m_reserved1 : 4;
}
__attribute__((packed))
CPUIDFeatureBits;

extern void KeCPUID();
extern uint32_t g_cpuidLastLeaf;
extern char g_cpuidNameEBX[];
extern CPUIDFeatureBits g_cpuidFeatureBits;

void KePrintSystemInfo()
{
	KeCPUID();
	LogMsg("Information about the system:");
	LogMsg("CPU Type: %s", g_cpuidNameEBX);
	LogMsg("Feature bits: %x", *((int*)&g_cpuidFeatureBits));
	LogMsgNoCr("x86 Family %d ", g_cpuidFeatureBits.m_familyID);
	LogMsgNoCr("Model %d ", g_cpuidFeatureBits.m_model);
	LogMsg("Stepping %d", g_cpuidFeatureBits.m_steppingID);
	LogMsg("g_cpuidLastLeaf: %d", g_cpuidLastLeaf);
}
extern int g_textX, g_textY;

