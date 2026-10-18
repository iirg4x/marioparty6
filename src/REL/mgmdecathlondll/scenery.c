/* Decathlon Park scenery models and their animation sequences. */
#include "REL/mgmdecathlondll/module_types.h"
#include "datadir_enum.h"
#include <math.h>
#include "dolphin/gx.h"
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
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "msm_se.h"

#define MSM_SE_DECA_EVENT_REVEAL 1239
#define MSM_SE_DECA_EVENT_READY 1240
#define DECATHLON_NO_AUDIO_HANDLE 4294967295U

/* Each leader marker's frame callback pauses its motion layers when the next main-motion
 * step would reach or pass the end. */
void fn_1_68C0(OMOBJ *obj) {
    f32 maxTime;

    maxTime = Hu3DMotionMaxTimeGet(*obj->mdlId);
    if ((Hu3DData[*obj->mdlId].motWork.speed + Hu3DMotionTimeGet(*obj->mdlId)) >= maxTime) {
        Hu3DModelAttrSet(*obj->mdlId,
                         HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                         HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
        obj->work[0] = 100;
    }
}

Vec lbl_1_data_358 = {0.0f, 0.0f, 185.0f};

/* Mode setup creates the foreground actor, its right-hand prop, and six motion choices. */
void fn_1_6960(void) {
    lbl_1_bss_A0 = omAddObjEx(lbl_1_bss_C0, 4608, 2U, 6U, -1, fn_1_6E80);
    lbl_1_bss_A0->data = NULL;
    lbl_1_bss_A0->work[0] = 0;
    lbl_1_bss_A0->work[1] = 0;
    lbl_1_bss_A0->work[2] = 0;
    lbl_1_bss_A0->work[3] = 0;
    lbl_1_bss_A0->stat = lbl_1_bss_A0->stat | OM_STAT_MODELPAUSE;
    *lbl_1_bss_A0->mdlId = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mgmdecathlon, 45), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(*lbl_1_bss_A0->mdlId, 1U);
    Hu3DModelLayerSet(*lbl_1_bss_A0->mdlId, 5);
    Hu3DModelPosSetV(*lbl_1_bss_A0->mdlId, &lbl_1_data_358);
    Hu3DModelAttrSet(*lbl_1_bss_A0->mdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(*lbl_1_bss_A0->mdlId, 1.0f, 1.0f, 1.0f);
    Hu3DModelShadowSet(*lbl_1_bss_A0->mdlId);
    Hu3DLLightCreate(*lbl_1_bss_A0->mdlId, 0.0f, 1500.0f, 1500.0f, 0.0f, -0.1f, -1.0f, 204U, 204U,
                     204U);
    lbl_1_bss_A0->mdlId[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mgmdecathlon, 52), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelHookSet(*lbl_1_bss_A0->mdlId, "f-itemhook_R", lbl_1_bss_A0->mdlId[1]);
    *lbl_1_bss_A0->mtnId = Hu3DJointMotion(*lbl_1_bss_A0->mdlId,
                                           HuDataSelHeapReadNum(
                                               DATANUM(DATA_mgmdecathlon, 46),
                                               HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(*lbl_1_bss_A0->mdlId, *lbl_1_bss_A0->mtnId);
    lbl_1_bss_A0->mtnId[1] = Hu3DJointMotion(
        *lbl_1_bss_A0->mdlId, HuDataSelHeapReadNum(
            DATANUM(DATA_mgmdecathlon, 47), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_A0->mtnId[2] = Hu3DJointMotion(
        *lbl_1_bss_A0->mdlId, HuDataSelHeapReadNum(
            DATANUM(DATA_mgmdecathlon, 48), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_A0->mtnId[3] = Hu3DJointMotion(
        *lbl_1_bss_A0->mdlId, HuDataSelHeapReadNum(
            DATANUM(DATA_mgmdecathlon, 49), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_A0->mtnId[4] = Hu3DJointMotion(
        *lbl_1_bss_A0->mdlId, HuDataSelHeapReadNum(
            DATANUM(DATA_mgmdecathlon, 50), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_A0->mtnId[5] = Hu3DJointMotion(
        *lbl_1_bss_A0->mdlId, HuDataSelHeapReadNum(
            DATANUM(DATA_mgmdecathlon, 51), HU_MEMNUM_OVL, HEAP_MODEL));
}

/* The final-results sequence requests the actor's repeating reaction or a return to idle. */
void fn_1_6DC4(u16 enabled) {
    if ((lbl_1_bss_A0->work[1] == 0U) && (enabled != 0U)) {
        lbl_1_bss_A0->work[1] = 1U;
    }
    if ((lbl_1_bss_A0->work[1] != 0U) && (enabled == 0U)) {
        lbl_1_bss_A0->work[1] = 2U;
    }
}

/* Dialogue starts the foreground actor's short response animation. */
void fn_1_6E38(void) {
    lbl_1_bss_A0->work[0] = 1;
}

/* The next-event introduction starts a 25-frame rising entrance for the foreground actor. */
void fn_1_6E50(void) {
    lbl_1_bss_A0->work[2] = 25;
}

/* Before launching an event, the sequence requests the actor's turn-away motion. */
void fn_1_6E68(void) {
    lbl_1_bss_A0->work[3] = 1;
}

/* The foreground actor's frame callback advances entrance, turn-away, reaction, or dialogue
 * motion. */
void fn_1_6E80(OMOBJ *object) {
    Vec position;
    f32 entranceRemainingFraction;
    f32 horizontalStepFraction;
    if (object->work[2] != 0) {
        /* The shrinking vertical step and growing horizontal step curve the entrance to the
         * right. */
        Hu3DModelPosGet(object->mdlId[0], &position);
        entranceRemainingFraction = object->work[2] / 25.0f;
        horizontalStepFraction = 1.0f - entranceRemainingFraction;
        position.x += 17.0f * horizontalStepFraction;
        position.y += 23.0f * entranceRemainingFraction;
        Hu3DModelPosSetV(object->mdlId[0], &position);
        object->work[2]--;
        if (object->work[2] == 0) {
            Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
            Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0], 0.0f,
                               4.0f, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(object->mdlId[0], 1.0f);
            object->work[0] = 0;
            object->work[1] = 0;
            object->work[2] = 0;
            return;
        }
        if (Hu3DMotionShiftIDGet(object->mdlId[0]) < 0 &&
            object->mtnId[3] != Hu3DMotionIDGet(object->mdlId[0])) {
            Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
            Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[3], 0.0f,
                               4.0f, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(object->mdlId[0], 1.0f);
            return;
        }
        if (Hu3DMotionShiftIDGet(object->mdlId[0]) < 0 &&
            object->mtnId[3] == Hu3DMotionIDGet(object->mdlId[0])) {
            if (Hu3DData[object->mdlId[0]].motWork.speed + Hu3DMotionTimeGet(object->mdlId[0]) >=
                Hu3DMotionMaxTimeGet(object->mdlId[0])) {
                Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
            }
        }
    } else if (object->work[3] != 0) {
        /* Once requested, the turn-away state keeps the actor facing forty degrees left. */
        Hu3DModelRotGet(object->mdlId[0], &position);
        position.y -= 1.0f;
        if (position.y <= -40.0f) {
            position.y = -40.0f;
        }
        Hu3DModelRotSetV(object->mdlId[0], &position);
        if (Hu3DMotionShiftIDGet(object->mdlId[0]) < 0 &&
            object->mtnId[5] != Hu3DMotionIDGet(object->mdlId[0])) {
            Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[5], 0.0f,
                               6.0f, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(object->mdlId[0], 1.0f);
        }
    } else if (object->work[1] != 0) {
        if (object->work[1] == 1) {
            if (Hu3DMotionShiftIDGet(object->mdlId[0]) < 0 &&
                object->mtnId[2] != Hu3DMotionIDGet(object->mdlId[0])) {
                Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[2], 0.0f,
                                   8.0f, HU3D_MOTATTR_LOOP);
                Hu3DMotionSpeedSet(object->mdlId[0], 1.0f);
                object->work[0] = 0;
            }
            if (Hu3DMotionShiftIDGet(object->mdlId[0]) < 0 &&
                object->mtnId[2] == Hu3DMotionIDGet(object->mdlId[0]) &&
                (Hu3DMotionTimeGet(object->mdlId[0]) == 6.0f ||
                 Hu3DMotionTimeGet(object->mdlId[0]) == 22.0f ||
                 Hu3DMotionTimeGet(object->mdlId[0]) == 38.0f ||
                 Hu3DMotionTimeGet(object->mdlId[0]) == 54.0f ||
                 Hu3DMotionTimeGet(object->mdlId[0]) == 70.0f)) {
                HuAudFXPlay(MSM_SE_GUIDE_16);
            }
        } else if (Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) {
            Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
            Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0], 0.0f,
                               8.0f, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(object->mdlId[0], 1.0f);
            object->work[1] = 0;
            object->work[0] = 0;
        }
    } else if (object->work[0] != 0) {
        if (Hu3DMotionShiftIDGet(object->mdlId[0]) < 0 &&
            object->mtnId[1] != Hu3DMotionIDGet(object->mdlId[0])) {
            Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[1], 0.0f,
                               8.0f, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(object->mdlId[0], 2.0f);
            return;
        }
        if (Hu3DMotionShiftIDGet(object->mdlId[0]) < 0 &&
            object->mtnId[1] == Hu3DMotionIDGet(object->mdlId[0])) {
            if (Hu3DData[object->mdlId[0]].motWork.speed + Hu3DMotionTimeGet(object->mdlId[0]) >=
                Hu3DMotionMaxTimeGet(object->mdlId[0])) {
                Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0], 0.0f,
                                   8.0f, HU3D_MOTATTR_LOOP);
                Hu3DMotionSpeedSet(object->mdlId[0], 1.0f);
                object->work[0] = 0;
            }
        }
    }
}

s32 memset(void *, s32, s32);
void fn_1_7A94(OMOBJ *obj);
s32 lbl_1_data_374[10] = {
    DATANUM(DATA_mgmdecathlon, 35), DATANUM(DATA_mgmdecathlon, 36),
    DATANUM(DATA_mgmdecathlon, 37), DATANUM(DATA_mgmdecathlon, 38),
    DATANUM(DATA_mgmdecathlon, 39), DATANUM(DATA_mgmdecathlon, 40),
    DATANUM(DATA_mgmdecathlon, 41), DATANUM(DATA_mgmdecathlon, 42),
    DATANUM(DATA_mgmdecathlon, 43), DATANUM(DATA_mgmdecathlon, 44)
};
Point3d lbl_1_data_39C = {-60.0f, -110.0f, 310.0f};

typedef struct MgmDecathlonAnimationData_s {
    ANIMDATA *animations[10]; /* one symbol animation per Decathlon event */
    s16 symbolAnimationId; /* active symbol-animation handle; -1 before the first entrance */
} MgmDecathlonAnimationData;

extern u8 MgModePlayNum;
extern s16 fn_1_9784(s16 playerNo);

/* Mode setup prepares the hidden announcement actor and the symbols for all ten events. */
void fn_1_75C4(void) {
    s32 animationIndex;
    MgmDecathlonAnimationData *animationData;

    lbl_1_bss_9C = omAddObjEx(lbl_1_bss_C0, 5120, 2U, 1U, -1, fn_1_7A94);
    lbl_1_bss_9C->stat = lbl_1_bss_9C->stat | OM_STAT_MODELPAUSE;
    lbl_1_bss_9C->work[0] = 0;
    lbl_1_bss_9C->data = HuMemDirectMallocNum(HEAP_HEAP, 44, HU_MEMNUM_OVL);
    memset(lbl_1_bss_9C->data, 0, 44);
    animationData = lbl_1_bss_9C->data;
    animationData->symbolAnimationId = -1;
    animationIndex = 0;
    while (animationIndex < 10) {
        animationData->animations[animationIndex] =
            HuSprAnimRead(HuDataSelHeapReadNum(
                lbl_1_data_374[animationIndex],
                HU_MEMNUM_OVL, HEAP_MODEL));
        animationIndex += 1;
    }
    *lbl_1_bss_9C->mdlId = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mgmdecathlon, 53), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(*lbl_1_bss_9C->mdlId, 1U);
    Hu3DModelLayerSet(*lbl_1_bss_9C->mdlId, 1);
    Hu3DModelPosSetV(*lbl_1_bss_9C->mdlId, &lbl_1_data_39C);
    lbl_1_bss_9C->mdlId[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mgmdecathlon, 3), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_9C->mdlId[1], 1U);
    Hu3DModelLayerSet(lbl_1_bss_9C->mdlId[1], 1);
    Hu3DModelHookSet(*lbl_1_bss_9C->mdlId, "f-itemhook_L", lbl_1_bss_9C->mdlId[1]);
    *lbl_1_bss_9C->mtnId = Hu3DJointMotion(*lbl_1_bss_9C->mdlId,
                                           HuDataSelHeapReadNum(
                                               DATANUM(DATA_mgmdecathlon, 54),
                                               HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(*lbl_1_bss_9C->mdlId, *lbl_1_bss_9C->mtnId);
    Hu3DModelAttrSet(*lbl_1_bss_9C->mdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(*lbl_1_bss_9C->mdlId,
                     HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                     HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
    Hu3DModelAttrSet(*lbl_1_bss_9C->mdlId, HU3D_ATTR_DISPOFF);
}

/* The next-event introduction selects its symbol, reveals the actor, and cues human players. */
void fn_1_78DC(void) {
    s16 playerNo;
    MgmDecathlonAnimationData *symbolData;

    if ((MgModePlayNum < 10) && (lbl_1_bss_9C->work[0] == 0)) {
        symbolData = lbl_1_bss_9C->data;
        if (symbolData->symbolAnimationId >= 0) {
            Hu3DAnimKill(symbolData->symbolAnimationId);
        }
        symbolData->symbolAnimationId = Hu3DAnimCreate(symbolData->animations[MgModePlayNum],
                                                       lbl_1_bss_9C->mdlId[1], "symbol01");
        Hu3DModelPosSetV(lbl_1_bss_9C->mdlId[0], &lbl_1_data_39C);
        Hu3DModelAttrReset(lbl_1_bss_9C->mdlId[0],
                           HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                           HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
        Hu3DModelAttrReset(lbl_1_bss_9C->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DMotionTimeSet(lbl_1_bss_9C->mdlId[0], 0.0f);
        lbl_1_bss_9C->work[0] = 1;
        HuAudFXPlay(MSM_SE_DECA_EVENT_REVEAL);
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (fn_1_9784(playerNo) == 0) {
                omVibrate(playerNo, 20, 4, 4);
            }
        }
    }
}

/* The main sequence polls this until the event-announcement actor finishes its entrance. */
s32 fn_1_7A6C(void) {
    if (lbl_1_bss_9C->work[0] == 2) {
        return 1;
    }
    return 0;
}

/* The announcement actor's frame callback raises it into view, then pauses its motion layers
 * when the next main-motion step would reach or pass the end. */
void fn_1_7A94(OMOBJ *object) {
    HuVecF position;
    f32 motionMaxTime;

    switch (object->work[0]) {
    case 0:
        Hu3DModelAttrSet(object->mdlId[0], HU3D_ATTR_DISPOFF);
        break;
    case 1:
        if (Hu3DMotionTimeGet(object->mdlId[0]) < 15.0f) {
            Hu3DModelPosGet(object->mdlId[0], &position);
            position.y += 6.6666665f;
            Hu3DModelPosSetV(object->mdlId[0], &position);
        }
        motionMaxTime = Hu3DMotionMaxTimeGet(object->mdlId[0]);
        if ((Hu3DData[object->mdlId[0]].motWork.speed +
             Hu3DMotionTimeGet(object->mdlId[0])) >= motionMaxTime) {
            object->work[0] = 2;
            Hu3DModelAttrSet(object->mdlId[0],
                             HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                             HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
            HuAudFXPlay(MSM_SE_DECA_EVENT_READY);
        }
        break;
    case 2:
        Hu3DModelAttrReset(object->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(object->mdlId[0],
                         HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                         HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
        break;
    }
}

s32 lbl_1_data_3C0[8] = {
    DATANUM(DATA_mgmdecathlon, 55), DATANUM(DATA_mgmdecathlon, 58),
    DATANUM(DATA_mgmdecathlon, 61), DATANUM(DATA_mgmdecathlon, 64),
    DATANUM(DATA_mgmdecathlon, 74), DATANUM(DATA_mgmdecathlon, 67),
    DATANUM(DATA_mgmdecathlon, 80), DATANUM(DATA_mgmdecathlon, 77)
};
s32 lbl_1_data_3E0[16] = {
    DATANUM(DATA_mgmdecathlon, 56), DATANUM(DATA_mgmdecathlon, 57),
    DATANUM(DATA_mgmdecathlon, 59), DATANUM(DATA_mgmdecathlon, 60),
    DATANUM(DATA_mgmdecathlon, 62), DATANUM(DATA_mgmdecathlon, 63),
    DATANUM(DATA_mgmdecathlon, 65), DATANUM(DATA_mgmdecathlon, 66),
    DATANUM(DATA_mgmdecathlon, 75), DATANUM(DATA_mgmdecathlon, 76),
    DATANUM(DATA_mgmdecathlon, 68), DATANUM(DATA_mgmdecathlon, 69),
    DATANUM(DATA_mgmdecathlon, 81), DATANUM(DATA_mgmdecathlon, 82),
    DATANUM(DATA_mgmdecathlon, 78), DATANUM(DATA_mgmdecathlon, 79)
};
Vec lbl_1_data_420[8] = { { 500.0f, 0.0f, -600.0f },   { -500.0f, 0.0f, -600.0f },
                          { 600.0f, 0.0f, -600.0f },   { -630.0f, 0.0f, -600.0f },
                          { 550.0f, 0.0f, -900.0f },   { -520.0f, 0.0f, -650.0f },
                          { 500.0f, 300.0f, -600.0f }, { -520.0f, 400.0f, -800.0f } };
extern void fn_1_8818(OMOBJ *object);
extern void fn_1_8708(void);

/* Mode setup creates the spectators and shows the arrangement for the current event count. */
void fn_1_7BF8(void) {
    OMOBJ *object;
    s16 modelIndex;
    s16 motionIndex;
    Vec position;

    object = omAddObjEx(lbl_1_bss_C0, 4288, 8, 16, -1,
                        fn_1_8818);
    lbl_1_bss_90 = object;
    object->stat |= OM_STAT_MODELPAUSE;
    object->data = NULL;
    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        object->work[modelIndex] = 0;
    }
    for (modelIndex = 0; modelIndex < 8; modelIndex++) {
        object->mdlId[modelIndex] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3C0[modelIndex],
                                                 HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(object->mdlId[modelIndex], 1);
        Hu3DModelLayerSet(object->mdlId[modelIndex], 1);
        Hu3DModelPosSetV(object->mdlId[modelIndex], &lbl_1_data_420[modelIndex]);
        for (motionIndex = 0; motionIndex < 2; motionIndex++) {
            object->mtnId[(modelIndex * 2) + motionIndex] =
                Hu3DJointMotion(object->mdlId[modelIndex],
                    HuDataSelHeapReadNum(lbl_1_data_3E0[(modelIndex * 2) + motionIndex],
                                         HU_MEMNUM_OVL, HEAP_MODEL));
        }
        Hu3DMotionSet(object->mdlId[modelIndex], object->mtnId[modelIndex * 2]);
        Hu3DModelAttrSet(object->mdlId[modelIndex], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(object->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
    }

    lbl_1_bss_98 = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_mgmdecathlon, 71),
                             HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_98, 1);
    Hu3DModelLayerSet(lbl_1_bss_98, 1);
    lbl_1_bss_96 = Hu3DJointMotion(
        lbl_1_bss_98, HuDataSelHeapReadNum(DATANUM(DATA_mgmdecathlon, 73),
                                          HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(lbl_1_bss_98, lbl_1_bss_96);
    Hu3DModelAttrSet(lbl_1_bss_98, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_98, HU3D_ATTR_DISPOFF);
    Hu3DModelScaleSet(object->mdlId[4], 1.2f,
                      1.2f, 1.2f);
    Hu3DModelScaleSet(lbl_1_bss_98, 1.2f,
                      1.2f, 1.2f);
    lbl_1_bss_94 = Hu3DJointMotion(
        object->mdlId[5], HuDataSelHeapReadNum(DATANUM(DATA_mgmdecathlon, 70),
                                               HU_MEMNUM_OVL, HEAP_MODEL));

    switch (MgModePlayNum) {
    case 0:
        Hu3DModelAttrReset(object->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[0], &lbl_1_data_420[1]);
        break;
    case 1:
        Hu3DModelAttrReset(object->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[0], &lbl_1_data_420[1]);
        Hu3DModelAttrReset(object->mdlId[1], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[1], &lbl_1_data_420[0]);
        break;
    case 2:
        Hu3DModelAttrReset(object->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[0], &lbl_1_data_420[1]);
        Hu3DModelAttrReset(object->mdlId[1], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[1], &lbl_1_data_420[0]);
        Hu3DModelAttrReset(object->mdlId[7], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[7], &lbl_1_data_420[6]);
        break;
    case 3:
        Hu3DModelAttrReset(object->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[0], &lbl_1_data_420[1]);
        Hu3DModelAttrReset(object->mdlId[1], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[1], &lbl_1_data_420[0]);
        Hu3DModelAttrReset(object->mdlId[7], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[7], &lbl_1_data_420[6]);
        Hu3DModelAttrReset(object->mdlId[6], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[6], &lbl_1_data_420[7]);
        break;
    case 4:
        Hu3DModelAttrReset(object->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[0], &lbl_1_data_420[1]);
        Hu3DModelAttrReset(object->mdlId[1], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[1], &lbl_1_data_420[0]);
        Hu3DModelAttrReset(object->mdlId[7], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[7], &lbl_1_data_420[6]);
        Hu3DModelAttrReset(object->mdlId[6], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[6], &lbl_1_data_420[7]);
        Hu3DModelAttrReset(object->mdlId[5], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[5], &lbl_1_data_420[4]);
        break;
    case 5:
        Hu3DModelAttrReset(object->mdlId[2], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[2], &lbl_1_data_420[1]);
        Hu3DModelAttrReset(object->mdlId[1], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[1], &lbl_1_data_420[0]);
        Hu3DModelAttrReset(object->mdlId[7], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[7], &lbl_1_data_420[6]);
        Hu3DModelAttrReset(object->mdlId[6], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[6], &lbl_1_data_420[7]);
        Hu3DModelAttrReset(object->mdlId[5], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[5], &lbl_1_data_420[4]);
        break;
    case 6:
        Hu3DModelAttrReset(object->mdlId[2], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[2], &lbl_1_data_420[1]);
        Hu3DModelAttrReset(object->mdlId[3], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[3], &lbl_1_data_420[0]);
        Hu3DModelAttrReset(object->mdlId[7], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[7], &lbl_1_data_420[6]);
        Hu3DModelAttrReset(object->mdlId[6], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[6], &lbl_1_data_420[7]);
        Hu3DModelAttrReset(object->mdlId[5], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[5], &lbl_1_data_420[4]);
        break;
    case 7:
        object->work[1] = 1;
        Hu3DModelAttrReset(object->mdlId[2], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[2], &lbl_1_data_420[1]);
        Hu3DModelAttrReset(object->mdlId[3], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[3], &lbl_1_data_420[0]);
        Hu3DModelAttrReset(object->mdlId[7], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[7], &lbl_1_data_420[6]);
        Hu3DModelAttrReset(object->mdlId[6], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[6], &lbl_1_data_420[7]);
        Hu3DModelAttrReset(object->mdlId[5], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[5], &lbl_1_data_420[4]);
        break;
    case 8:
        Hu3DModelAttrReset(object->mdlId[2], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[2], &lbl_1_data_420[1]);
        Hu3DModelAttrReset(object->mdlId[3], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[3], &lbl_1_data_420[0]);
        Hu3DModelAttrReset(object->mdlId[7], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[7], &lbl_1_data_420[6]);
        Hu3DModelAttrReset(object->mdlId[6], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[6], &lbl_1_data_420[7]);
        Hu3DModelAttrReset(object->mdlId[5], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[5], &lbl_1_data_420[4]);
        Hu3DModelAttrReset(object->mdlId[4], HU3D_ATTR_DISPOFF);
        position = lbl_1_data_420[5];
        position.y += 100.0f;
        Hu3DModelPosSetV(object->mdlId[4], &position);
        break;
    case 9:
    default:
        Hu3DModelAttrReset(object->mdlId[2], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[2], &lbl_1_data_420[1]);
        Hu3DModelAttrReset(object->mdlId[3], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[3], &lbl_1_data_420[0]);
        Hu3DModelAttrReset(object->mdlId[7], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[7], &lbl_1_data_420[6]);
        Hu3DModelAttrReset(object->mdlId[6], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[6], &lbl_1_data_420[7]);
        Hu3DModelAttrReset(object->mdlId[5], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[5], &lbl_1_data_420[4]);
        Hu3DModelAttrReset(object->mdlId[4], HU3D_ATTR_DISPOFF);
        position = lbl_1_data_420[5];
        position.y += 100.0f;
        Hu3DModelPosSetV(object->mdlId[4], &position);
        Hu3DModelAttrReset(object->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[0], &lbl_1_data_420[2]);
        Hu3DModelAttrReset(object->mdlId[1], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->mdlId[1], &lbl_1_data_420[3]);
        break;
    }
    fn_1_8708();
}

/* Scene transitions start the spectator cue if its model is visible and no cue handle is active. */
void fn_1_8708(void) {
    if (((Hu3DModelAttrGet(lbl_1_bss_90->mdlId[6]) & HU3D_ATTR_DISPOFF) == 0) &&
        (lbl_1_data_C90 < 0)) {
        lbl_1_data_C90 = HuAudFXPlayVolPan(MSM_SE_GUIDE_39, 100, 32);
    }
}

/* Result sequences request spectator reactions, or ask the crowd to blend back to idle. */
void fn_1_8778(u16 enabled) {
    if ((lbl_1_bss_90->work[0] == 0) && (enabled != 0)) {
        lbl_1_bss_90->work[0] = 1U;
    }
    if ((lbl_1_bss_90->work[0] != 0) && (enabled == 0)) {
        lbl_1_bss_90->work[0] = 2U;
    }
}

/* Before the eighth event launches, the sequence asks one spectator to turn and walk away. */
void fn_1_87EC(void) {
    if (MgModePlayNum == 7) {
        lbl_1_bss_90->work[2] = 1;
    }
}

extern void fn_1_9E50(Vec *center);
extern s32 fn_1_FB2C(Vec *direction, Vec *unitDirection);
extern f64 fn_1_FD44(Vec first, Vec second);

/* Accepts the spectator audio status without changing the scene. */
static inline void DebugValue(u32 audioStatusBits) { }

/* The spectator frame callback faces the player lineup and advances reactions and walking
 * motions. */
void fn_1_8818(OMOBJ *object) {
    Vec center;
    Vec modelPosition;
    Vec forwardDirection;
    f32 angle;
    s16 index;
    s16 anyMotionActive;

    fn_1_9E50(&center);
    forwardDirection.x = forwardDirection.y = 0.0f;
    forwardDirection.z = 1.0f;

    for (index = 0; index < 8; index++) {
        if ((index != 5) || (object->work[2] == 0)) {
            Hu3DModelPosGet(object->mdlId[index], &modelPosition);
            PSVECSubtract(&center, &modelPosition, &modelPosition);
            modelPosition.y = 0.0f;
            if (fn_1_FB2C(&modelPosition, &modelPosition) == 0) {
                modelPosition.x = modelPosition.y = 0.0f;
                modelPosition.z = 1.0f;
            }
            angle = (f32)fn_1_FD44(forwardDirection, modelPosition);
            if (modelPosition.x >= 0.0f) {
                modelPosition.x = modelPosition.z = 0.0f;
                modelPosition.y = 0.6f * angle;
            } else {
                modelPosition.x = modelPosition.z = 0.0f;
                modelPosition.y = -0.6f * angle;
            }
            Hu3DModelRotSetV(object->mdlId[index], &modelPosition);
        }
    }

    if ((u32)object->work[0] == 1) {
        for (index = 0; index < 8; index++) {
            if (((index != 5) || (object->work[2] == 0)) &&
                (Hu3DMotionShiftIDGet(object->mdlId[index]) < 0) &&
                (object->mtnId[(index * 2) + 1] != Hu3DMotionIDGet(object->mdlId[index]))) {
                Hu3DMotionShiftSet(object->mdlId[index], object->mtnId[(index * 2) + 1],
                                   0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                if ((Hu3DModelAttrGet(object->mdlId[index]) & HU3D_ATTR_DISPOFF) == 0) {
                    if (index == 0) {
                        HuAudFXPlayPan(MSM_SE_GUIDE_57, 32);
                    }
                    if (index == 1) {
                        HuAudFXPlayPan(MSM_SE_GUIDE_36, 96);
                    }
                    if (index == 2) {
                        HuAudFXPlayPan(MSM_SE_GUIDE_05, 32);
                    }
                    if (index == 3) {
                        HuAudFXPlayPan(MSM_SE_GUIDE_62, 96);
                    }
                    if (index == 4) {
                        HuAudFXPlayPan(MSM_SE_GUIDE_32, 32);
                    }
                    if (index == 7) {
                        HuAudFXPlayPan(MSM_SE_GUIDE_41, 96);
                    }
                }
            }
        }
        if (MgModePlayNum > 9) {
            DebugValue((u32)lbl_1_data_C8C >> 31);
        }
    } else if ((u32)object->work[0] != 0) {
        index = 0;
        anyMotionActive = 0;
        while (index < 8) {
            if ((index != 5) || (object->work[2] == 0)) {
                if ((Hu3DMotionShiftIDGet(object->mdlId[index]) < 0) &&
                    (object->mtnId[index * 2] != Hu3DMotionIDGet(object->mdlId[index]))) {
                    Hu3DMotionShiftSet(object->mdlId[index], object->mtnId[index * 2],
                                       0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                }
                if (object->mtnId[index * 2] != Hu3DMotionIDGet(object->mdlId[index])) {
                    anyMotionActive = 1;
                }
            }
            index++;
        }
        if (anyMotionActive == 0) {
            object->work[0] = 0;
        }
        if ((s32)lbl_1_data_C8C >= 0) {
            HuAudFXStop((s32)lbl_1_data_C8C);
        }
        lbl_1_data_C8C = DECATHLON_NO_AUDIO_HANDLE;
    }

    if (object->work[1] == 1) {
        /* This extra spectator paces between the left and right edges of the scene. */
        Hu3DModelAttrReset(lbl_1_bss_98, HU3D_ATTR_DISPOFF);
        Hu3DModelRotSet(lbl_1_bss_98, 0.0f, 90.0f, 0.0f);
        Hu3DModelPosSet(lbl_1_bss_98, -700.0f, 0.0f, -350.0f);
        object->work[1] = 2;
    } else if (object->work[1] == 2) {
        Hu3DModelAttrReset(lbl_1_bss_98, HU3D_ATTR_DISPOFF);
        Hu3DModelRotSet(lbl_1_bss_98, 0.0f, 90.0f, 0.0f);
        Hu3DModelPosGet(lbl_1_bss_98, &modelPosition);
        modelPosition.x += 1.5f;
        if (modelPosition.x > 700.0f) {
            object->work[1] = 3;
        }
        Hu3DModelPosSetV(lbl_1_bss_98, &modelPosition);
    } else if (object->work[1] == 3) {
        Hu3DModelAttrReset(lbl_1_bss_98, HU3D_ATTR_DISPOFF);
        Hu3DModelRotSet(lbl_1_bss_98, 0.0f, 270.0f, 0.0f);
        Hu3DModelPosSet(lbl_1_bss_98, 700.0f, 0.0f, -350.0f);
        object->work[1] = 4;
    } else if (object->work[1] == 4) {
        Hu3DModelAttrReset(lbl_1_bss_98, HU3D_ATTR_DISPOFF);
        Hu3DModelRotSet(lbl_1_bss_98, 0.0f, 270.0f, 0.0f);
        Hu3DModelPosGet(lbl_1_bss_98, &modelPosition);
        modelPosition.x -= 1.5f;
        if (modelPosition.x < -700.0f) {
            object->work[1] = 1;
        }
        Hu3DModelPosSetV(lbl_1_bss_98, &modelPosition);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_98, HU3D_ATTR_DISPOFF);
        object->work[1] = 0;
    }

    if (object->work[2] == 1) {
        Hu3DMotionShiftSet(object->mdlId[5], lbl_1_bss_94,
                           0.0f, 4.0f, HU3D_MOTATTR_LOOP);
        HuAudFXPlayPan(MSM_SE_GUIDE_77, 96);
        object->work[2] = 2;
        return;
    }
    if (object->work[2] != 0) {
        Hu3DModelRotGet(object->mdlId[5], &modelPosition);
        modelPosition.y -= 15.0f;
        if (modelPosition.y <= -90.0f) {
            modelPosition.y = -90.0f;
        }
        Hu3DModelRotSetV(object->mdlId[5], &modelPosition);
        if (modelPosition.y <= -85.0f) {
            Hu3DModelPosGet(object->mdlId[5], &modelPosition);
            modelPosition.x -= 6.0f;
            Hu3DModelPosSetV(object->mdlId[5], &modelPosition);
        }
    }
}

OMOBJ *lbl_1_bss_A0;
OMOBJ *lbl_1_bss_9C;
s16 lbl_1_bss_98;
s16 lbl_1_bss_96;
s16 lbl_1_bss_94;
OMOBJ *lbl_1_bss_90;
