/*****************************************
		NanoShell Operating System
		  (C) 2022 iProgramInCpp

        Virtual File System module
******************************************/

/**
 * A short description of what this module is responsible for:
 *
 * This module is responsible for handling the file system.
 * Its root is "/".  Storage drives will be later on mounted
 * here, but they will use storabs.c's functionality.
 */
 

#include <vfs.h>
#include <string.h>
#include <memory.h>
#include <misc.h>

#define FS_DEVICE_NAME "Device"
#define FS_FSROOT_NAME "FSRoot"

FileNode *g_pRoot = NULL;

FileNode* FsGetRootNode ()
{
	//TODO: initialize it?
	return g_pRoot;
}

uint32_t FsRead(FileNode* pNode, uint32_t offset, uint32_t size, void* pBuffer)
{
	if (pNode)
	{
		if (pNode->Read)
			return pNode->Read(pNode, offset, size, pBuffer);
		else return 0;
	}
	else return 0;
}
uint32_t FsWrite(FileNode* pNode, uint32_t offset, uint32_t size, void* pBuffer)
{
	if (pNode)
	{
		if (pNode->Write)
		{
			return pNode->Write(pNode, offset, size, pBuffer);
		}
		else return 0;
	}
	else return 0;
}
void FsOpen(FileNode* pNode, bool read, bool write)
{
	if (pNode)
	{
		if (pNode->Open)
			return pNode->Open(pNode, read, write);
		else return;
	}
	else return;
}
void FsClose(FileNode* pNode)
{
	if (pNode)
	{
		if (pNode->Close)
			pNode->Close(pNode);
		else return;
	}
	else return;
}
DirEnt* FsReadDir(FileNode* pNode, uint32_t index)
{
	if (pNode)
	{
		if (pNode->ReadDir && (pNode->m_type & FILE_TYPE_DIRECTORY))
			return pNode->ReadDir(pNode, index);
		else return NULL;
	}
	else return NULL;
}
FileNode* FsFindDir(FileNode* pNode, const char* pName)
{
	if (pNode)
	{
		if (pNode->FindDir && (pNode->m_type & FILE_TYPE_DIRECTORY))
			return pNode->FindDir(pNode, pName);
		else return NULL;
	}
	else return NULL;
}

// Null device
#if 1
static uint32_t FsNullRead(UNUSED FileNode* pNode, UNUSED uint32_t offset, uint32_t size, void* pBuffer)
{
	memset (pBuffer, 0, size);
	return size;
}

static uint32_t FsNullWrite(UNUSED FileNode* pNode, UNUSED uint32_t offset, uint32_t size, UNUSED void* pBuffer)
{
	return size;
}
#endif

// Random device
#if 1
static uint32_t FsRandomRead(UNUSED FileNode* pNode, UNUSED uint32_t offset, uint32_t size, void* pBuffer)
{
	char* pText = (char*)pBuffer;
	for (uint32_t i = 0; i < size; i++)
	{
		char out = (char)GetRandom() ^ 0x4E;
		out = out << 5 | out >> 3;
		out ^= 0x82;
		out = out << 2 | out >> 6;
		out ^= 0xB7;
		out ^= 0xFF;
		*(pText++) = out;
	}
	return size;
}

static uint32_t FsRandomWrite(UNUSED FileNode* pNode, UNUSED uint32_t offset, UNUSED uint32_t size, UNUSED void* pBuffer)
{
	return 0;
}
#endif

// Console stuff
#if 1
static Console *s_InodeToConsole[] = {
	NULL,
	&g_debugConsole,
	&g_debugSerialConsole,
};

enum {
	DEVICE_NULL,
	DEVICE_DEBUG_CONSOLE,
	DEVICE_DEBUG_E9_CONSOLE,
};

static uint32_t FsTeletypeRead(UNUSED FileNode* pNode, UNUSED uint32_t offset, UNUSED uint32_t size, UNUSED void* pBuffer)
{
	return 0; // Can't read anything!
}

static uint32_t FsTeletypeWrite(FileNode* pNode, UNUSED uint32_t offset, uint32_t size, void* pBuffer)
{
	const char* pText = (const char*)pBuffer;
	for (uint32_t i = 0; i < size; i++)
	{
		if (!s_InodeToConsole[pNode->m_inode]) return 0;
		CoPrintChar (s_InodeToConsole[pNode->m_inode], *(pText++));
	}
	return size;
}

#endif

//Initrd Stuff:
#if 1

InitRdHeader* g_pInitRdHeader;
InitRdFileHeader* g_pInitRdFileHeaders;
FileNode* g_pInitRdRoot;
FileNode* g_pInitRdDev;  //add a dirnode for /dev to mount stuff later
FileNode* g_pRootNodes; //list of root nodes.
uint32_t  g_nRootNodes; //number of root nodes.
FileNode* g_pDevNodes; //list of dev nodes.
uint32_t  g_nDevNodes; //number of dev nodes.

static DirEnt g_DirEnt; // directory /dev

static uint32_t FsInitRdRead(FileNode* pNode, uint32_t offset, uint32_t size, void* pBuffer)
{
	InitRdFileHeader* pHeader = &g_pInitRdFileHeaders[pNode->m_inode];
	
	//check lengths
	if (offset > pHeader->m_length)
		return 0;
	if (offset + size > pHeader->m_length)
		size = pHeader->m_length - offset;
	
	//read!
	memcpy (pBuffer, (uint8_t*)pHeader->m_offset + offset, size);
	return size;
}
//TODO: open, close etc
//TODO: make a different separate way to do this stuff?
static DirEnt* FsInitRdReadDir(FileNode* pNode, uint32_t index)
{
	if (pNode == g_pInitRdRoot && index == 0)
	{
		strcpy (g_DirEnt.m_name, FS_DEVICE_NAME);
		g_DirEnt.m_inode = 0;
		return &g_DirEnt;
	}
	
	if (index - 1 >= g_nRootNodes)
		return NULL;
	
	strcpy(g_DirEnt.m_name, g_pRootNodes[index-1].m_name);
	g_DirEnt.m_inode = g_pRootNodes[index-1].m_inode;
	return &g_DirEnt;
}

static FileNode* FsInitRdFindDir(FileNode* pNode, const char* pName)
{
	if (pNode == g_pInitRdRoot && strcmp(pName, FS_DEVICE_NAME) == 0)
		return g_pInitRdDev;
	
	for (uint32_t i = 0; i < g_nRootNodes; i++)
	{
		if (strcmp(pName, g_pRootNodes[i].m_name) == 0)
			return &g_pRootNodes[i];
	}
	
	return NULL;
}

static DirEnt* FsDevReadDir(UNUSED FileNode* pNode, uint32_t index)
{
	if (index >= g_nDevNodes)
		return NULL;
	
	strcpy(g_DirEnt.m_name, g_pDevNodes[index].m_name);
	g_DirEnt.m_inode = g_pDevNodes[index].m_inode;
	return &g_DirEnt;
}

static FileNode* FsDevFindDir(UNUSED FileNode* pNode, const char* pName)
{
	for (uint32_t i = 0; i < g_nDevNodes; i++)
	{
		if (strcmp(pName, g_pDevNodes[i].m_name) == 0)
			return &g_pDevNodes[i];
	}
	
	return NULL;
}

FileNode* FsResolvePath (const char* pPath)
{
	char path_copy[PATH_MAX]; //max path
	if (strlen (pPath) >= PATH_MAX-1) return NULL;
	strcpy (path_copy, pPath);
	
	TokenState state;
	state.m_bInitted = 0;
	char* initial_filename = Tokenize (&state, path_copy, "/");
	if (!initial_filename) return NULL;
	
	//is it just an empty string? if yes, we're using
	//an absolute path.  Otherwise we gotta append the CWD 
	//and run this function again.
	if (*initial_filename == 0)
	{
		//LogMsg("Resolving absolute path '%s'", pPath);
		FileNode *pNode = g_pInitRdRoot;//TODO
		while (true)
		{
			char* path = Tokenize (&state, NULL, "/");
			
			//are we done?
			if (path && *path)
			{
				//LogMsg("Finding directory: '%s'", path);
				//nope, resolve pNode again.
				pNode = FsFindDir (pNode, path);
				if (!pNode)
				{
					//LogMsg("File not found inside %s", path);
					return NULL;
				}
			}
			else
			{
				//LogMsg("Done!  Returning to caller...");
				return pNode;
			}
		}
	}
	else
	{
		//TODO
		//LogMsg("Not an absolute path");
		return NULL;
	}
}

FileNode* FsGetInitrdNode ()
{
	//TODO: initialize it?
	return g_pInitRdRoot;
}

void FsInitializeDevicesDir ()
{
	int devCount = 0, index = 0;
	
#define DEFINE_DEVICE(_1, _2, _3, _4) do\
	devCount++;\
while (0)
	#include "vfs_dev_defs.h"
	
	g_pDevNodes = (FileNode*)MmAllocate(sizeof(FileNode) * devCount);
	g_nDevNodes = devCount;
	
#undef DEFINE_DEVICE
	
#define DEFINE_DEVICE(name, read, write, inode) do {\
	g_pDevNodes[index].m_length = 0;\
	g_pDevNodes[index].m_inode  = inode;\
	g_pDevNodes[index].m_type   = FILE_TYPE_CHAR_DEVICE;\
	g_pDevNodes[index].m_perms  = PERM_READ|PERM_WRITE;\
	strcpy (g_pDevNodes[index].m_name, name);\
	\
	g_pDevNodes[index].Read  = read;\
	g_pDevNodes[index].Write = write;\
	g_pDevNodes[index].Open  = NULL;\
	g_pDevNodes[index].Close = NULL;\
	g_pDevNodes[index].ReadDir = NULL;\
	g_pDevNodes[index].FindDir = NULL;\
	index++;\
} while (0)
	
	#include "vfs_dev_defs.h"
	
#undef DEFINE_DEVICE
}

static bool EndsWith(const char* pText, const char* pCheck)
{
	int slt = strlen (pText), slc = strlen (pCheck);
	if (slt < slc) return false; //obviously, it can't.
	
	const char* pTextEnd = pText + slt;
	pTextEnd -= slc;
	return (strcmp (pTextEnd, pCheck) == 0);
}

void FsInitializeInitRd(void* pRamDisk)
{
	uint32_t location = (uint32_t)pRamDisk;
	//initialize the file headers and stuff.
	g_pInitRdHeader = (InitRdHeader*)pRamDisk;
	g_pInitRdFileHeaders  = (InitRdFileHeader*)(pRamDisk + sizeof(InitRdHeader));
	
	//initialize the root directory
	g_pInitRdRoot = (FileNode*)MmAllocate(sizeof(FileNode));
	strcpy(g_pInitRdRoot->m_name, FS_FSROOT_NAME);
	g_pInitRdRoot->m_flags = g_pInitRdRoot->m_inode = g_pInitRdRoot->m_length = g_pInitRdRoot->m_implData = g_pInitRdRoot->m_perms = 0;
	g_pInitRdRoot->m_type = FILE_TYPE_DIRECTORY;
	g_pInitRdRoot->m_perms = PERM_READ;
	g_pInitRdRoot->Read    = NULL;
	g_pInitRdRoot->Write   = NULL;
	g_pInitRdRoot->Open    = NULL;
	g_pInitRdRoot->Close   = NULL;
	g_pInitRdRoot->ReadDir = FsInitRdReadDir;
	g_pInitRdRoot->FindDir = FsInitRdFindDir;
	
	// Initialize the /dev dir.
	g_pInitRdDev = (FileNode*)MmAllocate(sizeof(FileNode));
	strcpy(g_pInitRdDev->m_name, FS_DEVICE_NAME);
	g_pInitRdDev->m_flags = g_pInitRdDev->m_inode = g_pInitRdDev->m_length = g_pInitRdDev->m_implData = g_pInitRdDev->m_perms = 0;
	g_pInitRdDev->m_type = FILE_TYPE_DIRECTORY;
	g_pInitRdDev->m_perms = PERM_READ | PERM_WRITE;
	g_pInitRdDev->Read    = NULL;
	g_pInitRdDev->Write   = NULL;
	g_pInitRdDev->Open    = NULL;
	g_pInitRdDev->Close   = NULL;
	g_pInitRdDev->ReadDir = FsDevReadDir;
	g_pInitRdDev->FindDir = FsDevFindDir;
	
	// Initialize devices
	FsInitializeDevicesDir();
	
	// Add files to the ramdisk.
	g_pRootNodes = (FileNode*)MmAllocate(sizeof(FileNode) * g_pInitRdHeader->m_nFiles);
	g_nRootNodes = g_pInitRdHeader->m_nFiles;
	
	// for every file
	for (int i = 0; i < g_pInitRdHeader->m_nFiles; i++)
	{
		/*if (g_pInitRdFileHeaders[i].m_magic != 0x2A2054EL)
		{
			LogMsg("\x01\x0C ERROR\x01\x0F: initrd failed to initialize -- is it corrupt?");
			KeStopSystem();
		}*/
		
		g_pInitRdFileHeaders[i].m_offset += location;
		//create a new filenode
		strcpy(g_pRootNodes[i].m_name, g_pInitRdFileHeaders[i].m_name);
		g_pRootNodes[i].m_length = g_pInitRdFileHeaders[i].m_length;
		g_pRootNodes[i].m_inode = i;
		g_pRootNodes[i].m_type = FILE_TYPE_FILE;
		g_pRootNodes[i].Read    = FsInitRdRead;
		g_pRootNodes[i].Write   = NULL;
		g_pRootNodes[i].Open    = NULL;
		g_pRootNodes[i].Close   = NULL;
		g_pRootNodes[i].ReadDir = NULL;
		g_pRootNodes[i].FindDir = NULL;
		g_pRootNodes[i].m_perms = PERM_READ;
		
		//if it ends with .nse...
		if (EndsWith(g_pRootNodes[i].m_name, ".nse"))
		{
			//also make it executable
			g_pRootNodes[i].m_perms |= PERM_EXEC;
		}
	}
}

#endif

// File Descriptor handlers:
#if 1

#define FD_MAX 256

typedef struct {
	bool      m_bOpen;
	FileNode *m_pNode;
	char      m_sPath[PATH_MAX+2];
	int       m_nStreamOffset;
	int       m_nFileEnd;
	bool      m_bIsFIFO; //is a char device, basically
	const char* m_openFile;
	int       m_openLine;
}
FileDescriptor;

FileDescriptor g_FileNodeToDescriptor[FD_MAX];

void FiDebugDump()
{
	LogMsg("Listing opened files.");
	for (int i = 0; i < FD_MAX; i++)
	{
		FileDescriptor* p = &g_FileNodeToDescriptor[i];
		if (p->m_bOpen)
		{
			LogMsg("FD:%d\tFL:%d\tFS:%d\tP:%d FN:%x FC:%s:%d\tFN:%s",i,p->m_nFileEnd,p->m_nStreamOffset,
					p->m_bIsFIFO, p->m_pNode, p->m_openFile, p->m_openLine, p->m_sPath);
		}
	}
	LogMsg("Done");
}

static int FiFindFreeFileDescriptor(const char* reqPath)
{
	for (int i = 0; i < FD_MAX; i++)
	{
		if (g_FileNodeToDescriptor[i].m_bOpen)
			if (strcmp (g_FileNodeToDescriptor[i].m_sPath, reqPath) == 0)
				return -EAGAIN;
	}
	for (int i = 0; i < FD_MAX; i++)
	{
		if (!g_FileNodeToDescriptor[i].m_bOpen)
			return i;
	}
	return -ENFILE;
}

int FiOpenD (const char* pFileName, int oflag, const char* srcFile, int srcLine)
{
	// find a free fd to open:
	int fd = FiFindFreeFileDescriptor(pFileName);
	if (fd < 0) return fd;
	
	//find the node:
	FileNode* pFile = FsResolvePath(pFileName);
	if (!pFile)
		return -EEXIST;
	
	//if we are trying to read, but we can't:
	if ((oflag & O_RDONLY) && !(pFile->m_perms & PERM_READ))
		return -EACCES;
	//if we are trying to write, but we can't:
	if ((oflag & O_WRONLY) && !(pFile->m_perms & PERM_WRITE))
		return -EACCES;
	//if we are trying to execute, but we can't:
	if ((oflag & O_EXEC) && !(pFile->m_perms & PERM_EXEC))
		return -EACCES;
	
	if (pFile->m_type & FILE_TYPE_DIRECTORY) return -EISDIR;
	
	//we have all the perms, let's write the filenode there:
	FileDescriptor *pDesc = &g_FileNodeToDescriptor[fd];
	pDesc->m_bOpen 			= true;
	strcpy(pDesc->m_sPath, pFileName);
	pDesc->m_pNode 			= pFile;
	pDesc->m_openFile	 	= srcFile;
	pDesc->m_openLine	 	= srcLine;
	pDesc->m_nStreamOffset 	= 0;
	pDesc->m_nFileEnd		= pFile->m_length;
	pDesc->m_bIsFIFO		= pFile->m_type == FILE_TYPE_CHAR_DEVICE;
	
	return fd;
}
bool FiIsValidDescriptor(int fd)
{
	if (fd < 0 || fd >= FD_MAX) return false;
	if (!g_FileNodeToDescriptor[fd].m_bOpen) return false;
	return true;
}

int FiClose (int fd)
{
	if (!FiIsValidDescriptor(fd)) return -EBADF;
	
	//closes the file:
	FileDescriptor *pDesc = &g_FileNodeToDescriptor[fd];
	pDesc->m_bOpen = false;
	strcpy(pDesc->m_sPath, "");
	pDesc->m_pNode = NULL;
	pDesc->m_nStreamOffset = 0;
	
	return -ENOTHING;
}

size_t FiRead (int fd, void *pBuf, int nBytes)
{
	if (!FiIsValidDescriptor(fd)) return -EBADF;
	
	if (nBytes < 0) return -EINVAL;
	
	int rv = FsRead (g_FileNodeToDescriptor[fd].m_pNode, (uint32_t)g_FileNodeToDescriptor[fd].m_nStreamOffset, (uint32_t)nBytes, pBuf);
	if (rv < 0) return rv;
	g_FileNodeToDescriptor[fd].m_nStreamOffset += rv;
	return rv;
}

//TODO
size_t FiWrite (int fd, void *pBuf, int nBytes)
{
	if (!FiIsValidDescriptor(fd)) return -EBADF;
	
	if (nBytes < 0) return -EINVAL;
	
	int rv = FsWrite (g_FileNodeToDescriptor[fd].m_pNode, (uint32_t)g_FileNodeToDescriptor[fd].m_nStreamOffset, (uint32_t)nBytes, pBuf);
	if (rv < 0) return rv;
	g_FileNodeToDescriptor[fd].m_nStreamOffset += rv;
	return rv;
}

int FiSeek (int fd, int offset, int whence)
{
	if (!FiIsValidDescriptor(fd)) return -EBADF;
	
	if (g_FileNodeToDescriptor[fd].m_bIsFIFO) return -ESPIPE;
	
	if (whence < 0 || whence > SEEK_END) return -EINVAL;
	int realOffset = offset;
	switch (whence)
	{
		case SEEK_CUR:
			realOffset += g_FileNodeToDescriptor[fd].m_nStreamOffset;
			break;
		case SEEK_END:
			realOffset += g_FileNodeToDescriptor[fd].m_nFileEnd;
			break;
	}
	if (realOffset > g_FileNodeToDescriptor[fd].m_nFileEnd)
		return -EOVERFLOW;
	
	return -ENOTHING;
}

int FiTell (int fd)
{
	if (!FiIsValidDescriptor(fd)) return -EBADF;
	return g_FileNodeToDescriptor[fd].m_nStreamOffset;
}

int FiTellSize (int fd)
{
	if (!FiIsValidDescriptor(fd)) return -EBADF;
	return g_FileNodeToDescriptor[fd].m_nFileEnd;
}


#endif


