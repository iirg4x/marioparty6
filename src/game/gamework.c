/* Stores shared save data, player totals, mode options, records, and unlock flags. */
#include "game/gamework.h"
#include "game/pad.h"
#include "game/flag.h"

#define GW_FLAG_WORD_BITS 32
#define GW_FLAG_BIT_MASK (GW_FLAG_WORD_BITS - 1)
#define GW_MG_UNLOCK_BIT(mgNo) (1 << (((mgNo) - 601) & GW_FLAG_BIT_MASK))
#define GW_MG_UNLOCK_ALL_BITS (~0U)

GW_DECA_SCORE lbl_802669DC[4];
/* Minigame bits recorded during the current single-player board session. */
u32 lbl_802669D0[3];
/* Default save-state template copied into the active common state at startup. */
GW_COMMON lbl_80266400;
/* Persistent options, records, board statistics, prizes, and bank progress. */
GW_COMMON lbl_80265E30;
/* Runtime board and minigame state, including the active mode options. */
GW_SYSTEM lbl_80265B70;
/* Current players' board totals and minigame results. */
GW_PLAYER lbl_80265750[4];
/* Character, controller, CPU difficulty, group, and control type for each player slot. */
GW_PLAYER_CONF lbl_80265728[4];

/* Prize bits earned during the current single-player board session. */
u32 lbl_802C0280[2];
/* Minigame wins in the current single-player session. */
static u16 SingleMgWinNum;
/* Minigame record improvements in the current single-player session. */
static u16 SingleMgRecordNum;
/* Play count for minigame-mode exit rewards; the add helper applies the 10000 cap after u16
 * arithmetic. */
static u16 MgPlayNum;
/* Board flow sets this to select daytime or nighttime minigame presentation. */
s16 lbl_802C0278;

s16 lbl_802BF788 = HUWIN_LANG_ENGLISH;
s16 lbl_802BF78A = -1;

/* Called once from main at startup to reset game state and choose initial human or CPU slots. */
void fn_8003C0E8(void)
{
    int slotNo;
    fn_8003C2DC();
    _InitFlag();
    memset(&lbl_80265728[0], 0, sizeof(lbl_80265728));
    memset(&lbl_80265750[0], 0, sizeof(lbl_80265750));
    memset(&lbl_80265B70, 0, sizeof(lbl_80265B70));
    for(slotNo=0; slotNo<4; slotNo++) {
        GW_PLAYER_CONF *playerConfig = &lbl_80265728[slotNo];
        playerConfig->charNo = slotNo;
        playerConfig->padNo = slotNo;
        playerConfig->comDif = 0;
        playerConfig->grpNo = slotNo;
        // A connected Game Boy Advance also makes this slot human-controlled.
        if(omPadErrChk(slotNo) == PAD_ERR_NONE || SIProbe(slotNo) == SI_GBA) {
            playerConfig->type = 0;
        } else {
            playerConfig->type = 1;
        }
    }
    GWLanguageSet(lbl_802BF788);
    GWVibrateSet(TRUE);
    GWMgInstDispSet(TRUE);
    GWMgComDispSet(TRUE);
    GWMessSpeedSet(1);
    GWSaveModeSet(0);
    GWPartySet(TRUE);
    for(slotNo=0; slotNo<4; slotNo++) {
        lbl_80265E30.singleMgWinNum[slotNo] = 0;
    }
    for(slotNo=0; slotNo<3; slotNo++) {
        lbl_80265E30.singleBoardPlayNum[slotNo] = 0;
        lbl_80265E30.singleBoardFlag[slotNo] = 0;
    }
}

/* Builds the default save state at startup, copies it to GwCommon, and resets runtime options. */
void fn_8003C2DC(void)
{
    GW_COMMON *commonP = &lbl_80266400;
    int entryNo;
    int boardNo;
    int charNo;
    memset(commonP, 0, sizeof(GW_COMMON));
    /* unk4 and unk5BC are never read by the game's code, so they keep their offset names. */
    commonP->magic[0] = 0;
    commonP->unk4 = 0;
    commonP->languageNo = lbl_802BF788;
    commonP->outputMode = 1; // Start with stereo output.
    commonP->vibrateF = TRUE;
    commonP->mic = 1; // Enable microphone use by default.
    commonP->time = 0;
    commonP->mgUnlock[0] = 0;
    commonP->mgUnlock[1] = 0;
    commonP->mgUnlock[2] = 0;
    commonP->mgUnlock[3] = 0;
    // Microphone minigames are available before other minigames are unlocked.
    commonP->mgUnlock[2] |= GW_MG_UNLOCK_BIT(665);
    commonP->mgUnlock[2] |= GW_MG_UNLOCK_BIT(666);
    commonP->mgUnlock[2] |= GW_MG_UNLOCK_BIT(667);
    commonP->mgUnlock[2] |= GW_MG_UNLOCK_BIT(669);
    commonP->mgUnlock[2] |= GW_MG_UNLOCK_BIT(670);
    commonP->record[0] = 60*60;
    commonP->record[1] = 1080;
    commonP->record[2] = 120*60;
    commonP->record[3] = 60*60;
    commonP->record[4] = 60*60;
    commonP->record[5] = 60*60;
    commonP->record[6] = 60*60;
    commonP->record[7] = 0;
    for(boardNo=0; boardNo<11; boardNo++) {
        for(charNo=0; charNo<14; charNo++) {
            commonP->charPlayNum[boardNo][charNo] = 0;
        }
        commonP->boardPlayNum[boardNo] = 0;
        for(entryNo=0; entryNo<14; entryNo++) {
            commonP->boardMaxStar[boardNo][entryNo] = 0;
            commonP->boardMaxCoin[boardNo][entryNo] = 0;
        }
    }
    commonP->saveEnableF = FALSE;
    commonP->map7Unlock = FALSE;
    commonP->veryHardUnlock = FALSE;
    commonP->m562VeryHardUnlock = FALSE;
    commonP->unkFlag4 = FALSE;
    commonP->viewOpening = FALSE;
    commonP->viewEnding = FALSE;
    commonP->storyMgInstDispF = commonP->partyMgInstDispF = TRUE;
    commonP->storyMgComDispF = commonP->partyMgComDispF = TRUE;
    commonP->storyMgPack = commonP->partyMgPack = 0;
    commonP->storyMessSpeed = commonP->partyMessSpeed = 1;
    commonP->storySaveMode = commonP->partySaveMode = 0;
    commonP->confTurnNum = 20;
    commonP->confBonusStar = TRUE;
    commonP->confTag = FALSE;
    commonP->confSingleDiff = 1;
    for(entryNo=0; entryNo<10; entryNo++) {
        commonP->decaScore[entryNo].charNo = -1;
    }
    // Initial Decathlon records are scores or frame counts, according to each event.
    commonP->decaMgRecord[0] = 10;
    commonP->decaMgRecord[1] = 100;
    commonP->decaMgRecord[2] = 60*60;
    commonP->decaMgRecord[3] = 1080;
    commonP->decaMgRecord[4] = 10;
    commonP->decaMgRecord[5] = 15;
    commonP->decaMgRecord[6] = 10;
    commonP->decaMgRecord[7] = 60*60;
    commonP->decaMgRecord[8] = 500;
    commonP->decaMgRecord[9] = 10;
    commonP->renshoMgRecordNum = 0;
    commonP->singlePrizeFlag[0] = commonP->singlePrizeFlag[1] = 0;
    commonP->bankStar = 0;
    commonP->bankStarAward = 0;
    commonP->bankFlag[0] = commonP->bankFlag[1] = 0;
    commonP->miracleBookFlag[0] = commonP->miracleBookFlag[1] = 0;
    commonP->mikeActRecord[0] = commonP->mikeActRecord[1] = commonP->mikeActRecord[2] = 300*60;
    commonP->unk5BC = 0;
    commonP->lastBoard = -1;
    memcpy(&lbl_80265E30, &lbl_80266400, sizeof(GW_COMMON));
    GWVibrateSet(TRUE);
    GWMgInstDispSet(TRUE);
    GWMgComDispSet(TRUE);
    GWMessSpeedSet(1);
    GWSaveModeSet(0);
    for(entryNo=0; entryNo<4; entryNo++) {
        lbl_80265E30.singleMgWinNum[entryNo] = 0;
    }
    for(entryNo=0; entryNo<3; entryNo++) {
        lbl_80265E30.singleBoardPlayNum[entryNo] = 0;
        lbl_80265E30.singleBoardFlag[entryNo] = 0;
    }
}

/* Board choice handlers call this for the CPU input delay in frames; an invalid delay also
 * resets the message-speed option to normal. */
int fn_8003C7C0(void)
{
    if(lbl_80265B70.comKeyDelay > 48) {
        GWMessSpeedSet(1);
    }
    return lbl_80265B70.comKeyDelay;
}

/* Minigame result code stores a new record here; practice results leave the saved record
 * unchanged. */
void fn_8003C80C(GW_RECORD_NO recordNo, u32 recordValue)
{
    if(!fn_8006A124(65551)) {
        lbl_80265E30.record[recordNo] = recordValue;
    }
}

/* Returns the saved record for minigame setup and record displays; units depend on the minigame. */
u32 fn_8003C860(GW_RECORD_NO recordNo)
{
    return lbl_80265E30.record[recordNo];
}

/* Copies the character color for display callers; characters beyond the eight listed entries
 * receive the zero-initialized color. */
void fn_8003C878(GW_CHARA_ID charNo, GXColor *colorOut)
{
    GXColor characterColors[14] = {
        { 227, 67, 67, 255 },
        { 68, 67, 227, 255 },
        { 241, 158, 220, 255 },
        { 67, 228, 68, 255 },
        { 138, 60, 180, 255 },
        { 146, 85, 55, 255 },
        { 227, 228, 68, 255 },
        { 40, 40, 40, 255 }
    };
    *colorOut = characterColors[charNo];
}

/* Sets a board play counter in saved statistics, limiting the supplied count to 99. */
void fn_8003CA78(GW_BOARD_NO boardNo, u8 playCount)
{
    if(playCount > 99) {
        playCount = 99;
    }
    lbl_80265E30.boardPlayNum[boardNo] = playCount;
}

/* Board result code adds to the saved play count; the sum narrows to u8 before the cap of 99. */
void fn_8003CAA4(GW_BOARD_NO boardNo, u8 playCount)
{
    // The parameter keeps the eight-bit sum before the displayed-count cap.
    playCount += lbl_80265E30.boardPlayNum[boardNo];
    if(playCount > 99) {
        playCount = 99;
    }
    lbl_80265E30.boardPlayNum[boardNo] = playCount;
}

/* Returns a board play counter for saved-statistics displays. */
u16 fn_8003CAE8(GW_BOARD_NO boardNo)
{
    return lbl_80265E30.boardPlayNum[boardNo];
}

/* Stores a character's board star record; the supplied value narrows to the u16 save field. */
void fn_8003CB00(GW_BOARD_NO boardNo, s32 starCount, u8 charNo)
{
    lbl_80265E30.boardMaxStar[boardNo][charNo] = starCount;
}

/* Returns a character's saved star record for the selected board. */
u16 fn_8003CB28(GW_BOARD_NO boardNo, u8 charNo)
{
    return lbl_80265E30.boardMaxStar[boardNo][charNo];
}

/* Stores a character's board coin record; the supplied value narrows to the u16 save field. */
void fn_8003CB4C(GW_BOARD_NO boardNo, s32 coinCount, u8 charNo)
{
    lbl_80265E30.boardMaxCoin[boardNo][charNo] = coinCount;
}

/* Returns a character's saved coin record for the selected board. */
u16 fn_8003CB74(GW_BOARD_NO boardNo, u8 charNo)
{
    return lbl_80265E30.boardMaxCoin[boardNo][charNo];
}

/* Board result code calls this for a human winner to increment the character's board counter,
 * capped at 99, and returns the updated count. */
int fn_8003CB98(GW_CHARA_ID charNo, GW_BOARD_NO boardNo)
{
    int playCount = lbl_80265E30.charPlayNum[boardNo][charNo]+1;
    if(playCount > 99) {
        playCount = 99;
    }
    lbl_80265E30.charPlayNum[boardNo][charNo] = playCount;
    return playCount;
}

/* Returns a character's saved board counter for statistics displays. */
int fn_8003CBFC(GW_CHARA_ID charNo, GW_BOARD_NO boardNo)
{
    return lbl_80265E30.charPlayNum[boardNo][charNo];
}

/* Sets a character's saved board counter directly, without applying a cap. */
void fn_8003CC1C(GW_CHARA_ID charNo, GW_BOARD_NO boardNo, int playCount)
{
    lbl_80265E30.charPlayNum[boardNo][charNo] = playCount;
}

/* Minigame lists and statistics displays query the saved unlock bit by minigame number. */
BOOL fn_8003CC40(int mgNo)
{
    int wordIndex;
    int bitIndex;
    // Convert the public minigame number to a bitset index.
    mgNo -= 601;
    wordIndex = mgNo >> 5;
    bitIndex = mgNo % 32;
    if(lbl_80265E30.mgUnlock[wordIndex] & (1 << bitIndex)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Sets the saved unlock bit when game flow unlocks a minigame; callers supply a valid number. */
void fn_8003CCAC(int mgNo)
{
    int wordIndex;
    int bitIndex;
    // Convert the public minigame number to a bitset index.
    mgNo -= 601;
    wordIndex = mgNo >> 5;
    bitIndex = mgNo % 32;
    lbl_80265E30.mgUnlock[wordIndex] |= (1 << bitIndex);
}

/* Custom-pack query entry point with no body; it does not supply a defined result. */
BOOL GWMgCustomGet(int mgNo)
{

}

/* Custom-pack entry point with no body; calling it does not change game state. */
void GWMgCustomSet(int mgNo)
{

}

/* Custom-pack entry point with no body; calling it does not change game state. */
void GWMgCustomReset(int mgNo)
{

}

/* Returns the current coin total for board and minigame callers. */
s16 fn_8003CD14(GW_PLAYER_ID playerNo)
{
    return lbl_80265750[playerNo].coin;
}

/* Board and minigame callers set coins here; practice mode skips the update, while normal
 * play clamps the total and raises the player's highest coin total when needed. */
void fn_8003CD2C(GW_PLAYER_ID playerNo, s16 coin)
{
    if(fn_8006A124(65551)) {
        return;
    }
    if(coin < 0) {
        coin = 0;
    }
    if(coin > 999) {
        coin = 999;
    }
    if(coin > lbl_80265750[playerNo].coinMax) {
        lbl_80265750[playerNo].coinMax = coin;
    }
    lbl_80265750[playerNo].coin = coin;
}

/* Board and minigame rewards adjust coins through the same practice check and limits as a set. */
void fn_8003CDDC(GW_PLAYER_ID playerNo, s16 coin)
{
    fn_8003CD2C(playerNo, coin+lbl_80265750[playerNo].coin);
}

/* Board callers set stars here, clamping the total and recording the player's highest total. */
void fn_8003CEAC(GW_PLAYER_ID playerNo, s16 star)
{
    if(star < 0) {
        star = 0;
    }
    if(star > 999) {
        star = 999;
    }
    if(star > lbl_80265750[playerNo].starMax) {
        lbl_80265750[playerNo].starMax = star;
    }
    lbl_80265750[playerNo].star = star;
}

/* Board rewards adjust stars through the same total and high-score limits as a set. */
void fn_8003CF18(GW_PLAYER_ID playerNo, s16 star)
{
    fn_8003CEAC(playerNo, star+lbl_80265750[playerNo].star);
}

/* Returns the current star total for board callers. */
s16 GWPlayerStarGet(GW_PLAYER_ID playerNo)
{
    return lbl_80265750[playerNo].star;
}

/* Single-player board events mark a prize earned in the current session; flags above the maximum
 * are ignored, and callers must pass a nonnegative flag. */
void fn_8003CFC4(GW_SINGLE_PRIZE_FLAG flag)
{
    if(flag > 63) {
        return;
    }
    lbl_802C0280[flag >> 5] |= 1 << (flag & GW_FLAG_BIT_MASK);
}

/* Single-player prize checks return zero or the selected session prize bit mask. */
BOOL fn_8003CFF4(GW_SINGLE_PRIZE_FLAG flag)
{
    if(flag > 63) {
        return FALSE;
    }
    return lbl_802C0280[flag >> 5] & (1 << (flag & GW_FLAG_BIT_MASK));
}

/* Single-player mode setup calls this to clear session prizes, minigame bits, wins, and records. */
void fn_8003D028(void)
{
    lbl_802C0280[0] = lbl_802C0280[1] = 0;
    lbl_802669D0[0] = lbl_802669D0[1] = lbl_802669D0[2] = 0;
    SingleMgWinNum = SingleMgRecordNum = 0;
}

/* Copies earned session prize bits into the saved prize set when single-player progress is kept. */
void fn_8003D070(void)
{
    lbl_80265E30.singlePrizeFlag[0] |= lbl_802C0280[0];
    lbl_80265E30.singlePrizeFlag[1] |= lbl_802C0280[1];
}

/* Prize displays query the saved prize set; the result is zero or the selected bit mask. */
BOOL fn_8003D0A8(GW_SINGLE_PRIZE_FLAG flag)
{
    if(flag > 63) {
        return FALSE;
    }
    return lbl_80265E30.singlePrizeFlag[flag >> 5] & (1 << (flag & GW_FLAG_BIT_MASK));
}

/* Single-player board flow records a minigame in the session bitset; returns its zero-based
 * index, including an out-of-range index that was not stored. */
int fn_8003D0E4(int mgNo)
{
    // Convert the public minigame number to a bitset index.
    mgNo -= 601;
    if(mgNo >= 96 || mgNo < 0) {
        return mgNo;
    }
    lbl_802669D0[mgNo >> 5] |= 1 << (mgNo & GW_FLAG_BIT_MASK);
    return mgNo;
}

/* Single-player list checks return zero or the recorded minigame bit mask for this session. */
BOOL fn_8003D12C(int mgNo)
{
    // Convert the public minigame number to a bitset index.
    mgNo -= 601;
    if(mgNo >= 96 || mgNo < 0) {
        return FALSE;
    }
    return lbl_802669D0[mgNo >> 5] & (1 << (mgNo & GW_FLAG_BIT_MASK));
}

/* Board results and minigame-mode exit rewards add stars to both the bank balance and award
 * counter, saturating each at GW_BANK_STAR_MAX. */
void fn_8003D174(u16 starCount)
{
    if(lbl_80265E30.bankStar+starCount > 9999) {
        lbl_80265E30.bankStar = 9999;
    } else {
        lbl_80265E30.bankStar += starCount;
    }
    if(lbl_80265E30.bankStarAward+starCount > 9999) {
        lbl_80265E30.bankStarAward = 9999;
    } else {
        lbl_80265E30.bankStarAward += starCount;
    }
}

/* Removes stars from the bank balance when requested, stopping at zero without changing awards. */
void fn_8003D200(u16 starCount)
{
    if(lbl_80265E30.bankStar < starCount) {
        lbl_80265E30.bankStar = 0;
    } else {
        lbl_80265E30.bankStar -= starCount;
    }
}

/* Returns the saved bank balance for bank menus and save-slot displays. */
u16 fn_8003D248(void)
{
    return lbl_80265E30.bankStar;
}

/* Returns the separate bank-star award counter when requested. */
u16 fn_8003D258(void)
{
    return lbl_80265E30.bankStarAward;
}

/* Clears the bank-star award counter without changing the bank balance. */
void fn_8003D268(void)
{
    lbl_80265E30.bankStarAward = 0;
}

/* Bank and mode menus record an unlocked option; flags above the maximum are ignored, and callers
 * must pass a nonnegative flag. */
void fn_8003D27C(GW_BANK_FLAG flag)
{
    if(flag > 63) {
        return;
    }
    lbl_80265E30.bankFlag[flag >> 5] |= 1 << (flag & GW_FLAG_BIT_MASK);
}

/* Clears a saved bank or mode option flag; flags above the maximum are ignored, and callers must
 * pass a nonnegative flag. */
void fn_8003D2B4(GW_BANK_FLAG flag)
{
    if(flag > 63) {
        return;
    }
    lbl_80265E30.bankFlag[flag >> 5] &= ~(1 << (flag & GW_FLAG_BIT_MASK));
}

/* Bank and mode menus query saved option flags; the result is zero or the selected bit mask. */
BOOL fn_8003D2F0(GW_BANK_FLAG flag)
{
    if(flag > 63) {
        return FALSE;
    }
    return lbl_80265E30.bankFlag[flag >> 5] & (1 << (flag & GW_FLAG_BIT_MASK));
}

/* Records a Miracle Book flag in saved state; flags above the maximum are ignored, and callers
 * must pass a nonnegative flag. */
void fn_8003D32C(GW_MIRACLE_BOOK_FLAG flag)
{
    if(flag > 63) {
        return;
    }
    lbl_80265E30.miracleBookFlag[flag >> 5] |= 1 << (flag & GW_FLAG_BIT_MASK);
}

/* Miracle Book display code queries saved flags; the result is zero or the selected bit mask. */
BOOL fn_8003D364(GW_MIRACLE_BOOK_FLAG flag)
{
    if(flag > 63) {
        return FALSE;
    }
    return lbl_80265E30.miracleBookFlag[flag >> 5] & (1 << (flag & GW_FLAG_BIT_MASK));
}

/* Minigame-mode entry resets its play counter here; the supplied count is stored without a cap. */
void fn_8003D3A0(u16 playCount)
{
    MgPlayNum = playCount;
}

/* Returns the minigame-mode play counter used to calculate exit rewards. */
u16 fn_8003D3A8(void)
{
    return MgPlayNum;
}

/* Instruction flow adjusts the minigame-mode play counter; negative underflow clamps to zero, then
 * the u16 result is capped at GW_MG_PLAY_NUM_MAX if it exceeds that limit. */
void fn_8003D3B0(s16 playCountDelta)
{
    if(playCountDelta < 0 && MgPlayNum < -playCountDelta) {
        MgPlayNum = 0;
        return;
    }
    MgPlayNum += playCountDelta;
    if(MgPlayNum > 10000) {
        MgPlayNum = 10000;
    }
}

/* Stores one microphone activity record when requested; record slots use activity-specific
 * units. */
void fn_8003D404(s16 recordNo, u32 recordValue)
{
    lbl_80265E30.mikeActRecord[recordNo] = recordValue;
}

/* Returns a microphone activity record for the record-result message. */
u32 fn_8003D420(s16 recordNo)
{
    return lbl_80265E30.mikeActRecord[recordNo];
}

/* Returns the current single-player session's minigame win count for prize checks. */
u16 fn_8003D43C(void)
{
    return SingleMgWinNum;
}

/* Single-player setup and win handling store the session minigame win count here. */
void fn_8003D444(u16 winCount)
{
    SingleMgWinNum = winCount;
}

/* Returns the current single-player session's new-record count for prize checks. */
u16 fn_8003D44C(void)
{
    return SingleMgRecordNum;
}

/* Single-player setup and record-prize handling store the session new-record count here. */
void fn_8003D454(u16 recordCount)
{
    SingleMgRecordNum = recordCount;
}

/* Debug save setup unlocks all minigame bits and the listed options, and sets bank stars to
 * 1000. */
void fn_8003D45C(void)
{
    s16 wordIndex;
    for(wordIndex=0; wordIndex<4; wordIndex++) {
        lbl_80265E30.mgUnlock[wordIndex] = GW_MG_UNLOCK_ALL_BITS;
    }
    lbl_80265E30.map7Unlock = TRUE;
    lbl_80265E30.veryHardUnlock = TRUE;
    lbl_80265E30.m562VeryHardUnlock = TRUE;
    lbl_80265E30.bankStar = 1000;
}
