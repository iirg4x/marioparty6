#include <dolphin/mtx/GeoTypes.h>

#define FALSE 0
#define TRUE 1
#define MDRESULT_MODE_INDEX 3
#define MDRESULT_TEAM_MODE 1
#define MDRESULT_PLAYER_COUNT 4
#define MDRESULT_TEAM_COUNT 2
#define MDRESULT_WINNER_RANK 0

typedef struct MdResultScoreWork_s {
    s16 playerIndex;
    s16 teamIndex;
    s16 rank;
    s16 star;
    s16 coin;
    s16 values[16];
} MDRESULT_SCORE_WORK;

extern MDRESULT_SCORE_WORK lbl_1_bss_10D4[4];
extern s16 lbl_1_bss_1278[16];

s32 fn_1_12C80(u8 *mask)
{
    u8 value;
    s16 playerCount;
    s16 zeroCount;
    s16 i;

    value = 0;
    playerCount = MDRESULT_PLAYER_COUNT;
    zeroCount = 0;
    if (lbl_1_bss_1278[MDRESULT_MODE_INDEX] == MDRESULT_TEAM_MODE) {
        playerCount = MDRESULT_TEAM_COUNT;
    }
    for (i = 0; i < playerCount; i++) {
        if (lbl_1_bss_10D4[i].rank == MDRESULT_WINNER_RANK) {
            zeroCount++;
        }
    }
    if (zeroCount == 1) {
        return FALSE;
    }
    for (i = 0; i < playerCount; i++) {
        if (lbl_1_bss_10D4[i].rank == MDRESULT_WINNER_RANK) {
            value |= 1 << i;
        }
    }
    *mask = value;
    return TRUE;
}
