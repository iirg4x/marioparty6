#include "REL/m621dll.h"
#include "game/data.h"
#include "game/memory.h"
#include "datadir_enum.h"
#include "string.h"

struct M621Effect_s {
    s16 active;
    HU3D_MODELID model;
};

void fn_1_5168(OMOBJ *obj);

void fn_1_5054(OMOBJ *obj)
{
    s32 i;
    M621Effect *work = obj->data;

    for (i = 0; i < 4; i++) {
        lbl_1_bss_2C[i] = Hu3DModelCreateData(lbl_1_data_9C[i][0]);
        lbl_1_bss_24[i] = Hu3DJointMotionData(lbl_1_bss_2C[i], lbl_1_data_9C[i][1]);
        Hu3DModelDispOff(lbl_1_bss_2C[i]);
    }
    memset(work, 0, 21 * sizeof(M621Effect));
    fn_1_140(obj, fn_1_5168);
}

void fn_1_5168(OMOBJ *obj)
{
    M621Effect *effect;
    s32 i;
    M621Effect *work = obj->data;

    for (i = 0; i < 21; i++) {
        effect = &work[i];
        if (effect->active == 1 && Hu3DMotionEndCheck(effect->model)) {
            Hu3DModelKill(effect->model);
            effect->active = 0;
        }
    }
}

void fn_1_51F4(s16 effectNo, HuVecF *pos)
{
    M621Effect *effect;
    s32 i;

    for (i = 0; i < 21; i++) {
        effect = &lbl_1_bss_20[i];
        if (effect->active == 0) {
            effect->model = Hu3DModelLink(lbl_1_bss_2C[effectNo]);
            Hu3DMotionSet(effect->model, lbl_1_bss_24[effectNo]);
            Hu3DModelDispOn(effect->model);
            Hu3DModelPosSetV(effect->model, pos);
            Hu3DModelLayerSet(effect->model, 2);
            effect->active = 1;
            break;
        }
    }
}
