/* Decathlon Park player introductions and entrance models. */
#include "REL/mgmdecathlondll/module_types.h"
#include "datadir_enum.h"
#include "msm_se.h"

#define MSM_SE_DECA_LAST_ENTRY 1247
#define MSM_SE_DECA_ENTRY 1246

s32 lbl_1_data_2B0[3] = {
    DATANUM(0x9e0000, 1),
    DATANUM(0x9e0000, 2),
    DATANUM(0x9e0000, 0),
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

    lbl_1_bss_8C = fn_8003AB38(lbl_1_bss_C0, 4096, 3, 0, -1, fn_1_5BCC);
    lbl_1_bss_8C->work[0] = lbl_1_bss_8C->work[1] =
        lbl_1_bss_8C->work[2] = lbl_1_bss_8C->work[3] = 0;
    lbl_1_bss_8C->stat |= (1 << 8);
    for (modelIndex = 0; modelIndex < 3; modelIndex++) {
        lbl_1_bss_8C->mdlId[modelIndex] =
            fn_80025A04(fn_80008DD0(lbl_1_data_2B0[modelIndex],
                                                 0x10000000, 2));
        fn_80026B64(lbl_1_bss_8C->mdlId[modelIndex], 1);
        fn_80026B8C(lbl_1_bss_8C->mdlId[modelIndex], 1);
        fn_800269E8(lbl_1_bss_8C->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
    }
    fn_80026FFC(lbl_1_bss_8C->mdlId[1]);
    fn_8002D86C(lbl_1_bss_8C->mdlId[0], 0.5f);
    fn_800269E8(lbl_1_bss_8C->mdlId[1],
                     ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                     ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
    fn_8002D414(lbl_1_bss_8C->mdlId[1], 0.0f);
    fn_1_5C50();
}

/* The mode introduction starts the shared model's main and shape animations here. */
void fn_1_5AFC(void) {
    if (lbl_1_bss_8C->work[0] == 0) {
        fn_80026A44(lbl_1_bss_8C->mdlId[1],
                           ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                           ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(lbl_1_bss_8C->mdlId[1], ((1 << 30) | (1 << 0)));
        fn_8002D414(lbl_1_bss_8C->mdlId[1], 0.0f);
        fn_8002CF98(lbl_1_bss_8C->mdlId[1], 0.0f);
        lbl_1_bss_8C->work[0] = 1U;
    }
}

/* When the shared model's main motion reaches its end, its frame callback pauses all layers
 * and clamps the main motion to its final frame. */
void fn_1_5BCC(OMOBJ *obj) {
    f32 maxTime;

    maxTime = fn_8002D530(obj->mdlId[1]);
    if (fn_8002D4E0(obj->mdlId[1]) >= maxTime) {
        fn_800269E8(obj->mdlId[1], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                          ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_8002D414(obj->mdlId[1], fn_8002D530(obj->mdlId[1]));
    }
}

/* Mode setup creates ten hidden models for the final score's staggered entrance. */
void fn_1_5C50(void) {
    OMOBJ *object;
    s16 modelIndex;

    object = fn_8003AB38(lbl_1_bss_C0, 4224, 10U, 0U, -1, fn_1_6224);
    lbl_1_bss_88 = object;
    object->stat |= (1 << 8);
    object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
    object->data = NULL;
    for (modelIndex = 0; modelIndex < 10; modelIndex++) {
        object->mdlId[modelIndex] =
            fn_80025A04(fn_80008DD0(
                DATANUM(0x9e0000, 4), 0x10000000, 2));
        fn_80026B64(object->mdlId[modelIndex], 1U);
        fn_80026B8C(object->mdlId[modelIndex], 1);
        fn_800268B0(object->mdlId[modelIndex], 1.5f, 1.5f, 1.5f);
        fn_800269E8(object->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
        fn_800269E8(object->mdlId[modelIndex],
                         ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                         ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_8002D414(object->mdlId[modelIndex], 0.0f);
        fn_800269E8(object->mdlId[modelIndex], (1 << 0));
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
    lbl_1_bss_88->data = HuMemDirectMallocNum(HEAP_HEAP, 280, 0x10000000);
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
                    fn_80026790(object->mdlId[player], &motion[player].position);
                    fn_80026838(object->mdlId[player], &motion[player].rotation);
                    fn_80026A44(object->mdlId[player],
                                       ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                           ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
                    fn_800269E8(object->mdlId[player],
                                     ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
                    fn_8002D414(object->mdlId[player], 0.0f);
                    fn_8002CF98(object->mdlId[player], 0.0f);
                    fn_80026A44(object->mdlId[player], (1 << 0));
                    /* The last active actor has a larger model and a stronger arrival cue. */
                    if ((u32)(object->work[0] - 1) == (u32)player) {
                        fn_800268B0(object->mdlId[player], 2.5f,
                                         2.5f, 2.5f);
                        HuAudFXPlay(MSM_SE_DECA_LAST_ENTRY);
                        for (controller = 0; controller < 4; controller++) {
                            if (fn_1_9784(controller) == 0) {
                                fn_8006A030(controller, 20, 20, 0);
                            }
                        }
                    } else {
                        HuAudFXPlay(MSM_SE_DECA_ENTRY);
                        for (controller = 0; controller < 4; controller++) {
                            if (fn_1_9784(controller) == 0) {
                                fn_8006A030(controller, 20, 4, 4);
                            }
                        }
                    }
                }
                motionEnd = fn_8002D530(object->mdlId[player]);
                if (1.0f + fn_8002D4E0(object->mdlId[player]) >= motionEnd) {
                    fn_800269E8(object->mdlId[player], (1 << 0));
                    fn_800269E8(object->mdlId[player],
                                     ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                         ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
                    fn_8002D414(object->mdlId[player], 0.0f);
                    fn_8002CF98(object->mdlId[player], 0.0f);
                    motion[player].countdown = -1000;
                }
            }
        }
    }
}

/* The final score celebration creates this marker for each player tied for the lead. */
void fn_1_65A0(s16 playerNo) {
    OMOBJ *object;
    HSF_OBJECT *modelObject;
    HSF_CONSTDATA *constantData;
    s16 playerIndex;

    playerIndex = playerNo;
    object = fn_8003AB38(lbl_1_bss_C0, 4224, 1, 0, -1, fn_1_68C0);
    lbl_1_bss_78[playerIndex] = object;
    object->stat |= (1 << 8);
    object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
    object->data = NULL;
    *object->mdlId = fn_80025A04(fn_80008DD0(
        DATANUM(0x9e0000, 83), 0x10000000, 2));
    fn_80026B64(*object->mdlId, 1);
    fn_80026B8C(*object->mdlId, 1);
    fn_80026790(*object->mdlId, (HuVecF *)&lbl_1_data_2BC[playerIndex]);
    fn_800268B0(*object->mdlId, 0.6f, 0.9f, 0.6f);
    fn_80026808(*object->mdlId, 0.0f, 0.0f, 0.0f);
    fn_800269E8(*object->mdlId, ((1 << 30) | (1 << 0)));
    fn_8002D414(*object->mdlId, 0.0f);
    fn_8002D86C(*object->mdlId, 2.5f);

    /* Hide the named paper and spotlight subobjects in each leader's marker model. */
    modelObject = fn_80026BF4(*object->mdlId, "obiTEST-paperAKA");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
    modelObject = fn_80026BF4(*object->mdlId, "obiTEST-spot_root");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
    modelObject = fn_80026BF4(*object->mdlId, "obiTEST-spothokori");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
    modelObject = fn_80026BF4(*object->mdlId, "obiTEST-spot");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
    modelObject = fn_80026BF4(*object->mdlId, "obiTEST-spot_efA");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
    modelObject = fn_80026BF4(*object->mdlId, "obiTEST-spot_efB");
    modelObject->flags |= 0x400;
    constantData = modelObject->constData;
    if (constantData) {
        constantData->attr |= 0x2000;
    }
}

OMOBJ *lbl_1_bss_8C;
OMOBJ *lbl_1_bss_88;
OMOBJ *lbl_1_bss_78[4];
