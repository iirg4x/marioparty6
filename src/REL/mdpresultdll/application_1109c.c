#include <dolphin/mtx/GeoTypes.h>

#define MDRESULT_MODE_INDEX 3
#define MDRESULT_FREE_FOR_ALL_MODE 0
#define MDRESULT_PLAYER_COUNT 4
#define MDRESULT_TEAM_COUNT 2

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

s16 fn_1_1109C(s16 index, u8 *mask)
{
    s16 scores[4];
    s16 playerCount;
    s16 maxScore;
    s16 winnerCount;
    s16 i;

    maxScore = 0;
    winnerCount = 0;
    if (lbl_1_bss_1278[MDRESULT_MODE_INDEX] == MDRESULT_FREE_FOR_ALL_MODE) {
        playerCount = MDRESULT_PLAYER_COUNT;
    } else {
        playerCount = MDRESULT_TEAM_COUNT;
    }
    for (i = 0; i < playerCount; i++) {
        scores[i] = lbl_1_bss_10D4[i].values[index];
    }
    *mask = 0;
    i = 0;
    maxScore = 0;
    for (; i < playerCount; i++) {
        if (maxScore <= scores[i]) {
            maxScore = scores[i];
        }
    }
    for (i = 0; i < playerCount; i++) {
        if (maxScore == scores[i]) {
            winnerCount++;
            *mask |= 1 << i;
        }
    }
    if (maxScore == 0) {
        winnerCount = 0;
        *mask = 0;
    }
    return winnerCount;
}
