%define enable 1

%ifdef enable

section .data
global g_testingElfStart, g_testingElfEnd, g_testingElf

g_testingElfStart:
g_testingElf:
	incbin 'application/helloworld/main.nse'
g_testingElfEnd:

global g_TestingFloppyImageStart, g_TestingFloppyImageEnd, g_TestingFloppyImage

g_TestingFloppyImageStart:
g_TestingFloppyImage:
	incbin 'test.img'
g_TestingFloppyImageEnd:

%endif