/* File Select memory-card checks, save repair, load, erase, and write flow. */
#include "dolphin/math.h"

#include "REL/fileseldll.h"

#include "game/audio.h"
#include "game/saveload_layout.h"
#include "game/pad.h"
#include "game/memory.h"
#include "game/main.h"
#include "game/wipe.h"
#include "messnum/file_select.h"

u32 lbl_1_data_568 = FILECARD_MESSID_NONE;       /* this TU: save-mes id (fileMesId), init -1 */

/* File Select save state supplied by the main executable. */
extern s16 curSlotNo;
extern s16 SLWinId;
extern char SLSaveFileName[];
extern char SLEraseStr[];
extern u8 saveBuf[][SAVE_BUF_SIZE];
extern s64 SLSerialNo[];
extern s32 SR_ExecResetMenu;
extern CARDFileInfo curFileInfo;

/* Save-library operations used by the card flow below. */
s32 SLSaveFlagGet(void);
void SLWinIdSet(s16 winId);
s32 SLSerialNoCheck(void);
void SLSerialNoGet(void);
s32 SLStatSet(s32 stat);
void SLCheckSumBoxAllSet(void);
void SLSaveBackup(void);
s32 SLCurBoxNoSet(s32 boxNo);
void SLSaveDataMake(s32 initializationMode, OSTime *time);
u16 SLCheckSumGet(s32 start, s32 len);
void SLCurSlotNoSet(s32 slotNo);
void SLSaveEmptySet(s16 slot, s16 idx);
s32 SLBoxDataOffsetGet(s16 boxNo);
s32 SLCheckSumCheck(void);
void SLBoxBackupLoad(s16 boxNo);
void SLCommonLoad(void);

/* ==================== forward decls: functions defined in this TU ==================== */
s32 FileCheckCardSpace(void);
s32 FileCardWarning(s16 winId);
s32 FileCardErrorExec(s32 err, s16 winId);
s32 FileCardOpen(const char *fileName);
s32 FileCardRead(s32 length, void *addr);
s32 FileCardClose(void);
s32 FileCardMount(s16 slot);
s32 FileCardFormat(s16 slot);
s32 FileCardLoad(void);
s32 FileCardCopy(const char *fileName, s32 size, void *addr);
s32 FileCardWrite(s32 length, const void *addr);
s32 FileSave(void);
s16 FileMessOut(s16 mode);
s32 FileCardChoice(s16 choiceFlags, s16 sourceWindowId);

/* ======================================================================== */
/* functions in strict ascending target-address order */
/* ======================================================================== */

/* 0xB5B4 */
/* After name entry, hide its sprites and all three stored window IDs. Only the first two
* IDs are initialized as keyboard windows; the third remains zero. */
void FileCommonInit(void)
{
    s16 i;

    for (i = 0; i < 18; i++) {
        HuSprAttrSet(lbl_1_bss_42E, i, 4);
    }
    for (i = 0; i < 3; i++) {
        HuWinDispOff(lbl_1_bss_3AC[i]);
    }
}

/* FileselMain calls this to select card slot A, inspect its three save boxes, and repair
 * damaged data. Any previously selected card slot is overwritten. */
s32 FileBoxInit(s16 sourceWindowId)
{
    s32 brokenFlag = 0;
    s16 i;
    s16 winId;
    s32 result;
    u16 *p;
    s32 boxStatus[3];
    OSTime time;

    UnMountCnt = 0;
    if (sourceWindowId == -1) {
        winId = HuWinWarningCreate(HUWIN_POS_CENTER, 160.0f, 478, 94);
    } else {
        winId = sourceWindowId;
    }
    SLWinIdSet(winId);
    SLCurSlotNoSet(0);
    while (1) {
        result = FileCardLoad();
        if (result == -4) {
            for (i = 0; i < 3; i++) {
                lbl_1_bss_D8[i].hasSave = 0;
            }
            result = FileCheckCardSpace();
            if (result == 0) {
                break;
            } else {
                goto cardError;
            }
        } else if (result != 0) {
        cardError:
            result = FileCardErrorExec(result, winId);
            if (result == -3 || result == -4 || result == -5) {
                continue;
            }
            if (result == FILESEL_RESULT_CANCEL) {
                HuWinWarningOpen(winId);
                HuWinAttrSet(winId, HUWIN_ATTR_NOCANCEL);
                HuWinMesSet(winId, FILE_SELECT_TITLE_RETURN);
                HuWinMesWait(winId);
                result = HuWinChoiceGet(winId, 0);
                HuWinWarningClose(winId);
                if (result != 0) {
                    continue;
                }
                result = -2;
            }
            goto cleanup;
        } else if (strncmp((char *)&saveBuf[curSlotNo][SAVE_COMMENT_OFS], "ERASE", 5) == 0) {
            time = OSGetTime();
            SLSaveDataMake(1, &time);
            for (i = 0; i < 3; i++) {
                SLSaveEmptySet(curSlotNo, i);
                lbl_1_bss_D8[i].hasSave = 0;
            }
            break;
        } else {
            brokenFlag = 0;
            for (i = 0; i < 3; i++) {
                boxStatus[i] = 0;
            }
            p = (u16 *)(saveBuf[curSlotNo] + SAVE_ICONBANNER_CHECKSUM_OFS);
            if (*p != SLCheckSumGet(0, SAVE_ICONBANNER_SIZE)) {
                OSReport("IconBanner Area Broken!\n");
                time = OSGetTime();
                SLSaveDataMake(0, &time);
                brokenFlag = 1;
            }
            for (i = 0; i < 3; i++) {
                SLCurBoxNoSet(i);
                if (strncmp((char *) (saveBuf[curSlotNo] + SLBoxDataOffsetGet(i)), "SAVE", 4) ==
                        0 ||
                    strncmp((char *) (saveBuf[curSlotNo] + SLBoxDataOffsetGet(i + 3)), "SAVE", 4) ==
                        0) {
                    if (SLCheckSumCheck() == 0) {
                        OSReport("Box%d Broken!\n", i);
                        SLBoxBackupLoad(i);
                        if (SLCheckSumCheck() == 0) {
                            SLSaveEmptySet(curSlotNo, i);
                            boxStatus[i] = 2;
                        } else {
                            boxStatus[i] = 1;
                        }
                    }
                } else if (strncmp((char *) (saveBuf[curSlotNo] + SLBoxDataOffsetGet(i)), "EMPT",
                                   4) != 0 &&
                           strncmp((char *) (saveBuf[curSlotNo] + SLBoxDataOffsetGet(i + 3)),
                                   "EMPT", 4) != 0) {
                    boxStatus[i] = 3;
                    SLSaveEmptySet(curSlotNo, i);
                } else if (strncmp((char *) (saveBuf[curSlotNo] + SLBoxDataOffsetGet(i)), "EMPT",
                                   4) == 0 ||
                           strncmp((char *) (saveBuf[curSlotNo] + SLBoxDataOffsetGet(i + 3)),
                                   "EMPT", 4) == 0) {
                    SLSaveEmptySet(curSlotNo, i);
                }
                SLCommonLoad();
                if (strncmp((char *)&GwCommon, "SAVE", 4) == 0) {
                    lbl_1_bss_D8[i].hasSave = 1;
                    OSTicksToCalendarTime(GwCommon.time, &lbl_1_bss_D8[i].saveTime);
                    memcpy(lbl_1_bss_D8[i].fileName, GwCommon.name, FILESEL_FILENAME_LENGTH);
                    lbl_1_bss_D8[i].displayNumber = GWBankStarGet();
                    lbl_1_bss_D8[i].patternVariant = GwCommon.lastBoard;
                } else {
                    lbl_1_bss_D8[i].hasSave = 0;
                }
                GwCommon.languageNo = GwLanguage;
            }
            if (brokenFlag == 0 && boxStatus[0] == 0 && boxStatus[1] == 0 && boxStatus[2] == 0) {
                break;
            }
            HuWinWarningOpen(winId);
            if (boxStatus[0] != 2 && boxStatus[0] != 3 && boxStatus[1] != 2 && boxStatus[1] != 3 &&
                boxStatus[2] != 2 && boxStatus[2] != 3) {
                HuWinInsertMesSet(winId, curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
                HuWinMesSet(winId, FILE_SELECT_SAVE_SLOT_REPAIR);
                HuWinMesWait(winId);
            } else {
                for (i = 0; i < 3; i++) {
                    char buf[8];
                    /* Formatting the box number also stores the format-string address in result;
                    * the later save call replaces it before the error checks. */
                    sprintf(buf, (char *)(result = (s32)"%d"), i + 1);
                    if (boxStatus[i] == 1) {
                        HuWinInsertMesSet(winId, (u32)buf, 0);
                        HuWinMesSet(winId, FILE_SELECT_SAVE_REPAIR);
                        HuWinMesWait(winId);
                    } else if (boxStatus[i] == 2) {
                        HuWinInsertMesSet(winId, (u32)buf, 0);
                        HuWinMesSet(winId, FILE_SELECT_SAVE_BROKEN);
                        HuWinMesWait(winId);
                    } else if (boxStatus[i] == 3) {
                        HuWinInsertMesSet(winId, curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
                        HuWinInsertMesSet(winId, (u32)buf, 1);
                        HuWinMesSet(winId, FILE_SELECT_INVALID_MAGIC);
                        HuWinMesWait(winId);
                    }
                }
            }
            HuWinWarningClose(winId);
            UnMountCnt = 0;
            result = FileSaveMesOpen(-1, FILE_SELECT_SAVING);
        }
        if (UnMountCnt != 0) {
            result = -4;
            UnMountCnt = 0;
        }
            if (result == FILESEL_RESULT_CANCEL) {
                HuWinWarningOpen(winId);
                HuWinAttrSet(winId, HUWIN_ATTR_NOCANCEL);
            HuWinMesSet(winId, FILE_SELECT_TITLE_RETURN);
            HuWinMesWait(winId);
            result = HuWinChoiceGet(winId, 0);
            HuWinWarningClose(winId);
            if (result != 0) {
                continue;
            }
            result = -2;
            goto cleanup;
        }
        if (result == -1) {
            goto cleanup;
        }
        if (result == -4) {
            continue;
        }
        if (result == -5) {
            continue;
        }
        break;
    }
    result = 0;
    for (i = 0; i < 3; i++) {
        fn_1_36C4(i, i);
    }
cleanup:
    if (sourceWindowId == -1) {
        HuWinWarningClose(winId);
        SLWinIdSet(-1);
        HuWinWarningKill(winId);
    }
    return result;
}

/* 0xBF9C */
/* FileBoxInit calls this after finding no save file to check whether a save can be created. */
s32 FileCheckCardSpace(void)
{
    s32 result;
    u32 byteNotUsed;
    u32 filesNotUsed;

    result = FileCardMount(curSlotNo);
    if (result < 0) {
        return result;
    }
    result = HuCardSectorSizeGet(curSlotNo);
    /* This second sector-size check rejects negative results only; an unexpected
    * positive size passes here even though FileCardMount checked the earlier size. */
    if (result < 0 && result != SAVE_SECTOR_SIZE) {
        FileMessOut(8);
        return CARD_RESULT_FATAL_ERROR;
    }
    /* Ignore the free-space query's status and decide from its byte and file counters. */
    result = HuCardFreeSpaceGet(curSlotNo, &byteNotUsed, &filesNotUsed);
    if (filesNotUsed == 0 && byteNotUsed < SAVE_BUF_SIZE) {
        FileMessOut(4);
        return -9;
    }
    if (filesNotUsed == 0) {
        FileMessOut(2);
        return -9;
    }
    if (byteNotUsed < SAVE_BUF_SIZE) {
        FileMessOut(3);
        return -9;
    }
    return 0;
}

/* 0xC098 */
/* Create or reuse a warning window, then try mounting and opening the save file.
 * On failure, discard that result and remount to check space. A passing space check
 * returns success without reopening the file. A newly created window starts hidden. */
s32 FileCardWarning(s16 winId)
{
    s32 warnId;
    s32 ret;
    s32 code;
    s32 status;
    u32 filesNotUsed;
    u32 byteNotUsed;

    if (winId == -1) {
        warnId = HuWinWarningCreate(HUWIN_POS_CENTER, 160.0f, 478, 94);
    } else {
        warnId = winId;
    }
    SLWinIdSet(warnId);
    ret = HuCardMount(curSlotNo);
    if (ret == 0 && (ret = HuCardOpen(curSlotNo, SLSaveFileName, &curFileInfo)) == 0) {
        ret = 0;
    } else {
        status = FileCardMount(curSlotNo);
        if (status < 0) {
            code = status;
        } else {
            status = HuCardSectorSizeGet(curSlotNo);
            /* This repeated sector-size check rejects negative results only; an unexpected
            * positive size passes here. */
            if (status < 0 && status != SAVE_SECTOR_SIZE) {
                FileMessOut(8);
                code = CARD_RESULT_FATAL_ERROR;
            } else {
                /* Ignore the free-space query's status and decide from its byte and file
                 * counters. */
                status = HuCardFreeSpaceGet(curSlotNo, &byteNotUsed, &filesNotUsed);
                if (filesNotUsed == 0 && byteNotUsed < SAVE_BUF_SIZE) {
                    FileMessOut(4);
                    code = -9;
                } else if (filesNotUsed == 0) {
                    FileMessOut(2);
                    code = -9;
                } else if (byteNotUsed < SAVE_BUF_SIZE) {
                    FileMessOut(3);
                    code = -9;
                } else {
                    code = 0;
                }
            }
        }
        ret = code;
        if (ret == 0) {
            ret = 0;
        } else {
            ret = FileCardErrorExec(ret, warnId);
        }
    }
    if (winId == -1) {
        SLWinIdSet(-1);
        HuWinWarningKill(warnId);
    }
    return ret;
}

/* 0xC278 */
/* After naming an empty box, FileselSelect uses this to test whether the card already has
* the save file. Return one only when both mounting and opening succeed. */
s32 FileTestOpen(void)
{
    s32 ret;

    if ((ret = HuCardMount(curSlotNo)) == 0 &&
        (ret = HuCardOpen(curSlotNo, SLSaveFileName, &curFileInfo)) == 0) {
        return 1;
    }
    return 0;
}

/* 0xC2F0 */
/* Card load/save callers use this dialog loop to handle removal, formatting, and retry choices. */
s32 FileCardErrorExec(s32 err, s16 winId)
{
    s32 result;

    if (UnMountCnt != 0) {
        HuWinWarningOpen(winId);
        HuWinInsertMesSet(winId, curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
        HuWinMesSet(winId, FILE_SELECT_CARD_REMOVED);
        HuWinMesWait(winId);
        HuWinWarningClose(winId);
        UnMountCnt = 0;
        return -4;
    }
    UnMountCnt = 0;
    if (err == -6) {
        for (;;) {
            result = FileCardChoice(FILECARD_FLAG_CANCEL | 0x7, winId);
            if (UnMountCnt != 0 && result == 2) {
                HuWinWarningOpen(winId);
                HuWinInsertMesSet(winId, curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
                HuWinMesSet(winId, FILE_SELECT_CARD_REMOVED);
                HuWinMesWait(winId);
                HuWinWarningClose(winId);
                UnMountCnt = 0;
                return -4;
            }
            if (result == FILESEL_RESULT_CANCEL) {
                HuWinWarningClose(winId);
                UnMountCnt = 0;
                return FILESEL_RESULT_CANCEL;
            }
            if (result == 2) {
                HuWinWarningOpen(winId);
                HuWinInsertMesSet(winId, curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
                HuWinMesSet(winId, FILE_SELECT_CARD_FORMAT);
                HuWinMesWait(winId);
                result = HuWinChoiceGet(winId, 1);
                if (result == 0) {
                    /* Request formatting, then return the retry result regardless of the
                     * format operation's result. */
                    result = FileCardFormat(curSlotNo);
                    HuWinWarningClose(winId);
                    return -3;
                }
                if (UnMountCnt != 0) {
                    HuWinInsertMesSet(winId, curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
                    HuWinMesSet(winId, FILE_SELECT_CARD_REMOVED);
                    HuWinMesWait(winId);
                    HuWinWarningClose(winId);
                    UnMountCnt = 0;
                    return -4;
                }
                FileMessOut(5);
                continue;
            }
            if (result == 1) {
                HuWinWarningClose(winId);
                UnMountCnt = 0;
                return -1;
            }
            HuWinWarningOpen(winId);
            HuWinInsertMesSet(winId, curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
            HuWinMesSet(winId, FILE_SELECT_CARD_INSERT);
            HuWinMesWait(winId);
            while ((HuPadBtnDown[0] & PAD_BUTTON_A) == 0) {
                HuPrcVSleep();
            }
            HuAudFXPlay(MSM_SE_CMN_02);
            HuWinWarningClose(winId);
            UnMountCnt = 0;
            return -5;
        }
    }
    if (err == -9) {
        for (;;) {
            result = FileCardChoice(FILECARD_FLAG_CANCEL | 0xD, winId);
            if (result == FILESEL_RESULT_CANCEL) {
                HuWinWarningClose(winId);
                UnMountCnt = 0;
                return FILESEL_RESULT_CANCEL;
            }
            if (result == 1) {
                HuWinWarningClose(winId);
                UnMountCnt = 0;
                return -1;
            }
            if (result == 8) {
                HuWinWarningOpen(winId);
                HuWinMesSet(winId, FILE_SELECT_GO_TO_IPL_WARNING);
                HuWinMesWait(winId);
                result = HuWinChoiceGet(winId, 1);
                if (result != 0) {
                    UnMountCnt = 0;
                    continue;
                }
                WipeCreate(2, 0, FILESEL_WIPE_DURATION);
                HuAudSStreamAllFadeOut(FILESEL_AUDIO_FADE_DURATION);
                while (WipeCheck()) {
                    HuPrcVSleep();
                }
                HuSRDisableF = 0;
                SR_ExecResetMenu = 1;
                for (;;) {
                    HuPrcVSleep();
                }
            }
            HuWinWarningOpen(winId);
            HuWinInsertMesSet(winId, curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
            HuWinMesSet(winId, FILE_SELECT_CARD_INSERT);
            HuWinMesWait(winId);
            while ((HuPadBtnDown[0] & PAD_BUTTON_A) == 0) {
                HuPrcVSleep();
            }
            HuAudFXPlay(MSM_SE_CMN_02);
            HuWinWarningClose(winId);
            UnMountCnt = 0;
            return -5;
        }
    }
    result = FileCardChoice(FILECARD_FLAG_CANCEL | 0x5, winId);
    if (result == FILESEL_RESULT_CANCEL) {
        HuWinWarningClose(winId);
        UnMountCnt = 0;
        return FILESEL_RESULT_CANCEL;
    }
    if (result == 1) {
        HuWinWarningClose(winId);
        UnMountCnt = 0;
        return -1;
    }
    HuWinWarningOpen(winId);
    HuWinInsertMesSet(winId, curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
    HuWinMesSet(winId, FILE_SELECT_CARD_INSERT);
    HuWinMesWait(winId);
    while ((HuPadBtnDown[0] & PAD_BUTTON_A) == 0) {
        HuPrcVSleep();
    }
    HuAudFXPlay(MSM_SE_CMN_02);
    HuWinWarningClose(winId);
    UnMountCnt = 0;
    return -5;
}

/* 0xC898 */
/* When saving is enabled, mount and open the named save file, reporting the handled card
 * errors. Unlisted open errors, including busy, return success. When saving is disabled,
 * return success without opening the file. */
s32 FileCardOpen(const char *fileName)
{
    s32 ret;

    if (SLSaveFlagGet() == 0) {
        return 0;
    }
    ret = FileCardMount(curSlotNo);
    if (ret < 0) {
        return ret;
    }
    ret = HuCardOpen(curSlotNo, fileName, &curFileInfo);
    if (ret == -4) {
        return -4;
    }
    if (ret == -2) {
        FileMessOut(7);
        return CARD_RESULT_FATAL_ERROR;
    }
    if (ret == CARD_RESULT_FATAL_ERROR) {
        FileMessOut(1);
        return CARD_RESULT_FATAL_ERROR;
    }
    if (ret == -3) {
        FileMessOut(0);
        return -3;
    }
    if (ret == -6) {
        ret = HuCardSectorSizeGet(curSlotNo);
        if (ret > 0 && ret != SAVE_SECTOR_SIZE) {
            FileMessOut(8);
            return -2;
        }
        UnMountCnt = 0;
        FileMessOut(5);
        return -6;
    }
    return 0;
}

/* 0xC9D0 */
/* When saving is enabled, read bytes from the open card file and report read errors; otherwise
 * return success without reading. */
s32 FileCardRead(s32 length, void *addr)
{
    s32 ret;

    if (SLSaveFlagGet() == 0) {
        return 0;
    }
    SLSerialNoGet();
    ret = HuCardRead(&curFileInfo, addr, length, 0);
    if (ret == -3) {
        FileMessOut(0);
    } else if (ret < 0) {
        FileMessOut(1);
    }
    return ret;
}

/* 0xCA58 */
/* Closes the current card file when card saving is enabled. */
s32 FileCardClose(void)
{
    s32 ret;

    if (SLSaveFlagGet() == 0) {
        return 0;
    }
    ret = HuCardClose(&curFileInfo);
    return ret;
}

/* Card operations call this to mount the active slot and validate its sector size.
* Unlisted mount results are discarded; a valid sector size can return success after an
* unlisted mount error. */
/* The slot argument is retained by this interface; mounting uses the active curSlotNo. */
s32 FileCardMount(s16 slot)
{
    s32 ret;

    ret = HuCardMount(curSlotNo);
    if (ret == -2) {
        FileMessOut(7);
        return ret;
    } else if (ret == CARD_RESULT_FATAL_ERROR) {
        FileMessOut(1);
        return CARD_RESULT_FATAL_ERROR;
    } else if (ret == -3) {
        FileMessOut(0);
        return -3;
    } else if (ret == -6) {
        ret = HuCardSectorSizeGet(curSlotNo);
        if (ret > 0 && ret != SAVE_SECTOR_SIZE) {
            FileMessOut(8);
            return -2;
        } else {
            UnMountCnt = 0;
            FileMessOut(5);
            return -6;
        }
    } else {
        ret = HuCardSectorSizeGet(curSlotNo);
        if (ret < 0) {
            FileMessOut(1);
            return ret;
        } else {
            if (ret != SAVE_SECTOR_SIZE) {
                FileMessOut(8);
                return -2;
            }
            return 0;
        }
    }
}
/* 0xCBC8 */
/* After format confirmation, format the active curSlotNo; slot only selects the warning text.
 * Removal before formatting returns success without formatting the card. HuCardFormat clears
 * reset/disc-error suppression even if it was already set. Unhandled negative format results
 * still request serial refresh, rebuild save metadata and checksums, and repeat the
 * message-close call. */
s32 FileCardFormat(s16 slot)
{
    s16 ret;
    s16 win;
    OSTime time;

    if (UnMountCnt & (1 << curSlotNo)) {
        FileMessOut(FILE_MESS_FORMAT_UNMOUNT);
        UnMountCnt = 0;
        return 0;
    }
    win = FileCardMesOpen(FILE_SELECT_CARD_FORMAT_WARNING, slot + FILE_SELECT_CARD_SLOT_A, -1, 70);
    HuPrcSleep(30);
    if (UnMountCnt & (1 << curSlotNo)) {
        FileCardMesClose(win);
        FileMessOut(FILE_MESS_FORMAT_UNMOUNT);
        UnMountCnt = 0;
        return 0;
    }
    ret = HuCardFormat(curSlotNo);
    SLSerialNo[curSlotNo] = 0;
    if (ret < 0) {
        FileCardMesClose(win);
    }
    if (ret == -128) {
        FileMessOut(6);
        FileMessOut(1);
        return -128;
    }
    if (ret == -3) {
        FileMessOut(0);
        return -3;
    }
    if (ret == -2) {
        FileMessOut(7);
        return ret;
    }
    SLSerialNoGet();
    FileCardMesClose(win);
    SLCurBoxNoSet(0);
    time = OSGetTime();
    SLSaveDataMake(0, &time);
    SLCheckSumBoxAllSet();
    return ret;
}

/* 0xCDA8 */
/* FileBoxInit reads the save image when saving is enabled; otherwise it uses the existing
 * buffer. Unlisted open errors, including busy, are treated as success before reading.
 * The card-close result and checksum comparison over the first SAVE_BOX_SIZE bytes are
 * ignored. */
s32 FileCardLoad(void)
{
    s32 rc;
    s32 err;
    s32 ret;
    s32 rd;
    u16 *p;
    s32 status;
    u16 sum;
    u8 *buf;

    if (SLSaveFlagGet() == 0) {
        err = 0;
    } else {
        rc = FileCardMount(curSlotNo);
        if (rc < 0) {
            err = rc;
        } else {
            rc = HuCardOpen(curSlotNo, SLSaveFileName, &curFileInfo);
            if (rc == -4) {
                err = -4;
            } else if (rc == -2) {
                FileMessOut(7);
                err = CARD_RESULT_FATAL_ERROR;
            } else if (rc == CARD_RESULT_FATAL_ERROR) {
                FileMessOut(1);
                err = CARD_RESULT_FATAL_ERROR;
            } else if (rc == -3) {
                FileMessOut(0);
                err = -3;
            } else if (rc == -6) {
                rc = HuCardSectorSizeGet(curSlotNo);
                if (rc > 0 && rc != SAVE_SECTOR_SIZE) {
                    FileMessOut(8);
                    err = -2;
                } else {
                    UnMountCnt = 0;
                    FileMessOut(5);
                    err = -6;
                }
            } else {
                err = 0;
            }
        }
    }
    ret = err;
    if (ret >= 0) {
        buf = saveBuf[curSlotNo];
        if (SLSaveFlagGet() == 0) {
            status = 0;
        } else {
            SLSerialNoGet();
            rd = HuCardRead(&curFileInfo, buf, SAVE_BUF_SIZE, 0);
            if (rd == -3) {
                FileMessOut(0);
            } else if (rd < 0) {
                FileMessOut(1);
            }
            status = rd;
        }
        ret = status;
        if (SLSaveFlagGet() != 0) {
            s32 closeRet = HuCardClose(&curFileInfo);
        }
        if (ret >= 0) {
            p = (u16 *)&saveBuf[curSlotNo][SAVE_BOX_SIZE];
            sum = SLCheckSumGet(0, SAVE_BOX_SIZE);
            /* This checksum comparison computes a result that is neither stored nor tested. */
            *p == sum;
        }
    }
    return ret;
}

/* 0xCFF4 */
/* When saving is enabled, create a card save file, write its erase marker and supplied image,
 * and update status; otherwise return success without writing. */
s32 FileCardCopy(const char *fileName, s32 size, void *addr)
{
    s32 warnA;
    s32 warnB;
    s32 ret;
    void *buf;
    u32 byteNotUsed;
    u32 filesNotUsed;

    if (SLSaveFlagGet() == 0) {
        return 0;
    }
    SLCheckSumBoxAllSet();
    SLSaveBackup();
    ret = FileCardMount(curSlotNo);
    if (ret < 0) {
        return ret;
    }
    ret = HuCardSectorSizeGet(curSlotNo);
    /* This repeated sector-size check rejects negative results only; an unexpected
    * positive size passes here. */
    if (ret < 0 && ret != SAVE_SECTOR_SIZE) {
        FileMessOut(8);
        return CARD_RESULT_FATAL_ERROR;
    }
    /* Ignore the free-space query's status and decide from its byte and file counters. */
    ret = HuCardFreeSpaceGet(curSlotNo, &byteNotUsed, &filesNotUsed);
    if (filesNotUsed == 0 && size > byteNotUsed) {
        FileMessOut(4);
        return -9;
    }
    if (filesNotUsed == 0) {
        FileMessOut(2);
        return -9;
    }
    if (size > byteNotUsed) {
        FileMessOut(3);
        return -9;
    }
    warnA =
        FileCardMesOpen(FILE_SELECT_CARD_CREATE_FILE, curSlotNo + FILE_SELECT_CARD_SLOT_A, -1, 160);
    warnB = FileStatusMesOpen(FILE_SELECT_SAVING, curSlotNo + FILE_SELECT_CARD_SLOT_A, -1, 70);
    /* Suppress reset and disc error handling during creation and writing; afterward the
    * shared flag is cleared even if it was already set on entry. */
    HuSRDisableF = 1;
    ret = HuCardCreate(curSlotNo, fileName, size, &curFileInfo);
    if (ret < 0) {
        FileCardMesClose(warnA);
        FileCardMesClose(warnB);
        HuSRDisableF = 0;
    }
    if (ret == -3) {
        FileMessOut(0);
        return ret;
    }
    if (ret == -6) {
        FileMessOut(5);
        return ret;
    }
    if (ret < 0) {
        FileMessOut(1);
        return ret;
    }
    SLSerialNoGet();
    buf = HuMemDirectMalloc(0, size);
    /* The zero-length fill writes nothing; the card is first given an ERASE marker before the save
     * data. */
    memset(buf, size, 0);
    memcpy(buf, SLEraseStr, 6);
    ret = HuCardWriteIdle(&curFileInfo, buf, size, 0);
    if (ret == 0) {
        ret = HuCardWriteIdle(&curFileInfo, addr, size, 0);
    }
    HuMemDirectFree(buf);
    if (ret < 0) {
        FileCardMesClose(warnA);
        FileCardMesClose(warnB);
        HuSRDisableF = 0;
    }
    if (ret == -3) {
        FileMessOut(0);
        return ret;
    }
    if (ret == -6) {
        FileMessOut(5);
        return ret;
    }
    if (ret < 0) {
        FileMessOut(1);
        return ret;
    }
    ret = SLStatSet(0);
    HuSRDisableF = 0;
    FileCardMesClose(warnA);
    FileCardMesClose(warnB);
    if (ret < 0) {
        return ret;
    }
    return 0;
}

/* Called when creating the first card save file to write the new selected box and its empty
 * neighboring boxes. */
s32 FileClear(s16 sourceWindowId)
{
    s32 winId;
    s32 ret;

    if (sourceWindowId == -1) {
        winId = HuWinWarningCreate(HUWIN_POS_CENTER, 160.0f, 478, 94);
    } else {
        winId = sourceWindowId;
    }
    SLWinIdSet(winId);
    do {
        ret = FileCardCopy(SLSaveFileName, SAVE_BUF_SIZE, saveBuf[curSlotNo]);
        if (ret == 0) {
            ret = 0;
            break;
        }
        ret = FileCardErrorExec(ret, winId);
    } while (ret == -3);
    if (sourceWindowId == -1) {
        SLWinIdSet(-1);
        HuWinWarningKill(winId);
    }
    return ret;
}

/* 0xD448 */
/* When saving is enabled, write the supplied bytes to the open card file and update card
 * status; otherwise return success without writing. */
s32 FileCardWrite(s32 length, const void *addr)
{
    s32 winId;
    s32 ret;

    if (SLSaveFlagGet() == 0) {
        return 0;
    }
    if (lbl_1_data_568 != -1) {
        winId = (s16)FileStatusMesOpen(lbl_1_data_568, curSlotNo + FILE_SELECT_CARD_SLOT_A, -1, 70);
    }
    /* Suppress reset and disc error handling for the write; afterward the shared flag is
    * cleared even if it was already set on entry. */
    HuSRDisableF = 1;
    HuPrcSleep(60);
    SLSerialNoGet();
    ret = HuCardWriteIdle(&curFileInfo, addr, length, 0);
    if (lbl_1_data_568 != -1) {
        FileStatusMesClose(winId);
    }
    if (ret == 0) {
        ret = SLStatSet(0);
    }
    HuSRDisableF = 0;
    return ret;
}

/* FileSaveMesOpen calls this to checksum and back up the save image, write it when saving is
 * enabled, and classify card errors. Unlisted open errors, including busy, are treated as
 * success. A serial mismatch preserves the classified open result (zero or -4) after showing
 * its message; the final card-close result is ignored. */
s32 FileSave(void)
{
    s32 rv;
    s32 ret;
    s32 stat;
    s32 wstat;
    s32 wret;
    s32 winId1;
    s32 winId2;
    u8 *buf;
    s32 closeRet;

    SLCheckSumBoxAllSet();
    SLSaveBackup();
    if (SLSaveFlagGet() == 0) {
        stat = 0;
    } else {
        ret = FileCardMount(curSlotNo);
        if (ret < 0) {
            stat = ret;
        } else {
            ret = HuCardOpen(curSlotNo, SLSaveFileName, &curFileInfo);
            if (ret == -4) {
                stat = -4;
            } else if (ret == -2) {
                FileMessOut(7);
                stat = CARD_RESULT_FATAL_ERROR;
            } else if (ret == CARD_RESULT_FATAL_ERROR) {
                FileMessOut(1);
                stat = CARD_RESULT_FATAL_ERROR;
            } else if (ret == -3) {
                FileMessOut(0);
                stat = -3;
            } else if (ret == -6) {
                ret = HuCardSectorSizeGet(curSlotNo);
                if (ret > 0 && ret != SAVE_SECTOR_SIZE) {
                    FileMessOut(8);
                    stat = -2;
                } else {
                    UnMountCnt = 0;
                    FileMessOut(5);
                    stat = -6;
                }
            } else {
                stat = 0;
            }
        }
    }
    rv = stat;
    if (rv == -4) {
        if (SLSerialNoCheck() == 0) {
            FileMessOut(9);
        } else {
            rv = FileCardCopy(SLSaveFileName, SAVE_BUF_SIZE, saveBuf[curSlotNo]);
            if (rv >= 0) {
                SLSerialNoGet();
            }
        }
    } else if (rv >= 0) {
        if (SLSerialNoCheck() == 0) {
            FileMessOut(9);
        } else {
            winId1 = (s16) FileCardMesOpen(FILE_SELECT_CARD_WRITE,
                                           curSlotNo + FILE_SELECT_CARD_SLOT_A, -1, 70);
            buf = saveBuf[curSlotNo];
            if (SLSaveFlagGet() == 0) {
                wret = 0;
            } else {
                if (lbl_1_data_568 != FILECARD_MESSID_NONE) {
                    winId2 = (s16) FileStatusMesOpen(lbl_1_data_568,
                                                     curSlotNo + FILE_SELECT_CARD_SLOT_A, -1, 70);
                }
                /* Suppress reset and disc error handling for this write; afterward the shared flag
                 * is cleared even if it was already set on entry. */
                HuSRDisableF = 1;
                HuPrcSleep(60);
                SLSerialNoGet();
                wstat = HuCardWriteIdle(&curFileInfo, buf, SAVE_BUF_SIZE, 0);
                if (lbl_1_data_568 != FILECARD_MESSID_NONE) {
                    FileStatusMesClose(winId2);
                }
                if (wstat == 0) {
                    wstat = SLStatSet(0);
                }
                HuSRDisableF = 0;
                wret = wstat;
            }
            rv = wret;
            FileCardMesClose(winId1);
            if (rv == -3) {
                FileMessOut(0);
            } else if (rv == -2) {
                FileMessOut(7);
            } else if (rv == -6) {
                /* The sector-size result replaces the write error; an unexpected positive
                 * size is returned after the invalid-card message. */
                rv = HuCardSectorSizeGet(curSlotNo);
                if (rv > 0 && rv != SAVE_SECTOR_SIZE) {
                    FileMessOut(8);
                } else {
                    FileMessOut(5);
                    return -6;
                }
            } else if (rv < 0) {
                FileMessOut(1);
            }
        }
    }
    if (SLSaveFlagGet() != 0) {
        closeRet = HuCardClose(&curFileInfo);
    }
    return rv;
}

/* 0xD90C */
/* Save, copy and erase callers use this retry loop to show progress and resolve card errors.
* A zero save result becomes -5 if the subsequent serial check detects a different card. */
s32 FileSaveMesOpen(s16 sourceWindowId, s32 statusMessageId)
{
    s32 winId;
    s32 ret;

    if (sourceWindowId == -1) {
        winId = HuWinWarningCreate(HUWIN_POS_CENTER, 160.0f, 478, 94);
    } else {
        winId = sourceWindowId;
    }
    SLWinIdSet(winId);
    lbl_1_data_568 = statusMessageId;
    do {
        ret = FileSave();
        if (SLSerialNoCheck() != 0) {
            if (ret == 0) {
                ret = 0;
                break;
            }
        } else {
            if (ret == 0) {
                ret = -5;
                break;
            }
        }
        ret = FileCardErrorExec(ret, winId);
    } while (ret == -3);
    if (sourceWindowId == -1) {
        SLWinIdSet(-1);
        HuWinWarningKill(winId);
    }
    lbl_1_data_568 = -1;
    return ret;
}

/* 0xDA18 */
/* Saving callers use this to show a centered status message sized for its text and inserts. */
HUWINID FileStatusMesOpen(u32 mesId, u32 insMes0, u32 insMes1, s16 windowY)
{
    HuVec2f maxSize;
    HUWINID winId;

    if (insMes0 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSizeGet(insMes0, 0);
    }
    if (insMes1 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSizeGet(insMes1, 1);
    }
    HuWinMesMaxSizeGet(1, &maxSize, mesId);
    winId = HuWinWarningCreate(HUWIN_POS_CENTER, (f32)windowY, maxSize.x, maxSize.y);
    HuWinWarningOpen(winId);
    if (insMes0 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSet(winId, insMes0, 0);
    }
    if (insMes1 != FILECARD_MESSID_NONE) {
        HuWinInsertMesSet(winId, insMes1, 1);
    }
    HuWinMesSet(winId, mesId);
    HuWinMesWait(winId);
    return winId;
}

/* 0xDB5C */
/* Closes and destroys a status window returned by FileStatusMesOpen. */
void FileStatusMesClose(s16 winId)
{
    if (winId >= 0) {
        HuWinWarningClose(winId);
        HuWinWarningKill(winId);
    }
}

/* 0xDBA0 */
/* Card operations use this to show a message in the shared warning window or a temporary one. */
HUWINID FileCardMesOpen(u32 messNum, u32 insMesNum1, u32 insMesNum2, s16 posY)
{
    HUWINID winId;
    HuVec2f maxSize;

    if (SLWinId == -1) {
        HuWinInit(1);
    }
    if (insMesNum1 != -1) {
        HuWinInsertMesSizeGet(insMesNum1, 0);
    }
    if (insMesNum2 != -1) {
        HuWinInsertMesSizeGet(insMesNum2, 1);
    }
    HuWinMesMaxSizeGet(1, &maxSize, messNum);
    if (SLWinId == -1) {
        winId = HuWinWarningCreate(
            HUWIN_POS_CENTER, posY, maxSize.x, maxSize.y);
    } else {
        winId = SLWinId;
    }
    HuWinWarningOpen(winId);
    if (insMesNum1 != -1) {
        HuWinInsertMesSet(winId, insMesNum1, 0);
    }
    if (insMesNum2 != -1) {
        HuWinInsertMesSet(winId, insMesNum2, 1);
    }
    HuWinMesSet(winId, messNum);
    HuWinMesWait(winId);
    return winId;
}

/* 0xDD24 */
/* Closes a temporary card message window while preserving the shared save window. */
void FileCardMesClose(s16 winId)
{
    if (SLWinId != winId && winId >= 0) {
        HuWinWarningClose(winId);
        HuWinWarningKill(winId);
    }
}

/* 0xDD84 */
/* Card operations call this to show their error message; selected modes also ask a choice or wait
 * for A. */
s16 FileMessOut(s16 mode)
{
    s16 warnId;
    s16 choice;
    u32 mesId;
    u32 insertMesId;
    s32 flag;
    HUWIN *winP;
    f32 pos[2];

    choice = -1;
    insertMesId = 0;
    flag = 0;
    if (SLWinId == -1) {
        HuWinInit(1);
    }
    switch (mode) {
    case 0:
        mesId = FILE_SELECT_CARD_NOCARD;
        HuWinInsertMesSizeGet(curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
        insertMesId = curSlotNo + FILE_SELECT_CARD_SLOT_A;
        UnMountCnt = 0;
        break;
    case 1:
        mesId = FILE_SELECT_CARD_FATAL_ERROR;
        HuWinInsertMesSizeGet(curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
        insertMesId = curSlotNo + FILE_SELECT_CARD_SLOT_A;
        break;
    case 2:
        HuWinInsertMesSizeGet(curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
        insertMesId = curSlotNo + FILE_SELECT_CARD_SLOT_A;
        mesId = FILE_SELECT_CARD_NO_FILE;
        break;
    case 3:
        mesId = FILE_SELECT_CARD_INSUFFICIENT_SPACE;
        HuWinInsertMesSizeGet(curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
        insertMesId = curSlotNo + FILE_SELECT_CARD_SLOT_A;
        break;
    case 4:
        mesId = FILE_SELECT_CARD_FULL;
        HuWinInsertMesSizeGet(curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
        insertMesId = curSlotNo + FILE_SELECT_CARD_SLOT_A;
        break;
    case 5:
        HuWinInsertMesSizeGet(curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
        insertMesId = curSlotNo + FILE_SELECT_CARD_SLOT_A;
        mesId = FILE_SELECT_CARD_FORMAT_CHOICE;
        break;
    case 6:
        mesId = FILE_SELECT_CARD_FORMAT_ERROR;
        break;
    case 7:
        HuWinInsertMesSizeGet(curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
        insertMesId = curSlotNo + FILE_SELECT_CARD_SLOT_A;
        mesId = FILE_SELECT_CARD_WRONG_DEVICE;
        break;
    case 8:
        HuWinInsertMesSizeGet(curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
        insertMesId = curSlotNo + FILE_SELECT_CARD_SLOT_A;
        mesId = FILE_SELECT_CARD_INVALID;
        break;
    case 9:
        mesId = FILE_SELECT_CARD_SERIAL_INVALID;
        break;
    case 10:
        mesId = FILE_SELECT_NO_SAVE_CHOICE;
        flag = 1;
        break;
    case 11:
        HuWinInsertMesSizeGet(curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
        insertMesId = curSlotNo + FILE_SELECT_CARD_SLOT_A;
        mesId = FILE_SELECT_CARD_REINSERT;
        break;
    case 12:
        mesId = FILE_SELECT_CARD_REMOVED;
        break;
    }
    if (SLWinId == -1) {
        pos[0] = 478.0f;
        pos[1] = 94.0f;
        warnId = HuWinWarningCreate(HUWIN_POS_CENTER, 160.0f, pos[0], pos[1]);
    } else {
        warnId = SLWinId;
    }
    winP = &winData[warnId];
    winP->padMask = 1;
    if (insertMesId != 0) {
        HuWinInsertMesSet(warnId, insertMesId, 0);
    }
    HuWinAttrSet(warnId, HUWIN_ATTR_NOCANCEL);
    HuWinWarningOpen(warnId);
    HuWinMesSet(warnId, mesId);
    HuWinMesWait(warnId);
    if (flag != 0) {
        choice = HuWinChoiceGet(warnId, 1);
        /* Only mode 10 sets flag, so this mode-5 confirmation branch is never reached. */
        if (mode == 5 && choice == 0) {
            HuWinInsertMesSet(warnId, curSlotNo + FILE_SELECT_CARD_SLOT_A, 0);
            HuWinMesSet(warnId, FILE_SELECT_CARD_FORMAT);
            HuWinMesWait(warnId);
            choice = HuWinChoiceGet(warnId, 1);
        }
    }
    if (mode == FILE_MESS_REINSERT) {
    while ((HuPadBtnDown[0] & PAD_BUTTON_A) == 0) {
            HuPrcVSleep();
        }
        HuAudFXPlay(MSM_SE_CMN_02);
    }
    if (SLWinId == -1) {
        HuWinWarningClose(warnId);
        HuWinWarningKill(warnId);
    }
    return choice;
}

/* 0xE210 */
/* FileCardErrorExec calls this to display the retry, format, and cancel choices for an error. */
s32 FileCardChoice(s16 choiceFlags, s16 sourceWindowId)
{
    s16 choices[10];
    HuVec2f maxSize;
    s32 msgId;
    HUWINID winId;
    s16 choice;
    s16 v;
    HUWIN *w;

    v = choiceFlags & ~FILECARD_FLAG_CANCEL;
    if (v == 7) {
        msgId = FILE_SELECT_CARD_CHOICE_1;
        choices[0] = 1;
        choices[1] = 4;
        choices[2] = 2;
    } else if (v == 13) {
        msgId = FILE_SELECT_CARD_CHOICE_2;
        choices[0] = 1;
        choices[1] = 4;
        choices[2] = 8;
    } else if (v == 5) {
        msgId = FILE_SELECT_CARD_CHOICE_3;
        choices[0] = 1;
        choices[1] = 4;
    }
    HuWinMesMaxSizeGet(1, &maxSize, msgId);
    w = &winData[sourceWindowId];
    if ((f32)w->winH < maxSize.y) {
        HuWinWarningClose(sourceWindowId);
        winId = HuWinWarningCreate(HUWIN_POS_CENTER, 160.0f, (s16)maxSize.x, (s16)maxSize.y);
    } else {
        winId = sourceWindowId;
    }
    HuWinWarningOpen(winId);
    if ((choiceFlags & FILECARD_FLAG_CANCEL) == 0) {
        HuWinAttrSet(winId, HUWIN_ATTR_NOCANCEL);
    }
    HuWinMesSet(winId, msgId);
    HuWinMesWait(winId);
    choice = HuWinChoiceGet(winId, 0);
    if (sourceWindowId != winId) {
        HuWinWarningClose(winId);
        HuWinWarningKill(winId);
    }
    if (choice == -1) {
        return FILESEL_RESULT_CANCEL;
    }
    return choices[choice];
}
