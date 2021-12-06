#include <stdint.h>

int main ()
{
	uint16_t* buffer = (uint16_t*)0xB8000;
	
	//"Hello, world!" printed out in a grotesque way by 
	//poking at some random address.  It's in red to illustrate
	//it's not a kernel app.
	*(buffer++) = 0x481f;
	*(buffer++) = 0x651f;
	*(buffer++) = 0x6c1f;
	*(buffer++) = 0x6c1f;
	*(buffer++) = 0x6f1f;
	*(buffer++) = 0x2c1f;
	*(buffer++) = 0x201f;
	*(buffer++) = 0x771f;
	*(buffer++) = 0x6f1f;
	*(buffer++) = 0x721f;
	*(buffer++) = 0x6c1f;
	*(buffer++) = 0x641f;
	*(buffer++) = 0x211f;
	
	return 42;//The answer to everything
}


