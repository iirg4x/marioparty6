/* Stores shared save data, player totals, mode options, records, and unlock flags. */
#include "game/gamework.h"
#include "game/pad.h"
#include "game/flag.h"

#define GW_FLAG_WORD_BITS 32
#define GW_FLAG_BIT_MASK (GW_FLAG_WORD_BITS - 1)
#define GW_MG_UNLOCK_BIT(mgNo) (1 << (((mgNo) - GW_MGNO_BASE) & GW_FLAG_BIT_MASK))
#define GW_MG_UNLOCK_ALL_BITS (~0U)

GW_DECA_SCORE GwMgDecaScore[GW_PLAYER_MAX];
/* Minigame bits recorded during the current single-player board session. */
u32 GwSingleMgFlag[3];
/* Default save-state template copied into the active common state at startup. */
GW_COMMON GwCommonOrig;
/* Persistent options, records, board statistics, prizes, and bank progress. */
GW_COMMON GwCommon;
/* Runtime board and minigame state, including the active mode options. */
GW_SYSTEM GwSystem;
/* Current players' board totals and minigame results. */
GW_PLAYER GwPlayer[GW_PLAYER_MAX];
/* Character, controller, CPU difficulty, group, and control type for each player slot. */
GW_PLAYER_CONF GwPlayerConf[GW_PLAYER_MAX];

/* Prize bits earned during the current single-player board session. */
u32 GwSinglePrizeFlag[2];
/* Minigame wins in the current single-player session. */
static u16 SingleMgWinNum;
/* Minigame record improvements in the current single-player session. */
static u16 SingleMgRecordNum;
/* Play count for minigame-mode exit rewards; the add helper applies the 10000 cap after u16
 * arithmetic. */
static u16 MgPlayNum;
/* Board flow sets this to select daytime or nighttime minigame presentation. */
s16 GwMgNightF;

s16 GwLanguage = HUWIN_LANG_ENGLISH;
s16 GwLanguageSave = HUWIN_LANG_NULL;

/* Called once from main at startup to reset game state and choose initial human or CPU slots. */
void GWInit(void)
{
    int slotNo;
    GWCommonInit();
    InitFlag();
    memset(&GwPlayerConf[0], 0, sizeof(GwPlayerConf));
    memset(&GwPlayer[0], 0, sizeof(GwPlayer));
    memset(&GwSystem, 0, sizeof(GwSystem));
    for(slotNo=0; slotNo<GW_PLAYER_MAX; slotNo++) {
        GW_PLAYER_CONF *playerConfig = &GwPlayerConf[slotNo];
        playerConfig->charNo = slotNo;
        playerConfig->padNo = slotNo;
        playerConfig->comDif = GW_PLAYER_COM_DIF_EASY;
        playerConfig->grpNo = slotNo;
        // A connected Game Boy Advance also makes this slot human-controlled.
        if(omPadErrChk(slotNo) == PAD_ERR_NONE || SIProbe(slotNo) == SI_GBA) {
            playerConfig->type = GW_PLAYER_TYPE_MAN;
        } else {
            playerConfig->type = GW_PLAYER_TYPE_COM;
        }
    }
    GWLanguageSet(GwLanguage);
    GWVibrateSet(TRUE);
    GWMgInstDispSet(TRUE);
    GWMgComDispSet(TRUE);
    GWMessSpeedSet(GW_MESS_SPEED_NORMAL);
    GWSaveModeSet(GW_SAVE_MODE_ALWAYS);
    GWPartySet(TRUE);
    for(slotNo=0; slotNo<GW_PLAYER_MAX; slotNo++) {
        GwCommon.singleMgWinNum[slotNo] = 0;
    }
    for(slotNo=0; slotNo<3; slotNo++) {
        GwCommon.singleBoardPlayNum[slotNo] = 0;
        GwCommon.singleBoardFlag[slotNo] = 0;
    }
}

/* Builds the default save state at startup, copies it to GwCommon, and resets runtime options. */
void GWCommonInit(void)
{
    GW_COMMON *commonP = &GwCommonOrig;
    int entryNo;
    int boardNo;
    int charNo;
    memset(commonP, 0, sizeof(GW_COMMON));
    /* unk4 and unk5BC are never read by the game's code, so they keep their offset names. */
    commonP->magic[0] = 0;
    commonP->unk4 = 0;
    commonP->languageNo = GwLanguage;
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
    commonP->record[GW_RECORD_M606] = 60*60;
    commonP->record[GW_RECORD_M608] = 1080;
    commonP->record[GW_RECORD_M618] = 120*60;
    commonP->record[GW_RECORD_M638] = 60*60;
    commonP->record[GW_RECORD_M648] = 60*60;
    commonP->record[GW_RECORD_M650] = 60*60;
    commonP->record[GW_RECORD_M652] = 60*60;
    commonP->record[GW_RECORD_M678] = 0;
    for(boardNo=0; boardNo<GW_BOARD_MAX; boardNo++) {
        for(charNo=0; charNo<GW_CHARA_MAX; charNo++) {
            commonP->charPlayNum[boardNo][charNo] = 0;
        }
        commonP->boardPlayNum[boardNo] = 0;
        for(entryNo=0; entryNo<GW_CHARA_MAX; entryNo++) {
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
    commonP->storyMgPack = commonP->partyMgPack = GW_MINIGAME_PACK_ALL;
    commonP->storyMessSpeed = commonP->partyMessSpeed = GW_MESS_SPEED_NORMAL;
    commonP->storySaveMode = commonP->partySaveMode = GW_SAVE_MODE_ALWAYS;
    commonP->confTurnNum = 20;
    commonP->confBonusStar = TRUE;
    commonP->confTag = FALSE;
    commonP->confSingleDiff = GW_PLAYER_COM_DIF_NORMAL;
    for(entryNo=0; entryNo<GW_DECA_SCORE_MAX; entryNo++) {
        commonP->decaScore[entryNo].charNo = GW_CHARA_NULL;
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
    commonP->lastBoard = GW_BOARD_NULL;
    memcpy(&GwCommon, &GwCommonOrig, sizeof(GW_COMMON));
    GWVibrateSet(TRUE);
    GWMgInstDispSet(TRUE);
    GWMgComDispSet(TRUE);
    GWMessSpeedSet(GW_MESS_SPEED_NORMAL);
    GWSaveModeSet(GW_SAVE_MODE_ALWAYS);
    for(entryNo=0; entryNo<GW_PLAYER_COM_DIF_MAX; entryNo++) {
        GwCommon.singleMgWinNum[entryNo] = 0;
    }
    for(entryNo=0; entryNo<3; entryNo++) {
        GwCommon.singleBoardPlayNum[entryNo] = 0;
        GwCommon.singleBoardFlag[entryNo] = 0;
    }
}

/* Board choice handlers call this for the CPU input delay in frames; an invalid delay also
 * resets the message-speed option to normal. */
int GWComKeyDelayGet(void)
{
    if(GwSystem.comKeyDelay > 48) {
        GWMessSpeedSet(GW_MESS_SPEED_NORMAL);
    }
    return GwSystem.comKeyDelay;
}

/* Minigame result code stores a new record here; practice results leave the saved record
 * unchanged. */
void GWRecordSet(GW_RECORD_NO recordNo, u32 recordValue)
{
    if(!CheckFlag(FLAG_MG_PRACTICE)) {
        GwCommon.record[recordNo] = recordValue;
    }
}

/* Returns the saved record for minigame setup and record displays; units depend on the minigame. */
u32 GWRecordGet(GW_RECORD_NO recordNo)
{
    return GwCommon.record[recordNo];
}

/* Copies the character color for display callers; characters beyond the eight listed entries
 * receive the zero-initialized color. */
void GWCharColorGet(GW_CHARA_ID charNo, GXColor *colorOut)
{
    GXColor characterColors[GW_CHARA_MAX] = {
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
void GWBoardPlayNumSet(GW_BOARD_NO boardNo, u8 playCount)
{
    if(playCount > 99) {
        playCount = 99;
    }
    GwCommon.boardPlayNum[boardNo] = playCount;
}

/* Board result code adds to the saved play count; the sum narrows to u8 before the cap of 99. */
void GWBoardPlayNumAdd(GW_BOARD_NO boardNo, u8 playCount)
{
    // The parameter keeps the eight-bit sum before the displayed-count cap.
    playCount += GwCommon.boardPlayNum[boardNo];
    if(playCount > 99) {
        playCount = 99;
    }
    GwCommon.boardPlayNum[boardNo] = playCount;
}

/* Returns a board play counter for saved-statistics displays. */
u16 GWBoardPlayNumGet(GW_BOARD_NO boardNo)
{
    return GwCommon.boardPlayNum[boardNo];
}

/* Stores a character's board star record; the supplied value narrows to the u16 save field. */
void GWBoardMaxStarSet(GW_BOARD_NO boardNo, s32 starCount, u8 charNo)
{
    GwCommon.boardMaxStar[boardNo][charNo] = starCount;
}

/* Returns a character's saved star record for the selected board. */
u16 GWBoardMaxStarGet(GW_BOARD_NO boardNo, u8 charNo)
{
    return GwCommon.boardMaxStar[boardNo][charNo];
}

/* Stores a character's board coin record; the supplied value narrows to the u16 save field. */
void GWBoardMaxCoinSet(GW_BOARD_NO boardNo, s32 coinCount, u8 charNo)
{
    GwCommon.boardMaxCoin[boardNo][charNo] = coinCount;
}

/* Returns a character's saved coin record for the selected board. */
u16 GWBoardMaxCoinGet(GW_BOARD_NO boardNo, u8 charNo)
{
    return GwCommon.boardMaxCoin[boardNo][charNo];
}

/* Board result code calls this for a human winner to increment the character's board counter,
 * capped at 99, and returns the updated count. */
int GWCharPlayNumInc(GW_CHARA_ID charNo, GW_BOARD_NO boardNo)
{
    int playCount = GwCommon.charPlayNum[boardNo][charNo]+1;
    if(playCount > 99) {
        playCount = 99;
    }
    GwCommon.charPlayNum[boardNo][charNo] = playCount;
    return playCount;
}

/* Returns a character's saved board counter for statistics displays. */
int GWCharPlayNumGet(GW_CHARA_ID charNo, GW_BOARD_NO boardNo)
{
    return GwCommon.charPlayNum[boardNo][charNo];
}

/* Sets a character's saved board counter directly, without applying a cap. */
void GWCharPlayNumSet(GW_CHARA_ID charNo, GW_BOARD_NO boardNo, int playCount)
{
    GwCommon.charPlayNum[boardNo][charNo] = playCount;
}

/* Minigame lists and statistics displays query the saved unlock bit by minigame number. */
BOOL GWMgUnlockGet(int mgNo)
{
    int wordIndex;
    int bitIndex;
    // Convert the public minigame number to a bitset index.
    mgNo -= GW_MGNO_BASE;
    wordIndex = mgNo >> 5;
    bitIndex = mgNo % 32;
    if(GwCommon.mgUnlock[wordIndex] & (1 << bitIndex)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Sets the saved unlock bit when game flow unlocks a minigame; callers supply a valid number. */
void GWMgUnlockSet(int mgNo)
{
    int wordIndex;
    int bitIndex;
    // Convert the public minigame number to a bitset index.
    mgNo -= GW_MGNO_BASE;
    wordIndex = mgNo >> 5;
    bitIndex = mgNo % 32;
    GwCommon.mgUnlock[wordIndex] |= (1 << bitIndex);
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
s16 GWPlayerCoinGet(GW_PLAYER_ID playerNo)
{
    return GwPlayer[playerNo].coin;
}

/* Board and minigame callers set coins here; practice mode skips the update, while normal
 * play clamps the total and raises the player's highest coin total when needed. */
void GWPlayerCoinSet(GW_PLAYER_ID playerNo, s16 coin)
{
    if(_CheckFlag(FLAG_MG_PRACTICE)) {
        return;
    }
    if(coin < 0) {
        coin = 0;
    }
    if(coin > GW_PLAYER_COIN_MAX) {
        coin = GW_PLAYER_COIN_MAX;
    }
    if(coin > GwPlayer[playerNo].coinMax) {
        GwPlayer[playerNo].coinMax = coin;
    }
    GwPlayer[playerNo].coin = coin;
}

/* Board and minigame rewards adjust coins through the same practice check and limits as a set. */
void GWPlayerCoinAdd(GW_PLAYER_ID playerNo, s16 coin)
{
    GWPlayerCoinSet(playerNo, coin+GwPlayer[playerNo].coin);
}

/* Board callers set stars here, clamping the total and recording the player's highest total. */
void GWPlayerStarSet(GW_PLAYER_ID playerNo, s16 star)
{
    if(star < 0) {
        star = 0;
    }
    if(star > GW_PLAYER_STAR_MAX) {
        star = GW_PLAYER_STAR_MAX;
    }
    if(star > GwPlayer[playerNo].starMax) {
        GwPlayer[playerNo].starMax = star;
    }
    GwPlayer[playerNo].star = star;
}

/* Board rewards adjust stars through the same total and high-score limits as a set. */
void GWPlayerStarAdd(GW_PLAYER_ID playerNo, s16 star)
{
    GWPlayerStarSet(playerNo, star+GwPlayer[playerNo].star);
}

/* Returns the current star total for board callers. */
s16 GWPlayerStarGet(GW_PLAYER_ID playerNo)
{
    return GwPlayer[playerNo].star;
}

/* Single-player board events mark a prize earned in the current session; flags above the maximum
 * are ignored, and callers must pass a nonnegative flag. */
void GWSinglePrizeFlagSet(GW_SINGLE_PRIZE_FLAG flag)
{
    if(flag > GW_SINGLE_PRIZE_FLAG_MAX) {
        return;
    }
    GwSinglePrizeFlag[flag >> 5] |= 1 << (flag & GW_FLAG_BIT_MASK);
}

/* Single-player prize checks return zero or the selected session prize bit mask. */
BOOL GWSinglePrizeFlagGet(GW_SINGLE_PRIZE_FLAG flag)
{
    if(flag > GW_SINGLE_PRIZE_FLAG_MAX) {
        return FALSE;
    }
    return GwSinglePrizeFlag[flag >> 5] & (1 << (flag & GW_FLAG_BIT_MASK));
}

/* Single-player mode setup calls this to clear session prizes, minigame bits, wins, and records. */
void GWSingleDataInit(void)
{
    GwSinglePrizeFlag[0] = GwSinglePrizeFlag[1] = 0;
    GwSingleMgFlag[0] = GwSingleMgFlag[1] = GwSingleMgFlag[2] = 0;
    SingleMgWinNum = SingleMgRecordNum = 0;
}

/* Copies earned session prize bits into the saved prize set when single-player progress is kept. */
void GWSinglePrizeSaveFlagSet(void)
{
    GwCommon.singlePrizeFlag[0] |= GwSinglePrizeFlag[0];
    GwCommon.singlePrizeFlag[1] |= GwSinglePrizeFlag[1];
}

/* Prize displays query the saved prize set; the result is zero or the selected bit mask. */
BOOL GWSinglePrizeSaveFlagGet(GW_SINGLE_PRIZE_FLAG flag)
{
    if(flag > GW_SINGLE_PRIZE_FLAG_MAX) {
        return FALSE;
    }
    return GwCommon.singlePrizeFlag[flag >> 5] & (1 << (flag & GW_FLAG_BIT_MASK));
}

/* Single-player board flow records a minigame in the session bitset; returns its zero-based
 * index, including an out-of-range index that was not stored. */
int GWSingleMgFlagSet(int mgNo)
{
    // Convert the public minigame number to a bitset index.
    mgNo -= GW_MGNO_BASE;
    if(mgNo >= 96 || mgNo < 0) {
        return mgNo;
    }
    GwSingleMgFlag[mgNo >> 5] |= 1 << (mgNo & GW_FLAG_BIT_MASK);
    return mgNo;
}

/* Single-player list checks return zero or the recorded minigame bit mask for this session. */
BOOL GWSingleMgFlagGet(int mgNo)
{
    // Convert the public minigame number to a bitset index.
    mgNo -= GW_MGNO_BASE;
    if(mgNo >= 96 || mgNo < 0) {
        return FALSE;
    }
    return GwSingleMgFlag[mgNo >> 5] & (1 << (mgNo & GW_FLAG_BIT_MASK));
}

/* Board results and minigame-mode exit rewards add stars to both the bank balance and award
 * counter, saturating each at GW_BANK_STAR_MAX. */
void GWBankStarAdd(u16 starCount)
{
    if(GwCommon.bankStar+starCount > GW_BANK_STAR_MAX) {
        GwCommon.bankStar = GW_BANK_STAR_MAX;
    } else {
        GwCommon.bankStar += starCount;
    }
    if(GwCommon.bankStarAward+starCount > GW_BANK_STAR_MAX) {
        GwCommon.bankStarAward = GW_BANK_STAR_MAX;
    } else {
        GwCommon.bankStarAward += starCount;
    }
}

/* Removes stars from the bank balance when requested, stopping at zero without changing awards. */
void GWBankStarSub(u16 starCount)
{
    if(GwCommon.bankStar < starCount) {
        GwCommon.bankStar = 0;
    } else {
        GwCommon.bankStar -= starCount;
    }
}

/* Returns the saved bank balance for bank menus and save-slot displays. */
u16 GWBankStarGet(void)
{
    return GwCommon.bankStar;
}

/* Returns the separate bank-star award counter when requested. */
u16 GWBankStarAwardGet(void)
{
    return GwCommon.bankStarAward;
}

/* Clears the bank-star award counter without changing the bank balance. */
void GWBankStarAwardReset(void)
{
    GwCommon.bankStarAward = 0;
}

/* Bank and mode menus record an unlocked option; flags above the maximum are ignored, and callers
 * must pass a nonnegative flag. */
void GWBankFlagSet(GW_BANK_FLAG flag)
{
    if(flag > GW_BANK_FLAG_MAX) {
        return;
    }
    GwCommon.bankFlag[flag >> 5] |= 1 << (flag & GW_FLAG_BIT_MASK);
}

/* Clears a saved bank or mode option flag; flags above the maximum are ignored, and callers must
 * pass a nonnegative flag. */
void GWBankFlagReset(GW_BANK_FLAG flag)
{
    if(flag > GW_BANK_FLAG_MAX) {
        return;
    }
    GwCommon.bankFlag[flag >> 5] &= ~(1 << (flag & GW_FLAG_BIT_MASK));
}

/* Bank and mode menus query saved option flags; the result is zero or the selected bit mask. */
BOOL GWBankFlagGet(GW_BANK_FLAG flag)
{
    if(flag > GW_BANK_FLAG_MAX) {
        return FALSE;
    }
    return GwCommon.bankFlag[flag >> 5] & (1 << (flag & GW_FLAG_BIT_MASK));
}

/* Records a Miracle Book flag in saved state; flags above the maximum are ignored, and callers
 * must pass a nonnegative flag. */
void GWMiracleBookFlagSet(GW_MIRACLE_BOOK_FLAG flag)
{
    if(flag > GW_MIRACLE_BOOK_FLAG_MAX) {
        return;
    }
    GwCommon.miracleBookFlag[flag >> 5] |= 1 << (flag & GW_FLAG_BIT_MASK);
}

/* Miracle Book display code queries saved flags; the result is zero or the selected bit mask. */
BOOL GWMiracleBookFlagGet(GW_MIRACLE_BOOK_FLAG flag)
{
    if(flag > GW_MIRACLE_BOOK_FLAG_MAX) {
        return FALSE;
    }
    return GwCommon.miracleBookFlag[flag >> 5] & (1 << (flag & GW_FLAG_BIT_MASK));
}

/* Minigame-mode entry resets its play counter here; the supplied count is stored without a cap. */
void GWMgPlayNumSet(u16 playCount)
{
    MgPlayNum = playCount;
}

/* Returns the minigame-mode play counter used to calculate exit rewards. */
u16 GWMgPlayNumGet(void)
{
    return MgPlayNum;
}

/* Instruction flow adjusts the minigame-mode play counter; negative underflow clamps to zero, then
 * the u16 result is capped at GW_MG_PLAY_NUM_MAX if it exceeds that limit. */
void GWMgPlayNumAdd(s16 playCountDelta)
{
    if(playCountDelta < 0 && MgPlayNum < -playCountDelta) {
        MgPlayNum = 0;
        return;
    }
    MgPlayNum += playCountDelta;
    if(MgPlayNum > GW_MG_PLAY_NUM_MAX) {
        MgPlayNum = GW_MG_PLAY_NUM_MAX;
    }
}

/* Stores one microphone activity record when requested; record slots use activity-specific
 * units. */
void GWMikeActRecordSet(s16 recordNo, u32 recordValue)
{
    GwCommon.mikeActRecord[recordNo] = recordValue;
}

/* Returns a microphone activity record for the record-result message. */
u32 GWMikeActRecordGet(s16 recordNo)
{
    return GwCommon.mikeActRecord[recordNo];
}

/* Returns the current single-player session's minigame win count for prize checks. */
u16 GWSingleMgWinNumGet(void)
{
    return SingleMgWinNum;
}

/* Single-player setup and win handling store the session minigame win count here. */
void GWSingleMgWinNumSet(u16 winCount)
{
    SingleMgWinNum = winCount;
}

/* Returns the current single-player session's new-record count for prize checks. */
u16 GWSingleMgRecordNumGet(void)
{
    return SingleMgRecordNum;
}

/* Single-player setup and record-prize handling store the session new-record count here. */
void GWSingleMgRecordNumSet(u16 recordCount)
{
    SingleMgRecordNum = recordCount;
}

/* Debug save setup unlocks all minigame bits and the listed options, and sets bank stars to
 * 1000. */
void GWSaveDebugSet(void)
{
    s16 wordIndex;
    for(wordIndex=0; wordIndex<4; wordIndex++) {
        GwCommon.mgUnlock[wordIndex] = GW_MG_UNLOCK_ALL_BITS;
    }
    GwCommon.map7Unlock = TRUE;
    GwCommon.veryHardUnlock = TRUE;
    GwCommon.m562VeryHardUnlock = TRUE;
    GwCommon.bankStar = 1000;
}
