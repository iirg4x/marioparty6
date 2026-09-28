#define _MATH_H
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "string.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#include "game/printfunc.h"
#include "game/saveload.h"
#include "game/window.h"

/* External views of existing retail owners, with target-confirmed f32 width. */
extern s16 lbl_1_bss_0;
extern char lbl_1_data_84E[];
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_13C;
extern const f32 lbl_1_rodata_140;
extern const f32 lbl_1_rodata_168;
extern const f32 lbl_1_rodata_178;

void fn_1_5BF4(OMOBJ *object)
{
    omSetStatBit(object, 256U);
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(9830447, 268435456, HEAP_MODEL));
    object->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(9830448, 268435456, HEAP_MODEL));
    object->mtnId[0] = Hu3DJointMotion(object->mdlId[0], HuDataSelHeapReadNum(9830449, 268435456, HEAP_MODEL));
    object->mtnId[1] = Hu3DJointMotion(object->mdlId[0], HuDataSelHeapReadNum(9830450, 268435456, HEAP_MODEL));
    object->mtnId[2] = Hu3DJointMotion(object->mdlId[0], HuDataSelHeapReadNum(9830451, 268435456, HEAP_MODEL));
    object->mtnId[3] = Hu3DJointMotion(object->mdlId[0], HuDataSelHeapReadNum(9830452, 268435456, HEAP_MODEL));
    object->mtnId[4] = Hu3DJointMotion(object->mdlId[0], HuDataSelHeapReadNum(9830453, 268435456, HEAP_MODEL));
    Hu3DModelHookSet(object->mdlId[0], lbl_1_data_84E, object->mdlId[1]);
    if (lbl_1_bss_0 == 0) {
        Hu3DModelPosSet(object->mdlId[0], lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_74);
        Hu3DModelRotSet(object->mdlId[0], lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_74);
    } else {
        Hu3DModelPosSet(object->mdlId[0], lbl_1_rodata_13C, lbl_1_rodata_74, lbl_1_rodata_140);
        Hu3DModelRotSet(object->mdlId[0], lbl_1_rodata_74, lbl_1_rodata_168, lbl_1_rodata_74);
    }
    Hu3DModelScaleSet(object->mdlId[0], lbl_1_rodata_178, lbl_1_rodata_178, lbl_1_rodata_178);
    Hu3DModelLayerSet(object->mdlId[0], 1);
    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0], lbl_1_rodata_74, lbl_1_rodata_74, 1073741825U);
    Hu3DModelShadowSet(object->mdlId[0]);
    object->objFunc = NULL;
}
