/* Creates and updates the Lunar-tics arena, spaceship, and background scene. */
#include "REL/m657Dll.h"
#include "game/memory.h"
#include "game/audio.h"
#include "string.h"

/* Arena entrance sound-effect resource IDs. */
#define M657_SFX_START_CUE_A 2087
#define M657_SFX_START_CUE_B 2088

OMOBJ *lbl_1_bss_28;

/* Twelve zero-filled bytes with no demonstrated gameplay use in this module. */
u8 lbl_1_data_80[12] = {0};

char lbl_1_data_8C[5][10] = {
    "col", "post_R", "post_L", "post_R_C", "post_L_C"
};

/* Creates the paused arena object and allocates its scene state during setup. */
void fn_1_15C0(OMOBJMAN *objman)
{
    OMOBJ *obj;
    M657ArenaWork *arenaWork;

    obj = omAddObjEx(objman, 30, 6, 3, -1, fn_1_1B84);
    lbl_1_bss_28 = obj;
    obj->stat |= OM_STAT_MODELPAUSE;
    arenaWork = obj->data = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(M657ArenaWork), HU_MEMNUM_OVL);
    memset(arenaWork, 0, sizeof(M657ArenaWork));
}

void fn_1_1658(void)
{
}

/* Creates the arena background model and applies its initial camera and color. */
void fn_1_165C(OMOBJ *obj)
{
    M657ArenaWork *arenaWork = obj->data;
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

/* Creates the arena floor and the spaceship door model with its motion. */
void fn_1_1720(void)
{
    M657ArenaItem *item;
    OMOBJ *obj = lbl_1_bss_28;
    HU3D_MODELID model;
    M657ArenaWork *arenaWork = obj->data;
    HU3D_MOTIONID motion;

    item = &arenaWork->item;
    item->pos.x = 0.0f;
    item->pos.y = 2573.0f;
    item->pos.z = 0.0f;
    item->unknownValue = 0;
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

/* Creates the looping arena decoration and places it above the playfield. */
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

/* Creates the result-scene model used when the sequence changes camera views. */
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

/* Starts the spaceship door motion at normal playback speed. */
void fn_1_1AD4(void)
{
    HU3D_MODELID model = lbl_1_bss_28->mdlId[2];
    Hu3DMotionSpeedSet(model, 1.0f);
}

/* Reports whether the spaceship door motion has ended. */
s32 fn_1_1B20(void)
{
    OMOBJ *obj = lbl_1_bss_28;
    M657ArenaWork *arenaWork = obj->data;
    HU3D_MODELID model = obj->mdlId[2];

    if (Hu3DMotionEndCheck(model)) {
        return TRUE;
    }
    return FALSE;
}

/* Object-create callback that builds the arena models and installs its update. */
void fn_1_1B84(OMOBJ *obj)
{
    fn_1_165C(obj);
    fn_1_1720();
    fn_1_1884(obj);
    fn_1_19B4(obj);
    obj->objFunc = fn_1_1F6C;
}

/* Waits for the main game sequence and starts the arena entrance sound cues. */
void fn_1_1F6C(OMOBJ *obj)
{
    M657ArenaWork *arenaWork = obj->data;

    switch (arenaWork->state) {
        case 0:
            arenaWork->state++;
            break;
        case 1:
            arenaWork->state++;
            break;
        case 2:
            if (MgSeqModeGet() == MGSEQ_MODE_START) {
                arenaWork->state++;
                HuAudFXPlayPan(M657_SFX_START_CUE_A, 48);
                HuAudFXPlayPan(M657_SFX_START_CUE_B, 80);
            }
            break;
        case 3:
            break;
    }
}
