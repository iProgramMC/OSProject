#include <stdint.h>

int addtodata=3;
int addtobss;

void LogMsg(const char*text);
int main ()
{
	addtodata++;
	addtobss = 1;
	uint16_t* buffer = (uint16_t*)0xC00B8000;
	
	//"Hello, world!" printed out in a grotesque way by 
	//poking at some random address.  It's in red to illustrate
	//it's not a kernel app.
	*(buffer++) = 0x3f48;
	LogMsg("HELLO WORLD!!!");
	
	return 42;//The answer to everything
}


