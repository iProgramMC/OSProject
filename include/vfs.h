/*****************************************
		NanoShell Operating System
		  (C) 2022 iProgramInCpp

   Virtual FileSystem module headerfile
******************************************/
#ifndef _VFS_H
#define _VFS_H

#include <main.h>

struct FSNodeS;
struct DirEntS;

enum
{
	FILE_TYPE_NONE = 0,
	FILE_TYPE_FILE,
	FILE_TYPE_DIRECTORY,
	FILE_TYPE_CHAR_DEVICE,
	FILE_TYPE_BLOCK_DEVICE,
	FILE_TYPE_MOUNTPOINT = 8 //to be OR'd into the other flags
};

// Function pointer definitions so we can just call `file_node->Read(...);` etc.
typedef uint32_t 		(*FileReadFunc)    (struct FSNodeS*, uint32_t, uint32_t, void*);
typedef uint32_t 		(*FileWriteFunc)   (struct FSNodeS*, uint32_t, uint32_t, void*);
typedef void     		(*FileOpenFunc)    (struct FSNodeS*, bool, bool);
typedef void     		(*FileCloseFunc)   (struct FSNodeS*);
typedef struct DirEntS* (*FileReadDirFunc) (struct FSNodeS*, uint32_t);
typedef struct FSNodeS* (*FileFindDirFunc) (struct FSNodeS*, const char* pName);

typedef struct FSNodeS
{
	char 	        m_name[128]; //+nullterm, so 127 concrete chars
	uint32_t        m_type;
	uint32_t        m_perms;
	uint32_t        m_flags;
	uint32_t        m_inode;     //device specific
	uint32_t        m_length;    //file size
	uint32_t        m_implData;  //implementation data. TODO
	FileReadFunc    Read;
	FileWriteFunc   Write;
	FileOpenFunc    Open;
	FileCloseFunc   Close;
	FileReadDirFunc ReadDir;     //returns the n-th child of a directory
	FileFindDirFunc FindDir;     //try to find a child in a directory by name
}
FileNode;

typedef struct DirEntS
{
	char     m_name[128]; //+nullterm, so 127 concrete chars
	uint32_t m_inode;     //device specific
}
DirEnt;

/**
 * Gets the root entry of the filesystem.
 */
FileNode* FsGetRootNode();;

//Standard read/write/open/close functions.  They are prefixed with Fs to distinguish them
//from FiRead/FiWrite/FiOpen/FiClose, which deal with file handles not nodes.

//Remember the definitions above.

uint32_t FsRead   (FileNode* pNode, uint32_t offset, uint32_t size, void* pBuffer);
uint32_t FsWrite  (FileNode* pNode, uint32_t offset, uint32_t size, void* pBuffer);
void     FsOpen   (FileNode* pNode, bool read, bool write);
void     FsClose  (FileNode* pNode);
DirEnt*  FsReadDir(FileNode* pNode, uint32_t index);
FileNode*FsFindDir(FileNode* pNode, const char* pName);



//Initrd stuff:
#if 1
	typedef struct
	{
		int m_nFiles;
	}
	InitRdHeader;
	
	//! Same as in `fsmaker`
	typedef struct
	{
		uint32_t m_magic;
		char m_name[64];
		uint32_t m_offset, m_length;
	}
	InitRdFileHeader;

	/**
	* Gets the root entry of the initrd.
	*/
	FileNode* FsGetInitrdNode ();
	
	/**
	 * Initializes the initial ramdisk.
	 */
	void FsInitializeInitRd(void* pRamDisk);
#endif

#endif//_VFS_H