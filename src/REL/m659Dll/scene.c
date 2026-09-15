#include "game/object.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/mg/seqman.h"
#include "string.h"

/* Unreferenced initialized storage: twelve zero bytes and five ten-byte
 * string cells. The original aggregate type and purpose are unknown.
 * The following two bytes are ordinary section alignment, not array data. */
char lbl_1_data_198[62] =
    "\0\0\0\0\0\0\0\0\0\0\0\0"
    "col\0\0\0\0\0\0\0"
    "post_R\0\0\0\0"
    "post_L\0\0\0\0"
    "post_R_C\0\0"
    "post_L_C";

/* The first 40 bytes preserve storage whose original field types are unknown. */
typedef struct M659Scene {
    unsigned char unobserved_00[40];
    HuVecF pos;
    float scale;
    float z;
    float speed;
} M659Scene;

OMOBJ *lbl_1_bss_58;
void fn_1_329C(OMOBJMAN *manager);
void fn_1_530C(OMOBJ *obj);
void fn_1_564C(OMOBJ *obj);
int fn_1_1840(s16 player);

void fn_1_4E68(OMOBJMAN *manager)
{
    M659Scene *work;
    OMOBJ *obj;
    obj = omAddObjEx(manager, 30, 5, 3, -1, fn_1_530C);
    lbl_1_bss_58 = obj;
    obj->stat |= 256;
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659Scene), 268435456);
    memset(work, 0, sizeof(M659Scene));
    work->scale = 0.6f;
    work->z = 7500;
    work->speed = work->z / 2040;
    fn_1_329C(manager);
}

void fn_1_4F48(void)
{
}

void fn_1_4F4C(HuVecF *pos)
{
    M659Scene *work = lbl_1_bss_58->data;
    pos->x = work->pos.x;
    pos->y = work->pos.y;
    pos->z = work->pos.z;
}

void fn_1_4F88(OMOBJ *obj)
{
    M659Scene *work = obj->data;
    HU3D_MODELID model = 0;
    HU3D_MOTIONID motion = 0;
    model = obj->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(7733248, 268435456, HEAP_MODEL));
    motion = obj->mtnId[0] = Hu3DJointMotion(model, HuDataSelHeapReadNum(7733249, 268435456, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, 1);
    Hu3DModelAttrSet(model, 1073741825);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0, 0, 17000);
    Hu3DModelRotSet(model, 0, 0, 5);
    Hu3DModelScaleSet(model, work->scale, work->scale, work->scale);
    Hu3DModelCameraSet(model, 3);
    model = obj->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(7733250, 268435456, HEAP_MODEL));
    motion = obj->mtnId[1] = Hu3DJointMotion(model, HuDataSelHeapReadNum(7733251, 268435456, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, 1);
    Hu3DModelAttrSet(model, 1073741825);
    Hu3DModelLayerSet(model, 5);
    Hu3DModelPosSet(model, 0, 0, 17000);
    Hu3DModelRotSet(model, 0, 0, 5);
    Hu3DModelScaleSet(model, work->scale, work->scale, work->scale);
    Hu3DModelCameraSet(model, 3);
    Hu3DZClearLayerSet(1);
    Hu3DBGColorSet(0, 0, 50);
}

void fn_1_51F0(OMOBJ *obj)
{
    M659Scene *work = obj->data;
    HU3D_MODELID model;
    HU3D_MOTIONID motion;
    model = obj->mdlId[3] = Hu3DModelCreate(HuDataSelHeapReadNum(7733252, 268435456, HEAP_MODEL));
    motion = obj->mtnId[2] = Hu3DJointMotion(model, HuDataSelHeapReadNum(7733253, 268435456, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, 1);
    Hu3DModelLayerSet(model, 1);
    Hu3DModelPosSet(model, 0, 0, work->z);
    Hu3DModelRotSet(model, 0, 180, 0);
    Hu3DModelCameraSet(model, 3);
}

void fn_1_530C(OMOBJ *obj)
{
    fn_1_4F88(obj);
    fn_1_51F0(obj);
    obj->objFunc = fn_1_564C;
}

void fn_1_564C(OMOBJ *obj)
{
    M659Scene *work = obj->data;
    float scale = work->scale;
    HU3D_MODELID model;
    if (MgSeqModeGet() == 5 || MgSeqModeGet() == 3 ||
        (MgSeqModeGet() == 6 && fn_1_1840(0) == 0 && fn_1_1840(1) == 0)) {
        model = obj->mdlId[0];
        Hu3DModelScaleSet(model, scale, scale, scale);
        model = obj->mdlId[1];
        Hu3DModelScaleSet(model, scale, scale, scale);
        work->scale += 0.0002f;
        if (work->scale > 1.0f) {
            work->scale = 1.0f;
        }
        model = obj->mdlId[3];
        Hu3DModelPosSet(model, 0, 0, work->z);
        work->z -= work->speed;
        if (work->z < 0.0f) {
            work->z = 0.0f;
        }
    }
}

void fn_1_57BC(void)
{
    M659Scene *work = lbl_1_bss_58->data;
    float scale = work->scale;
    HU3D_MODELID model;
    model = lbl_1_bss_58->mdlId[0];
    Hu3DModelScaleSet(model, 1, 1, 1);
    Hu3DModelPosSet(model, 0, 0, 12000);
    model = lbl_1_bss_58->mdlId[1];
    Hu3DModelScaleSet(model, 1, 1, 1);
    Hu3DModelPosSet(model, 0, 0, 12000);
    model = lbl_1_bss_58->mdlId[3];
    Hu3DModelPosSet(model, 0, 0, 0);
    Hu3DModelScaleSet(model, 0.7f, 0.7f, 1);
}
