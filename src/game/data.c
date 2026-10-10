// Loads, tracks, decodes, and closes files from the game's data archives.
#include "game/data.h"
#include "game/dvd.h"

#include "game/armem.h"
#include "game/process.h"
#include "dolphin/dvd.h"

#define STAT_ID_ARAM 65536
#define DATA_DIR_ID_MASK 0xFFFF0000
#define DATA_EVEN_SIZE_MASK 0x1

#define PTR_OFFSET(ptr, offset) (void *)(((u8 *)(ptr)+(u32)(offset)))
#define DATA_EFF_SIZE(size) (((size)+1) & ~DATA_EVEN_SIZE_MASK)
#define DATA_BUFFER_SIZE_MASK (~3)

static void **HuDataReadMultiSub(int *dataNumbers, BOOL useMemoryTag, s32 memoryTag);

#define DATA_MAX_READSTAT 128

typedef struct DataDirStat_s {
    char *archivePath; // DVD path of this data archive.
    s32 dvdEntryNumber; // DVD directory entry for the archive, or -1 before initialization.
} DATADIRSTAT;

#define DATADIR(name) { "data/" #name ".bin", -1 },

static DATADIRSTAT DataDirStat[] = {
    #include "datadir_table.h"
    { NULL, -1 }
};

#undef DATADIR

u32 DirDataSize; // Full file length in bytes; DVD reads and resident archive lookups update it.
static u32 DataDirMax; // Number of archives in the path table, excluding its NULL terminator.
static s32 shortAccessSleep; // Nonzero yields each frame while direct DVD reads are pending.
static HUDATASTAT ATTRIBUTE_ALIGN(32) ReadDataStat[DATA_MAX_READSTAT]; // Active archive pool.

// Game startup resolves the archive paths and clears the fixed pool of active archive reads.
void HuDataInit(void)
{
    s32 archiveIndex = 0;
    DATADIRSTAT *dirStat = DataDirStat;
    HUDATASTAT *readStat;
    while(dirStat->archivePath) {
        if((dirStat->dvdEntryNumber = DVDConvertPathToEntrynum(dirStat->archivePath)) == -1) {
            OSReport("data.c: Data File Error(%s)\n", dirStat->archivePath);
            OSPanic("data.c", 66, "\n");
        }
        archiveIndex++;
        dirStat++;
    }
    DataDirMax = archiveIndex;
    for (archiveIndex = 0, readStat = ReadDataStat; archiveIndex < DATA_MAX_READSTAT;
         archiveIndex++, readStat++) {
        readStat->dirId = HU_DATANUM_NONE;
        readStat->used = FALSE;
        readStat->status = 0;
    }
}

// Archive-loading helpers use this to find the first unused slot in the read pool.
static s32 HuDataReadStatusGet(void)
{
    s32 statusIndex;
    for(statusIndex=0; statusIndex<DATA_MAX_READSTAT; statusIndex++) {
        if(ReadDataStat[statusIndex].dirId == HU_DATANUM_NONE) {
            break;
        }
    }
    if(statusIndex >= DATA_MAX_READSTAT) {
        statusIndex = HU_DATA_STAT_NONE;
    }
    return statusIndex;
}

// Resource loaders find a resident archive by the upper half of a packed data number.
s32 HuDataReadChk(int dataNum)
{
    s32 statusIndex;
    dataNum >>= 16;
    for(statusIndex=0; statusIndex<DATA_MAX_READSTAT; statusIndex++) {
        if(ReadDataStat[statusIndex].dirId == dataNum && ReadDataStat[statusIndex].status != 1) {
            break;
        }
    }
    if(statusIndex >= DATA_MAX_READSTAT) {
        statusIndex = HU_DATA_STAT_NONE;
    }
    return statusIndex;
}

// Character and ARAM loaders use a registered archive pointer to find its status record.
HUDATASTAT *HuDataGetStatus(void *archivePtr)
{
    s32 statusIndex;
    for(statusIndex=0; statusIndex<DATA_MAX_READSTAT; statusIndex++) {
        if(ReadDataStat[statusIndex].dirP == archivePtr) {
            break;
        }
    }
    // The strict comparison leaves an unmatched pointer referring past the pool.
    if(statusIndex > DATA_MAX_READSTAT) {
        return NULL;
    }
    return &ReadDataStat[statusIndex];
}

// Returns the loaded archive pointer for a data number, if its archive is active.
void *HuDataGetDirPtr(int dirNum)
{
    s32 statId = HuDataReadChk(dirNum);
    if(statId < 0) {
        return NULL;
    }
    return ReadDataStat[statId].dirP;
}

// Entry readers and ARAM loaders obtain the containing archive, waiting for a queued ARAM copy.
HUDATASTAT *HuDataDirRead(int dataNum)
{
    HUDATASTAT *readStat;
    s32 statId;
    s32 dirId;
    dirId  = dataNum >> 16;
    if(DataDirMax <= dirId) {
        OSReport("data.c: Data Number Error(%d)\n", dataNum);
        return NULL;
    }

    if((statId = HuDataReadChk(dataNum)) < 0) {
        AMEM_PTR aramArchiveAddress;
        if(aramArchiveAddress = HuARDirCheck(dataNum)) {
            HuAR_ARAMtoMRAM(aramArchiveAddress);
            while(HuARDMACheck());
            statId = HuDataReadChk(dataNum);
            readStat = &ReadDataStat[statId];
        } else {
            if((statId = HuDataReadStatusGet()) == HU_DATA_STAT_NONE) {
                OSReport("data.c: Data Work Max Error\n");
                return NULL;
            }
            readStat = &ReadDataStat[statId];
            readStat->dirP = HuDvdDataFastRead(DataDirStat[dirId].dvdEntryNumber);
            if(readStat->dirP) {
                HuMemMemoryFileSet(readStat->dirP, dataNum & DATA_DIR_ID_MASK);
                readStat->dirId = dirId;
            }
        }
    } else {
        readStat = &ReadDataStat[statId];
        DirDataSize = readStat->dvdFile.length;
    }
    return readStat;
}

// Called by numbered readers; reuses an archive or loads it for the requested memory tag.
static HUDATASTAT *HuDataDirReadNum(int dataNum, s32 memoryTag)
{
    HUDATASTAT *readStat;
    s32 statId;
    s32 dirId;
    dirId  = dataNum >> 16;
    if(DataDirMax <= dirId) {
        OSReport("data.c: Data Number Error(%d)\n", dataNum);
        return NULL;
    }

    if((statId = HuDataReadChk(dataNum)) < 0) {
        AMEM_PTR aramArchiveAddress;
        if((aramArchiveAddress = HuARDirCheck(dataNum))) {
            OSReport("ARAM data num %x\n", dataNum);
            HuAR_ARAMtoMRAMNum(aramArchiveAddress, memoryTag);
            while(HuARDMACheck());
            statId = HuDataReadChk(dataNum);
            readStat = &ReadDataStat[statId];
            readStat->used = TRUE;
            readStat->num = memoryTag;
        } else {
            OSReport("data num %x\n", dataNum);
            if((statId = HuDataReadStatusGet()) == HU_DATA_STAT_NONE) {
                OSReport("data.c: Data Work Max Error\n");
                return NULL;
            }
            readStat = &ReadDataStat[statId];
            readStat->dirP = HuDvdDataFastReadNum(DataDirStat[dirId].dvdEntryNumber, memoryTag);
            if(readStat->dirP) {
                HuMemMemoryFileSet(readStat->dirP, dataNum & DATA_DIR_ID_MASK);
                readStat->dirId = dirId;
                readStat->used = TRUE;
                readStat->num = memoryTag;
            }
        }
    } else {
        readStat = &ReadDataStat[statId];
    }
    return readStat;
}

// The ARAM completion callback registers its copied archive in the read-status pool.
HUDATASTAT *HuDataDirSet(void *archivePtr, int dataNum)
{
    HUDATASTAT *readStat = HuDataGetStatus(archivePtr);
    s32 statId;
    if((statId = HuDataReadChk(readStat->dirId << 16)) >= 0) {
        HuDataDirClose(dataNum);
    }
    if((statId = HuDataReadStatusGet()) == HU_DATA_STAT_NONE) {
        OSReport("data.c: Data Work Max Error\n");
        return NULL;
    } else {
        readStat = &ReadDataStat[statId];
        readStat->dirP = archivePtr;
        readStat->dirId = dataNum >>16;
        return readStat;
    }
}

// DVD completion advances an archive read in blocks until completion or cancellation.
void HuDataDirReadAsyncCallBack(s32 dvdResult, DVDFileInfo* dvdFileInfo)
{
    HUDATASTAT *readStat;
    s32 statusIndex;
    for(statusIndex=0; statusIndex<DATA_MAX_READSTAT; statusIndex++) {
        if (ReadDataStat[statusIndex].status == 1 &&
            ReadDataStat[statusIndex].dvdFile.startAddr == dvdFileInfo->startAddr) {
            break;
        }
    }
    if(statusIndex >= DATA_MAX_READSTAT) {
        OSPanic("data.c", 364, "dvd.c AsyncCallBack Error");
    }
    readStat = &ReadDataStat[statusIndex];
    if(dvdResult == DVD_RESULT_CANCELED) {
        DVDClose(&readStat->dvdFile);
        readStat->status = 0;
        return;
    }
    // Every result other than cancellation advances progress, including read errors.
    readStat->readOfs += HU_DVD_BLOCKSIZE;
    if(readStat->readLen > readStat->readOfs) {
        u32 readSize = readStat->readLen-readStat->readOfs;
        if(readSize > HU_DVD_BLOCKSIZE) {
            readSize = HU_DVD_BLOCKSIZE;
        }
        // Continue writing at the next byte in the archive buffer.
        DVDReadAsyncPrio(dvdFileInfo, ((u8 *) readStat->dirP) + readStat->readOfs,
                         OSRoundUp32B(readSize), readStat->readOfs, HuDataDirReadAsyncCallBack, 3);
    } else {
        readStat->status = 0;
        DVDClose(&readStat->dvdFile);
    }
}

// Cancels an archive read, waits for its callback to finish, then closes its archive.
void HuDataDirCancel(s16 statId)
{
    s32 cancelResult = DVDCancel(&ReadDataStat[statId].dvdFile.cb);
    // The cancel result is unused; the callback's status determines when the wait ends.
    while(ReadDataStat[statId].status);
    HuDataDirClose(ReadDataStat[statId].dirId << 16);
}

// Requests cancellation of an archive read without waiting for completion.
void HuDataDirCancelAsync(s16 statId)
{
    DVDCancelAsync(&ReadDataStat[statId].dvdFile.cb, NULL);
}

// Closes the archive for a completed async read; returns FALSE while it is still active.
BOOL HuDataDirCloseAsync(s16 statId)
{
    if(ReadDataStat[statId].status) {
        return FALSE;
    }
    HuDataDirClose(ReadDataStat[statId].dirId << 16);
    return TRUE;
}

// Character motion and board loaders request an archive asynchronously; ARAM copies return a shared
// sentinel, while resident archives return no slot.
s32 HuDataDirReadAsync(int dataNum)
{
    HUDATASTAT *readStat;
    s32 statId;
    s32 dirId;
    dirId  = dataNum >> 16;
    if(DataDirMax <= dirId) {
        OSReport("data.c: Data Number Error(%d)\n", dataNum);
        return HU_DATA_STAT_NONE;
    }
    if((statId = HuDataReadChk(dataNum)) < 0) {
        AMEM_PTR aramArchiveAddress;
        if(aramArchiveAddress = HuARDirCheck(dataNum)) {
            OSReport("ARAM data num %x\n", dataNum);
            HuAR_ARAMtoMRAM(aramArchiveAddress);
            statId = STAT_ID_ARAM;
        } else {
            statId = HuDataReadStatusGet();
            if(statId == HU_DATA_STAT_NONE) {
                OSReport("data.c: Data Work Max Error\n");
                return HU_DATA_STAT_NONE;
            }
            readStat = &ReadDataStat[statId];
            readStat->status = 1;
            readStat->dirId = dirId;
            readStat->dirP = HuDvdDataFastReadAsync(DataDirStat[dirId].dvdEntryNumber, readStat);
        }
    } else {
        statId = HU_DATA_STAT_NONE;
    }
    return statId;
}

// Numbered asynchronous readers start a tagged DVD read for a missing archive when a slot is free.
// This path skips ARAM lookup and returns no slot when the archive is already resident.
s32 HuDataDirReadNumAsync(int dataNum, s32 memoryTag)
{
    HUDATASTAT *readStat;
    s32 statId;
    s32 dirId;
    dirId  = dataNum >> 16;
    if(DataDirMax <= dirId) {
        OSReport("data.c: Data Number Error(%d)\n", dataNum);
        return HU_DATA_STAT_NONE;
    }
    if((statId = HuDataReadChk(dataNum)) < 0) {
        if((statId = HuDataReadStatusGet()) == HU_DATA_STAT_NONE) {
            OSReport("data.c: Data Work Max Error\n");
            return HU_DATA_STAT_NONE;
        }
        ReadDataStat[statId].status = TRUE;
        ReadDataStat[statId].dirId = dirId;
        readStat = &ReadDataStat[statId];
        readStat->used = TRUE;
        readStat->num = memoryTag;
        // The tag tracks archive closure; the asynchronous DVD buffer itself is unnumbered.
        readStat->dirP = HuDvdDataFastReadAsync(DataDirStat[dirId].dvdEntryNumber, readStat);
    } else {
        statId = HU_DATA_STAT_NONE;
    }
    return statId;
}

// Asynchronous loaders poll a DVD slot or the shared ARAM sentinel for completion.
BOOL HuDataGetAsyncStat(s32 statId)
{
    if(statId == STAT_ID_ARAM) {
        // The ARAM sentinel waits for all queued DMA, rather than just this archive.
        return HuARDMACheck() == 0;
    } else {
        return ReadDataStat[statId].status == 0;
    }
}

// Entry readers select the loaded archive's payload and record its decoded size and format.
static void GetFileInfo(HUDATASTAT *readStat, s32 fileNum)
{
    u32 *entryOffset;
    entryOffset = (u32 *)PTR_OFFSET(readStat->dirP, (fileNum * 4))+1;
    readStat->fileDataP = PTR_OFFSET(readStat->dirP, *entryOffset);
    entryOffset = readStat->fileDataP;
    readStat->rawLen = *entryOffset++;
    readStat->decodeType = *entryOffset++;
    readStat->fileDataP = entryOffset;
}

// Called by game systems that need decoded data; loads one entry into the default heap.
void *HuDataRead(int dataNum)
{
    HUDATASTAT *readStat;
    s32 statusIndex;
    void *decodedData;
    if(!HuDataDirRead(dataNum)) {
        return NULL;
    }
    if((statusIndex = HuDataReadChk(dataNum)) == HU_DATA_STAT_NONE) {
        return NULL;
    }
    readStat = &ReadDataStat[statusIndex];
    GetFileInfo(readStat, dataNum & 0xFFFF);
    decodedData = HuMemDirectMalloc(HEAP_HEAP, DATA_EFF_SIZE(readStat->rawLen));
    if(decodedData) {
        HuDecodeData(readStat->fileDataP, decodedData, readStat->rawLen, readStat->decodeType);
        HuMemMemoryFileSet(decodedData, dataNum);
    }
    return decodedData;
}

// Called by resource loaders that track allocations by group; decodes one entry into that group.
void *HuDataReadNum(int dataNum, s32 memoryTag)
{
    HUDATASTAT *readStat;
    s32 statId;
    void *decodedData;
    if(!HuDataDirReadNum(dataNum, memoryTag)) {
        return NULL;
    }
    if((statId = HuDataReadChk(dataNum)) == HU_DATA_STAT_NONE) {
        return NULL;
    }
    readStat = &ReadDataStat[statId];
    GetFileInfo(readStat, dataNum & 0xFFFF);
    decodedData = HuMemDirectMallocNum(HEAP_HEAP, DATA_EFF_SIZE(readStat->rawLen), memoryTag);
    if(decodedData) {
        HuDecodeData(readStat->fileDataP, decodedData, readStat->rawLen, readStat->decodeType);
        HuMemMemoryFileSet(decodedData, dataNum);
    }
    return decodedData;
}

// Called by resource loaders that choose a heap; decodes one entry into that heap.
void *HuDataSelHeapRead(int dataNum, HEAPID heap)
{
    HUDATASTAT *readStat;
    s32 statId;
    void *decodedData;
    if(!HuDataDirRead(dataNum)) {
        return NULL;
    }
    if((statId = HuDataReadChk(dataNum)) == HU_DATA_STAT_NONE) {
        return NULL;
    }
    readStat = &ReadDataStat[statId];
    GetFileInfo(readStat, dataNum & 0xFFFF);
    switch(heap) {
        case HEAP_SOUND:
            decodedData = HuMemDirectMalloc(HEAP_SOUND, DATA_EFF_SIZE(readStat->rawLen));
            break;

        case HEAP_MODEL:
            decodedData = HuMemDirectMalloc(HEAP_MODEL, DATA_EFF_SIZE(readStat->rawLen));
            break;

        case HEAP_DVD:
            decodedData = HuMemDirectMalloc(HEAP_DVD, DATA_EFF_SIZE(readStat->rawLen));
            break;

        default:
            decodedData = HuMemDirectMalloc(HEAP_HEAP, DATA_EFF_SIZE(readStat->rawLen));
            break;
    }
    if(decodedData) {
        HuDecodeData(readStat->fileDataP, decodedData, readStat->rawLen, readStat->decodeType);
        HuMemMemoryFileSet(decodedData, dataNum);
    }
    return decodedData;
}

// Called by resource loaders that choose a heap and memory tag; decodes one entry for them.
void *HuDataSelHeapReadNum(int dataNum, s32 memoryTag, HEAPID heap)
{
    HUDATASTAT *readStat;
    s32 statId;
    void *decodedData;
    if(!HuDataDirReadNum(dataNum, memoryTag)) {
        return NULL;
    }
    if((statId = HuDataReadChk(dataNum)) == HU_DATA_STAT_NONE) {
        return NULL;
    }
    readStat = &ReadDataStat[statId];
    GetFileInfo(readStat, dataNum & 0xFFFF);
    switch(heap) {
        case HEAP_SOUND:
            // Sound-heap buffers are unnumbered even when a memory tag was requested.
            decodedData = HuMemDirectMalloc(HEAP_SOUND, DATA_EFF_SIZE(readStat->rawLen));
            break;

        case HEAP_MODEL:
            decodedData =
                HuMemDirectMallocNum(HEAP_MODEL, DATA_EFF_SIZE(readStat->rawLen), memoryTag);
            break;

        case HEAP_DVD:
            decodedData =
                HuMemDirectMallocNum(HEAP_DVD, DATA_EFF_SIZE(readStat->rawLen), memoryTag);
            break;

        default:
            decodedData =
                HuMemDirectMallocNum(HEAP_HEAP, DATA_EFF_SIZE(readStat->rawLen), memoryTag);
            break;
    }
    if(decodedData) {
        HuDecodeData(readStat->fileDataP, decodedData, readStat->rawLen, readStat->decodeType);
        HuMemMemoryFileSet(decodedData, dataNum);
    }
    return decodedData;
}

// Called by game systems loading several assets together; reads a sentinel-terminated list.
void **HuDataReadMulti(int *dataNumbers)
{
    return HuDataReadMultiSub(dataNumbers, FALSE, 0);
}

// HuDataReadMulti loads each missing archive once, then decodes the requested entries in order.
static void **HuDataReadMultiSub(int *dataNumbers, BOOL useMemoryTag, s32 memoryTag)
{
    s32 *archiveIds;
    char **archivePaths;
    void **loadedArchives;
    void **decodedFiles;
    s32 fileIndex, missingArchiveCount, numFiles;
    u32 archiveId;
    for (fileIndex = 0, missingArchiveCount = 0; dataNumbers[fileIndex] != HU_DATANUM_NONE;
         fileIndex++) {
        archiveId = dataNumbers[fileIndex] >> 16;
        if(DataDirMax <= archiveId) {
            OSReport("data.c: Data Number Error(%d)\n", dataNumbers[fileIndex]);
            return NULL;
        }
        if(HuDataReadChk(dataNumbers[fileIndex]) < 0) {
            missingArchiveCount++;
        }
    }
    numFiles = fileIndex;
    archiveIds = HuMemDirectMalloc(HEAP_HEAP, (missingArchiveCount+1)*sizeof(s32));
    for(fileIndex=0; fileIndex<missingArchiveCount+1; fileIndex++) {
        archiveIds[fileIndex] = HU_DATANUM_NONE;
    }
    archivePaths = HuMemDirectMalloc(HEAP_HEAP, (missingArchiveCount+1)*sizeof(char *));
    for (fileIndex = 0, missingArchiveCount = 0; dataNumbers[fileIndex] != HU_DATANUM_NONE;
         fileIndex++) {
        archiveId = dataNumbers[fileIndex] >> 16;
        if(HuDataReadChk(dataNumbers[fileIndex]) < 0) {
            s32 archiveSearchIndex;
            for (archiveSearchIndex = 0; archiveIds[archiveSearchIndex] != HU_DATANUM_NONE;
                 archiveSearchIndex++) {
                if(archiveIds[archiveSearchIndex] == archiveId){
                    break;
                }
            }
            if(archiveIds[archiveSearchIndex] == HU_DATANUM_NONE) {
                archiveIds[archiveSearchIndex] = archiveId;
                archivePaths[missingArchiveCount++] = DataDirStat[archiveId].archivePath;
            }
        }
    }
    // The DVD helper scans paths until NULL, but this list's terminator is not written here.
    loadedArchives = HuDvdDataReadMulti(archivePaths);
    for(fileIndex=0; archiveIds[fileIndex] != HU_DATANUM_NONE; fileIndex++) {
        s32 statId;
        if((statId = HuDataReadStatusGet()) == HU_DATA_STAT_NONE) {
            OSReport("data.c: Data Work Max Error\n");
            (void)missingArchiveCount; // Discard the count; only the ID and path lists are freed.
            HuMemDirectFree(archiveIds);
            HuMemDirectFree(archivePaths);
            return NULL;
        } else {
            ReadDataStat[statId].dirP = loadedArchives[fileIndex];
            ReadDataStat[statId].dirId = archiveIds[fileIndex];
        }
    }
    HuMemDirectFree(archiveIds);
    HuMemDirectFree(archivePaths);
    HuMemDirectFree(loadedArchives);
    if(useMemoryTag) {
        decodedFiles = HuMemDirectMallocNum(HEAP_HEAP, (numFiles+1)*sizeof(void *), memoryTag);
    } else {
        decodedFiles = HuMemDirectMalloc(HEAP_HEAP, (numFiles+1)*sizeof(void *));
    }
    for(fileIndex=0; dataNumbers[fileIndex] != HU_DATANUM_NONE; fileIndex++) {
        if(useMemoryTag) {
            decodedFiles[fileIndex] = HuDataReadNum(dataNumbers[fileIndex], memoryTag);
        } else {
            decodedFiles[fileIndex] = HuDataRead(dataNumbers[fileIndex]);
        }
    }
    decodedFiles[fileIndex] = NULL;
    return decodedFiles;
}

// Returns the decoded size of a loaded archive entry, rounded to an even byte count.
s32 HuDataGetSize(int dataNum)
{
    HUDATASTAT *readStat;
    s32 statId;
    if((statId = HuDataReadChk(dataNum)) == HU_DATA_STAT_NONE) {
        return -1;
    }
    readStat = &ReadDataStat[statId];
    GetFileInfo(readStat, dataNum & 0xFFFF);
    return DATA_EFF_SIZE(readStat->rawLen);
}

// Frees one decoded data buffer.
void HuDataClose(void *ptr)
{
    if(ptr) {
        HuMemDirectFree(ptr);
    }
}

// Resource owners release a NULL-terminated buffer list; the list pointer itself must be valid.
void HuDataCloseMulti(void **ptrs)
{
    s32 bufferIndex;
    for(bufferIndex=0; ptrs[bufferIndex]; bufferIndex++) {
        void *dataBuffer = ptrs[bufferIndex];
        if(dataBuffer) {
            HuMemDirectFree(dataBuffer);
        }
    }
    if(ptrs) {
        HuMemDirectFree(ptrs);
    }
}

// Cancels any active read and releases the loaded archive for a data number.
void HuDataDirClose(int dataNum)
{
    HUDATASTAT *readStat;
    s32 statusIndex;
    s32 archiveId = dataNum >> 16;
    for(statusIndex=0; statusIndex<DATA_MAX_READSTAT; statusIndex++) {
        if(ReadDataStat[statusIndex].dirId == archiveId) {
            break;
        }
    }
    if(statusIndex >= DATA_MAX_READSTAT) {
        return;
    }
    readStat = &ReadDataStat[statusIndex];
    if(readStat->status == 1) {
        DVDCancel(&ReadDataStat[statusIndex].dvdFile.cb);
        while(ReadDataStat[statusIndex].status);
    }
    readStat->dirId = HU_DATANUM_NONE;
    HuDvdDataClose(readStat->dirP);
    readStat->dirP = NULL;
    readStat->used = FALSE;
    readStat->status = 0;
}

// Called when a memory tag is released; closes every archive tracked under that tag.
void HuDataDirCloseNum(s32 memoryTag)
{
    s32 statusIndex;
    for(statusIndex=0; statusIndex<DATA_MAX_READSTAT; statusIndex++) {
        if(ReadDataStat[statusIndex].used == TRUE && ReadDataStat[statusIndex].num == memoryTag) {
            HuDataDirClose(ReadDataStat[statusIndex].dirId << 16);
        }
    }
}

// Cancels outstanding DVD reads and releases every tracked archive.
void HuDataDirCloseAll(void)
{
    HUDATASTAT *readStat;
    s32 statusIndex;
    for(statusIndex=0; statusIndex<DATA_MAX_READSTAT; statusIndex++) {
        if(ReadDataStat[statusIndex].dirId != -1) {
            readStat = &ReadDataStat[statusIndex];
            if(readStat->status == 1) {
                DVDCancel(&ReadDataStat[statusIndex].dvdFile.cb);
                while(ReadDataStat[statusIndex].status);
            }
            readStat->dirId = HU_DATANUM_NONE;
            HuDvdDataClose(readStat->dirP);
            readStat->dirP = NULL;
            readStat->used = FALSE;
            readStat->status = 0;
        }
    }

}

// Short forced reads open the containing archive directly, without using the resident pool.
static BOOL HuDataDVDdirDirectOpen(int dataNum, DVDFileInfo *fileInfo)
{
	s32 archiveId = dataNum >> 16;
	if(archiveId >= (s32)DataDirMax) {
		OSReport("data.c: Data Number Error(0x%08x)\n", dataNum);
		return FALSE;
	}
	if(!DVDFastOpen(DataDirStat[archiveId].dvdEntryNumber, fileInfo)) {
		char panicMessage[48];
		sprintf(panicMessage, "HuDataDVDdirDirectOpen: File Open Error(%08x)", dataNum);
		OSPanic("data.c", 951, panicMessage);
	}
	return TRUE;
}

// Short forced reads wait for a DVD range, yielding frames only when shortAccessSleep is set.
// The returned value is the submission result, not the number of bytes read.
static s32 HuDataDVDdirDirectRead(DVDFileInfo *fileInfo, void *readBuffer, s32 readBytes,
                                  s32 readOffset)
{
	s32 result = DVDReadAsync(fileInfo, readBuffer, readBytes, readOffset, NULL);
	if(result != 1) {
		OSPanic("data.c", 960, "HuDataDVDdirDirectRead: File Read Error");
	}
	while(DVDGetCommandBlockStatus(&fileInfo->cb)) {
		if(shortAccessSleep) {
			HuPrcVSleep();
		}
	}
	return result;
}

// Called by HuDataReadNumHeapShortForce after its DVD read; decodes the entry into the chosen heap.
static void *HuDataDecodeIt(u8 *alignedReadBuffer, s32 entryHeaderOffset, s32 memoryTag,
                            HEAPID targetHeap)
{
    void *encodedData;
    s32 *entryHeader;
    s32 decodedSize, decodeType;

    void *decodedData;
    // The selected entry header may be unaligned within the aligned DVD read buffer.
    entryHeader = (s32 *)&alignedReadBuffer[entryHeaderOffset];
    if((u32)entryHeader & 0x3) {
        u8 *headerByte = (u8 *)entryHeader;
        decodedSize = *headerByte++ << 24;
        decodedSize += *headerByte++ << 16;
        decodedSize += *headerByte++ << 8;
        decodedSize += *headerByte++;
        decodeType = *headerByte++ << 24;
        decodeType += *headerByte++ << 16;
        decodeType += *headerByte++ << 8;
        decodeType += *headerByte++;
        encodedData = headerByte;
    } else {
        s32 *headerWord = entryHeader;
        decodedSize = *headerWord++;
        decodeType = *headerWord++;
        encodedData = headerWord;
    }
    switch(targetHeap) {
        case HEAP_SOUND:
            // Sound buffers are unnumbered here, as in the selected-heap entry reader.
            decodedData = HuMemDirectMalloc(HEAP_SOUND, DATA_EFF_SIZE(decodedSize));
            break;

        case HEAP_MODEL:
            decodedData = HuMemDirectMallocNum(HEAP_MODEL, DATA_EFF_SIZE(decodedSize), memoryTag);
            break;

        case HEAP_DVD:
            decodedData = HuMemDirectMallocNum(HEAP_DVD, DATA_EFF_SIZE(decodedSize), memoryTag);
            break;

        default:
            decodedData = HuMemDirectMallocNum(HEAP_HEAP, DATA_EFF_SIZE(decodedSize), memoryTag);
            break;
    }
    if(decodedData) {
        HuDecodeData(encodedData, decodedData, decodedSize, decodeType);
    }
    return decodedData;
}

// Character motion loading reads just the selected entry from DVD, then decodes into its heap.
void *HuDataReadNumHeapShortForce(u32 dataNum, s32 memoryTag, HEAPID targetHeap)
{
	DVDFileInfo fileInfo;
	s32 *entryOffset;
	s32 *archiveHeader;
	void *encodedReadBuffer;
    s32 fileNum;
	s32 readLengthBytes;

	s32 entryOffsetBytes;
	s32 dvdReadOffsetBytes;
	s32 entrySpanOrOffsetBytes;
	void *decodedData;
	s32 archiveId;
	s32 archiveHeaderReadBytes;
	s32 archiveFileCount;

	if(!HuDataDVDdirDirectOpen(dataNum, &fileInfo)) {
		return NULL;
	}
	archiveId = dataNum >> 16;
	fileNum = dataNum & 0xFFFF;
    OSReport("Dir:%d file:%d\n", archiveId, fileNum);
	// Read through the following offset so a non-final entry's encoded span can be found.
	entryOffsetBytes = (fileNum*4)+4;
	archiveHeaderReadBytes = OSRoundUp32B(entryOffsetBytes+8);
	archiveHeader = HuMemDirectMalloc(HEAP_HEAP, archiveHeaderReadBytes);
	if(!HuDataDVDdirDirectRead(&fileInfo, archiveHeader, archiveHeaderReadBytes, 0)) {
		HuMemDirectFree(archiveHeader);
		DVDClose(&fileInfo);
		return NULL;
	}

	archiveFileCount = *archiveHeader;
	if(archiveFileCount <= fileNum) {
		HuMemDirectFree(archiveHeader);
		OSReport("data.c%d: Data Number Error(0x%08x)\n", 1061, dataNum);
		DVDClose(&fileInfo);
		return NULL;
	}
	entryOffset = archiveHeader;
	entryOffset += fileNum+1;
	entryOffsetBytes = *entryOffset;
	dvdReadOffsetBytes = OSRoundDown32B(entryOffsetBytes);
	// DVD reads begin at a 32-byte boundary and extend to the next entry or archive end.
	if(archiveFileCount <= fileNum+1) {
		readLengthBytes = fileInfo.length;
		entrySpanOrOffsetBytes = readLengthBytes-dvdReadOffsetBytes;
	} else {
		entryOffset++;
		entrySpanOrOffsetBytes = (*entryOffset)-dvdReadOffsetBytes;
		readLengthBytes = fileInfo.length;
	}

	readLengthBytes = OSRoundUp32B(entrySpanOrOffsetBytes);
	HuMemDirectFree(archiveHeader);
	encodedReadBuffer = HuMemDirectMalloc(HEAP_HEAP, (readLengthBytes+4) & DATA_BUFFER_SIZE_MASK);
	if(encodedReadBuffer == NULL) {
		OSReport("data.c: couldn't allocate read buffer(0x%08x)\n", dataNum);
		DVDClose(&fileInfo);
		return NULL;
	}
	if(!HuDataDVDdirDirectRead(&fileInfo, encodedReadBuffer, readLengthBytes, dvdReadOffsetBytes)) {
		HuMemDirectFree(encodedReadBuffer);
		DVDClose(&fileInfo);
		return NULL;
	}
	DVDClose(&fileInfo);
	// Reuse the span variable for the entry header's byte offset inside the aligned buffer.
	entrySpanOrOffsetBytes = entryOffsetBytes-dvdReadOffsetBytes;
	decodedData = HuDataDecodeIt(encodedReadBuffer, entrySpanOrOffsetBytes, memoryTag, targetHeap);
    HuMemMemoryFileSet(decodedData, dataNum);
	HuMemDirectFree(encodedReadBuffer);
    return decodedData;
}

char lbl_8011FDA6[] = "** dcnt %d tmp %08x sp1 %08x\n";
char lbl_8011FDC4[] = "** dcnt %d lastNum %08x\n";
