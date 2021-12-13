
void StartupSystem (unsigned long magic, unsigned long mbi)
{
	mbi += 0xc0000000;
	FirstThingEver(mbi);
	
	SwitchMode (1);
	g_textX = g_textY = 0;
	
	//print the hello text, to see if the os booted properly
	LogMsg("iProgramInCpp's Operating System " VersionString "\nmultiboot parms:");
	LogInt(magic);
	LogInt(mbi);
	LogMsg("\nHello world!\n\n");
	
	InitMemoryManager();
	LogInt (e_placement);
	LogMsg("\n");
	
	void *pPage = AllocateSinglePage();
	LogInt((int)pPage);
	FreePage(pPage);
	LogMsg("\n");
	
	// try allocating something:
	void *a = Allocate (8100); // 2 pages
	void *b = Allocate(12000); // 3 pages
	
	InvalidateTLB();
	*((uint32_t*)a) = 0xAAAA;
	*((uint32_t*)b) = 0xBBBB;
	
	LogInt ((int)a);
	LogInt ((int)b);
	LogMsg("   ----   ");
	LogInt (*((uint32_t*)a));
	LogInt (*((uint32_t*)b));
	LogMsg("\n");
	
	Free(a);
	void *c = Allocate(12000); //3 pages, should not have same address as a
	*((uint32_t*)c) = 0xCCCC;
	void *d = Allocate (4000); //only one page, it should have the same addr as a
	*((uint32_t*)d) = 0xDDDD;
	LogInt ((int)c);
	LogInt ((int)d);
	LogMsg("   ----   ");
	LogInt (*((uint32_t*)c));
	LogInt (*((uint32_t*)d));
	LogInt (*((uint32_t*)b));
	LogMsg("\n");
	
	Free(a);
	Free(b);
	Free(c);
	Free(d);
	a = b = c = d = NULL;
	
	StopSystem();
}