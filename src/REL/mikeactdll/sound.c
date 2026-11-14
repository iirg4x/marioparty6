/* Sound playback gates and selection callbacks for the microphone activity. */
#include "REL/mikeactdll/sound.h"
#include "REL/mikeactdll/sequence_context.h"
#include "math.h"

#include "game/audio.h"

#include "include/game/audio.h"

#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "game/process.h"

#include "game/saveload.h"

extern s16 lbl_1_bss_58;

extern s16 lbl_1_bss_E;

extern s16 lbl_1_bss_C;

void fn_1_2FB4(MCResponse_s *context)
{
    s16 mode;
    s16 modeForDispatch;

    mode = lbl_1_bss_58;
    modeForDispatch = mode;
    if (modeForDispatch == 3) {
        lbl_1_bss_E = *context->result;
    } else if (modeForDispatch == 4) {
        lbl_1_bss_C = *context->result;
    }
}

struct MikeActPlayerRecord_s;

s32 fn_1_3024(Point3d *position)
{
    Point3d projectedPosition;
    s32 result;

    Hu3D3Dto2D(position, 1, &projectedPosition);
    result = (s32)(32.0f +
        64.0f * (projectedPosition.x / 576.0f));
    return result;
}

extern u16 lbl_1_bss_6A;

s32 fn_1_30A0(void *soundWork, s32 soundId)
{
    if (lbl_1_bss_6A >= 4U) {
        return -1;
    }
    return HuAudFXPlay(soundId);
}

s32 fn_1_30E4(s32 soundId)
{
    if (lbl_1_bss_6A >= 4U) {
        return -1;
    }
    return HuAudFXPlay(soundId);
}

void fn_1_3128(s32 soundId)
{
    HuAudFXStop(soundId);
}

void fn_1_3150(s16 streamId) {
    HuAudSStreamChanPlay(streamId, 0);
}

void fn_1_317C(s16 streamId) {
    HuAudSStreamParamSet(0, 0x5A, 500);
    HuAudSStreamPauseFadeOut(0, 1, 500);
    HuAudSStreamChanPlay(streamId, 2);
}
