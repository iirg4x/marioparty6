// Reads game files from the DVD and reports changes in the drive status.
#include "game/dvd.h"
#include "game/data.h"

#include "dolphin/dvd.h"
#include "dolphin/os.h"

static int CallBackStatus;
static s32 beforeDvdStatus;

// Marks one chunk complete so the synchronous reader can continue its wait loop.
// The callback result and file info are ignored, including read errors.
static void HuDVDReadAsyncCallBack(s32 ignoredResult, DVDFileInfo* ignoredFileInfo)
{
    CallBackStatus = 1;
}

// Allocates a file buffer, then either reads every chunk here or starts the data loader callback.
// Called by the path-based and fast-entry read helpers below.
static void *HuDvdDataReadWait(DVDFileInfo *dvdFile, int allocationMode, int heapOrAllocationNumber,
                               BOOL readAsync)
{
    u32 fileLength;
    u8 *dataBuffer;
    fileLength = dvdFile->length;
    // Preserve the full file size for callers, including when an asynchronous read starts in
    // chunks.
    DirDataSize = fileLength;
    // Modes select the DVD heap, numbered DVD heap, selected heap, or selected heap tail.
    switch(allocationMode) {
        case 0:
            dataBuffer = HuMemDirectMalloc(HEAP_DVD, OSRoundUp32B(fileLength));
            break;
            
        case 1:
            dataBuffer =
                HuMemDirectMallocNum(HEAP_DVD, OSRoundUp32B(fileLength), heapOrAllocationNumber);
            break;
            
        case 2:
            dataBuffer = HuMemDirectMalloc(heapOrAllocationNumber, OSRoundUp32B(fileLength));
            break;
         
        case 3:
            dataBuffer = HuMemDirectTailMalloc(heapOrAllocationNumber, OSRoundUp32B(fileLength));
            break;
        
        default:
            OSPanic("dvd.c", 58, "dvd.c: HuDvdDataReadWait Mode Error");
            break;
    }
    if(!dataBuffer) {
        OSReport("dvd.c: Memory Allocation Error (Length %x) (mode %d)\n", fileLength,
                 allocationMode);
        OSReport("Rest Memory %x\n", HuRestMemGet(HEAP_DVD));
        OSPanic("dvd.c", 63, "\n");
        return NULL;
    }
    OSReport("Rest Memory %x\n", HuRestMemGet(HEAP_DVD));
    // Read-submission results are ignored; the synchronous path waits for its callback below.
    if(readAsync) {
        if(fileLength > HU_DVD_BLOCKSIZE) {
            fileLength = HU_DVD_BLOCKSIZE;
        }
        DVDReadAsyncPrio(dvdFile, dataBuffer, OSRoundUp32B(fileLength), 0,
                         HuDataDirReadAsyncCallBack, 3);
    } else {
        u32 chunkSize;
        u32 readOffset;
        for(readOffset=chunkSize=0; readOffset<fileLength; readOffset += HU_DVD_BLOCKSIZE) {
            chunkSize = fileLength-readOffset;
            if(chunkSize > HU_DVD_BLOCKSIZE) {
                chunkSize = HU_DVD_BLOCKSIZE;
            }
            CallBackStatus = 0;
            DVDReadAsyncPrio(dvdFile, &dataBuffer[readOffset], OSRoundUp32B(chunkSize),
                             readOffset, HuDVDReadAsyncCallBack, 2);
            while(CallBackStatus == 0) {
                HuDvdErrorWatch();
            }
            HuDvdErrorWatch();
        }
    }
    
    return dataBuffer;
}

// Opens and fully reads a named file; window.c uses this for the selected language's messages.
void *HuDvdDataRead(char *filePath)
{
    DVDFileInfo dvdFile;
    void *fileData = NULL;
    if(!DVDOpen(filePath, &dvdFile)) {
        OSPanic("dvd.c", 109, "dvd.c: File Open Error");
    } else {
        fileData = HuDvdDataReadWait(&dvdFile, 0, 0, FALSE);
        DVDClose(&dvdFile);
    }
    return fileData;
}

// Reads each null-terminated path into a parallel array; HuDataReadMultiSub uses this for
// directories.
void **HuDvdDataReadMulti(char **filePaths)
{
    DVDFileInfo dvdFile;
    int pathIndex;
    u32 pathCount;
    void **fileDataArray;
    pathCount = 0;
    while(filePaths[pathCount]) {
        pathCount++;
    }
    fileDataArray = HuMemDirectMalloc(0, pathCount*sizeof(void *));
    for(pathIndex=0; pathIndex<pathCount; pathIndex++) {
        if(!DVDOpen(filePaths[pathIndex], &dvdFile)) {
            OSPanic("dvd.c", 145, "dvd.c: File Open Error");
            return NULL;
        } else {
            fileDataArray[pathIndex] = HuDvdDataReadWait(&dvdFile, 0, 0, FALSE);
            DVDClose(&dvdFile);
        }
    }
    return fileDataArray;
}

// Reads a named file into the requested heap; objdll.c uses it to load REL modules.
void *HuDvdDataReadDirect(char *filePath, HEAPID targetHeap)
{
    DVDFileInfo dvdFile;
    void *fileData = NULL;
    if(!DVDOpen(filePath, &dvdFile)) {
        OSPanic("dvd.c", 164, "dvd.c: File Open Error");
    } else {
        fileData = HuDvdDataReadWait(&dvdFile, 2, targetHeap, FALSE);
        DVDClose(&dvdFile);
    }
    return fileData;
}

// Reads one directory-table entry into the DVD heap; data.c uses this for directory loads.
void *HuDvdDataFastRead(s32 entryNumber)
{
    DVDFileInfo dvdFile;
    void *fileData = NULL;
    if(!DVDFastOpen(entryNumber, &dvdFile)) {
        OSPanic("dvd.c", 205, "dvd.c: File Open Error");
    } else {
        fileData = HuDvdDataReadWait(&dvdFile, 0, 0, FALSE);
        DVDClose(&dvdFile);
    }
    return fileData;
}

// Reads a directory-table entry using allocationTag for its DVD heap block; called by data.c.
void *HuDvdDataFastReadNum(s32 entryNumber, s32 allocationTag)
{
    DVDFileInfo dvdFile;
    void *fileData = NULL;
    if(!DVDFastOpen(entryNumber, &dvdFile)) {
        OSPanic("dvd.c", 220, "dvd.c: File Open Error");
    } else {
        fileData = HuDvdDataReadWait(&dvdFile, 1, allocationTag, FALSE);
        DVDClose(&dvdFile);
    }
    return fileData;
}

// Starts an asynchronous directory read and leaves the open file and progress in its status record.
// Called by data.c; HuDataDirReadAsyncCallBack advances later chunks and closes the file.
void *HuDvdDataFastReadAsync(s32 entryNumber, HUDATASTAT *readStatus)
{
    DVDFileInfo unusedFileInfo; // The open file is stored in readStatus->dvdFile.
    void *fileData = NULL;
    if(!DVDFastOpen(entryNumber, &readStatus->dvdFile)) {
        OSPanic("dvd.c", 236, "dvd.c: File Open Error");
    } else {
        readStatus->readOfs = 0;
        readStatus->readLen = readStatus->dvdFile.length;
        fileData = HuDvdDataReadWait(&readStatus->dvdFile, 0, 0, TRUE);
    }
    return fileData;
}

// Frees a buffer returned by one of the DVD read helpers; data.c uses this when closing directory
// data.
void HuDvdDataClose(void *fileData)
{
    if(fileData) {
        HuMemDirectFree(fileData);
    }
}

// main.c checks the drive each frame; changed disk errors are printed and fatal errors halt the
// game.
void HuDvdErrorWatch()
{
    int driveStatus = DVDGetDriveStatus();
    if(driveStatus == beforeDvdStatus) {
        return;
    }
    beforeDvdStatus = driveStatus;
    switch(driveStatus+1) {
        case DVD_STATE_FATAL_ERROR + 1:
            OSReport("DVD ERROR:Fatal error occurred\n***HALT***");
            while(1);
            break;
            
        case DVD_STATE_NO_DISK + 1:
            OSReport("DVD ERROR:No disk\n");
            break;
            
        case DVD_STATE_COVER_OPEN + 1:
            OSReport("DVD ERROR:Cover open\n");
            break;
            
        case DVD_STATE_WRONG_DISK + 1:
            OSReport("DVD ERROR:Wrong disk\n");
            break;
            
        case DVD_STATE_RETRY + 1:
            OSReport("DVD ERROR:Please retry\n");
            break;
            
        default:
            break;
    }
}
