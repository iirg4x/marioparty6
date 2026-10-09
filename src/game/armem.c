/* Manages ARAM blocks and transfers data between ARAM and main memory. */
#include "game/armem.h"
#include "game/data.h"

#define ARMEM_BLOCK_MAX 64
#define ARQ_OWNER_HU 4660
#define ARMEM_DIRECTORY_ID_MASK 0xFFFF0000
#define ARMEM_NO_DIRECTORY_ID 65535
#define ARMEM_ARCHIVE_ALIGN_DOWN_MASK 0xFFFFFFFE0
#define ARMEM_FILE_SIZE_ALIGN_DOWN_MASK 0xFFFFFFFE
#define ARMEM_FILE_TRANSFER_ROUNDUP_BIAS 63

typedef struct ARMemBlock_s ARMEM_BLOCK;

struct ARMemBlock_s {
    /* 0x00 */ u8 inUse; /* 1 while reserved or allocated, 0 while free; the sentinel is 1. */
    /* 0x02 */ u16 dataDirId; /* Data directory ID, or 0xFFFF when unassociated. */
    /* 0x04 */ AMEM_PTR aramAddress; /* Start address of this range in ARAM. */
    /* 0x08 */ u32 size; /* Length of this ARAM range in bytes. */
    /* 0x0C */ ARMEM_BLOCK *next; /* Next range in address order, ending at the sentinel. */
}; // Size 0x10

typedef struct ARQueReq_s {
    /* 0x00 */ ARQRequest request; /* ARAM-to-main-memory request for the resource copy. */
    /* 0x20 */ s32 dataNum; /* Directory-base data number (directory ID in the upper 16 bits) used
                             * to register the returned archive buffer. */
    /* 0x24 */ void *destination; /* Main-memory buffer filled by the ARAM transfer. */
} ARQUEREQ; // Size 0x28

static void ArqCallBack(u32 requestAddress);
static void ArqCallBackAM(u32 requestAddress);
static void ArqCallBackAMFileRead(u32 requestAddress);

static s32 ATTRIBUTE_ALIGN(32) preLoadBuf[16];
static ARQUEREQ ARQueBuf[16];
static ARQRequest arqReq;
static ARMEM_BLOCK ARInfo[ARMEM_BLOCK_MAX];

static AMEM_PTR ARBase;
static s32 arqCnt;
static s16 arqIdx;

/* HuSysInit calls this during startup to initialize ARAM and seed the free-range list. */
void HuARInit(void) {
    s32 availableBytes;
    s16 blockIndex;

    if(!ARCheckInit()) {
        ARInit(NULL, 0);
        ARQInit();
    }
    for (blockIndex=0; blockIndex<ARMEM_BLOCK_MAX; blockIndex++) {
        ARInfo[blockIndex].aramAddress = 0;
    }
    availableBytes = ARGetSize() - HU_AMEM_BASE;
    ARBase = HU_AMEM_BASE;
    ARInfo[0].aramAddress = ARBase;
    ARInfo[0].size = availableBytes;
    ARInfo[0].inUse = 0;
    ARInfo[0].next = &ARInfo[1];
    ARInfo[0].dataDirId = ARMEM_NO_DIRECTORY_ID;
    ARInfo[1].aramAddress = -1;
    ARInfo[1].size = 0;
    ARInfo[1].inUse = 1;
    ARInfo[1].next = 0;
    ARInfo[1].dataDirId = ARMEM_NO_DIRECTORY_ID;
    arqCnt = 0;
}

/* Resource loaders call this to reserve a 32-byte-rounded range from the ARAM free list. */
AMEM_PTR HuARMalloc(u32 size) {
    ARMEM_BLOCK *previousBlock;
    ARMEM_BLOCK *newBlock;
    ARMEM_BLOCK *currentBlock;
    s16 blockIndex;

    size = OSRoundUp32B(size); /* ARQ transfers and ARAM addresses use 32-byte alignment. */
    currentBlock = previousBlock = ARInfo;
    while(currentBlock->next != 0) {
        if(currentBlock->inUse == 0 && currentBlock->size >= size) {
            break;
        }
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;
    }
    if(currentBlock->next == 0) {
        OSReport("Can't ARAM Allocated %x\n", size);
        HuAMemDump();
        return 0;
    }
    currentBlock->inUse = 1;
    if(currentBlock->size == size && previousBlock != currentBlock) {
        currentBlock->dataDirId = ARMEM_NO_DIRECTORY_ID;
    } else {
        newBlock = &ARInfo[1];
        for (blockIndex=0; blockIndex<ARMEM_BLOCK_MAX-1; blockIndex++, newBlock++) {
            if(!newBlock->aramAddress) {
                break;
            }
        }
        if(blockIndex == ARMEM_BLOCK_MAX-1) {
            OSReport("Can't ARAM Allocated %x\n", size);
            return 0;
        }
        /* Split the selected free range so its unused tail remains available for later loads. */
        newBlock->next = currentBlock->next;
        currentBlock->next = newBlock;
        newBlock->size = currentBlock->size - size;
        newBlock->aramAddress = currentBlock->aramAddress + size;
        currentBlock->size = size;
        currentBlock->dataDirId = newBlock->dataDirId = ARMEM_NO_DIRECTORY_ID;
        newBlock->inUse = 0;
    }
    return currentBlock->aramAddress;
}

/* Resource release paths call this to free a range and merge neighboring free ranges. */
void HuARFree(AMEM_PTR aramAddress) {
    ARMEM_BLOCK *previousBlock;
    ARMEM_BLOCK *nextBlock;
    ARMEM_BLOCK *currentBlock;

    currentBlock = previousBlock = ARInfo;
    while(currentBlock->next) {
        if(currentBlock->aramAddress == aramAddress) {
            break;
        }
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;
    }
    if(currentBlock->inUse) {
        if(!currentBlock->next && currentBlock->aramAddress != aramAddress) {
            OSReport("Can't ARAM Free %x\n", aramAddress);
            return;
        }
        nextBlock = currentBlock->next;
        if(nextBlock->next && nextBlock->inUse == FALSE) {
            if(currentBlock->aramAddress > nextBlock->aramAddress) {
                currentBlock->aramAddress = nextBlock->aramAddress;
            }
            currentBlock->size += nextBlock->size;
            currentBlock->next = nextBlock->next;
            nextBlock->aramAddress = 0;
        }
        if(previousBlock != currentBlock && previousBlock->next && previousBlock->inUse == FALSE) {
            if(previousBlock->aramAddress > currentBlock->aramAddress) {
                previousBlock->aramAddress = currentBlock->aramAddress;
            }
            previousBlock->size += currentBlock->size;
            previousBlock->next = currentBlock->next;
            currentBlock->aramAddress = 0;
        }
        currentBlock->inUse = 0;
        currentBlock->dataDirId = ARMEM_NO_DIRECTORY_ID;
    }
}

/* ARAM-to-main-memory loaders use this to get the recorded span for a range start address. */
static u32 HuARSizeGet(AMEM_PTR aramAddress) {
    ARMEM_BLOCK *currentBlock;
    ARMEM_BLOCK *previousBlock;

    currentBlock = previousBlock = ARInfo;
    while(currentBlock->next) {
        if(currentBlock->aramAddress == aramAddress) {
            break;
        }
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;
    }
    if(currentBlock->next == FALSE && currentBlock->aramAddress != aramAddress) {
        OSReport("Can't Find ARAM %x\n", aramAddress);
        return 0;
    } else {
        return currentBlock->size;
    }
}

/* Transfer helpers use this to find the block record for an ARAM range start address. */
static ARMEM_BLOCK *HuARInfoGet(AMEM_PTR aramAddress) {
    ARMEM_BLOCK *currentBlock;
    ARMEM_BLOCK *previousBlock;

    currentBlock = previousBlock = ARInfo;
    while(currentBlock->next) {
        if(currentBlock->aramAddress == aramAddress) {
            break;
        }
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;
    }
    if(currentBlock->next == FALSE && currentBlock->aramAddress != aramAddress) {
        OSReport("Can't Find ARAM %x\n", aramAddress);
        return NULL;
    } else {
        return currentBlock;
    }
}

/* Allocation failure paths and explicit diagnostics call this to print the ARAM range list. */
void HuAMemDump(void) {
    ARMEM_BLOCK *currentBlock;

    OSReport("ARAM DUMP ======================\n");
    OSReport("AMemPtr  Stat Length\n");
    for(currentBlock=ARInfo; currentBlock->next; currentBlock=currentBlock->next) {
        OSReport("%08x:%04x,%08x,%08x\n", currentBlock->aramAddress, currentBlock->inUse,
            currentBlock->size, currentBlock->dataDirId);
    }
    OSReport("%08x:%04x,%08x\n", currentBlock->aramAddress, currentBlock->inUse,
        currentBlock->size);
    OSReport("================================\n");
}

/* When a directory is not already in ARAM, resource loaders use this to get it into main memory if
 * needed, queue its copy to ARAM, and wait until pending ARQ transfers finish. */
AMEM_PTR HuAR_DVDtoARAM(unsigned int dataNum) {
    HUDATASTAT *dataStatus;
    ARMEM_BLOCK *allocatedBlock;
    AMEM_PTR aramAddress;

    aramAddress = HuARDirCheck(dataNum);
    if(aramAddress) {
        return aramAddress;
    }
    dataStatus = HuDataDirRead(dataNum);
    DirDataSize = OSRoundUp32B(DirDataSize);
    aramAddress = HuARMalloc(DirDataSize);
    if(!aramAddress) {
        return 0;
    }
    allocatedBlock = HuARInfoGet(aramAddress);
    allocatedBlock->dataDirId = (dataNum >> 16);
    arqCnt++;
    ARQPostRequest(&arqReq, ARQ_OWNER_HU, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_LOW,
        (u32) dataStatus->dirP, aramAddress, DirDataSize, ArqCallBack);
    OSReport("ARAM Trans %x\n", aramAddress);
    while(HuARDMACheck());
    HuDataDirClose(dataNum);
    return aramAddress;
}

/* ARQ invokes this when a queued main-memory-to-ARAM transfer completes. */
static void ArqCallBack(u32 requestAddress) {
    arqCnt--;
    (void)requestAddress;
}

/* Character and module loaders use this wrapper to copy an already loaded resource into ARAM. */
AMEM_PTR HuAR_MRAMtoARAM(int dataNum) {
    return HuAR_MRAMtoARAM2(HuDataGetDirPtr(dataNum));
}

/* Loaders pass a resident data pointer here to queue its archive copy to ARAM and record the
 * directory ID; if that directory is already in ARAM, it returns the existing address. */
AMEM_PTR HuAR_MRAMtoARAM2(void *sourceData) {
    ARMEM_BLOCK *allocatedBlock;
    HUDATASTAT *dataStatus;
    u32 alignedSize;
    AMEM_PTR aramAddress;

    dataStatus = HuDataGetStatus(sourceData);
    aramAddress = HuARDirCheck(dataStatus->dirId << 16);
    if(aramAddress) {
        return aramAddress;
    }
    alignedSize = HuMemMemorySizeGet(sourceData);
    alignedSize = OSRoundUp32B(alignedSize);
    aramAddress = HuARMalloc(alignedSize);
    if(!aramAddress) {
        return 0;
    }
    allocatedBlock = HuARInfoGet(aramAddress);
    allocatedBlock->dataDirId = dataStatus->dirId;
    arqCnt++;
    ARQPostRequest(&arqReq, ARQ_OWNER_HU, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_LOW,
        (u32)sourceData, aramAddress, alignedSize, ArqCallBack);
    return aramAddress;
}

/* HuDataDirRead and HuDataDirReadAsync call this to queue an ARAM-to-main-memory copy; the
 * synchronous caller waits and then looks up its data record. */
/* HuDataDirRead paths call this wrapper; it starts the transfer without returning its result. */
void *HuAR_ARAMtoMRAM(AMEM_PTR aramAddress) {
    HuAR_ARAMtoMRAMNum(aramAddress, 0);
}

/* HuDataDirReadNum uses this to start copying an ARAM-resident archive; its callback registers the
 * destination when DMA completes. */
void *HuAR_ARAMtoMRAMNum(AMEM_PTR aramAddress, s32 allocationNumber) {
    void *destination;
    ARMEM_BLOCK *allocatedBlock;
    s32 transferSize;
    
    allocatedBlock = HuARInfoGet(aramAddress);
    if(HuDataReadChk(allocatedBlock->dataDirId << 16) >= 0) {
        /* The resource is already present; this path returns no buffer pointer. */
        return;
    }
    transferSize = HuARSizeGet(aramAddress);
    destination = HuMemDirectMallocNum(HEAP_DVD, transferSize, allocationNumber);
    if(!destination) {
        return 0;
    }
    HuMemMemoryFileSet(destination, (allocatedBlock->dataDirId << 16) & ARMEM_DIRECTORY_ID_MASK);
    /* The destination cache range is flushed before ARAM DMA writes into it. */
    DCFlushRangeNoSync(destination, transferSize);
    ARQueBuf[arqIdx].dataNum = (allocatedBlock->dataDirId << 16);
    ARQueBuf[arqIdx].destination = destination;
    arqCnt++;
    PPCSync();
    ARQPostRequest(&ARQueBuf[arqIdx].request, ARQ_OWNER_HU, ARQ_TYPE_ARAM_TO_MRAM,
        ARQ_PRIORITY_LOW, aramAddress, (u32) destination, transferSize, ArqCallBackAM);
    arqIdx++;
    arqIdx &= 0xF;
    return destination;
}

/* ARQ invokes this after an ARAM-to-main-memory copy so the data loader can register the buffer. */
static void ArqCallBackAM(u32 requestAddress) {
    ARQUEREQ *request = (ARQUEREQ *) requestAddress;

    arqCnt--;
    HuDataDirSet(request->destination, request->dataNum);
}

/* Data loading paths poll this until all queued ARAM DMA callbacks have completed. */
s32 HuARDMACheck(void) {
    return arqCnt;
}

/* Data loaders and resource managers use this to find a directory's allocated ARAM range. */
AMEM_PTR HuARDirCheck(unsigned int dataNum) {
    ARMEM_BLOCK *currentBlock;

    currentBlock = ARInfo;
    dataNum >>= 16;
    while(currentBlock->next != 0) {
        if(currentBlock->inUse == 1 && currentBlock->dataDirId == dataNum) {
            return currentBlock->aramAddress;
        }
        currentBlock = currentBlock->next;
    }
    return 0;
}

/* Resource managers call this when they release a directory's resident ARAM copy. */
void HuARDirFree(unsigned int dataNum) {
    ARMEM_BLOCK *currentBlock;

    currentBlock = ARInfo;
    dataNum >>= 16;
    while(currentBlock->next) {
        if(currentBlock->dataDirId == dataNum) {
            HuARFree(currentBlock->aramAddress);
            break;
        }
        currentBlock = currentBlock->next;
    }
}

/* Character and message loaders call this to copy and decode one file from a resident archive. */
void *HuAR_ARAMtoMRAMFileRead(unsigned int dataNum, u32 allocationNumber, HEAPID heap) {
    s32 *directoryEntry;
    void *decodedData;
    void *transferBuffer;
    AMEM_PTR sourceAddress;
    s32 fileOffset;
    s32 transferSize;
    AMEM_PTR archiveAddress;

    if((archiveAddress = HuARDirCheck(dataNum)) == 0) {
        OSReport("Error: data none on ARAM %0x\n", dataNum);
        HuAMemDump();
        return 0;
    }
    DCInvalidateRange(&preLoadBuf, sizeof(preLoadBuf));
    /* Read a 64-byte window starting at the 32-byte-aligned address that contains the archive's
     * directory word pair. */
    sourceAddress =
        archiveAddress + (u32) ((u32) (((u16) dataNum + 1) * 4) & ARMEM_ARCHIVE_ALIGN_DOWN_MASK);
    arqCnt++;
    ARQPostRequest(&ARQueBuf[arqIdx].request, ARQ_OWNER_HU, ARQ_TYPE_ARAM_TO_MRAM,
        ARQ_PRIORITY_LOW, sourceAddress, (u32) &preLoadBuf, sizeof(preLoadBuf),
        ArqCallBackAMFileRead);
    arqIdx++;
    arqIdx &= 0xF;
    while(HuARDMACheck());
    directoryEntry = &preLoadBuf[(dataNum + 1) & 0x7];
    fileOffset = directoryEntry[0];
    /* Compute the span to the next file offset, or to the allocated archive range end for the last
     * entry, then include padding for the aligned DMA source and transfer length. */
    sourceAddress = archiveAddress + (u32)(fileOffset & ARMEM_ARCHIVE_ALIGN_DOWN_MASK);
    if(directoryEntry[1] - fileOffset < 0) {
        transferSize =
            (HuARSizeGet(archiveAddress) - fileOffset + ARMEM_FILE_TRANSFER_ROUNDUP_BIAS) &
            ARMEM_ARCHIVE_ALIGN_DOWN_MASK;
    } else {
        transferSize = (directoryEntry[1] - fileOffset + ARMEM_FILE_TRANSFER_ROUNDUP_BIAS) &
                       ARMEM_ARCHIVE_ALIGN_DOWN_MASK;
    }
    transferBuffer = HuMemDirectMalloc(HEAP_DVD, transferSize);
    if(!transferBuffer) {
        return 0;
    }
    DCFlushRangeNoSync(transferBuffer, transferSize);
    arqCnt++;
    PPCSync();
    ARQPostRequest(&ARQueBuf[arqIdx].request, ARQ_OWNER_HU, ARQ_TYPE_ARAM_TO_MRAM,
        ARQ_PRIORITY_LOW, sourceAddress, (u32) transferBuffer, (u32) transferSize,
        ArqCallBackAMFileRead);
    arqIdx++;
    arqIdx &= 0xF;
    while(HuARDMACheck());
    directoryEntry = (s32*) ((u8*) transferBuffer + (fileOffset & 0x1F));
    decodedData = HuMemDirectMallocNum(
        heap, (directoryEntry[0] + 1) & ARMEM_FILE_SIZE_ALIGN_DOWN_MASK, allocationNumber);
    if(!decodedData) {
        /* The temporary transfer buffer is left allocated on this failure path. */
        return 0;
    }
    HuDecodeData(&directoryEntry[2], decodedData, directoryEntry[0], directoryEntry[1]);
    HuMemMemoryFileSet(decodedData, dataNum);
    HuMemDirectFree(transferBuffer);
    return decodedData;
}

/* ARQ invokes this after each archive directory or payload DMA completes. */
static void ArqCallBackAMFileRead(u32 requestAddress) {
    arqCnt--;
    (void)requestAddress;
}
