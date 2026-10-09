// Handles the memory-card checks and file operations used by game and file-select save/load flows.
#include "dolphin.h"
#include "game/memory.h"
#include "game/card.h"
#include "game/main.h"

// Work area shared with the CARD library for mounting cards.
static void *cardWork;
// Bitmask of slots whose mounted cards have been detached.
u8 UnMountCnt;
// Set before a write starts; the callback clears it, but a direct HuCardWriteAsync call leaves it
// set if CARDWriteAsync returns an immediate error. HuCardWriteIdle clears it on that error path.
static BOOL WriteStatus;
// Result most recently stored by WriteCallBack.
static s32 WriteResult;

static void MountCallBack(s32 cardSlot, s32 cardResult);

// Initializes the CARD library and allocates its work area during game startup.
void HuCardInit(void)
{
	CARDInit();
	cardWork = HuMemDirectMalloc(HEAP_HEAP, CARD_WORKAREA_SIZE);
}

// Returns a two-bit slot mask, setting a bit when CARDProbeEx reports READY or WRONGDEVICE.
s32 HuCardCheck(void)
{
	s32 availableSlots = 0;
	s32 memoryBytes, sectorBytes;
	s32 probeResult;
	do {
		probeResult = CARDProbeEx(0, &memoryBytes, &sectorBytes);
	} while(probeResult == CARD_RESULT_BUSY);
	if(probeResult == CARD_RESULT_READY || probeResult == CARD_RESULT_WRONGDEVICE) {
		availableSlots |= 0x1;
		OSReport("SlotA Card MemSize %x,Sector Size %x\n", memoryBytes, sectorBytes);
	}
	do {
		probeResult = CARDProbeEx(1, &memoryBytes, &sectorBytes);
	} while(probeResult == CARD_RESULT_BUSY);
	if(probeResult == CARD_RESULT_READY || probeResult == CARD_RESULT_WRONGDEVICE) {
		availableSlots |= 0x2;
		OSReport("SlotB Card MemSize %x,Sector Size %x\n", memoryBytes, sectorBytes);
	}
	return availableSlots;
}

// Returns a slot's sector size in bytes, or its CARD error, for save/load sizing.
s32 HuCardSlotCheck(s16 cardSlot)
{
	s32 memoryBytes, sectorBytes;
	s32 probeResult;
	do {
		probeResult = CARDProbeEx(cardSlot, &memoryBytes, &sectorBytes);
	} while(probeResult == CARD_RESULT_BUSY);
	if(probeResult < 0) {
		return probeResult;
	} else {
		return sectorBytes;
	}
}

// Save/load callers use this before opening files; it mounts the slot and checks the card.
s32 HuCardMount(s16 cardSlot)
{
	s32 memoryBytes, sectorBytes;
	s32 cardResult;
	do {
		cardResult = CARDProbeEx(cardSlot, &memoryBytes, &sectorBytes);
	} while(cardResult == CARD_RESULT_BUSY);
	if(cardResult < 0) {
		return cardResult;
	} else {
		cardResult = CARDMount(cardSlot, cardWork, MountCallBack);
		if(cardResult == CARD_RESULT_FATAL_ERROR || cardResult == CARD_RESULT_IOERROR) {
			return CARD_RESULT_FATAL_ERROR;
		}
		if(cardResult == CARD_RESULT_ENCODING) {
			return CARD_RESULT_BROKEN;
		}
		if(cardResult < 0 && cardResult != CARD_RESULT_BROKEN) {
			return cardResult;
		}
		cardResult = CARDCheck(cardSlot);
		if(cardResult == CARD_RESULT_FATAL_ERROR || cardResult == CARD_RESULT_IOERROR) {
			return CARD_RESULT_FATAL_ERROR;
		}
		if(cardResult == CARD_RESULT_ENCODING) {
			return CARD_RESULT_BROKEN;
		}
		return cardResult;
	}
}

// Save/load callers release the selected slot with this after finishing card access.
void HuCardUnMount(s16 cardSlot)
{
	CARDUnmount(cardSlot);
}

// The save/load erase flow formats the selected slot; it suppresses DVD-error display and
// soft-reset post-processing during the call, then forces the shared flag off.
s32 HuCardFormat(s16 cardSlot)
{
    s32 cardResult;
    // Suppresses DVD-error display and soft-reset post-processing, then unconditionally clears the
    // shared flag.
    HuSRDisableF = TRUE;
    cardResult = CARDFormat(cardSlot);
    HuSRDisableF = FALSE;
	if(cardResult == CARD_RESULT_FATAL_ERROR || cardResult == CARD_RESULT_IOERROR) {
		return CARD_RESULT_FATAL_ERROR;
	}
	return cardResult;
}

// CARDMount invokes this on detach; record the slot bit and ignore CARD's result code.
static void MountCallBack(s32 cardSlot, s32 cardResult)
{
	UnMountCnt |= (1 << cardSlot);
}

// Save/load callers open a named file on a slot after mounting it.
s32 HuCardOpen(s16 cardSlot, const char *fileName, CARDFileInfo *fileInfo)
{
	s32 cardResult = CARDOpen(cardSlot, fileName, fileInfo);
	if(cardResult == CARD_RESULT_FATAL_ERROR || cardResult == CARD_RESULT_IOERROR) {
		return CARD_RESULT_FATAL_ERROR;
	}
	if(cardResult == CARD_RESULT_ENCODING) {
		return CARD_RESULT_BROKEN;
	}
	return cardResult;
}

// Save/load callers read file bytes into their destination buffer at the requested offset.
s32 HuCardRead(CARDFileInfo *fileInfo, void *destination, s32 byteCount, s32 fileOffset)
{
	s32 cardResult = CARDRead(fileInfo, destination, byteCount, fileOffset);
	if(cardResult == CARD_RESULT_FATAL_ERROR || cardResult == CARD_RESULT_IOERROR) {
		return CARD_RESULT_FATAL_ERROR;
	}
	return cardResult;
}

// Save/load creation checks free space and directory entries before creating the file.
s32 HuCardCreate(s16 cardSlot, const char *fileName, s32 requestedBytes, CARDFileInfo *fileInfo)
{
	s32 sectorBytes;
	s32 availableBytes, availableFiles;
	s32 cardResult;
	BOOL previousResetSuppression;
	
	cardResult = CARDGetSectorSize(cardSlot, (u32 *)&sectorBytes);
	if(cardResult == CARD_RESULT_FATAL_ERROR || cardResult == CARD_RESULT_NOCARD) {
		return cardResult;
	}
	if(requestedBytes % sectorBytes) {
		// For an unaligned request, this multiplies its size by sectorBytes instead of rounding up.
		requestedBytes = sectorBytes*(requestedBytes+((sectorBytes-1)/sectorBytes));
	}
	cardResult = CARDFreeBlocks(cardSlot, &availableBytes, &availableFiles);
	if(cardResult != 0) {
		return cardResult;
	}
	if(availableFiles <= 0 || availableBytes < requestedBytes) {
		return CARD_RESULT_INSSPACE;
	}
	previousResetSuppression = HuSRDisableF;
    HuSRDisableF = TRUE;
	cardResult = CARDCreate(cardSlot, fileName, requestedBytes, fileInfo);
	HuSRDisableF = previousResetSuppression;
	return cardResult;
}

// Save/load callers close the file after their read or write operation finishes.
s32 HuCardClose(CARDFileInfo *fileInfo)
{
	s32 cardResult = CARDClose(fileInfo);
	return cardResult;
}

// Save/load callers write file bytes while preserving the reset-suppression flag's prior value.
s32 HuCardWrite(CARDFileInfo *fileInfo, const void *source, s32 byteCount, s32 fileOffset)
{
	s32 cardResult;
	BOOL previousResetSuppression;
    previousResetSuppression = HuSRDisableF;
    HuSRDisableF = TRUE;
    cardResult = CARDWrite(fileInfo, source, byteCount, fileOffset);
    HuSRDisableF = previousResetSuppression;
	return cardResult;
}

static void WriteCallBack(s32 cardSlot, s32 cardResult);

// Save/load callers start writes here; a new write waits for WriteStatus to clear. If a prior
// direct
// HuCardWriteAsync call returned an immediate error, no callback clears it and this wait can block.
s32 HuCardWriteAsync(CARDFileInfo *fileInfo, const void *source, s32 byteCount, s32 fileOffset)
{
	s32 cardResult;
	if(WriteStatus) {
		// The completed earlier write's result is overwritten by the new CARDWriteAsync result.
		while(!HuCardWriteCheck(&cardResult)) {
			HuPrcVSleep();
		}
	}
	WriteStatus = TRUE;
	cardResult = CARDWriteAsync(fileInfo, source, byteCount, fileOffset, WriteCallBack);
	return cardResult;
}

// CARDWriteAsync invokes this on completion to publish its result for HuCardWriteCheck.
// The slot argument is provided by CARD but is not needed here.
static void WriteCallBack(s32 cardSlot, s32 cardResult)
{
	WriteStatus = FALSE;
	WriteResult = cardResult;
}

// HuCardWriteAsync and HuCardWriteIdle use this to check completion and retrieve the result.
BOOL HuCardWriteCheck(s32 *completedWriteResult)
{
	if(WriteStatus) {
		return FALSE;
	}
	*completedWriteResult = WriteResult;
	return TRUE;
}

// Starts an asynchronous write; if CARD reports READY, sleeps until its callback completes,
// otherwise clears WriteStatus and returns the immediate result.
s32 HuCardWriteIdle(CARDFileInfo *fileInfo, const void *source, s32 byteCount, s32 fileOffset)
{
	s32 cardResult = HuCardWriteAsync(fileInfo, source, byteCount, fileOffset);
	if(cardResult != CARD_RESULT_READY) {
		WriteStatus = FALSE;
		return cardResult;
	}
	while(!HuCardWriteCheck(&cardResult)) {
		HuPrcVSleep();
	}
	return cardResult;
}

// The save/load erase flow deletes a named file; it suppresses DVD-error display and soft-reset
// post-processing during the call, then forces the shared flag off.
s32 HuCardDelete(s16 cardSlot, const char *fileName)
{
	s32 cardResult;
    HuSRDisableF = TRUE;
    cardResult = CARDDelete(cardSlot, fileName);
    HuSRDisableF = FALSE;
	return cardResult;
}

// Save/load callers use the slot's sector size in bytes to size card file operations.
s32 HuCardSectorSizeGet(s16 cardSlot)
{
	u32 sectorBytes;
	s32 cardResult = CARDGetSectorSize(cardSlot, &sectorBytes);
	if(cardResult < 0) {
		return cardResult;
	} else {
		return sectorBytes;
	}
}

// Save/load callers retrieve the mounted slot's free bytes and directory-entry counts.
s32 HuCardFreeSpaceGet(s16 cardSlot, u32 *availableBytes, u32 *availableFiles)
{
	s32 cardResult = CARDFreeBlocks(cardSlot, (s32 *)availableBytes, (s32 *)availableFiles);
	return cardResult;
}
