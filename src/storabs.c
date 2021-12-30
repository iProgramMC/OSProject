/*****************************************
		NanoShell Operating System
		  (C) 2021 iProgramInCpp

        Storage Abstraction module
******************************************/

/**
 * A short description of what this module is responsible for:
 *
 * This module is responisble for parsing away drive numbers, and using
 * the drivers meant for them.
 *
 * The mapping of the drive numbers is as follows:
 *
 *    - 0x00 - 0x03: IDE drives
 *    - 0x10 - 0x11: Floppy drives
 *    - 0xF0 - 0xFF: RAM disk drives 
 *    - 0xE0 - 0xEF: USB flash drives (?)
 *    - 0xAA       : Special reserved flag. This drive ID is always empty.
 *    - 0x20 - 0x2F: AHCI drives (?)
 */
 
#include <storabs.h>

static DriveType StGetDriveType(DriveID driveNum)
{
	if (                    driveNum <= 0x03) return DEVICE_IDE;
	if (driveNum >= 0x10 && driveNum <= 0x11) return DEVICE_FLOPPY;
	if (driveNum >= 0xF0)                     return DEVICE_RAMDISK;
	return DEVICE_UNKNOWN;
}

//note: driveID is internal.
static DriveStatus StNoDriveRead(
	__attribute__((unused)) uint32_t lba, 
	__attribute__((unused)) void* pDest, 
	__attribute__((unused)) uint8_t driveID, 
	__attribute__((unused)) uint8_t nBlocks
)
{
	//does not do anything
	return DEVERR_NOTFOUND;
}
static DriveStatus StNoDriveWrite(
	__attribute__((unused)) uint32_t lba, 
	__attribute__((unused)) const void* pSrc, 
	__attribute__((unused)) uint8_t driveID, 
	__attribute__((unused)) uint8_t nBlocks
)
{
	//does not do anything
	return DEVERR_NOTFOUND;
}
static uint8_t StNoDriveGetSubID (__attribute__((unused)) DriveID did)
{
	return 0xFF;
}

static RamDisk g_RAMDisks[RAMDISK_MAX];
bool StIsRamDiskMounted (int num)
{
	return(g_RAMDisks[num].m_bMounted);
}
bool StIsRamDiskReadOnly (int num)
{
	return(g_RAMDisks[num].m_bReadOnly);
}

static DriveStatus StRamDiskWrite(uint32_t lba, const void* pSrc, uint8_t driveID, uint8_t nBlocks)
{
	if (!StIsRamDiskMounted(driveID))
		return DEVERR_NOTFOUND;
	if (!StIsRamDiskReadOnly(driveID))
		return DEVERR_NOWRITE;
	
	//get offset for a memcpy:
	int offset = lba * SECTOR_SIZE;
	
	//then write!
	fast_memcpy (&(g_RAMDisks[driveID].m_pDriveContents[offset]), pSrc, nBlocks * SECTOR_SIZE);
	return DEVERR_SUCCESS;
}
static DriveStatus StRamDiskRead(uint32_t lba, void* pDest, uint8_t driveID, uint8_t nBlocks)
{
	if (!StIsRamDiskMounted(driveID))
	{
		return DEVERR_NOTFOUND;
	}
	
	//get offset for a memcpy:
	int offset = lba * SECTOR_SIZE;
	
	//then read!
	fast_memcpy (pDest, &(g_RAMDisks[driveID].m_pDriveContents[offset]), nBlocks * SECTOR_SIZE);
	return DEVERR_SUCCESS;
}
static uint8_t StRamDiskGetSubID (DriveID did)
{
	return (int)did & 0xF;
}

static DriveReadCallback g_ReadCallbacks[] = {
	StNoDriveRead, //Unknown
	StNoDriveRead, //IDE -- TODO!
	StNoDriveRead, //Floppy -- TODO!
	StRamDiskRead, //RAM disk,
	StNoDriveRead, //Count
};
static DriveWriteCallback g_WriteCallbacks[] = {
	StNoDriveWrite, //Unknown
	StNoDriveWrite, //IDE -- TODO!
	StNoDriveWrite, //Floppy -- TODO!
	StRamDiskWrite, //RAM disk,
	StNoDriveWrite, //Count
};
static DriveGetSubIDCallback g_GetSubIDCallbacks[] = {
	StNoDriveGetSubID, //Unknown
	StNoDriveGetSubID, //IDE -- TODO!
	StNoDriveGetSubID, //Floppy -- TODO!
	StRamDiskGetSubID, //RAM disk,
	StNoDriveGetSubID, //Count
};

DriveStatus StDeviceRead(uint32_t lba, void* pDest, DriveID driveId, uint8_t nBlocks)
{
	DriveType driveType = StGetDriveType(driveId);
	if (driveType == DEVICE_UNKNOWN)
		return DEVERR_NOTFOUND;
	
	uint8_t driveSubId = g_GetSubIDCallbacks[driveType](driveId);
	
	return g_ReadCallbacks[driveType](lba, pDest, driveSubId, nBlocks);
}
DriveStatus StDeviceWrite(uint32_t lba, const void* pSrc, DriveID driveId, uint8_t nBlocks)
{
	DriveType driveType = StGetDriveType(driveId);
	if (driveType == DEVICE_UNKNOWN)
		return DEVERR_NOTFOUND;
	
	uint8_t driveSubId = g_GetSubIDCallbacks[driveType](driveId);
	
	return g_WriteCallbacks[driveType](lba, pSrc, driveSubId, nBlocks);
}
extern uint8_t g_TestingFloppyImage[];
DriveID StMountTestRamDisk ()
{
	/*int numDisk = -1;
	for (int i = 0; i < RAMDISK_MAX; i++)
	{
		if (!StIsRamDiskMounted(i))
		{
			numDisk = i;
			break;
		}
	}
	if (numDisk == -1) return 0xAA;//nope
	
	//initialize the ram disk:
	RamDisk* pRD = &g_RAMDisks[numDisk];
	
	pRD->m_bMounted = true;
	pRD->m_bReadOnly = true;
	
	//for now, just use a simple floppy image:
	pRD->m_CapacityBlocks = 1440 * 1024 / 512; //1440K = 2880 Blocks.
	pRD->m_pDriveContents = g_TestingFloppyImage;
	
	return 0xF0 + numDisk;*/
	return 0xAA;
}
