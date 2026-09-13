#include "REL/m621dll.h"
#include "game/gamework.h"

void fn_1_52D4(s32 playerNo, s16 coins)
{
    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
        GwPlayer[playerNo].mgCoinBonus = coins;
    }
}

void fn_1_5328(s32 playerNo, s32 score)
{
    GwPlayer[playerNo].mgScore = score;
}

s16 fn_1_5340(void)
{
    return GwMgNightF;
}

float fn_1_5350(float value)
{
    return fn_1_537C(value);
}

double fn_1_537C(double value)
{
    return __fabs(value);
}
