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
			return pNode->Write(pNode, offset, size, pBuffer);
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

//Initrd Stuff:
#if 1

InitRdHeader* g_pInitRdHeader;
InitRdFileHeader* g_pFileHeaders;
FileNode* g_pInitRdRoot;
FileNode* g_pInitRdDev;  //add a dirnode for /dev to mount stuff later
FileNode* g_pRootNodes; //list of root nodes.
uint32_t  g_nRootNodes; //number of root nodes.

static DirEnt g_DirEnt; // directory /dev

static uint32_t FsInitRdRead(FileNode* pNode, uint32_t offset, uint32_t size, void* pBuffer)
{
	InitRdFileHeader* pHeader = &g_pFileHeaders[pNode->m_inode];
	
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
		strcpy (g_DirEnt.m_name, "dev");
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
	if (pNode == g_pInitRdRoot && strcmp(pName, "dev") == 0)
		return g_pInitRdDev;
	
	for (uint32_t i = 0; i < g_nRootNodes; i++)
	{
		if (strcmp(pName, g_pRootNodes[i].m_name) == 0)
			return &g_pRootNodes[i];
	}
	
	return NULL;
}


FileNode* FsGetInitrdNode ()
{
	//TODO: initialize it?
	return g_pInitRdRoot;
}

void FsInitializeInitRd(void* pRamDisk)
{
	uint32_t location = (uint32_t)pRamDisk;
	//initialize the file headers and stuff.
	g_pInitRdHeader = (InitRdHeader*)pRamDisk;
	g_pFileHeaders  = (InitRdFileHeader*)(pRamDisk + sizeof(InitRdHeader));
	
	//initialize the root directory
	g_pInitRdRoot = (FileNode*)MmAllocate(sizeof(FileNode));
	strcpy(g_pInitRdRoot->m_name, "initrd");
	g_pInitRdRoot->m_flags = g_pInitRdRoot->m_inode = g_pInitRdRoot->m_length = g_pInitRdRoot->m_implData = g_pInitRdRoot->m_perms = 0;
	g_pInitRdRoot->m_type = FILE_TYPE_DIRECTORY;
	g_pInitRdRoot->Read    = NULL;
	g_pInitRdRoot->Write   = NULL;
	g_pInitRdRoot->Open    = NULL;
	g_pInitRdRoot->Close   = NULL;
	g_pInitRdRoot->ReadDir = FsInitRdReadDir;
	g_pInitRdRoot->FindDir = FsInitRdFindDir;
	
	// Initialize the /dev dir.
	g_pInitRdDev = (FileNode*)MmAllocate(sizeof(FileNode));
	strcpy(g_pInitRdDev->m_name, "dev");
	g_pInitRdDev->m_flags = g_pInitRdDev->m_inode = g_pInitRdDev->m_length = g_pInitRdDev->m_implData = g_pInitRdDev->m_perms = 0;
	g_pInitRdDev->m_type = FILE_TYPE_DIRECTORY;
	g_pInitRdDev->Read    = NULL;
	g_pInitRdDev->Write   = NULL;
	g_pInitRdDev->Open    = NULL;
	g_pInitRdDev->Close   = NULL;
	g_pInitRdDev->ReadDir = FsInitRdReadDir;
	g_pInitRdDev->FindDir = FsInitRdFindDir;
	
	// Add files to the ramdisk.
	g_pRootNodes = (FileNode*)MmAllocate(sizeof(FileNode) * g_pInitRdHeader->m_nFiles);
	g_nRootNodes = g_pInitRdHeader->m_nFiles;
	
	// for every file
	for (int i = 0; i < g_pInitRdHeader->m_nFiles; i++)
	{
		/*if (g_pFileHeaders[i].m_magic != 0x2A2054EL)
		{
			LogMsg("\x01\x0C ERROR\x01\x0F: initrd failed to initialize -- is it corrupt?");
			KeStopSystem();
		}*/
		
		g_pFileHeaders[i].m_offset += location;
		//create a new filenode
		strcpy(g_pRootNodes[i].m_name, g_pFileHeaders[i].m_name);
		g_pRootNodes[i].m_length = g_pFileHeaders[i].m_length;
		g_pRootNodes[i].m_inode = i;
		g_pRootNodes[i].m_type = FILE_TYPE_FILE;
		g_pRootNodes[i].Read    = FsInitRdRead;
		g_pRootNodes[i].Write   = NULL;
		g_pRootNodes[i].Open    = NULL;
		g_pRootNodes[i].Close   = NULL;
		g_pRootNodes[i].ReadDir = NULL;
		g_pRootNodes[i].FindDir = NULL;
	}
}

#endif