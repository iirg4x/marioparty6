// Manages the memory-card save file, its three board-save boxes, and save-screen flow.
#define _MATH_H
#include "datanum/win.h"
#include "game/audio.h"
#include "game/pad.h"
#include "game/hu3d.h"
#include "game/wipe.h"
#include "game/data.h"
#include "game/frand.h"

#include "game/window.h"
#include "game/saveload.h"
#include "game/card.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/sprite.h"

#define SAVE_BOX_COMMON_OFS 0
#define SAVE_BOX_BOARD_SYSTEM_OFS (sizeof(GW_COMMON))
#define SAVE_BOX_BOARD_PLAYER_OFS (SAVE_BOX_BOARD_SYSTEM_OFS+sizeof(GW_SYSTEM))
#define SL_ERASE_FILL_BIT_PATTERN 0xFF
#define SL_BANK_AWARD_TINT_RED 48
#define SL_BANK_AWARD_TINT_GREEN 38
#define SL_BANK_AWARD_TINT_BLUE 84
#define SL_BANK_AWARD_BACKDROP_GREEN 39

#define SL_CUR_SLOT_MESS (MESS_MPSYSTEM_CARD_SLOTA+curSlotNo)

#define SL_MESSID_NONE -1

#define SL_MESS_NOCARD 0
#define SL_MESS_FATAL_ERROR 1
#define SL_MESS_CARD_NOENT 2
#define SL_MESS_CARD_INSSPACE 3
#define SL_MESS_CARD_FULL 4
#define SL_MESS_FORMAT_CHOICE 5
#define SL_MESS_FORMAT_ERROR 6
#define SL_MESS_WRONGDEVICE 7
#define SL_MESS_CARD_INVALID 8
#define SL_MESS_SERIAL_INVALID 9
#define SL_MESS_NOSAVE_CHOICE 10
#define SL_MESS_CARD_REINSERT 11
#define SL_MESS_CARD_FORMAT_UNMOUNT 12

#define MESS_MPSYSTEM_CARD_NOCARD 0x90001
#define MESS_MPSYSTEM_CARD_FORMAT_CHOICE 0x90002
#define MESS_MPSYSTEM_CARD_WRONGDEVICE 0x90003
#define MESS_MPSYSTEM_CARD_FATAL_ERROR 0x90004
#define MESS_MPSYSTEM_CARD_INSSPACE 0x90005
#define MESS_MPSYSTEM_CARD_NOENT 0x90006
#define MESS_MPSYSTEM_CARD_CREATEFILE 0x90007
#define MESS_MPSYSTEM_MES_CARD_WRITE 0x9000b
#define MESS_MPSYSTEM_NOSAVE_CHOICE 0x90018
#define MESS_MPSYSTEM_CARD_FORMAT 0x9001a
#define MESS_MPSYSTEM_CARD_FORMAT_WARN 0x9001b
#define MESS_MPSYSTEM_CARD_FORMAT_ERROR 0x9001c
#define MESS_MPSYSTEM_CARD_INVALID 0x90021
#define MESS_MPSYSTEM_CARD_SERIAL_INVALID 0x90022
#define MESS_MPSYSTEM_CARD_FORMAT_UNMOUNT 0x90026
#define MESS_MPSYSTEM_MES_SAVE_SAVE 0x9002a
#define MESS_MPSYSTEM_CARD_FULL 0x90035
#define MESS_MPSYSTEM_CARD_SLOTA 0x90036
#define MESS_MPSYSTEM_CARD_REINSERT 0x9003a
#define MESS_MPSYSTEM_MES_BANK_AWARD 0x90043

void SLCurBoxNoSet(s16 boxNo);
static HUWINID SLWinOpen(s32 mesNum, s32 insertMesNum1, s32 insertMesNum2, s16 posY);
static void SLWinClose(HUWINID winId);
static HUWINID SLBankAwardExec(s16 award);

GW_COMMON SLGwCommonBackup;
u8 ATTRIBUTE_ALIGN(32) saveBuf[2][SAVE_BUF_SIZE];
CARDFileInfo curFileInfo;

u64 SLSerialNo[2] = {0, 0};
static u32 boxDataOfs[SAVE_BOX_MAX*2] = {
    SAVE_BOX_OFS(0),
    SAVE_BOX_OFS(1),
    SAVE_BOX_OFS(2),
    SAVE_BOXBACKUP_OFS(0),
    SAVE_BOXBACKUP_OFS(1),
    SAVE_BOXBACKUP_OFS(2)
};

static u8 commentTbl[2][CARD_COMMENT_SIZE/2] = {
    "Mario Party 6",
    "00/00/0000"
};

BOOL SaveEnableF = TRUE;
static int SLCurWinId = HUWIN_NONE;
char SLSaveFileName[] = "MARIPA6";
char SLEraseStr[] = "ERASE";
HUWINID SLWinId = HUWIN_NONE;

static f32 bottomWinPosY;
static f32 topWinPosY;
static char bankAwardMes[8];
BOOL saveExecF;
u8 curBoxNo;
s16 curSlotNo;

// Resets the shared save window handle and the default message-window positions.
void SLWinInit(void)
{
    SLWinId = HUWIN_NONE;
    bottomWinPosY = 294.0f;
    topWinPosY = 50.0f;
}

// Called before save-file access; mounts the selected card and offers format for broken media.
s32 SLFileOpen(char *fileName)
{
    s32 result;
    if(SaveEnableF == FALSE) {
        return CARD_RESULT_READY;
    }
    mount:
    result = SLCardMount(curSlotNo);
    if(result < 0) {
        return result;
    }
    result = HuCardOpen(curSlotNo, fileName, &curFileInfo);
    if(result == CARD_RESULT_NOFILE) {
        return CARD_RESULT_NOFILE;
    } else if(result == CARD_RESULT_WRONGDEVICE) {
        SLMessOut(SL_MESS_WRONGDEVICE);
        return CARD_RESULT_FATAL_ERROR;
    } else if(result == CARD_RESULT_FATAL_ERROR) {
        SLMessOut(SL_MESS_FATAL_ERROR);
        return CARD_RESULT_FATAL_ERROR;
    } else if(result == CARD_RESULT_NOCARD) {
        SLMessOut(SL_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    } else if(result == CARD_RESULT_BROKEN) {
        result = HuCardSectorSizeGet(curSlotNo);
        if(result > 0 && result != SAVE_SECTOR_SIZE) {
            SLMessOut(SL_MESS_CARD_INVALID);
            return CARD_RESULT_WRONGDEVICE;
        } else {
            UnMountCnt = 0;
            result = SLMessOut(SL_MESS_FORMAT_CHOICE);
            if(result == 0) {
                result = SLFormat(curSlotNo);
                if(result == CARD_RESULT_READY) {
                    goto mount;
                } else {
                    return result;
                }
            } else {
                return CARD_RESULT_NOFILE;
            }
        }
    }
    return CARD_RESULT_READY;
}

// Called by SLSave for a new file; creates it, writes an ERASE marker buffer, then save data.
s32 SLFileCreate(char *fileName, u32 size, void *saveData)
{
    s32 result;
    u32 freeBytes;
    u32 freeFileEntries;
    void *erasedFileData;
    int winId;
    if(SaveEnableF == FALSE) {
        return CARD_RESULT_READY;
    }
    result = SLCardMount(curSlotNo);
    if(result < 0) {
        return result;
    }
    result = HuCardSectorSizeGet(curSlotNo);
    // Positive nonstandard sector sizes pass this negative-result-only check.
    if(result < 0 && result != SAVE_SECTOR_SIZE) {
        SLMessOut(SL_MESS_CARD_INVALID);
        return CARD_RESULT_FATAL_ERROR;
    }
    result = HuCardFreeSpaceGet(curSlotNo, &freeBytes, &freeFileEntries);
    if(freeFileEntries == 0 && size > freeBytes) {
        SLMessOut(SL_MESS_CARD_FULL);
        return CARD_RESULT_INSSPACE;
    }
    if(freeFileEntries == 0) {
        SLMessOut(SL_MESS_CARD_NOENT);
        return CARD_RESULT_INSSPACE;
    }
    if(size > freeBytes) {
        SLMessOut(SL_MESS_CARD_INSSPACE);
        return CARD_RESULT_INSSPACE;
    }
    winId =
        SLWinOpen(MESS_MPSYSTEM_CARD_CREATEFILE, SL_CUR_SLOT_MESS, SL_MESSID_NONE, bottomWinPosY);
    HuSRDisableF = TRUE;
    result = HuCardCreate(curSlotNo, fileName, size, &curFileInfo);
    if(result < 0) {
        SLWinClose(winId);
        HuSRDisableF = FALSE;
    }
    if(result == CARD_RESULT_NOCARD) {
        SLMessOut(SL_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    }
    if(result < 0) {
        SLMessOut(SL_MESS_FATAL_ERROR);
        return CARD_RESULT_FATAL_ERROR;
    }
    erasedFileData = HuMemDirectMalloc(HEAP_HEAP, size);
    // memset has a zero byte count, so only the six-byte ERASE marker is initialized before the
    // full size is written to the card; the remaining bytes are uninitialized.
    memset(erasedFileData, size, 0);
    memcpy(erasedFileData, SLEraseStr, sizeof(SLEraseStr));
    result = HuCardWriteIdle(&curFileInfo, erasedFileData, size, 0);
    if(result == CARD_RESULT_READY) {
        result = HuCardWriteIdle(&curFileInfo, saveData, size, 0);
    }
    HuMemDirectFree(erasedFileData);
    if(result < 0) {
        SLWinClose(winId);
        HuSRDisableF = FALSE;
    }
    if(result == CARD_RESULT_NOCARD) {
        SLMessOut(SL_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    }
    if(result < 0) {
        SLMessOut(SL_MESS_FATAL_ERROR);
        return CARD_RESULT_FATAL_ERROR;
    }
    result = SLStatSet(FALSE);
    HuSRDisableF = FALSE;
    SLWinClose(winId);
    if(result < 0) {
        return result;
    } else {
        return CARD_RESULT_READY;
    }
}

// Called by SLSave for an existing file; shows the writing message and updates card metadata.
s32 SLFileWrite(s32 length, void *saveData)
{
    int winId;
    s32 result;
    HuVec2f size;
    if(SaveEnableF == FALSE) {
        return CARD_RESULT_READY;
    }
    HuWinInit(1);
    HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
    HuWinMesMaxSizeGet(1, &size, MESS_MPSYSTEM_MES_CARD_WRITE);
    winId = HuWinWarningCreate(-10000.0f, bottomWinPosY, size.x, size.y);
    HuWinWarningOpen(winId);
    HuWinInsertMesSet(winId, SL_CUR_SLOT_MESS, 0);
    HuWinMesSet(winId, MESS_MPSYSTEM_MES_CARD_WRITE);
    HuWinMesWait(winId);
    SLSerialNoGet();
    HuSRDisableF = TRUE;
    result = HuCardWriteIdle(&curFileInfo, saveData, length, 0);
    if(result == CARD_RESULT_READY) {
        result = SLStatSet(FALSE);
    }
    HuSRDisableF = FALSE;
    HuWinWarningClose(winId);
    HuWinWarningKill(winId);
    return result;
}

// Called by SLLoad after opening the file; reads bytes and displays card or I/O errors.
s32 SLFileRead(s32 length, void *readBuffer)
{
    s32 result;
    if(SaveEnableF == FALSE) {
        return CARD_RESULT_READY;
    }
    SLSerialNoGet();
    result = HuCardRead(&curFileInfo, readBuffer, length, 0);
    if(result == CARD_RESULT_NOCARD) {
        SLMessOut(SL_MESS_NOCARD);
    } else if(result < 0) {
        SLMessOut(SL_MESS_FATAL_ERROR);
    }
    return result;
}

// Called after save-file reads or writes to close the open file unless saving is disabled.
s32 SLFileClose(void)
{
    s32 result;
    if(SaveEnableF == FALSE) {
        return CARD_RESULT_READY;
    }
    result = HuCardClose(&curFileInfo);
    return result;
}

// Selects memory-card slot A or B for subsequent save operations.
void SLCurSlotNoSet(s16 slotNo)
{
    curSlotNo = slotNo;
}

// Returns the selected memory-card slot.
s16 SLCurSlotNoGet(void)
{
    return curSlotNo;
}

// Selects one of the three board-save boxes.
void SLCurBoxNoSet(s16 boxNo)
{
    curBoxNo = boxNo;
}

// Returns the selected board-save box.
s16 SLCurBoxNoGet(void)
{
    return curBoxNo;
}

// Enables or disables saving; disabling also clears the common save-enable flag.
void SLSaveFlagSet(BOOL saveFlag)
{
    if(saveFlag == FALSE) {
        GwCommon.saveEnableF = FALSE;
    }
    SaveEnableF = saveFlag;
}

// Returns whether save operations are enabled.
BOOL SLSaveFlagGet(void)
{
    return SaveEnableF;
}

// Marks both the selected box and its backup as empty with the save-format tag.
void SLSaveEmptySet(s16 slotNo, s16 boxNo)
{
    memcpy(&saveBuf[slotNo][SLBoxDataOffsetGet(boxNo)], "EMPT", 4);
    memcpy(&saveBuf[slotNo][SLBoxDataOffsetGet(boxNo+SAVE_BOXNO_BACKUP)], "EMPT", 4);
}

// Builds card-save metadata; when eraseF is true, first fills the entire slot buffer with the erase
// pattern.
void SLSaveDataSlotMake(s16 slotNo, BOOL eraseF, OSTime *saveTime)
{
    u8 *slotBuffer = &saveBuf[slotNo][0];
    ANIMDATA *bannerAnimation;
    u16 checksum;
    if(eraseF) {
        memset(slotBuffer, SL_ERASE_FILL_BIT_PATTERN, SAVE_BUF_SIZE);
    }
    memcpy(slotBuffer, &commentTbl[0][0], CARD_COMMENT_SIZE/2);
    memcpy(slotBuffer+SAVE_COMMENT_DATE_OFS, &commentTbl[1][0], CARD_COMMENT_SIZE/2);
    bannerAnimation = HuSprAnimDataRead(WIN_ANM_save_banner);
    memcpy(slotBuffer + SAVE_BANNER_OFS, bannerAnimation->bmp->data,
           CARD_BANNER_WIDTH * CARD_BANNER_HEIGHT);
    memcpy(slotBuffer+SAVE_BANNER_TLUT_OFS, bannerAnimation->bmp->palData, 512);
    SLSaveDataInfoSet(slotNo, saveTime);
    checksum = SLCheckSumSlotGet(slotNo, 0, SAVE_ICONBANNER_SIZE);
    saveBuf[slotNo][SAVE_ICONBANNER_CHECKSUM_OFS] = checksum >> 8;
    saveBuf[slotNo][SAVE_ICONBANNER_CHECKSUM_OFS+1] = checksum & 0xFF;
}

// Called after card format to rebuild metadata for the currently selected slot.
void SLSaveDataMake(BOOL eraseF, OSTime *saveTime)
{
    SLSaveDataSlotMake(curSlotNo, eraseF, saveTime);
}

// Writes the save date and a randomly selected icon, then refreshes the icon/banner checksum.
void SLSaveDataInfoSet(s16 slotNo, OSTime *saveTime)
{
    OSCalendarTime calendarTime;
    s16 digit;
    u16 year;
    ANIMDATA *iconAnimation;
    u16 checksum;
    OSTicksToCalendarTime(*saveTime, &calendarTime);
    digit = (calendarTime.mon+1)/10;
    // Calendar months are zero-based; the saved comment uses a human-readable month number.
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS] = digit+'0';
    digit = (calendarTime.mon+1)%10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+1] = digit+'0';
    digit = (calendarTime.mday)/10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+3] = digit+'0';
    digit = (calendarTime.mday)%10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+4] = digit+'0';
    year = calendarTime.year;
    digit = year/1000;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+6] = digit+'0';
    year -= digit*1000;
    digit = year/100;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+7] = digit+'0';
    year -= digit*100;
    digit = year/10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+8] = digit+'0';
    year -= digit*10;
    saveBuf[slotNo][SAVE_COMMENT_DATE_OFS+9] = year+'0';
    iconAnimation = HuSprAnimDataRead(WIN_ANM_save_icon1+frandmod(4));
    memcpy(&saveBuf[slotNo][SAVE_ICON_OFS], iconAnimation->bmp->data,
           CARD_ICON_WIDTH * CARD_ICON_HEIGHT * 4);
    memcpy(&saveBuf[slotNo][SAVE_ICON_TLUT_OFS], iconAnimation->bmp->palData, 512);
    HuSprAnimKill(iconAnimation);
    checksum = SLCheckSumGet(0, SAVE_ICONBANNER_SIZE);
    saveBuf[slotNo][SAVE_ICONBANNER_CHECKSUM_OFS] = checksum >> 8;
    saveBuf[slotNo][SAVE_ICONBANNER_CHECKSUM_OFS+1] = checksum & 0xFF;
}

// Before saving a box, stamps common state with the current time and "SAVE", copies it, and updates
// the save date.
void SLCommonSet(void)
{
    u32 boxDataOffset = boxDataOfs[curBoxNo];
    OSTime time = OSGetTime();
    GW_COMMON commonBackup;
    GW_COMMON *commonSnapshot;
    GwCommon.time = time;
    memcpy(&GwCommon.magic[0], "SAVE", 4);
    memcpy(&saveBuf[curSlotNo][boxDataOffset+SAVE_BOX_COMMON_OFS], &GwCommon, sizeof(GW_COMMON));
    SLSaveDataInfoSet(curSlotNo, &time);
    commonBackup = GwCommon;
    commonSnapshot = &commonBackup;
    memcpy(&SLGwCommonBackup, commonSnapshot, sizeof(GW_COMMON));
}

// Copies a common-state snapshot into an arbitrary slot and box in the save buffer.
void SLCommonSaveCopy(GW_COMMON *commonData, s16 slotNo, s16 boxNo)
{
    u32 boxDataOffset = boxDataOfs[boxNo];
    memcpy(&saveBuf[slotNo][boxDataOffset+SAVE_BOX_COMMON_OFS], commonData, sizeof(GW_COMMON));
}

// Called by board save flows to copy system and all player state into the selected box.
void SLBoardSave(void)
{
    u32 boxDataOffset = boxDataOfs[curBoxNo];
    s16 i;
    memcpy(&saveBuf[curSlotNo][boxDataOffset + SAVE_BOX_BOARD_SYSTEM_OFS], &GwSystem,
           sizeof(GW_SYSTEM));
    for(i=0; i<GW_PLAYER_MAX; i++) {
        memcpy(&saveBuf[curSlotNo]
                       [boxDataOffset + SAVE_BOX_BOARD_PLAYER_OFS + (i * sizeof(GW_PLAYER))],
               &GwPlayer[i], sizeof(GW_PLAYER));
    }
}

// Called by board and mode save flows to write all boxes, handling card errors and retries.
s32 SLSave(void)
{
    s32 result;
    BOOL fatalF = FALSE;
    SLCheckSumBoxAllSet();
    SLSaveBackup();
    repeat:
    fatalF = FALSE;
    result = SLFileOpen(SLSaveFileName);
    if(result == CARD_RESULT_NOFILE) {
        if(!SLSerialNoCheck()) {
            SLMessOut(SL_MESS_SERIAL_INVALID);
            goto savefail;
        }
        SLCurWinId =
            SLWinOpen(MESS_MPSYSTEM_MES_SAVE_SAVE, SL_CUR_SLOT_MESS, SL_MESSID_NONE, topWinPosY);
        result = SLFileCreate(SLSaveFileName, SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
        SLWinClose(SLCurWinId);
        SLCurWinId = HUWIN_NONE;
        if(result < 0) {
            goto savefail;
        }
        SLSerialNoGet();
    } else if(result == CARD_RESULT_NOCARD) {
        result = SLMessOut(SL_MESS_NOSAVE_CHOICE);
        if(result != 0) {
            SLMessOut(SL_MESS_CARD_REINSERT);
            goto repeat;
        }
        SLSaveFlagSet(FALSE);
        goto unmount;
    } else if(result == CARD_RESULT_FATAL_ERROR) {
        fatalF = TRUE;
        goto savefail;
    } else if(result < 0) {
        goto savefail;
    } else {
        if(!SLSerialNoCheck()) {
            SLMessOut(SL_MESS_SERIAL_INVALID);
            goto savefail;
        }
        SLCurWinId =
            SLWinOpen(MESS_MPSYSTEM_MES_SAVE_SAVE, SL_CUR_SLOT_MESS, SL_MESSID_NONE, topWinPosY);
        result = SLFileWrite(SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
        SLWinClose(SLCurWinId);
        SLCurWinId = HUWIN_NONE;
        if(result == CARD_RESULT_NOCARD) {
            SLMessOut(SL_MESS_NOCARD);
        } else if(result == CARD_RESULT_WRONGDEVICE) {
            SLMessOut(SL_MESS_WRONGDEVICE);
        } else if(result == CARD_RESULT_BROKEN) {
            result = HuCardSectorSizeGet(curSlotNo);
            if(result > 0 && result != SAVE_SECTOR_SIZE) {
                SLMessOut(SL_MESS_CARD_INVALID);
                goto savefail;
            } else {
                UnMountCnt = 0;
                result = SLMessOut(SL_MESS_FORMAT_CHOICE);
                if(result == 0) {
                    result = SLFormat(curSlotNo);
                    if(result == CARD_RESULT_READY) {
                        goto repeat;
                    } else {
                        return result;
                    }
                } else {
                    result = CARD_RESULT_BROKEN;
                }
            }
        } else if(result < 0) {
            SLMessOut(SL_MESS_FATAL_ERROR);
            fatalF = TRUE;
        }
    }
    SLFileClose();
    if(result >= 0) {
        HuCardUnMount(curSlotNo);
        return TRUE;
    }
    savefail:
    result = SLMessOut(SL_MESS_NOSAVE_CHOICE);
    if(result != 0) {
        if(!fatalF) {
            SLMessOut(SL_MESS_CARD_REINSERT);
        }
        goto repeat;
    }
    SLSaveFlagSet(FALSE);
    unmount:
    HuCardUnMount(curSlotNo);
    return FALSE;
}

// Reads the file, then compares a checksum of bytes [0, SAVE_BOX_SIZE) with the u16 at offset
// SAVE_BOX_SIZE; discards the result.
s32 SLLoad(void)
{
    s32 result = SLFileOpen(SLSaveFileName);
    if(result >= 0) {
        result = SLFileRead(SAVE_BUF_SIZE, &saveBuf[curSlotNo][0]);
        SLFileClose();
        if(result >= 0) {
            u16 *storedChecksum = ((u16 *)&saveBuf[curSlotNo][SAVE_BOX_SIZE]);
            u16 calculatedChecksum = SLCheckSumGet(0, SAVE_BOX_SIZE);
            // This comparison has no effect; the routine still returns FALSE below.
            *storedChecksum == calculatedChecksum;
        }
    }
    HuCardUnMount(curSlotNo);
    return FALSE;
}

// Called after selecting a loaded box; restores common state and its save-change snapshot.
void SLCommonLoad(void)
{
    u32 boxDataOffset = boxDataOfs[curBoxNo];
    GW_COMMON commonBackup;
    GW_COMMON *commonSnapshot;
    memcpy(&GwCommon, &saveBuf[curSlotNo][boxDataOffset+SAVE_BOX_COMMON_OFS], sizeof(GW_COMMON));
    commonBackup = GwCommon;
    commonSnapshot = &commonBackup;
    memcpy(&SLGwCommonBackup, commonSnapshot, sizeof(GW_COMMON));
}

// Copies common state from an arbitrary save-buffer slot and box to the caller's structure.
void SLCommonLoadCopy(GW_COMMON *commonData, s16 slotNo, s16 boxNo)
{
    u32 boxDataOffset = boxDataOfs[boxNo];
    memcpy(commonData, &saveBuf[slotNo][boxDataOffset+SAVE_BOX_COMMON_OFS], sizeof(GW_COMMON));
}

// Called when resuming a board; restores system/player state and player-selection settings.
void SLBoardLoad(void)
{
    u32 boxDataOffset = boxDataOfs[curBoxNo];
    s16 i;
    memcpy(&GwSystem, &saveBuf[curSlotNo][boxDataOffset + SAVE_BOX_BOARD_SYSTEM_OFS],
           sizeof(GW_SYSTEM));
    for(i=0; i<GW_PLAYER_MAX; i++) {
        memcpy(&GwPlayer[i],
               &saveBuf[curSlotNo]
                       [boxDataOffset + SAVE_BOX_BOARD_PLAYER_OFS + (i * sizeof(GW_PLAYER))],
               sizeof(GW_PLAYER));
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].comDif = GwPlayer[i].comDif;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].grpNo = GwPlayer[i].team;
    }
}

// Reads the selected card's serial number for later replacement-card detection.
s32 SLSerialNoGet(void)
{
    return CARDGetSerialNo(curSlotNo, &SLSerialNo[curSlotNo]);
}

// Checked during save access; accepts an unset serial or failed read, otherwise detects card swaps.
BOOL SLSerialNoCheck(void)
{
    u64 serialNo;
    if(SLSerialNo[curSlotNo] == 0) {
        return TRUE;
    } else {
        s32 result = CARDGetSerialNo(curSlotNo, &serialNo);
        if(result < 0) {
            return TRUE;
        }
        if(SLSerialNo[curSlotNo] != serialNo) {
            return FALSE;
        } else {
            return TRUE;
        }
    }
}

// Used before accepting a save box; compares its stored trailing checksum with its data bytes.
BOOL SLCheckSumBoxSlotCheck(s16 slotNo, s16 boxNo)
{
    u32 boxDataOffset = boxDataOfs[boxNo];
    u16 *storedChecksum = (u16 *)&saveBuf[slotNo][boxDataOffset+SAVE_BOX_SIZE-2];
    u16 calculatedChecksum = SLCheckSumSlotGet(slotNo, boxDataOffset, SAVE_BOX_SIZE-2);
    if(*storedChecksum == calculatedChecksum) {
        return TRUE;
    } else {
        return FALSE;
    }
}

// Checks the currently selected box in the currently selected card slot.
BOOL SLCheckSumCheck(void)
{
    return SLCheckSumBoxSlotCheck(curSlotNo, curBoxNo);
}

// Returns the 16-bit complement of the byte sum over a range in one card-slot buffer.
u16 SLCheckSumSlotGet(s16 slotNo, u32 offset, u32 byteCount)
{
    u32 byteSum;
    u32 i;
    for(i=byteSum=0; i<byteCount; i++) {
        byteSum = byteSum+saveBuf[slotNo][offset+i];
    }
    byteSum = ~byteSum;
    (void)byteSum;
    (void)byteSum;
    (void)byteSum;
    (void)i;
    (void)i;
    return ((u16)byteSum) & 0xFFFF;
}

// Calculates a checksum over the selected card-slot buffer.
u16 SLCheckSumGet(u32 offset, u32 byteCount)
{
    return SLCheckSumSlotGet(curSlotNo, offset, byteCount);
}

// Stores the selected box's checksum in its final two bytes, most-significant byte first.
void SLCheckSumBoxSet(void)
{
    u32 boxDataOffset = boxDataOfs[curBoxNo];
    u16 checksum = SLCheckSumGet(boxDataOffset, SAVE_BOX_SIZE-2);
    saveBuf[curSlotNo][boxDataOffset+SAVE_BOX_SIZE-2] = (checksum >> 8) & 0xFF;
    saveBuf[curSlotNo][boxDataOffset+SAVE_BOX_SIZE-1] = checksum & 0xFF;
}

// Updates all three primary box checksums, then restores the previously selected box.
void SLCheckSumBoxAllSet(void)
{
    s16 oldBoxNo = curBoxNo;
    for(curBoxNo=0; curBoxNo<SAVE_BOX_MAX; curBoxNo++) {
        SLCheckSumBoxSet();
    }
    curBoxNo = oldBoxNo;
}

// Copies all primary boxes into the corresponding backup-box region.
void SLSaveBackup(void)
{
    memcpy(&saveBuf[curSlotNo][SAVE_BOXBACKUP_OFS(0)], &saveBuf[curSlotNo][SAVE_BOX_OFS(0)],
           SAVE_BOX_SIZE * SAVE_BOX_MAX);
}

// Replaces one primary box with its backup copy in the specified card-slot buffer.
void SLBoxBackupSlotLoad(s16 slotNo, s16 boxNo)
{
    memcpy(&saveBuf[slotNo][boxDataOfs[boxNo]],
           &saveBuf[slotNo][boxDataOfs[boxNo + SAVE_BOXNO_BACKUP]], SAVE_BOX_SIZE);
}

// Restores a backup into the selected card slot for the requested box.
void SLBoxBackupLoad(s16 boxNo)
{
    SLBoxBackupSlotLoad(curSlotNo, boxNo);
}

// Returns the byte offset of a primary or backup box in the save-file buffer.
u32 SLBoxDataOffsetGet(s16 boxNo)
{
    return boxDataOfs[boxNo];
}

// Configures card-file comment/icon addresses and banner/icon formats and animation, restoring the
// prior reset-suppression flag.
s32 SLStatSet(BOOL errorOutF)
{
    CARDStat stat;
    s32 fileNo = curFileInfo.fileNo;
    s32 previousResetDisableF;
    s32 result = CARDGetStatus(curSlotNo, fileNo, &stat);
    if(result == CARD_RESULT_NOCARD) {
        if(errorOutF) {
            SLMessOut(SL_MESS_NOCARD);
        }
        return CARD_RESULT_NOCARD;
    } else if(result < 0) {
        if(errorOutF) {
            SLMessOut(SL_MESS_FATAL_ERROR);
        }
        return CARD_RESULT_FATAL_ERROR;
    }
    previousResetDisableF = HuSRDisableF;
    HuSRDisableF = TRUE;
    CARDSetCommentAddress(&stat, 0);
    CARDSetIconAddress(&stat, 64);
    CARDSetBannerFormat(&stat, CARD_STAT_BANNER_C8);
    CARDSetIconFormat(&stat, 0, CARD_STAT_ICON_C8);
    CARDSetIconFormat(&stat, 1, CARD_STAT_ICON_C8);
    CARDSetIconFormat(&stat, 2, CARD_STAT_ICON_C8);
    CARDSetIconFormat(&stat, 3, CARD_STAT_ICON_C8);
    CARDSetIconSpeed(&stat, 0, CARD_STAT_SPEED_MIDDLE);
    CARDSetIconSpeed(&stat, 1, CARD_STAT_SPEED_MIDDLE);
    CARDSetIconSpeed(&stat, 2, CARD_STAT_SPEED_MIDDLE);
    CARDSetIconSpeed(&stat, 3, CARD_STAT_SPEED_MIDDLE);
    CARDSetIconSpeed(&stat, 4, CARD_STAT_SPEED_END);
    CARDSetIconAnim(&stat, CARD_STAT_ANIM_BOUNCE);
    result = CARDSetStatus(curSlotNo, fileNo, &stat);
    HuSRDisableF = previousResetDisableF;
    if(result == CARD_RESULT_NOCARD) {
        if(errorOutF) {
            SLMessOut(SL_MESS_NOCARD);
        }
        return CARD_RESULT_NOCARD;
    } else if(result < 0) {
        if(errorOutF) {
            SLMessOut(SL_MESS_FATAL_ERROR);
        }
        return CARD_RESULT_FATAL_ERROR;
    }
    return result;
}

// Called by file open/create; mounts curSlotNo and validates sector size (slotNo is unused).
s32 SLCardMount(s16 slotNo)
{
    s32 result;
    mount:
    result = HuCardMount(curSlotNo);
    if(result == CARD_RESULT_WRONGDEVICE) {
        SLMessOut(SL_MESS_WRONGDEVICE);
        return result;
    } else if(result == CARD_RESULT_FATAL_ERROR) {
        SLMessOut(SL_MESS_FATAL_ERROR);
        return CARD_RESULT_FATAL_ERROR;
    } else if(result == CARD_RESULT_NOCARD) {
        SLMessOut(SL_MESS_NOCARD);
        return CARD_RESULT_NOCARD;
    } else if(result == CARD_RESULT_BROKEN) {
        result = HuCardSectorSizeGet(curSlotNo);
        if(result > 0 && result != SAVE_SECTOR_SIZE) {
            SLMessOut(SL_MESS_CARD_INVALID);
            return CARD_RESULT_WRONGDEVICE;
        } else {
            UnMountCnt = 0;
            result = SLMessOut(SL_MESS_FORMAT_CHOICE);
            if(result == 0) {
                result = SLFormat(curSlotNo);
                if(result == CARD_RESULT_READY) {
                    goto mount;
                } else {
                    return result;
                }
            } else {
                return CARD_RESULT_FATAL_ERROR;
            }
        }
    } else {
        result = HuCardSectorSizeGet(curSlotNo);
        if(result < 0) {
            SLMessOut(SL_MESS_FATAL_ERROR);
            return result;
        } else if(result != SAVE_SECTOR_SIZE) {
            SLMessOut(SL_MESS_CARD_INVALID);
            return CARD_RESULT_WRONGDEVICE;
        } else {
            return CARD_RESULT_READY;
        }
    }
}

// Called after a format prompt; formats curSlotNo and initializes save data (slotNo is unused).
s32 SLFormat(s16 slotNo)
{
    s16 result;
    if(UnMountCnt & (1 << curSlotNo)) {
        SLMessOut(SL_MESS_CARD_FORMAT_UNMOUNT);
        UnMountCnt = 0;
        return CARD_RESULT_READY;
    } else {
        HUWINID winId = SLWinOpen(MESS_MPSYSTEM_CARD_FORMAT_WARN, SL_CUR_SLOT_MESS, SL_MESSID_NONE,
                                  bottomWinPosY);
        HuPrcSleep(30);
        (void)winId;
        (void)winId;
        if(UnMountCnt & (1 << curSlotNo)) {
            SLWinClose(winId);
            SLMessOut(SL_MESS_CARD_FORMAT_UNMOUNT);
            UnMountCnt = 0;
            return CARD_RESULT_READY;
        } else {
            result = HuCardFormat(curSlotNo);
            SLSerialNo[curSlotNo] = 0;
            if(result < 0) {
                SLWinClose(winId);
            }
            if(result == CARD_RESULT_FATAL_ERROR) {
                SLMessOut(SL_MESS_FORMAT_ERROR);
                SLMessOut(SL_MESS_FATAL_ERROR);
                return CARD_RESULT_FATAL_ERROR;
            } else if(result == CARD_RESULT_NOCARD) {
                SLMessOut(SL_MESS_NOCARD);
                return CARD_RESULT_NOCARD;
            } else if(result == CARD_RESULT_WRONGDEVICE) {
                SLMessOut(SL_MESS_WRONGDEVICE);
                return result;
            } else {
                OSTime time;
                SLSerialNoGet();
                SLWinClose(winId);
                time = OSGetTime();
                SLSaveDataMake(FALSE, &time);
                SLCheckSumBoxAllSet();
                return result;
            }
        }
    }
    (void)result;
    (void)result;
}

// Registers a window owned by the caller so save messages reuse it.
void SLWinIdSet(HUWINID winId)
{
    SLWinId = winId;
}

// Opens or reuses a message window, installs optional insert strings, and waits for the message to
// finish.
static HUWINID SLWinOpen(s32 mesNum, s32 insertMesNum1, s32 insertMesNum2, s16 posY)
{
    HUWINID winId;
    HuVec2f size;
    if(SLWinId == HUWIN_NONE) {
        HuWinInit(1);
    }
    if(insertMesNum1 != SL_MESSID_NONE) {
        HuWinInsertMesSizeGet(insertMesNum1, 0);
    }
    if(insertMesNum2 != SL_MESSID_NONE) {
        HuWinInsertMesSizeGet(insertMesNum2, 1);
    }
    HuWinMesMaxSizeGet(1, &size, mesNum);
    if(SLWinId == HUWIN_NONE) {
        winId = HuWinWarningCreate(-10000.0f, posY, size.x, size.y);
        HuWinWarningOpen(winId);
    } else {
        winId = SLWinId;
    }
    if(insertMesNum1 != SL_MESSID_NONE) {
        HuWinInsertMesSet(winId, insertMesNum1, 0);
    }
    if(insertMesNum2 != SL_MESSID_NONE) {
        HuWinInsertMesSet(winId, insertMesNum2, 1);
    }
    HuWinMesSet(winId, mesNum);
    HuWinMesWait(winId);
    return winId;
}

// Closes a temporary warning window but leaves a caller-owned save window open.
static void SLWinClose(HUWINID winId)
{
    if(SLWinId == winId) {
        return;
    }
    if(winId < 0) {
        return;
    }
    HuWinWarningClose(winId);
    HuWinWarningKill(winId);
}

// Displays card/save messages, waits for A on the reinsert prompt, and returns choices for prompts
// that offer one.
s16 SLMessOut(s16 messId)
{
    u32 messageId;
    HUWINID winId;
    s16 choiceNo = -1;
    u32 insertedMessageId = 0;
    BOOL hasChoicePrompt = FALSE;
    HUWIN *window;
    if(SLWinId == HUWIN_NONE) {
        HuWinInit(1);
    }
    SLWinClose(SLCurWinId);
    SLCurWinId = HUWIN_NONE;
    switch(messId) {
        case SL_MESS_NOCARD:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertedMessageId = SL_CUR_SLOT_MESS;
            messageId = MESS_MPSYSTEM_CARD_NOCARD;
            break;

        case SL_MESS_FATAL_ERROR:
            messageId = MESS_MPSYSTEM_CARD_FATAL_ERROR;
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertedMessageId = SL_CUR_SLOT_MESS;
            break;

        case SL_MESS_CARD_NOENT:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertedMessageId = SL_CUR_SLOT_MESS;
            messageId = MESS_MPSYSTEM_CARD_NOENT;
            break;

        case SL_MESS_CARD_INSSPACE:
            messageId = MESS_MPSYSTEM_CARD_INSSPACE;
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertedMessageId = SL_CUR_SLOT_MESS;
            break;

        case SL_MESS_CARD_FULL:
            messageId = MESS_MPSYSTEM_CARD_FULL;
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertedMessageId = SL_CUR_SLOT_MESS;
            break;

        case SL_MESS_FORMAT_CHOICE:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertedMessageId = SL_CUR_SLOT_MESS;
            messageId = MESS_MPSYSTEM_CARD_FORMAT_CHOICE;
            hasChoicePrompt = TRUE;
            break;

        case SL_MESS_FORMAT_ERROR:
            messageId = MESS_MPSYSTEM_CARD_FORMAT_ERROR;
            break;

        case SL_MESS_WRONGDEVICE:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertedMessageId = SL_CUR_SLOT_MESS;
            messageId = MESS_MPSYSTEM_CARD_WRONGDEVICE;
            break;

        case SL_MESS_CARD_INVALID:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertedMessageId = SL_CUR_SLOT_MESS;
            messageId = MESS_MPSYSTEM_CARD_INVALID;
            break;

        case SL_MESS_SERIAL_INVALID:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertedMessageId = SL_CUR_SLOT_MESS;
            messageId = MESS_MPSYSTEM_CARD_SERIAL_INVALID;
            break;

        case SL_MESS_NOSAVE_CHOICE:
            messageId = MESS_MPSYSTEM_NOSAVE_CHOICE;
            hasChoicePrompt = TRUE;
            break;

        case SL_MESS_CARD_REINSERT:
            HuWinInsertMesSizeGet(SL_CUR_SLOT_MESS, 0);
            insertedMessageId = SL_CUR_SLOT_MESS;
            messageId = MESS_MPSYSTEM_CARD_REINSERT;
            break;

        case SL_MESS_CARD_FORMAT_UNMOUNT:
            messageId = MESS_MPSYSTEM_CARD_FORMAT_UNMOUNT;
            break;
    }
    if(SLWinId == HUWIN_NONE) {
        HuVec2f size;
        size.x = 478;
        size.y = 94;
        winId = HuWinWarningCreate(-10000.0f, bottomWinPosY, size.x, size.y);
    } else {
        winId = SLWinId;
    }
    window = &winData[winId];
    window->padMask = HUWIN_PLAYER_1;
    if(insertedMessageId) {
        HuWinInsertMesSet(winId, insertedMessageId, 0);
    }
    HuWinAttrSet(winId, HUWIN_ATTR_NOCANCEL);
    HuWinWarningOpen(winId);
    HuWinMesSet(winId, messageId);
    HuWinMesWait(winId);
    if(hasChoicePrompt) {
        if(messId == SL_MESS_FORMAT_CHOICE) {
            HuWinInsertMesSet(winId, SL_CUR_SLOT_MESS, 0);
            HuWinMesSet(winId, MESS_MPSYSTEM_CARD_FORMAT);
            HuWinMesWait(winId);
        }
        choiceNo = HuWinChoiceGet(winId, -1);
    }
    if(messId == SL_MESS_CARD_REINSERT) {
        while(!(HuPadBtnDown[0] & PAD_BUTTON_A)) {
            HuPrcVSleep();
        }
        HuAudFXPlay(MSM_SE_CMN_02);
    }
    if(SLWinId == HUWIN_NONE) {
        HuWinWarningClose(winId);
        HuWinWarningKill(winId);
    }
    return choiceNo;
}

// At the per-turn save point, saves under a wipe when enabled and permitted, forcing
// GwCommon.saveEnableF true in the saved state.
void SLSaveBoardTurnExec(void)
{
    Hu3DAllKill();
    HuSprClose();
    HuSprInit();
    espInit();
    HuWinComKeyReset();
    if(!SaveEnableF || GWSaveModeGet() == GW_SAVE_MODE_NEVER) {
        return;
    }
    HuPrcVSleep();
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 1);
    WipeWait();
    bottomWinPosY = 190.0f;
    topWinPosY = 80.0f;
    GwCommon.saveEnableF = TRUE;
    SLBoardSave();
    SLCommonSet();
    if(SLSave()) {
        saveExecF = TRUE;
    } else {
        saveExecF = FALSE;
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_PREV, 1);
    WipeWait();
}

// At board end, saves when enabled; presents and clears any bank-star award first, and stores
// saveEnableF false in common state.
void SLSaveBoardEndExec(void)
{
    s16 foregroundSpriteId;
    s16 backgroundSpriteId;
    Hu3DAllKill();
    HuSprClose();
    HuSprInit();
    espInit();
    HuWinComKeyReset();
    if(!SaveEnableF) {
        return;
    }
    HuPrcVSleep();
    if(GwCommon.bankStarAward == 0) {
        foregroundSpriteId = espEntry(WIN_ANM_52, 5010, 0);
        espPosSet(foregroundSpriteId, 288.0f, 240.0f);
        espScaleSet(foregroundSpriteId, 10, 10);
        espColorSet(foregroundSpriteId, 0, 0, 0);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 1);
        WipeWait();
        bottomWinPosY = 190.0f;
        topWinPosY = 80.0f;
        GwCommon.saveEnableF = FALSE;
        SLBoardSave();
        SLCommonSet();
        if(SLSave()) {
            saveExecF = TRUE;
        } else {
            saveExecF = FALSE;
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_PREV, 1);
        WipeWait();
        espKill(foregroundSpriteId);
    } else {
        foregroundSpriteId = espEntry(WIN_ANM_52, 5010, 0);
        espPosSet(foregroundSpriteId, 288.0f, 240.0f);
        espScaleSet(foregroundSpriteId, 10, 10);
        espColorSet(foregroundSpriteId, SL_BANK_AWARD_TINT_RED, SL_BANK_AWARD_TINT_GREEN,
                    SL_BANK_AWARD_TINT_BLUE);
        backgroundSpriteId = espEntry(WIN_ANM_save_bg, 5000, 0);
        espPosSet(backgroundSpriteId, 288.0f, 209.0f);
        espAttrReset(backgroundSpriteId, HUSPR_ATTR_DISPOFF);
        Hu3DBGColorSet(SL_BANK_AWARD_TINT_RED, SL_BANK_AWARD_BACKDROP_GREEN,
                       SL_BANK_AWARD_TINT_BLUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 20);
        WipeWait();
        SLBankAwardExec(GwCommon.bankStarAward);
        GwCommon.bankStarAward = 0;
        GwCommon.saveEnableF = FALSE;
        SLBoardSave();
        SLCommonSet();
        if(SLSave()) {
            saveExecF = TRUE;
        } else {
            saveExecF = FALSE;
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 20);
        WipeWait();
        espKill(backgroundSpriteId);
        espKill(foregroundSpriteId);
    }
}

// Displays up to 9,999 bank stars in a result window, waits for the message to finish, then closes
// the window.
static HUWINID SLBankAwardExec(s16 award)
{
    HUWINID winId = HUWIN_NONE;
    HuVec2f size;
    HuWinInit(1);
    if(award > 9999) {
        award = 9999;
    }
    sprintf(bankAwardMes, "%d", award);
    HuWinInsertMesSizeGet((s32)bankAwardMes, 0);
    HuWinMesMaxSizeGet(1, &size, MESS_MPSYSTEM_MES_BANK_AWARD);
    winId = HuWinExCreateFrame(-10000.0f, 480.0f - size.y - 16.0f - 40.0f, size.x, size.y - 16.0f,
                               -1, 0);
    HuWinInsertMesSet(winId, (s32)bankAwardMes, 0);
    HuWinExOpen(winId);
    HuWinMesSet(winId, MESS_MPSYSTEM_MES_BANK_AWARD);
    HuWinMesWait(winId);
    HuWinExClose(winId);
    HuWinExKill(winId);
    return winId;
}

// Saves changed common state from options/bank overlays, clearing any displayed bank-star award
// before saving; unusedModeFlag is ignored.
void SLSaveModeExec(s16 unusedModeFlag)
{
    s16 foregroundSpriteId;
    s16 backgroundSpriteId;
    HuWinComKeyReset();
    if(!SaveEnableF) {
        return;
    }
    if(!SLSaveCheck()) {
        return;
    }
    HuPrcVSleep();
    if(GwCommon.bankStarAward == 0) {
        foregroundSpriteId = espEntry(WIN_ANM_52, 5010, 0);
        espPosSet(foregroundSpriteId, 288.0f, 240.0f);
        espScaleSet(foregroundSpriteId, 10, 10);
        espColorSet(foregroundSpriteId, 0, 0, 0);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 1);
        WipeWait();
        bottomWinPosY = 190.0f;
        topWinPosY = 80.0f;
        SLCommonSet();
        if(SLSave()) {
            saveExecF = TRUE;
        } else {
            saveExecF = FALSE;
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_PREV, 1);
        WipeWait();
        espKill(foregroundSpriteId);
    } else {
        foregroundSpriteId = espEntry(WIN_ANM_52, 5010, 0);
        espPosSet(foregroundSpriteId, 288.0f, 240.0f);
        espScaleSet(foregroundSpriteId, 10, 10);
        espColorSet(foregroundSpriteId, SL_BANK_AWARD_TINT_RED, SL_BANK_AWARD_TINT_GREEN,
                    SL_BANK_AWARD_TINT_BLUE);
        backgroundSpriteId = espEntry(WIN_ANM_save_bg, 5000, 0);
        espPosSet(backgroundSpriteId, 288.0f, 209.0f);
        espAttrReset(backgroundSpriteId, HUSPR_ATTR_DISPOFF);
        Hu3DBGColorSet(SL_BANK_AWARD_TINT_RED, SL_BANK_AWARD_BACKDROP_GREEN,
                       SL_BANK_AWARD_TINT_BLUE);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 20);
        WipeWait();
        SLBankAwardExec(GwCommon.bankStarAward);
        GwCommon.bankStarAward = 0;
        SLCommonSet();
        if(SLSave()) {
            saveExecF = TRUE;
        } else {
            saveExecF = FALSE;
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 20);
        WipeWait();
        espKill(backgroundSpriteId);
        espKill(foregroundSpriteId);
    }
}

// Compares common state with both time fields zeroed, restores GwCommon.time, and leaves
// SLGwCommonBackup.time zero.
s32 SLSaveCheck(void)
{
    OSTime time = GwCommon.time;
    s32 result;
    GW_COMMON commonBackup;
    GW_COMMON *commonSnapshot;
    GwCommon.time = 0;
    SLGwCommonBackup.time = 0;
    commonBackup = SLGwCommonBackup;
    commonSnapshot = &commonBackup;
    result = memcmp(&GwCommon, commonSnapshot, sizeof(GW_COMMON));
    GwCommon.time = time;
    return result;
}
