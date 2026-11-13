// Loads, tracks, decodes, and closes files from the game's data archives.
#include "game/data.h"
#include "game/dvd.h"

#include "game/armem.h"
#include "game/process.h"
#include "dolphin/dvd.h"

#define STAT_ID_ARAM 0x10000
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

u32 lbl_802C0028; // DVD-reported file length, or a resident slot's dvdFile.length; ARAM uploads
                 // round it to 32 bytes.
// Synchronous and ARAM archive loads do not populate the slot's dvdFile.length.
static u32 DataDirMax; // Number of archives in the path table, excluding its NULL terminator.
static s32 shortAccessSleep; // Yield flag for direct DVD waits; it stays zero because this file
                             // never sets it.
static HUDATASTAT ATTRIBUTE_ALIGN(32) ReadDataStat[DATA_MAX_READSTAT]; // Active archive pool.

// Game startup resolves the archive paths and clears the fixed pool of active archive reads.
void fn_80007668(void)
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
        readStat->dirId = -1;
        readStat->used = FALSE;
        readStat->status = 0;
    }
}

// Archive-loading helpers use this to find the first unused slot in the read pool.
static s32 HuDataReadStatusGet(void)
{
    s32 statusIndex;
    for(statusIndex=0; statusIndex<DATA_MAX_READSTAT; statusIndex++) {
        if(ReadDataStat[statusIndex].dirId == -1) {
            break;
        }
    }
    if(statusIndex >= DATA_MAX_READSTAT) {
        statusIndex = -1;
    }
    return statusIndex;
}

// Resource loaders find a resident archive by the upper half of a packed data number.
s32 fn_80007738(int dataNum)
{
    s32 statusIndex;
    dataNum >>= 16;
    for(statusIndex=0; statusIndex<DATA_MAX_READSTAT; statusIndex++) {
        if(ReadDataStat[statusIndex].dirId == dataNum && ReadDataStat[statusIndex].status != 1) {
            break;
        }
    }
    if(statusIndex >= DATA_MAX_READSTAT) {
        statusIndex = -1;
    }
    return statusIndex;
}

// Character and ARAM loaders use a registered archive pointer to find its status record.
HUDATASTAT *fn_800077AC(void *archivePtr)
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
void *fn_80007810(int dirNum)
{
    s32 statId = fn_80007738(dirNum);
    if(statId < 0) {
        return NULL;
    }
    return ReadDataStat[statId].dirP;
}

// Entry readers and ARAM loaders obtain the containing archive, waiting for a queued ARAM copy.
HUDATASTAT *fn_800078C8(int dataNum)
{
    HUDATASTAT *readStat;
    s32 statId;
    s32 dirId;
    dirId  = dataNum >> 16;
    if(DataDirMax <= dirId) {
        OSReport("data.c: Data Number Error(%d)\n", dataNum);
        return NULL;
    }

    if((statId = fn_80007738(dataNum)) < 0) {
        AMEM_PTR aramArchiveAddress;
        if(aramArchiveAddress = HuARDirCheck(dataNum)) {
            fn_80055D40(aramArchiveAddress);
            while(fn_80055FB4());
            statId = fn_80007738(dataNum);
            readStat = &ReadDataStat[statId];
        } else {
            if((statId = HuDataReadStatusGet()) == -1) {
                OSReport("data.c: Data Work Max Error\n");
                return NULL;
            }
            readStat = &ReadDataStat[statId];
            readStat->dirP = HuDvdDataFastRead(DataDirStat[dirId].dvdEntryNumber);
            if(readStat->dirP) {
                fn_8000C164(readStat->dirP, dataNum & DATA_DIR_ID_MASK);
                readStat->dirId = dirId;
            }
        }
    } else {
        readStat = &ReadDataStat[statId];
        lbl_802C0028 = readStat->dvdFile.length;
    }
    return readStat;
}

// Called by numbered readers; loads missing archives under the requested tag, but reuses resident
// archives without changing their tag.
HUDATASTAT *HuDataDirReadNum(int dataNum, s32 memoryTag)
{
    HUDATASTAT *readStat;
    s32 statId;
    s32 dirId;
    dirId  = dataNum >> 16;
    if(DataDirMax <= dirId) {
        OSReport("data.c: Data Number Error(%d)\n", dataNum);
        return NULL;
    }

    if((statId = fn_80007738(dataNum)) < 0) {
        AMEM_PTR aramArchiveAddress;
        if((aramArchiveAddress = HuARDirCheck(dataNum))) {
            OSReport("ARAM data num %x\n", dataNum);
            fn_80055D6C(aramArchiveAddress, memoryTag);
            while(fn_80055FB4());
            statId = fn_80007738(dataNum);
            readStat = &ReadDataStat[statId];
            readStat->used = TRUE;
            readStat->num = memoryTag;
        } else {
            OSReport("data num %x\n", dataNum);
            if((statId = HuDataReadStatusGet()) == -1) {
                OSReport("data.c: Data Work Max Error\n");
                return NULL;
            }
            readStat = &ReadDataStat[statId];
            readStat->dirP = HuDvdDataFastReadNum(DataDirStat[dirId].dvdEntryNumber, memoryTag);
            if(readStat->dirP) {
                fn_8000C164(readStat->dirP, dataNum & DATA_DIR_ID_MASK);
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
// Its close test uses the pointer's existing record's dirId, but the close call uses dataNum;
// an untracked pointer follows HuDataGetStatus's one-past-pool path before a slot is chosen.
HUDATASTAT *fn_80007B00(void *archivePtr, int dataNum)
{
    HUDATASTAT *readStat = fn_800077AC(archivePtr);
    s32 statId;
    if((statId = fn_80007738(readStat->dirId << 16)) >= 0) {
        fn_800097F4(dataNum);
    }
    if((statId = HuDataReadStatusGet()) == -1) {
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
void fn_80007C88(s32 dvdResult, DVDFileInfo* dvdFileInfo)
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
        DVDReadAsyncPrio(dvdFileInfo, readStat->dirBytes + readStat->readOfs,
                         OSRoundUp32B(readSize), readStat->readOfs, fn_80007C88, 3);
    } else {
        readStat->status = 0;
        DVDClose(&readStat->dvdFile);
    }
}

// Cancels an archive read, waits for its callback to finish, then closes its archive.
void fn_80007DE4(s16 statId)
{
    s32 cancelResult = DVDCancel(&ReadDataStat[statId].dvdFile.cb);
    // The cancel result is unused; the callback's status determines when the wait ends.
    while(ReadDataStat[statId].status);
    fn_800097F4(ReadDataStat[statId].dirId << 16);
}

// Requests cancellation of an archive read without waiting for completion.
void fn_80007E6C(s16 statId)
{
    DVDCancelAsync(&ReadDataStat[statId].dvdFile.cb, NULL);
}

// Closes the archive for a completed async read; returns FALSE while it is still active.
BOOL fn_80007EAC(s16 statId)
{
    if(ReadDataStat[statId].status) {
        return FALSE;
    }
    fn_800097F4(ReadDataStat[statId].dirId << 16);
    return TRUE;
}

// Character motion and board loaders request an archive asynchronously; ARAM copies return a shared
// sentinel, while resident archives return no slot.
s32 fn_80007F20(int dataNum)
{
    HUDATASTAT *readStat;
    s32 statId;
    s32 dirId;
    dirId  = dataNum >> 16;
    if(DataDirMax <= dirId) {
        OSReport("data.c: Data Number Error(%d)\n", dataNum);
        return -1;
    }
    if((statId = fn_80007738(dataNum)) < 0) {
        AMEM_PTR aramArchiveAddress;
        if(aramArchiveAddress = HuARDirCheck(dataNum)) {
            OSReport("ARAM data num %x\n", dataNum);
            fn_80055D40(aramArchiveAddress);
            statId = STAT_ID_ARAM;
        } else {
            statId = HuDataReadStatusGet();
            if(statId == -1) {
                OSReport("data.c: Data Work Max Error\n");
                return -1;
            }
            readStat = &ReadDataStat[statId];
            readStat->status = 1;
            readStat->dirId = dirId;
            readStat->dirP = HuDvdDataFastReadAsync(DataDirStat[dirId].dvdEntryNumber, readStat);
        }
    } else {
        statId = -1;
    }
    return statId;
}

// Numbered asynchronous readers track archive closure under the requested tag while starting an
// unnumbered DVD read.
s32 fn_800080C8(int dataNum, s32 memoryTag)
{
    HUDATASTAT *readStat;
    s32 statId;
    s32 dirId;
    dirId  = dataNum >> 16;
    if(DataDirMax <= dirId) {
        OSReport("data.c: Data Number Error(%d)\n", dataNum);
        return -1;
    }
    if((statId = fn_80007738(dataNum)) < 0) {
        if((statId = HuDataReadStatusGet()) == -1) {
            OSReport("data.c: Data Work Max Error\n");
            return -1;
        }
        ReadDataStat[statId].status = TRUE;
        ReadDataStat[statId].dirId = dirId;
        readStat = &ReadDataStat[statId];
        readStat->used = TRUE;
        readStat->num = memoryTag;
        // The tag tracks archive closure; the asynchronous DVD buffer itself is unnumbered.
        readStat->dirP = HuDvdDataFastReadAsync(DataDirStat[dirId].dvdEntryNumber, readStat);
    } else {
        statId = -1;
    }
    return statId;
}

// Asynchronous loaders poll a DVD slot or the shared ARAM sentinel for completion.
BOOL fn_80008270(s32 statId)
{
    if(statId == STAT_ID_ARAM) {
        // The ARAM sentinel waits for all queued DMA, rather than just this archive.
        return fn_80055FB4() == 0;
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
void *fn_800082D0(int dataNum)
{
    HUDATASTAT *readStat;
    s32 statusIndex;
    void *decodedData;
    if(!fn_800078C8(dataNum)) {
        return NULL;
    }
    if((statusIndex = fn_80007738(dataNum)) == -1) {
        return NULL;
    }
    readStat = &ReadDataStat[statusIndex];
    GetFileInfo(readStat, dataNum & 0xFFFF);
    decodedData = HuMemDirectMalloc(HEAP_HEAP, DATA_EFF_SIZE(readStat->rawLen));
    if(decodedData) {
        fn_8000A384(readStat->fileDataP, decodedData, readStat->rawLen, readStat->decodeType);
        fn_8000C164(decodedData, dataNum);
    }
    return decodedData;
}

// Called by resource loaders that track allocations by group; decodes one entry into that group.
void *fn_80008634(int dataNum, s32 memoryTag)
{
    HUDATASTAT *readStat;
    s32 statId;
    void *decodedData;
    if(!HuDataDirReadNum(dataNum, memoryTag)) {
        return NULL;
    }
    if((statId = fn_80007738(dataNum)) == -1) {
        return NULL;
    }
    readStat = &ReadDataStat[statId];
    GetFileInfo(readStat, dataNum & 0xFFFF);
    decodedData = HuMemDirectMallocNum(HEAP_HEAP, DATA_EFF_SIZE(readStat->rawLen), memoryTag);
    if(decodedData) {
        fn_8000A384(readStat->fileDataP, decodedData, readStat->rawLen, readStat->decodeType);
        fn_8000C164(decodedData, dataNum);
    }
    return decodedData;
}

// Decodes one entry into HEAP_SOUND, HEAP_MODEL, or HEAP_DVD; other heap IDs use HEAP_HEAP.
void *fn_800089EC(int dataNum, HEAPID heap)
{
    HUDATASTAT *readStat;
    s32 statId;
    void *decodedData;
    if(!fn_800078C8(dataNum)) {
        return NULL;
    }
    if((statId = fn_80007738(dataNum)) == -1) {
        return NULL;
    }
    readStat = &ReadDataStat[statId];
    GetFileInfo(readStat, dataNum & 0xFFFF);
    switch(heap) {
        case 1:
            decodedData = HuMemDirectMalloc(1, DATA_EFF_SIZE(readStat->rawLen));
            break;

        case 2:
            decodedData = HuMemDirectMalloc(2, DATA_EFF_SIZE(readStat->rawLen));
            break;

        case 3:
            decodedData = HuMemDirectMalloc(3, DATA_EFF_SIZE(readStat->rawLen));
            break;

        default:
            decodedData = HuMemDirectMalloc(HEAP_HEAP, DATA_EFF_SIZE(readStat->rawLen));
            break;
    }
    if(decodedData) {
        fn_8000A384(readStat->fileDataP, decodedData, readStat->rawLen, readStat->decodeType);
        fn_8000C164(decodedData, dataNum);
    }
    return decodedData;
}

// Decodes into HEAP_SOUND, HEAP_MODEL, or HEAP_DVD; other heap IDs use HEAP_HEAP, and only
// non-sound allocations use the tag.
void *fn_80008DD0(int dataNum, s32 memoryTag, HEAPID heap)
{
    HUDATASTAT *readStat;
    s32 statId;
    void *decodedData;
    if(!HuDataDirReadNum(dataNum, memoryTag)) {
        return NULL;
    }
    if((statId = fn_80007738(dataNum)) == -1) {
        return NULL;
    }
    readStat = &ReadDataStat[statId];
    GetFileInfo(readStat, dataNum & 0xFFFF);
    switch(heap) {
        case 1:
            // Sound-heap buffers are unnumbered even when a memory tag was requested.
            decodedData = HuMemDirectMalloc(1, DATA_EFF_SIZE(readStat->rawLen));
            break;

        case 2:
            decodedData =
                HuMemDirectMallocNum(2, DATA_EFF_SIZE(readStat->rawLen), memoryTag);
            break;

        case 3:
            decodedData =
                HuMemDirectMallocNum(3, DATA_EFF_SIZE(readStat->rawLen), memoryTag);
            break;

        default:
            decodedData =
                HuMemDirectMallocNum(HEAP_HEAP, DATA_EFF_SIZE(readStat->rawLen), memoryTag);
            break;
    }
    if(decodedData) {
        fn_8000A384(readStat->fileDataP, decodedData, readStat->rawLen, readStat->decodeType);
        fn_8000C164(decodedData, dataNum);
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
    s32 fileIndex, missingArchiveCount, numFiles;
    void **decodedFiles;
    u32 archiveId;
    for (fileIndex = 0, missingArchiveCount = 0; dataNumbers[fileIndex] != -1;
         fileIndex++) {
        archiveId = dataNumbers[fileIndex] >> 16;
        if(DataDirMax <= archiveId) {
            OSReport("data.c: Data Number Error(%d)\n", dataNumbers[fileIndex]);
            return NULL;
        }
        if(fn_80007738(dataNumbers[fileIndex]) < 0) {
            missingArchiveCount++;
        }
    }
    numFiles = fileIndex;
    archiveIds = HuMemDirectMalloc(HEAP_HEAP, (missingArchiveCount+1)*sizeof(s32));
    for(fileIndex=0; fileIndex<missingArchiveCount+1; fileIndex++) {
        archiveIds[fileIndex] = -1;
    }
    archivePaths = HuMemDirectMalloc(HEAP_HEAP, (missingArchiveCount+1)*sizeof(char *));
    for (fileIndex = 0, missingArchiveCount = 0; dataNumbers[fileIndex] != -1;
         fileIndex++) {
        archiveId = dataNumbers[fileIndex] >> 16;
        if(fn_80007738(dataNumbers[fileIndex]) < 0) {
            s32 archiveSearchIndex;
            for (archiveSearchIndex = 0; archiveIds[archiveSearchIndex] != -1;
                 archiveSearchIndex++) {
                if(archiveIds[archiveSearchIndex] == archiveId){
                    break;
                }
            }
            if(archiveIds[archiveSearchIndex] == -1) {
                archiveIds[archiveSearchIndex] = archiveId;
                archivePaths[missingArchiveCount++] = DataDirStat[archiveId].archivePath;
            }
        }
    }
    // The DVD helper scans paths until NULL, but this list's terminator is not written here.
    loadedArchives = HuDvdDataReadMulti(archivePaths);
    for(fileIndex=0; archiveIds[fileIndex] != -1; fileIndex++) {
        s32 statId;
        if((statId = HuDataReadStatusGet()) == -1) {
            OSReport("data.c: Data Work Max Error\n");
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
    for(fileIndex=0; dataNumbers[fileIndex] != -1; fileIndex++) {
        if(useMemoryTag) {
            decodedFiles[fileIndex] = fn_80008634(dataNumbers[fileIndex], memoryTag);
        } else {
            decodedFiles[fileIndex] = fn_800082D0(dataNumbers[fileIndex]);
        }
    }
    decodedFiles[fileIndex] = NULL;
    return decodedFiles;
}

// Returns the decoded size of a loaded archive entry, rounded to an even byte count.
s32 fn_8000963C(int dataNum)
{
    HUDATASTAT *readStat;
    s32 statId;
    if((statId = fn_80007738(dataNum)) == -1) {
        return -1;
    }
    readStat = &ReadDataStat[statId];
    GetFileInfo(readStat, dataNum & 0xFFFF);
    return DATA_EFF_SIZE(readStat->rawLen);
}

// Frees one decoded data buffer.
void fn_80009740(void *ptr)
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
void fn_800097F4(int dataNum)
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
    readStat->dirId = -1;
    HuDvdDataClose(readStat->dirP);
    readStat->dirP = NULL;
    readStat->used = FALSE;
    readStat->status = 0;
}

// Called when a memory tag is released; closes every archive tracked under that tag.
void fn_800098E4(s32 memoryTag)
{
    s32 statusIndex;
    for(statusIndex=0; statusIndex<DATA_MAX_READSTAT; statusIndex++) {
        if(ReadDataStat[statusIndex].used == TRUE && ReadDataStat[statusIndex].num == memoryTag) {
            fn_800097F4(ReadDataStat[statusIndex].dirId << 16);
        }
    }
}

// Cancels outstanding DVD reads and releases every tracked archive.
void fn_80009A30(void)
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
            readStat->dirId = -1;
            HuDvdDataClose(readStat->dirP);
            readStat->dirP = NULL;
            readStat->used = FALSE;
            readStat->status = 0;
        }
    }

}

// Rejects archive indices at or above DataDirMax, then opens the DVD entry; open failures panic.
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

// Submits a direct DVD range read and returns its submission result after command state 0.
// Nonzero states, including errors, keep the wait running; shortAccessSleep controls yielding.
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

// Decodes into HEAP_SOUND, HEAP_MODEL, or HEAP_DVD; other heap IDs use HEAP_HEAP, and only
// non-sound allocations use the tag.
static void *HuDataDecodeIt(u8 *alignedReadBuffer, s32 entryHeaderOffset, s32 memoryTag,
                            HEAPID targetHeap)
{
    void *encodedData;
    void *entryHeader;
    s32 decodedSize, decodeType;

    void *decodedData;
    // The selected entry header may be unaligned within the aligned DVD read buffer.
    entryHeader = &alignedReadBuffer[entryHeaderOffset];
    if((u32)entryHeader & 0x3) {
        u8 *headerByte = entryHeader;
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
        case 1:
            // Sound buffers are unnumbered here, as in the selected-heap entry reader.
            decodedData = HuMemDirectMalloc(1, DATA_EFF_SIZE(decodedSize));
            break;

        case 2:
            decodedData = HuMemDirectMallocNum(2, DATA_EFF_SIZE(decodedSize), memoryTag);
            break;

        case 3:
            decodedData = HuMemDirectMallocNum(3, DATA_EFF_SIZE(decodedSize), memoryTag);
            break;

        default:
            decodedData = HuMemDirectMallocNum(HEAP_HEAP, DATA_EFF_SIZE(decodedSize), memoryTag);
            break;
    }
    if(decodedData) {
        fn_8000A384(encodedData, decodedData, decodedSize, decodeType);
    }
    return decodedData;
}

// Character motion loading reads an aligned DVD span containing the selected entry, then decodes
// into HEAP_SOUND, HEAP_MODEL, or HEAP_DVD; other heap IDs use HEAP_HEAP.
void *fn_80009B04(u32 dataNum, s32 memoryTag, HEAPID targetHeap)
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
		// This file-length assignment is overwritten by the rounded span below.
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
    fn_8000C164(decodedData, dataNum);
	HuMemDirectFree(encodedReadBuffer);
    return decodedData;
}

char lbl_8011FDA6[] = "** dcnt %d tmp %08x sp1 %08x\n";
char lbl_8011FDC4[] = "** dcnt %d lastNum %08x\n";
