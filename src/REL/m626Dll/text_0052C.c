/* Resolves round winners, awards the round bonus, and tracks player sides. */
#include "dolphin/types.h"
#include "game/flag.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "game/charman.h"

s32 fn_1_824(s16 mode, s16 frameNo);
s32 fn_1_8B90(s32 player);
void fn_1_8D40(void);
void fn_1_44D4(void);
void fn_1_E2F0(void);

extern s16 lbl_1_bss_0[4];
extern s16 lbl_1_bss_8[4];
extern s16 lbl_1_bss_10[4];
extern s16 lbl_1_bss_18[4];
extern s32 lbl_1_bss_24[3];

typedef struct M626RoundCounts_s {
    s32 side[2];
} M626RoundCounts;

static const M626RoundCounts lbl_1_rodata_28 = {{0, 0}};

/* Sets the winning players at round start and advances after the result delay. */
void fn_1_52C(s16 mode, s16 frameNo)
{
    M626RoundCounts totals;
    s32 index;

    if (fn_1_824(mode, frameNo) == 0) {
        if (frameNo == 0) {
            u16 winner;
            totals = lbl_1_rodata_28;
            for (index = 0; index < 4; index++) {
                if (fn_1_8B90(index) != 0) {
                    totals.side[lbl_1_bss_8[index]]++;
                }
            }
            for (index = 0; index < 4; index++) {
                lbl_1_bss_10[index] = -1;
                if (totals.side[lbl_1_bss_8[index]] != 0 &&
                    totals.side[lbl_1_bss_8[index] ^ 1] == 0) {
                    lbl_1_bss_10[index] = lbl_1_bss_0[index];
                }
            }
            winner = MgSeqWinnerSet(lbl_1_bss_10[0], lbl_1_bss_10[1],
                                    lbl_1_bss_10[2], lbl_1_bss_10[3]);
        }
        if (lbl_1_bss_24[0] == 0 && frameNo >= 120) {
            (void)MgSeqModeNext();
            return;
        }
    }
}

/* Awards ten bonus coins to each winner on the result callback's first frame. */
void fn_1_6EC(s16 mode, s16 frameNo)
{
    s16 newBonus;
    s16 oldBonus;
    s32 playerIndex;

    if (fn_1_824(mode, frameNo) == 0 && frameNo == 0) {
        for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
            if (lbl_1_bss_10[playerIndex] >= 0) {
                oldBonus = GwPlayer[playerIndex].mgCoinBonus;
                newBonus = oldBonus + 10;
                if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                    GwPlayer[playerIndex].mgCoinBonus = newBonus;
                }
            }
        }
    }
}

/* Leaves the result callback idle while another sequence phase is active. */
void fn_1_7C4(short mode, short state)
{
    if (fn_1_824(mode, state) != 0) {
        return;
    }
}

/* Clears the character and stage effects when the sequence leaves the result phase. */
void fn_1_7FC(s16 mode, s16 frameNo)
{
    fn_1_8D40();
    fn_1_44D4();
    fn_1_E2F0();
}

s32 fn_1_824(s16 mode, s16 frameNo)
{
    return 0;
}

void fn_1_82C(void)
{
}

/* Builds a Snow Brawl data number and selects its night variant when present. */
s32 fn_1_830(s32 fileIndex)
{
    if (fileIndex >= 8) {
        s16 nightMode = GwMgNightF;
        if (nightMode == 1) {
            fileIndex += 16;
        }
    }
    return fileIndex + DATA_m626;
}

void fn_1_86C(void)
{
    MgSeqTimerValueGet();
}

/* Returns the winning player ID stored for one player slot. */
s16 fn_1_88C(s32 index)
{
    return lbl_1_bss_10[index];
}

/* Resolves a side ID to the live winner state for this round. */
u32 fn_1_8A4(u32 sideId)
{
    s32 index;
    s32 result;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_10[index] >= 0) {
            break;
        }
    }
    if (index >= 4) {
        return 0;
    }
    for (index = 0; index < 4; index++) {
        if (sideId == (u32)lbl_1_bss_8[index]) {
            if (lbl_1_bss_10[index] >= 0) {
                result = 1;
            } else {
                result = -1;
            }
            return result;
        }
    }
    return sideId;
}

/* Stores the side and result player ID assigned to one of four slots. */
void fn_1_960(s32 playerIndex, s32 sideId, s32 winnerId)
{
    playerIndex &= 3;
    lbl_1_bss_8[playerIndex] = (s16)sideId;
    lbl_1_bss_0[playerIndex] = (s16)winnerId;
}

/* Marks a player slot as present for the current round. */
void fn_1_998(unsigned int player)
{
    player &= 3;
    lbl_1_bss_18[player] = 1;
}

/* Reports whether all four player slots have been initialized. */
int fn_1_9B8(void)
{
    s32 player;
    for (player = 0; player < 4; player++) {
        if (lbl_1_bss_18[player] == 0) {
            return 0;
        }
    }
    return 1;
}
