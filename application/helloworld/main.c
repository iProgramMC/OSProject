#include <stdint.h>

void LogMsg(const char*text);
int main ()
{
	uint16_t* buffer = (uint16_t*)0xC00B8000;
	
	//"Hello, world!" printed out in a grotesque way by 
	//poking at some random address.  It's in red to illustrate
	//it's not a kernel app.
	*(buffer++) = 0x3f48;
	/**(buffer++) = 0x3f65;
	*(buffer++) = 0x3f6c;
	*(buffer++) = 0x3f6c;
	*(buffer++) = 0x3f6f;
	*(buffer++) = 0x3f2c;
	*(buffer++) = 0x3f20;
	*(buffer++) = 0x3f77;
	*(buffer++) = 0x3f6f;
	*(buffer++) = 0x3f72;
	*(buffer++) = 0x3f6c;
	*(buffer++) = 0x3f64;
	*(buffer++) = 0x3f21;*/
	LogMsg("HELLO WORLD!!!");
	
	return 42;//The answer to everything
}


