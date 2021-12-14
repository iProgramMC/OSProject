#include <stdint.h>

int main ()
{
	uint16_t* buffer = (uint16_t*)0xC00B8000;
	
	//"Hello, world!" printed out in a grotesque way by 
	//poking at some random address.  It's in red to illustrate
	//it's not a kernel app.
	*(buffer++) = 0x1f48;
	*(buffer++) = 0x1f65;
	*(buffer++) = 0x1f6c;
	*(buffer++) = 0x1f6c;
	*(buffer++) = 0x1f6f;
	*(buffer++) = 0x1f2c;
	*(buffer++) = 0x1f20;
	*(buffer++) = 0x1f77;
	*(buffer++) = 0x1f6f;
	*(buffer++) = 0x1f72;
	*(buffer++) = 0x1f6c;
	*(buffer++) = 0x1f64;
	*(buffer++) = 0x1f21;
	
	return 42;//The answer to everything
}


