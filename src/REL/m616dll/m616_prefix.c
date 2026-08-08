#include "dolphin.h"

#include "game/audio.h"
#include "game/gamemes.h"
#include "game/mg/seqman.h"

s32 fn_1_A0(s32 streamNo, s32 bgmId)
{
    s32 result = streamNo;

    if (result == -1) {
        if (GameMesFXPlayCheck(MgSeqGameMesIdGet()) != 0) {
            result = HuAudBGMPlay((s16)bgmId);
        }
    }
    return result;
}

void fn_1_104(s32 streamNo)
{
    if (streamNo != -1) {
        HuAudSStreamFadeOut(streamNo, 100);
    }
}

void fn_1_140(void)
{
    MgSeqModeNext();
}
