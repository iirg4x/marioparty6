#include "dolphin.h"

#include "game/hu3d.h"
#include "game/object.h"

typedef struct M616Work {
    OMOBJMAN *objectManager;
    s16 sequenceState;
    s16 unk_06;
    s32 sequenceFrame;
    HU3D_MODELID activeCameraModel;
} M616Work;

extern M616Work lbl_1_bss_10;

BOOL fn_1_20D8(void)
{
    return Hu3DMotionEndCheck(lbl_1_bss_10.activeCameraModel);
}
