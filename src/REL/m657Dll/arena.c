#include "REL/m657Dll.h"
#include "game/memory.h"
#include "game/audio.h"
#include "string.h"

OMOBJ *lbl_1_bss_28;

char lbl_1_data_8C[5][10] = {
    "col", "post_R", "post_L", "post_R_C", "post_L_C"
};

void fn_1_15C0(OMOBJMAN *objman)
{
    OMOBJ *obj;
    M657ArenaWork *work;

    obj = omAddObjEx(objman, 30, 6, 3, -1, fn_1_1B84);
    lbl_1_bss_28 = obj;
    obj->stat |= OM_STAT_MODELPAUSE;
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(M657ArenaWork), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(M657ArenaWork));
}

void fn_1_1658(void)
{
}

void fn_1_165C(OMOBJ *obj)
{
    M657ArenaWork *work = obj->data;
    HU3D_MODELID model = 0;
    HU3D_MOTIONID motion = 0;

    model = obj->mdlId[0] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 6), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(model, 1);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet(model, 3);
    Hu3DZClearLayerSet(1);
    Hu3DBGColorSet(0, 0, 50);
}

void fn_1_1720(void)
{
    M657ArenaItem *item;
    OMOBJ *obj = lbl_1_bss_28;
    HU3D_MODELID model;
    M657ArenaWork *work = obj->data;
    HU3D_MOTIONID motion;

    item = &work->item;
    item->pos.x = 0.0f;
    item->pos.y = 2573.0f;
    item->pos.z = 0.0f;
    item->unk_0C = 0;
    model = obj->mdlId[1] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 0), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(model, item->pos.x, item->pos.y, item->pos.z);
    Hu3DModelLayerSet(model, 1);
    Hu3DModelCameraSet(model, 3);
    model = obj->mdlId[2] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 2), HU_MEMNUM_OVL, HEAP_MODEL));
    motion = obj->mtnId[0] = Hu3DJointMotion(model,
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 3), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, 0.0f);
    Hu3DModelLayerSet(model, 1);
    Hu3DModelCameraSet(model, 3);
    Hu3DModelHookSet(obj->mdlId[1], "spaceship00-door_target", model);
}

void fn_1_1884(OMOBJ *obj)
{
    HU3D_MODELID model = 0;
    HU3D_MOTIONID motion = 0;

    model = obj->mdlId[4] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 7), HU_MEMNUM_OVL, HEAP_MODEL));
    motion = obj->mtnId[1] = Hu3DJointMotion(model,
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 8), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, 1.0f);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(model, 0.0f, 500.0f, 0.0f);
    Hu3DModelScaleSet(model, 1.0f, 1.0f, 1.0f);
    Hu3DModelLayerSet(model, 1);
    Hu3DModelCameraSet(model, 3);
}

void fn_1_19B4(OMOBJ *obj)
{
    HU3D_MODELID model = 0;
    HU3D_MOTIONID motion = 0;

    model = obj->mdlId[5] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 20), HU_MEMNUM_OVL, HEAP_MODEL));
    motion = obj->mtnId[2] = Hu3DJointMotion(model,
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 21), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, 1.0f);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelLayerSet(model, 1);
    Hu3DModelCameraSet(model, 3);
}

void fn_1_1AD4(void)
{
    HU3D_MODELID model = lbl_1_bss_28->mdlId[2];
    Hu3DMotionSpeedSet(model, 1.0f);
}

s32 fn_1_1B20(void)
{
    OMOBJ *obj = lbl_1_bss_28;
    M657ArenaWork *work = obj->data;
    HU3D_MODELID model = obj->mdlId[2];

    if (Hu3DMotionEndCheck(model)) {
        return TRUE;
    }
    return FALSE;
}

void fn_1_1B84(OMOBJ *obj)
{
    fn_1_165C(obj);
    fn_1_1720();
    fn_1_1884(obj);
    fn_1_19B4(obj);
    obj->objFunc = fn_1_1F6C;
}

void fn_1_1F6C(OMOBJ *obj)
{
    M657ArenaWork *work = obj->data;

    switch (work->state) {
        case 0:
            work->state++;
            break;
        case 1:
            work->state++;
            break;
        case 2:
            if (MgSeqModeGet() == MGSEQ_MODE_START) {
                work->state++;
                HuAudFXPlayPan(2087, 48);
                HuAudFXPlayPan(2088, 80);
            }
            break;
        case 3:
            break;
    }
}
