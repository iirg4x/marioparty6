/* Decathlon Park player introductions and entrance models. */
#include "REL/mgmdecathlondll/module_types.h"
#include "datadir_enum.h"
#include "msm_se.h"

#define MSM_SE_DECA_LAST_ENTRY 1247
#define MSM_SE_DECA_ENTRY 1246

s32 lbl_1_data_2B0[3] = {
    DATANUM(DATA_mgmdecathlon, 1),
    DATANUM(DATA_mgmdecathlon, 2),
    DATANUM(DATA_mgmdecathlon, 0),
};
HuVecF lbl_1_data_2BC[4] = {
    {-280.0f, -77.0f, 0.0f},
    {-120.0f, -77.0f, 0.0f},
    {120.0f, -77.0f, 0.0f},
    {280.0f, -77.0f, 0.0f},
};
extern void fn_1_6224(OMOBJ *object);
void fn_1_5BCC(OMOBJ *object);
void fn_1_5C50(void);

/* Stores the clamped stereo pan used by Decathlon Park's sound cues. */
void fn_1_58D0(s16 pan) {
    if (pan < 32) {
        pan = 32;
    }
    if (pan > 96) {
        pan = 96;
    }
    lbl_1_data_2A8 = pan;
}

/* Mode setup creates three shared entrance-scene models and prepares the final-score models. */
void fn_1_5900(void) {
    s32 modelIndex;

    lbl_1_bss_8C = omAddObjEx(lbl_1_bss_C0, 4096, 3, 0, -1, fn_1_5BCC);
    lbl_1_bss_8C->work[0] = lbl_1_bss_8C->work[1] =
        lbl_1_bss_8C->work[2] = lbl_1_bss_8C->work[3] = 0;
    lbl_1_bss_8C->stat |= OM_STAT_MODELPAUSE;
    for (modelIndex = 0; modelIndex < 3; modelIndex++) {
        lbl_1_bss_8C->mdlId[modelIndex] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_2B0[modelIndex],
                                                 HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_8C->mdlId[modelIndex], 1);
        Hu3DModelLayerSet(lbl_1_bss_8C->mdlId[modelIndex], 1);
        Hu3DModelAttrSet(lbl_1_bss_8C->mdlId[modelIndex], HU3D_MOTATTR_LOOP);
    }
    Hu3DModelShadowMapSet(lbl_1_bss_8C->mdlId[1]);
    Hu3DMotionSpeedSet(lbl_1_bss_8C->mdlId[0], 0.5f);
    Hu3DModelAttrSet(lbl_1_bss_8C->mdlId[1],
                     HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                     HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
    Hu3DMotionTimeSet(lbl_1_bss_8C->mdlId[1], 0.0f);
    fn_1_5C50();
}

/* The mode introduction starts the shared model's main and shape animations here. */
void fn_1_5AFC(void) {
    if (lbl_1_bss_8C->work[0] == 0) {
        Hu3DModelAttrReset(lbl_1_bss_8C->mdlId[1],
                           HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                           HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
        Hu3DModelAttrSet(lbl_1_bss_8C->mdlId[1], HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_8C->mdlId[1], 0.0f);
        Hu3DMotionShapeTimeSet(lbl_1_bss_8C->mdlId[1], 0.0f);
        lbl_1_bss_8C->work[0] = 1U;
    }
}

/* When the shared model's main motion reaches its end, its frame callback pauses all layers
 * and clamps the main motion to its final frame. */
void fn_1_5BCC(OMOBJ *obj) {
    f32 maxTime;

    maxTime = Hu3DMotionMaxTimeGet(obj->mdlId[1]);
    if (Hu3DMotionTimeGet(obj->mdlId[1]) >= maxTime) {
        Hu3DModelAttrSet(obj->mdlId[1], HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                          HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
        Hu3DMotionTimeSet(obj->mdlId[1], Hu3DMotionMaxTimeGet(obj->mdlId[1]));
    }
}

/* Mode setup creates ten hidden models for the final score's staggered entrance. */
void fn_1_5C50(void) {
    OMOBJ *object;
    s16 modelIndex;

    object = omAddObjEx(lbl_1_bss_C0, 4224, 10U, 0U, -1, fn_1_6224);
    lbl_1_bss_88 = object;
    object->stat |= OM_STAT_MODELPAUSE;
    object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
    object->data = NULL;
    for (modelIndex = 0; modelIndex < 10; modelIndex++) {
        object->mdlId[modelIndex] =
            Hu3DModelCreate(HuDataSelHeapReadNum(
                DATANUM(DATA_mgmdecathlon, 4), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(object->mdlId[modelIndex], 1U);
        Hu3DModelLayerSet(object->mdlId[modelIndex], 1);
        Hu3DModelScaleSet(object->mdlId[modelIndex], 1.5f, 1.5f, 1.5f);
        Hu3DModelAttrSet(object->mdlId[modelIndex], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(object->mdlId[modelIndex],
                         HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                         HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
        Hu3DMotionTimeSet(object->mdlId[modelIndex], 0.0f);
        Hu3DModelAttrSet(object->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
    }
}

typedef struct DecathlonMotionPath {
    Vec position; /* entrance position in world units */
    f32 pitch; /* X rotation in degrees */
    f32 yaw; /* Y rotation in degrees */
    f32 roll; /* Z rotation in degrees */
    s16 delay; /* frames until this model appears */
} DecathlonMotionPath;

/* The final results sequence randomizes model positions and schedules score-dependent entrances. */
void fn_1_5DF8(void) {
    f32 horizontalPositions[10];
    f32 positionSwap;
    s16 highestScore;
    s16 player;
    s16 swapIndex;
    DecathlonMotionPath *paths;

    highestScore = 0;
    for (player = 0; player < 4; player++) {
        if (highestScore < MgModeScore[player]) {
            highestScore = MgModeScore[player];
        }
    }
    highestScore = (highestScore / 1000) + 1;
    if (highestScore > 10) {
        highestScore = 10;
    }
    /* Show one model per thousand points, plus one, up to the ten available models. */
    lbl_1_bss_88->work[0] = highestScore;
    if (lbl_1_bss_88->data) {
        HuMemDirectFree(lbl_1_bss_88->data);
        lbl_1_bss_88->data = NULL;
    }
    lbl_1_bss_88->data = HuMemDirectMallocNum(HEAP_HEAP, 280, HU_MEMNUM_OVL);
    memset(lbl_1_bss_88->data, 0, 280);
    paths = lbl_1_bss_88->data;
    for (player = 0; player < 10; player++) {
        horizontalPositions[player] = (player * 120) - 540;
    }
    for (player = 0; player < 10; player++) {
        for (swapIndex = 0; swapIndex < 10; swapIndex++) {
            if (frandmod(2) != 0) {
                positionSwap = horizontalPositions[player];
                horizontalPositions[player] = horizontalPositions[swapIndex];
                horizontalPositions[swapIndex] = positionSwap;
            }
        }
    }
    for (player = 0; player < 10; player++) {
        paths[player].position.x = horizontalPositions[player];
        paths[player].position.y = (frandmod(12) * 10) + 800;
        paths[player].position.z = -900.0f;
        paths[player].pitch = 0.0f;
        paths[player].yaw = 10.0f * (1.0f + frandmod(36));
        paths[player].roll = 0.0f;
        paths[player].delay = (player * 60) + 60;
    }
}

typedef struct MgmDecathlonRecord_s {
    u8 poseBytes[24]; /* position and rotation, not read by the completion poll */
    s16 countdown; /* -1000 after completion; -1 while the entrance motion runs */
    u8 reservedBytes[2]; /* trailing storage, not read by the completion poll */
} MgmDecathlonRecord;

/* The results flow polls this until every active entrance actor has finished. */
s32 fn_1_6188(void) {
    s16 index;
    s32 result;
    MgmDecathlonRecord *records;

    records = lbl_1_bss_88->data;
    if (records == 0) {
        return 1;
    }
    index = 0;
    result = 1;
    while (index < 10) {
        if ((lbl_1_bss_88->work[0] > index) && (records[index].countdown >= -1)) {
            result = 0;
        }
        index++;
    }
    return result;
}

typedef struct DecathlonMotionState {
    Vec position; /* world position applied on the first active frame */
    Vec rotation; /* rotation in degrees applied on the first active frame */
    s16 countdown; /* frames until appearance; -1 playing; -1000 finished */
} DecathlonMotionState;

extern s16 fn_1_9784(s16 controller);

/* The entrance object's per-frame callback reveals timed models and hides each after its motion. */
void fn_1_6224(OMOBJ *object) {
    DecathlonMotionState *motion;
    s16 player;
    s16 controller;
    f32 motionEnd;

    motion = object->data;
    if (motion) {
        for (player = 0; player < 10; player++) {
            if ((u32)object->work[0] > (u32)player && motion[player].countdown >= -1) {
                motion[player].countdown--;
                /* Keep an active actor at -1 until its motion finishes. */
                if (motion[player].countdown < -1) {
                    motion[player].countdown = -1;
                }
                if (motion[player].countdown == 0) {
                    Hu3DModelPosSetV(object->mdlId[player], &motion[player].position);
                    Hu3DModelRotSetV(object->mdlId[player], &motion[player].rotation);
                    Hu3DModelAttrReset(object->mdlId[player],
                                       HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                           HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                    Hu3DModelAttrSet(object->mdlId[player],
                                     HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP);
                    Hu3DMotionTimeSet(object->mdlId[player], 0.0f);
                    Hu3DMotionShapeTimeSet(object->mdlId[player], 0.0f);
                    Hu3DModelAttrReset(object->mdlId[player], HU3D_ATTR_DISPOFF);
                    /* The last active actor has a larger model and a stronger arrival cue. */
                    if ((u32)(object->work[0] - 1) == (u32)player) {
                        Hu3DModelScaleSet(object->mdlId[player], 2.5f,
                                         2.5f, 2.5f);
                        HuAudFXPlay(MSM_SE_DECA_LAST_ENTRY);
                        for (controller = 0; controller < 4; controller++) {
                            if (fn_1_9784(controller) == 0) {
                                omVibrate(controller, 20, 20, 0);
                            }
                        }
                    } else {
                        HuAudFXPlay(MSM_SE_DECA_ENTRY);
                        for (controller = 0; controller < 4; controller++) {
                            if (fn_1_9784(controller) == 0) {
                                omVibrate(controller, 20, 4, 4);
                            }
                        }
                    }
                }
                motionEnd = Hu3DMotionMaxTimeGet(object->mdlId[player]);
                if (1.0f + Hu3DMotionTimeGet(object->mdlId[player]) >= motionEnd) {
                    Hu3DModelAttrSet(object->mdlId[player], HU3D_ATTR_DISPOFF);
                    Hu3DModelAttrSet(object->mdlId[player],
                                     HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                         HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                    Hu3DMotionTimeSet(object->mdlId[player], 0.0f);
                    Hu3DMotionShapeTimeSet(object->mdlId[player], 0.0f);
                    motion[player].countdown = -1000;
                }
            }
        }
    }
}

void fn_1_68C0(OMOBJ *object);

/* The final score celebration creates this marker for each player tied for the lead. */
void fn_1_65A0(s16 playerNo) {
    OMOBJ *object;
    HSF_OBJECT *modelObject;
    HSF_CONSTDATA *constantData;
    s16 playerIndex;

    playerIndex = playerNo;
    object = omAddObjEx(lbl_1_bss_C0, 4224, 1, 0, -1, fn_1_68C0);
    lbl_1_bss_78[playerIndex] = object;
    object->stat |= OM_STAT_MODELPAUSE;
    object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
    object->data = NULL;
    *object->mdlId = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mgmdecathlon, 83), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(*object->mdlId, 1);
    Hu3DModelLayerSet(*object->mdlId, 1);
    Hu3DModelPosSetV(*object->mdlId, (HuVecF *)&lbl_1_data_2BC[playerIndex]);
    Hu3DModelScaleSet(*object->mdlId, 0.6f, 0.9f, 0.6f);
    Hu3DModelRotSet(*object->mdlId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(*object->mdlId, HU3D_MOTATTR_LOOP);
    Hu3DMotionTimeSet(*object->mdlId, 0.0f);
    Hu3DMotionSpeedSet(*object->mdlId, 2.5f);

    /* Hide the named paper and spotlight subobjects in each leader's marker model. */
    modelObject = Hu3DModelObjPtrGet(*object->mdlId, "obiTEST-paperAKA");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
    modelObject = Hu3DModelObjPtrGet(*object->mdlId, "obiTEST-spot_root");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
    modelObject = Hu3DModelObjPtrGet(*object->mdlId, "obiTEST-spothokori");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
    modelObject = Hu3DModelObjPtrGet(*object->mdlId, "obiTEST-spot");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
    modelObject = Hu3DModelObjPtrGet(*object->mdlId, "obiTEST-spot_efA");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
    modelObject = Hu3DModelObjPtrGet(*object->mdlId, "obiTEST-spot_efB");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
}

OMOBJ *lbl_1_bss_8C;
OMOBJ *lbl_1_bss_88;
OMOBJ *lbl_1_bss_78[4];
