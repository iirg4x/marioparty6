/* Stage actor setup, event sequencing, player selection, and model animation. */
#include "REL/mikeactdll/stage_collision.h"
#include "REL/mikeactdll/sequence_context.h"
#include "REL/mikeactdll/position.h"
#include <stdlib.h>

#include "math.h"

#include "string.h"

#include "REL/mikeactdll/stage_types.h"

#include "game/audio.h"

#include "REL/mikeactdll/event_callback.h"

#include "game/object.h"

#include "game/mg/seqman.h"

#include "game/mg/actman.h"

#include "game/sprite.h"

#include "include/game/gamework.h"

#include "REL/mikeactdll/module_types.h"

#include "game/hu3d.h"

#include "datanum/effect.h"

#include "game/process.h"

#include "game/charman.h"

#include "messdir_enum.h"

#include "game/mic.h"

#include "msm_se.h"

#include "game/main.h"

#include "game/gamework.h"
#include "REL/mikeactdll/microphone_init.h"
#include "REL/mikeactdll/module.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

int fn_1_1632C(COL_NARROW_PARAM *, COL_NARROW_PARAM *);
int fn_1_16334(COL_NARROW_PARAM *, COL_NARROW_PARAM *);
int fn_1_16624(COL_NARROW_PARAM *, COL_NARROW_PARAM *);
int fn_1_1662C(COL_NARROW_PARAM *, COL_NARROW_PARAM *);
Point3d lbl_1_data_1100 = { 0.0f, 5000.0f, 50000.0f };
Point3d lbl_1_data_110C = { 0.1f, -1.0f, -0.8f };
GXColor lbl_1_data_1118 = { 96, 96, 120, 255 };
Point3d lbl_1_data_111C[13] = { { 5300.0f, 0.0f, 0.0f }, { 7000.0f, 0.0f, 300.0f }, { 9000.0f, 0.0f, 0.0f }, { 15900.0f, 0.0f, -300.0f }, { 36300.0f, 0.0f, 300.0f }, { 37500.0f, 0.0f, 0.0f }, { 42200.0f, 0.0f, -300.0f }, { 42200.0f, 0.0f, 0.0f }, { 43400.0f, 0.0f, 0.0f }, { 42200.0f, 0.0f, 300.0f }, { 44700.0f, 0.0f, -300.0f }, { 45800.0f, 0.0f, 0.0f }, { 47300.0f, 0.0f, 300.0f } };
Point3d lbl_1_data_11B8[1] = { { 24000.0f, 0.0f, 0.0f } };
Point3d lbl_1_data_11C4[12] = { { 3200.0f, 0.0f, 300.0f }, { 9900.0f, 0.0f, -300.0f }, { 14700.0f, 0.0f, 300.0f }, { 17200.0f, 0.0f, -300.0f }, { 18200.0f, 0.0f, 300.0f }, { 30200.0f, 0.0f, -300.0f }, { 37300.0f, 0.0f, 300.0f }, { 36300.0f, 0.0f, 0.0f }, { 43400.0f, 0.0f, -300.0f }, { 43400.0f, 0.0f, 300.0f }, { 45800.0f, 0.0f, -300.0f }, { 48300.0f, 0.0f, 300.0f } };
Point3d lbl_1_data_1254[6] = { { 9900.0f, 0.0f, 300.0f }, { 16600.0f, 0.0f, 300.0f }, { 28400.0f, 0.0f, 0.0f }, { 31800.0f, 0.0f, 0.0f }, { 32100.0f, 0.0f, 0.0f }, { 46100.0f, 0.0f, 300.0f } };
MGACTOR_PARAM lbl_1_data_129C = { 2000.0f, 149.9f, 12, 5, fn_1_16624, 32, 0, 0 };
Point3d lbl_1_data_12BC[9] = { { 3200.0f, 0.0f, -300.0f }, { 8100.0f, 0.0f, 0.0f }, { 27600.0f, 0.0f, 0.0f }, { 27600.0f, 0.0f, 300.0f }, { 29800.0f, 0.0f, 0.0f }, { 30700.0f, 0.0f, 0.0f }, { 31000.0f, 0.0f, 300.0f }, { 38500.0f, 0.0f, -300.0f }, { 47000.0f, 0.0f, -300.0f } };
MGACTOR_PARAM lbl_1_data_1328 = { 100.0f, 149.9f, 13, 5, fn_1_16334, 32, 0, 0 };
MikeActSceneLayoutEntry lbl_1_data_1348[3][25] = { { { 2, 20, 0, 0 }, { 2, 30, 1, 0 }, { 2, 10, 2, 0 }, { -1, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }, { { 5, 33, 0, 0 }, { 5, 28, 1, 0 }, { 5, 33, 1, 0 }, { 5, 38, 1, 0 }, { 5, 28, 2, 0 }, { 5, 33, 0, 1 }, { 5, 36, 0, 1 }, { 5, 39, 0, 1 }, { 5, 28, 3, 1 }, { 5, 31, 3, 1 }, { 5, 34, 3, 1 }, { 5, 37, 3, 1 }, { 6, 13, 1, 0 }, { 6, 13, 2, 0 }, { 6, 20, 0, 0 }, { 6, 20, 1, 0 }, { 6, 27, 1, 0 }, { 6, 27, 2, 0 }, { 7, 3, 1, 0 }, { 7, 3, 1, 1 }, { 7, 6, 1, 1 }, { 7, 3, 2, 1 }, { 7, 6, 2, 1 }, { 8, 46, 2, 0 }, { -1, 0, 0, 0 } }, { { 9, 35, 0, 1 }, { 9, 38, 0, 1 }, { 9, 41, 0, 1 }, { 9, 35, 1, 1 }, { 9, 38, 1, 1 }, { 9, 41, 1, 1 }, { 9, 35, 2, 1 }, { 9, 38, 2, 1 }, { 9, 41, 2, 1 }, { 9, 35, 3, 1 }, { 9, 38, 3, 1 }, { 9, 41, 3, 1 }, { 10, 10, 1, 0 }, { 10, 31, 1, 0 }, { -1, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } } };
MGACTOR_PARAM lbl_1_data_17F8 = { 100.0f, 149.9f, 14, 5, fn_1_1632C, 32, 0, 0 };
MGACTOR_PARAM lbl_1_data_1818 = { 25.0f, 25.0f, 4, 5, fn_1_1662C, 34, 0, 0 };
s32 lbl_1_data_1838[2] = { 1, 1 };
f32 lbl_1_data_1840[2] = { 19500.0f, 34200.0f };
OM_CAMERA_VIEW lbl_1_data_1848 = { { 11740.0f, 353.5f, 148.0f }, { -30.0f, -3.0f, 0.0f }, 3544.0f };
Point3d lbl_1_data_1864[4] = { { 10900.0f, 0.0f, 0.0f }, { 10800.0f, 0.0f, -300.0f }, { 11000.0f, 0.0f, -300.0f }, { 11200.0f, 0.0f, -300.0f } };
Point3d lbl_1_data_1894[4] = { { 11200.0f, 0.0f, 0.0f }, { 11440.0f, 100.0f, -300.0f }, { 11440.0f, 100.0f, 0.0f }, { 11440.0f, 100.0f, 300.0f } };
OM_CAMERA_VIEW lbl_1_data_18C4 = { { 40101.0f, 352.0f, 120.0f }, { -21.0f, 0.0f, 0.0f }, 3670.0f };
Point3d lbl_1_data_18E0[4] = { { 39400.0f, 0.0f, 0.0f }, { 39260.0f, 0.0f, -254.0f }, { 39420.0f, 0.0f, -254.0f }, { 39580.0f, 0.0f, -254.0f } };
Point3d lbl_1_data_1910[4] = { { 39500.0f, 0.0f, 0.0f }, { 39560.0f, 0.0f, -254.0f }, { 39720.0f, 0.0f, -254.0f }, { 39880.0f, 0.0f, -254.0f } };
Point3d lbl_1_data_1940[6] = { { 15510.0f, 0.0f, -590.0f }, { 17410.0f, 0.0f, -590.0f }, { 20510.0f, 0.0f, -590.0f }, { 22410.0f, 0.0f, -590.0f }, { 25510.0f, 0.0f, -590.0f }, { 27410.0f, 0.0f, -590.0f } };
s32 lbl_1_data_1988 = -1;

s32 fn_1_15B9C(void *playerContext, void *eventTarget) { return 0; }

extern MikeActSceneEffectRow lbl_1_bss_363C[2][3];

extern s32 lbl_1_bss_3400;

extern s16 lbl_1_bss_35E4[3];

extern HUPROCESS *lbl_1_bss_6C;

extern void fn_1_16804(OMOBJ *object);

s32 fn_1_15BA4(MGACTOR *actor)
{
    Point3d groundPosition;
    Point3d effectPosition;
    Point3d stagePosition;
    Point3d soundPosition;
    MikeActPlayerRecord *playerRecord;
    OMOBJ *effectObject;
    OMOBJ *soundEffectObject;
    s16 stateIndex;
    s32 direction;
    s32 soundDirection;
    s32 soundId;
    s32 triggerIndex;

    if (actor->param >= 0xA) {
        return;
    }
    if (lbl_1_bss_C4[actor->param]->player_no != 0) {
        return;
    }

    playerRecord = lbl_1_bss_C4[actor->param];
    fn_80086BC8(actor, &groundPosition);
    if ((actor->colGroundAttr & 4) != 0) {
        if (lbl_1_data_1988 != 1) {
            fn_8005F77C(lbl_1_bss_C4[0]->character_no, 5);
            lbl_1_data_1988 = 1;
        }
    } else if (lbl_1_data_1988 != 0) {
        fn_8005F77C(lbl_1_bss_C4[0]->character_no, 0);
        lbl_1_data_1988 = 0;
    }

    if ((actor->colGroundAttr & 0x40) != 0) {
        for (triggerIndex = 0; triggerIndex < 2; triggerIndex++) {
            f32 triggerX = lbl_1_bss_363C[triggerIndex][0].position.x;
            if (triggerX - 200.0f < groundPosition.x &&
                groundPosition.x < 200.0f + triggerX &&
                lbl_1_bss_363C[triggerIndex][0].state == 0) {
                lbl_1_bss_363C[triggerIndex][0].state = 1;
            }
        }
    }

    if ((actor->colGroundAttr & 0x210) == 0) {
        return;
    }
    if (fn_1_45D8(playerRecord, 4) == 0) {
        return;
    }
    stateIndex = fn_1_1110C((s16)actor->colMesh);
    if (stateIndex == -1) {
        return;
    }
        if (lbl_1_bss_1B8[stateIndex].event_state == 3) {
        fn_1_10F98(stateIndex, 0);
        fn_80026A44(lbl_1_bss_1B8[stateIndex].model_id, 0x40000002U);
        fn_800267CC(lbl_1_bss_1B8[stateIndex].model_id, &groundPosition);
        fn_80086BC8(actor, &effectPosition);
        if (groundPosition.x - effectPosition.x > 0.0f) {
            direction = 0x32;
        } else {
            direction = -0x32;
        }
        effectPosition.x += (f32)direction;
        fn_1_16910(&effectPosition);
        HuAudFXPlay(0x552);
        effectObject = fn_8003AB38(lbl_1_bss_6C, 0x2300, 1, 1, -1,
                                  fn_1_16804);
        effectObject->work[0] = (u32)stateIndex;
        effectObject->work[3] = lbl_1_bss_1B8[stateIndex].event_state - 1;
        effectObject->mdlId[0] = fn_80025D4C(
            lbl_1_bss_35E4[effectObject->work[3]]);
        effectObject->trans.x = groundPosition.x;
        effectObject->trans.y = groundPosition.y;
        effectObject->trans.z = groundPosition.z;
        effectObject->rot.y = (f32)lbl_1_bss_1B8[stateIndex].callback_state;
        fn_80026A44(effectObject->mdlId[0], 1U);
        fn_80026A44(effectObject->mdlId[0], 0x40000002U);
        fn_80026B8C(effectObject->mdlId[0], 7);

        stagePosition.x = (f32)(((lbl_1_bss_3400 / 3) * 0x64) - 0x384);
        stagePosition.y = 0.0f;
        stagePosition.z = (f32)(((lbl_1_bss_3400 % 3) * 0x12C) - 0x12C);
        fn_80026790(lbl_1_bss_1B8[stateIndex].model_id, &stagePosition);
        fn_80026790(lbl_1_bss_1B8[stateIndex].linked_model_id,
                         &stagePosition);
        fn_1_10F68(stateIndex, &stagePosition);
        lbl_1_bss_3400++;
        return;
    }

    if (lbl_1_bss_1B8[stateIndex].event_state > 0) {
        fn_1_10FB4(stateIndex);
        fn_80026A44(lbl_1_bss_1B8[stateIndex].model_id, 0x40000002U);
        fn_800267CC(lbl_1_bss_1B8[stateIndex].model_id, &groundPosition);
        fn_80086BC8(actor, &soundPosition);
        if (groundPosition.x - soundPosition.x > 0.0f) {
            soundDirection = 0x32;
        } else {
            soundDirection = -0x32;
        }
        soundPosition.x += (f32)soundDirection;
        fn_1_16910(&soundPosition);
        if (lbl_1_bss_1B8[stateIndex].event_state == 1) {
            soundId = 0x551;
        } else {
            soundId = 0x546;
        }
        HuAudFXPlay(soundId);
        soundEffectObject = fn_8003AB38(lbl_1_bss_6C, 0x2300, 1, 1, -1,
                                       fn_1_16804);
        soundEffectObject->work[0] = (u32)stateIndex;
        soundEffectObject->work[3] =
            lbl_1_bss_1B8[stateIndex].event_state - 1;
        soundEffectObject->mdlId[0] = fn_80025D4C(
            lbl_1_bss_35E4[soundEffectObject->work[3]]);
        soundEffectObject->trans.x = groundPosition.x;
        soundEffectObject->trans.y = groundPosition.y;
        soundEffectObject->trans.z = groundPosition.z;
        soundEffectObject->rot.y =
            (f32)lbl_1_bss_1B8[stateIndex].callback_state;
        if (soundEffectObject->work[3] == 1 &&
            lbl_1_data_1254[3].x == groundPosition.x) {
            lbl_1_data_1838[0] = 0;
        }
        if (soundEffectObject->work[3] == 1 &&
            lbl_1_data_1254[4].x == groundPosition.x) {
            lbl_1_data_1838[1] = 0;
        }
        fn_80026A44(soundEffectObject->mdlId[0], 1U);
        fn_80026A44(soundEffectObject->mdlId[0], 0x40000002U);
        fn_1_10E64(stateIndex);
}
}

int fn_1_1632C(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b) { return 0; }

int fn_1_16334(COL_NARROW_PARAM *eventSource,
                COL_NARROW_PARAM *eventContext)
{
    s16 stateIndex;
    s32 direction;
    s32 soundId;
    HuVecF modelPosition;
    HuVecF effectPosition;
    OMOBJ *effectObject;

    if (eventContext->paramB >= 0 && eventContext->paramB < 4 &&
        fn_1_45D8(lbl_1_bss_C4[eventContext->paramB], 4) != 0) {
        stateIndex = fn_1_1106C(eventSource->paramA);
        if (stateIndex != -1) {
            fn_1_10FB4(stateIndex);
            fn_80026A44(lbl_1_bss_1B8[stateIndex].model_id, 0x40000002U);
            fn_800267CC(lbl_1_bss_1B8[stateIndex].model_id, &modelPosition);
            effectPosition = eventContext->point;
            if (modelPosition.x - effectPosition.x > 0.0f) {
                direction = 0x32;
            } else {
                direction = -0x32;
            }
            effectPosition.x += (f32)direction;
            fn_1_16910(&effectPosition);
            if (lbl_1_bss_1B8[stateIndex].event_state == 1) {
                soundId = 0x551;
            } else {
                soundId = 0x546;
            }
            HuAudFXPlay(soundId);

            effectObject = fn_8003AB38(lbl_1_bss_6C, 0x2300, 1, 1, -1,
                                      fn_1_16804);
            effectObject->work[0] = stateIndex;
            effectObject->work[3] = lbl_1_bss_1B8[stateIndex].event_state - 1;
            effectObject->mdlId[0] = fn_80025D4C(
                lbl_1_bss_35E4[effectObject->work[3]]);
            effectObject->trans.x = modelPosition.x;
            effectObject->trans.y = modelPosition.y;
            effectObject->trans.z = modelPosition.z;
            effectObject->rot.y =
                (f32)lbl_1_bss_1B8[stateIndex].callback_state;
            if (effectObject->work[3] == 1 &&
                lbl_1_data_1254[3].x == modelPosition.x) {
                lbl_1_data_1838[0] = 0;
            }
            if (effectObject->work[3] == 1 &&
                lbl_1_data_1254[4].x == modelPosition.x) {
                lbl_1_data_1838[1] = 0;
            }
            fn_80026A44(effectObject->mdlId[0], 1U);
            fn_80026A44(effectObject->mdlId[0], 0x40000002U);
            fn_1_10E64(stateIndex);
        }
    }
    return 0;
}

int fn_1_16624(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b) { return 0; }

int fn_1_1662C(COL_NARROW_PARAM *event, COL_NARROW_PARAM *eventState)
{
    s16 playerNo;

    if (eventState->paramB == 0) {
        playerNo = fn_1_1106C(event->paramA);
        if (lbl_1_bss_1B8[(s16) playerNo].event_state != 0) {
            return 0;
        }
        lbl_1_bss_1B8[(s16) playerNo].event_state = 1;
        if (fn_1_45D8(lbl_1_bss_C4[eventState->paramB], 4)) {
            fn_1_16910(&event->point);
        } else {
            fn_1_4A70(lbl_1_bss_C4[eventState->paramB]);
        }
    }
    return 0;
}

extern MikeActArenaObject lbl_1_bss_36FC[3];

void fn_1_16718(s32 index, f32 positionX)
{
    lbl_1_bss_36FC[index].position.x = positionX;
    lbl_1_bss_36FC[index].position.y = 35.0f;
    fn_80086B04(lbl_1_bss_36FC[index].actor, &lbl_1_bss_36FC[index].position);
    lbl_1_bss_36FC[index].state->event_state = 0;
    lbl_1_bss_36FC[index].state->callback_state = 0;
    fn_80026CBC(lbl_1_bss_36FC[index].linkedModelId, 0.0f);
}

void fn_1_186D0(HU3D_MODEL *model, HU3D_PARTICLE *manager, Mtx matrix);

void fn_1_18C54(void);

typedef struct {
    s16 modelId;
} MikeActEffectModel;

void fn_1_16804(OMOBJ *object)
{
    s16 modelId;
    s32 limit;

    fn_80026B8C(object->mdlId[0], 7);
    switch (object->work[1]) {
    case 0:
        object->work[2]++;
        if (object->work[3] == 2) {
            limit = 60;
        } else {
            limit = 50;
        }
        if (object->work[2] == (u32) limit) {
            object->work[1] = 1;
            object->work[2] = 0;
        }
        break;
    case 1:
        object->work[2]++;
        modelId = object->mdlId[0];
        if ((object->work[2] & 1) != 0) {
            fn_800269E8(modelId, (1 << 0));
        } else {
            fn_80026A44(modelId, (1 << 0));
        }
        if (object->work[2] == 30) {
            fn_800262D0(object->mdlId[0]);
            omDelObjEx(fn_8000DD9C(), object);
        }
        break;
    }
}

extern s16 lbl_1_bss_35EA;

void fn_1_16910(HuVecF *position)
{
    HU3D_MODELID model = fn_80036EBC(lbl_1_bss_35EA);
    fn_80026A44(model, (1 << 0));
    fn_80026A44(model, 0x40000002);
    fn_800269E8(model, (1 << 27));
    fn_80026B8C(model, 7);
    fn_80026790(model, position);
}

extern s32 lbl_1_bss_375C[25];

typedef struct {
    MikeActStateRecord *stateRecord;
    s16 primaryModelId;
    s16 secondaryModelId;
} MikeActScenePair;

extern s16 lbl_1_bss_3420;

extern s16 lbl_1_bss_3422;

extern s16 lbl_1_bss_35F6;

extern s16 lbl_1_bss_35F8;

extern s16 lbl_1_bss_35FA;

extern s16 lbl_1_bss_35FC;

extern s16 lbl_1_bss_35FE;

extern s16 lbl_1_bss_3600[3];

extern s16 lbl_1_bss_3606[3];

extern s16 lbl_1_bss_360C[6];

extern s16 lbl_1_bss_3618[5];

extern s16 lbl_1_bss_3622;

extern MikeActScenePair lbl_1_bss_3624[3];

void fn_1_1A230(void);

void fn_1_1698C(void)
{
    s16 firstModel;
    s16 secondModel;
    s32 slotIndex;
    s16 thirdModel;
    s16 linkedModelId;
    s32 index;
    s32 activeIndex;
    Point3d arenaPosition;
    for (index = 0; index < 13; index++) {
        lbl_1_data_111C[index].x += 150.0f;
    }
    for (index = 0; index < 1; index++) {
        lbl_1_data_11B8[index].x += 150.0f;
    }
    for (index = 0; index < 12; index++) {
        lbl_1_data_11C4[index].x += 250.0f;
    }
    for (index = 0; index < 6; index++) {
        lbl_1_data_1254[index].x += 150.0f;
    }
    for (index = 0; index < 9; index++) {
        lbl_1_data_12BC[index].x += 150.0f;
    }

    lbl_1_bss_35F6 = fn_800286D0(&lbl_1_data_1100, &lbl_1_data_110C,
                                        &lbl_1_data_1118);
    Hu3DGLightStaticSet(lbl_1_bss_35F6, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_35F6);
    fn_1_111A8(lbl_1_data_111C, 13);
    fn_1_11690(lbl_1_data_11B8, 1);

    for (index = 0; index < 12; index++) {
        if (index == 0) {
            firstModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 50), 0x10000000, 2));
            secondModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 51), 0x10000000, 2));
            linkedModelId = firstModel;
            thirdModel = secondModel;
        } else {
            linkedModelId = fn_80025D4C(firstModel);
            thirdModel = fn_80025D4C(secondModel);
        }
        fn_80026FFC(linkedModelId);
        fn_80026B8C(linkedModelId, 6);
        fn_800269E8(linkedModelId, 0x40000001U);
        fn_1_10AE0(linkedModelId, thirdModel,
                   &lbl_1_data_11C4[index]);
    }

    for (index = 0; index < 9; index++) {
        if (index == 0) {
            firstModel = fn_80025A04(fn_80008DD0((DATANUM(0xa70000, 7)), 0x10000000, 2));
            linkedModelId = firstModel;
        } else {
            linkedModelId = fn_80025D4C(firstModel);
        }
        fn_80026760(linkedModelId, lbl_1_data_12BC[index].x - 240.0f, 0.0f,
                        lbl_1_data_12BC[index].z);
        fn_1_107EC(linkedModelId);
    }

    {
    for (index = 0; index < 6; index++) {
        if (index == 0) {
            firstModel = fn_80025A04(fn_80008DD0((DATANUM(0xa70000, 58)), 0x10000000, 2));
            secondModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 60), 0x10000000, 2));
            linkedModelId = firstModel;
            thirdModel = secondModel;
        } else {
            linkedModelId = fn_80025D4C(firstModel);
            thirdModel = fn_80025D4C(secondModel);
        }
        fn_80026FFC(linkedModelId);
        fn_80026B8C(linkedModelId, 6);
        activeIndex = fn_1_10AE0(linkedModelId, thirdModel,
                                 &lbl_1_data_1254[index]);
        lbl_1_bss_1B8[activeIndex].event_state = 2;
    }
    }

    {
    for (index = 0, slotIndex = 0; index < 24; index++) {
        if (index == 0) {
            firstModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 52), 0x10000000, 2));
            secondModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 54), 0x10000000, 2));
            linkedModelId = firstModel;
            thirdModel = secondModel;
        } else {
            linkedModelId = fn_80025D4C(firstModel);
            thirdModel = fn_80025D4C(secondModel);
        }
        fn_80026B8C(linkedModelId, 6);
        if (slotIndex > 0 || lbl_1_data_1348[0][index].layoutId == -1) {
            arenaPosition.x = 59000.0f + (f32)((slotIndex / 3) * 100);
            arenaPosition.y = 0.0f;
            arenaPosition.z = (f32)(((slotIndex % 3) * 300) - 300);
            slotIndex++;
        } else {
            arenaPosition.x = (f32)(((lbl_1_data_1348[0][index].layoutId - 1) *
                                     5000) - 1000 +
                                    (lbl_1_data_1348[0][index].horizontalIndex * 100));
            arenaPosition.y = 0.0f;
            arenaPosition.z = (f32)((lbl_1_data_1348[0][index].depthIndex * 300) - 450);
            if (lbl_1_data_1348[0][index].rotationIndex == 0) {
                arenaPosition.z += 150.0f;
            } else {
                arenaPosition.x += 150.0f;
                fn_80026808(linkedModelId, 0.0f, 90.0f, 0.0f);
                fn_80026808(thirdModel, 0.0f, 90.0f, 0.0f);
            }
        }
        fn_80026790(linkedModelId, &arenaPosition);
        fn_80026790(thirdModel, &arenaPosition);
        activeIndex = fn_1_10AE0(linkedModelId, thirdModel, &arenaPosition);
        lbl_1_bss_1B8[activeIndex].event_state = 3;
        lbl_1_bss_1B8[activeIndex].callback_state =
            lbl_1_data_1348[0][index].rotationIndex * 0x5A;
        lbl_1_bss_375C[index] = activeIndex;
    }
    }

    {
        s32 modelId;
        s32 activeIndex;
        s16 auxiliaryBase;
    for (index = 0; index < 2; index++) {
        for (slotIndex = 0; slotIndex < 3; slotIndex++) {
            Point3d linkedPosition = {0.0f, 0.0f, 0.0f};
            if (index == 0 && slotIndex == 0) {
                firstModel = fn_80025A04(
                    fn_80008DD0(DATANUM(0xa70000, 61), 0x10000000, 2));
                auxiliaryBase = fn_80025A04(
                    fn_80008DD0(DATANUM(0xa70000, 63), 0x10000000, 2));
                secondModel = fn_80025A04(
                    fn_80008DD0(DATANUM(0xa70000, 62), 0x10000000, 2));
                linkedModelId = firstModel;
                modelId = auxiliaryBase;
                thirdModel = secondModel;
            } else {
                linkedModelId = fn_80025D4C(firstModel);
                modelId = fn_80025D4C(auxiliaryBase);
                thirdModel = fn_80025D4C(secondModel);
            }
            fn_80026B8C(linkedModelId, 6);
            fn_8002D730(linkedModelId, 0.0f, 60.0f);
            fn_800269E8(linkedModelId, 0x40000001U);
            fn_80026A44(linkedModelId, 0x40000002U);
            linkedPosition.x = lbl_1_data_1840[index] + 150.0f;
            linkedPosition.y = 0.0f;
            linkedPosition.z = (f32)((slotIndex * 300) - 300);
            activeIndex = fn_1_10AE0(linkedModelId, thirdModel, &linkedPosition);
            lbl_1_bss_363C[index][slotIndex].state = 0;
            lbl_1_bss_363C[index][slotIndex].flags = 0;
            lbl_1_bss_363C[index][slotIndex].position = linkedPosition;
            lbl_1_bss_363C[index][slotIndex].stateRecord =
                &lbl_1_bss_1B8[activeIndex];
            lbl_1_bss_363C[index][slotIndex].primaryModelId = linkedModelId;
            lbl_1_bss_363C[index][slotIndex].secondaryModelId = thirdModel;
            lbl_1_bss_363C[index][slotIndex].auxiliaryModelId = modelId;
            linkedPosition.x += 900.0f;
            fn_1_10CFC(modelId, &linkedPosition);
            fn_8002D730(modelId, 0.0f, 60.0f);
            fn_800269E8(modelId, 0x40000001U);
            fn_80026A44(modelId, 0x40000002U);
            fn_80026B8C(modelId, 6);
        }
    }
    }

    for (index = 0; index < 3; index++) {
        Point3d pairPosition = {-2000.0f, -1000.0f, 0.0f};
        if (index == 0) {
            firstModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 4), 0x10000000, 2));
            secondModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 6), 0x10000000, 2));
            linkedModelId = firstModel;
            thirdModel = secondModel;
        } else {
            linkedModelId = fn_80025D4C(firstModel);
            thirdModel = fn_80025D4C(secondModel);
        }
        lbl_1_bss_3624[index].primaryModelId = linkedModelId;
        lbl_1_bss_3624[index].secondaryModelId = thirdModel;
        fn_800269E8(linkedModelId, 0x40000001U);
        fn_80026B8C(linkedModelId, 6);
        activeIndex = fn_1_10AE0(linkedModelId, thirdModel, &pairPosition);
        lbl_1_bss_3624[index].stateRecord = &lbl_1_bss_1B8[activeIndex];
    }

    lbl_1_bss_3622 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 71), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_3622, 1U);
    fn_80026A44(lbl_1_bss_3622, 0x40000002U);
    fn_80026B8C(lbl_1_bss_3622, 6);
    lbl_1_bss_35EA = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 3), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_35EA, 1U);
    fn_800269E8(lbl_1_bss_35EA, 0x40000002U);
    fn_800269E8(lbl_1_bss_35EA, 0x400000U);

    for (index = 0; index < 3; index++) {
        s32 displayAssets[3] = {DATANUM(0xa70000, 56), DATANUM(0xa70000, 59), DATANUM(0xa70000, 53)};
        lbl_1_bss_35E4[index] = fn_80025A04(
            fn_80008DD0(displayAssets[index], 0x10000000, 2));
        fn_800269E8(lbl_1_bss_35E4[index], 1U);
        fn_800269E8(lbl_1_bss_35E4[index], 0x40000002U);
        fn_80026B8C(lbl_1_bss_35E4[index], 7);
    }

    for (index = 0; index < 5; index++) {
        Point3d model40Positions[5] = {
            {31940.0f, 190.0f, -550.0f},
            {34020.0f, 190.0f, -550.0f},
            {36020.0f, 190.0f, -550.0f},
            {38020.0f, 190.0f, -550.0f},
            {39055.0f, 190.0f, -550.0f}
        };
        if (index == 0) {
            firstModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 64), 0x10000000, 2));
            linkedModelId = firstModel;
        } else {
            linkedModelId = fn_80025D4C(firstModel);
        }
        fn_800269E8(linkedModelId, 0x40000001U);
        fn_80026B8C(linkedModelId, 6);
        fn_1_10CFC(linkedModelId, &model40Positions[index]);
        lbl_1_bss_3618[index] = linkedModelId;
    }

    for (index = 0; index < 6; index++) {
        if (index == 0) {
            firstModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 65), 0x10000000, 2));
            linkedModelId = firstModel;
        } else {
            linkedModelId = fn_80025D4C(firstModel);
        }
        fn_800269E8(linkedModelId, 0x40000002U);
        fn_800269E8(linkedModelId, 0x40000001U);
        fn_80026B8C(linkedModelId, 6);
        fn_1_10CFC(linkedModelId, &lbl_1_data_1940[index]);
        lbl_1_bss_360C[index] = linkedModelId;
    }

    for (index = 0; index < 3; index++) {
        if (index == 0) {
            linkedModelId = firstModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 72), 0x10000000, 2));
        } else {
            linkedModelId = fn_80025D4C(firstModel);
        }
        fn_800269E8(linkedModelId, 1U);
        fn_80026B8C(linkedModelId, 5);
        lbl_1_bss_3606[index] = linkedModelId;
    }

    for (index = 0; index < 3; index++) {
        Point3d sceneryPosition = lbl_1_data_1910[index + 1];
        if (index == 0) {
            firstModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 68), 0x10000000, 2));
            linkedModelId = firstModel;
        } else {
            linkedModelId = fn_80025D4C(firstModel);
        }
        fn_800269E8(linkedModelId, 0x40000002U);
        fn_80026B8C(linkedModelId, 5);
        sceneryPosition.y += 75.0f;
        sceneryPosition.z += -75.0f;
        fn_80026790(linkedModelId, &sceneryPosition);
        lbl_1_bss_3600[index] = linkedModelId;
    }

    for (index = 0; index < 4; index++) {
        Point3d model54Positions[4] = {
            {11590.0f, 525.0f, 220.0f},
            {39000.0f, 0.0f, 0.0f},
            {40400.0f, 0.0f, 0.0f},
            {40400.0f, 0.0f, 0.0f}
        };
        s32 modelAssets[4] = {DATANUM(0xa70000, 66), DATANUM(0xa70000, 67), DATANUM(0xa70000, 70), DATANUM(0xa70000, 69)};
        linkedModelId = fn_80025A04(fn_80008DD0(modelAssets[index], 0x10000000, 2));
        fn_800269E8(linkedModelId, 0x40000002U);
        fn_80026B8C(linkedModelId, 6);
        fn_1_10CFC(linkedModelId, &model54Positions[index]);
        if (index == 0) {
            lbl_1_bss_35FE = linkedModelId;
        } else if (index == 1) {
            lbl_1_bss_35FC = linkedModelId;
        } else if (index == 2) {
            lbl_1_bss_35FA = linkedModelId;
        } else if (index == 3) {
            lbl_1_bss_35F8 = linkedModelId;
        }
    }

    MikeActPreActorCallbackSet(fn_1_18C54);
    MikeActPostActorCallbackSet(fn_1_1A230);
    lbl_1_bss_3422 = 0;
    lbl_1_bss_3420 = 0;
}

extern Point3d lbl_1_bss_34E4[16];

extern s16 lbl_1_bss_3414;

extern Point3d lbl_1_bss_3424[16];

extern s16 lbl_1_bss_35A4[16];

extern s16 lbl_1_bss_35C4[16];

extern s16 lbl_1_bss_35EC;

extern ANIMDATA *lbl_1_bss_35F0;

extern s16 lbl_1_bss_35F4;

void fn_1_17B5C(void)
{
    COL_ATTRPARAM collisionParam;
    MGACTOR *actor;
    s16 lastModelId;
    s16 modelId;
    s32 playerIndex;
    s32 actorIndex;
    f32 previousX;
    f32 randomX;
    s32 index;

    for (index = 0; index < 9; index++) {
        if (index == 0) {
            lastModelId = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 55), 0x10000000, 2));
            fn_800269E8(lastModelId, 0x40000002U);
            modelId = lastModelId;
        } else {
            modelId = fn_80025D4C(lastModelId);
        }
        actor = fn_80080B04(&lbl_1_data_1328, modelId);
        fn_80026FFC(modelId);
        fn_80080DD4(actor, 4U);
        playerIndex = fn_1_1092C(actor, &lbl_1_data_12BC[index]);
        lbl_1_bss_1B8[playerIndex].event_state = 1;
    }

    previousX = -1000.0f;
    for (index = 0; index < 3; index++) {
        Point3d pushDirection = {0.0f, 0.0f, 6.0f};
        if (index == 0) {
            lastModelId = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 73), 0x10000000, 2));
            fn_800269E8(lastModelId, 0x40000002U);
            modelId = lastModelId;
        } else {
            modelId = fn_80025D4C(lastModelId);
        }
        fn_8002C95C(modelId,
            fn_80030458(modelId,
                fn_80008DD0(DATANUM(0xa70000, 74), 0x10000000, 2)));
        fn_80026E44(modelId);
        fn_800269E8(modelId, 0x40000001U);
        fn_80026A44(modelId, 0x40000002U);
        fn_80026B8C(modelId, 6);
        actor = fn_80080B04(&lbl_1_data_1818, modelId);
        fn_80080DD4(actor, 1U);
        fn_80086CBC(actor, 0.0f);
        fn_80086CA8(actor, 270.0f);
        fn_80086C8C(actor, &pushDirection);
        lbl_1_bss_36FC[index].position.x = -2000.0f;
        lbl_1_bss_36FC[index].position.y = 35.0f;
        lbl_1_bss_36FC[index].position.z = (f32)((index * 0x12C) - 0x12C);
        actorIndex = fn_1_1092C(actor, &lbl_1_bss_36FC[index].position);
        lbl_1_bss_36FC[index].state = &lbl_1_bss_1B8[actorIndex];
        lbl_1_bss_36FC[index].linkedModelId = modelId;
        lbl_1_bss_36FC[index].actor = actor;
        lbl_1_bss_36FC[index].modelId = 0;
    }

    for (index = 0; index < 16; index++) {
        do {
            randomX = (560.0f * ((f32)rand8() / 255.0f)) - 280.0f;
        } while ((previousX - 50.0f) < randomX &&
                 randomX < (50.0f + previousX));
        if (index == -1) {
            lastModelId = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 73), 0x10000000, 2));
            modelId = lastModelId;
        } else {
            modelId = fn_80025D4C(lastModelId);
        }
        fn_8002C95C(modelId,
            fn_80030458(modelId,
                fn_80008DD0(DATANUM(0xa70000, 74), 0x10000000, 2)));
        lbl_1_bss_35C4[index] = modelId;
        previousX = randomX;
        lbl_1_bss_34E4[index].x =
            40400.0f + (((f32)(index % 4) - 1.5f) * 180.0f) +
            ((((f32)rand8() / 255.0f) - 0.5f) * 130.0f);
        lbl_1_bss_34E4[index].y = 40.0f;
        lbl_1_bss_34E4[index].z =
            (((f32)(index / 4) - 1.5f) * 200.0f) +
            ((((f32)rand8() / 255.0f) - 0.5f) * 150.0f);
        fn_80026790(lbl_1_bss_35C4[index],
                         &lbl_1_bss_34E4[index]);
        fn_80026808(lbl_1_bss_35C4[index], 0.0f,
                        360.0f * ((f32)rand8() / 256.0f), 0.0f);
        fn_800269E8(lbl_1_bss_35C4[index], 1U);
        fn_800269E8(lbl_1_bss_35C4[index], 0x40000001U);
        fn_80026A44(lbl_1_bss_35C4[index], 0x40000002U);
        fn_8002D414(lbl_1_bss_35C4[index],
                          ((f32)rand8() / 256.0f) *
                              fn_8002D530(lbl_1_bss_35C4[index]));
        fn_80026B8C(lbl_1_bss_35C4[index], 6);
        lbl_1_bss_35A4[index] = ((s32)rand8() % 256) + 0xA;
        lbl_1_bss_3424[index].x =
            40400.0f + (((f32)(index % 4) - 1.5f) * 200.0f) +
            ((((f32)rand8() / 255.0f) - 0.5f) * 110.0f);
        lbl_1_bss_3424[index].y = 40.0f;
        lbl_1_bss_3424[index].z =
            (((f32)(index / 4) - 1.5f) * 200.0f) +
            ((((f32)rand8() / 255.0f) - 0.5f) * 133.0f);
    }

    lbl_1_bss_35F0 = HuSprAnimRead(fn_800082D0(EFFECT_ANM_danger));
    lbl_1_bss_35F4 = fn_8003216C(lbl_1_bss_35F0, 0x10);
    fn_80026760(lbl_1_bss_35F4, 0.0f, 0.0f, 0.0f);
    fn_800268B0(lbl_1_bss_35F4, 1.0f, 1.0f, 1.0f);
    fn_80026B8C(lbl_1_bss_35F4, 6);
    fn_800325B0(lbl_1_bss_35F4, 0xFFU, 0xFFU, 0xFFU);
    fn_800324E0(lbl_1_bss_35F4, 1.0f);
    fn_800326D0(lbl_1_bss_35F4, fn_1_186D0);
    fn_1_4518(fn_1_15B9C);
    fn_1_45A0((void (*)(void *, s32))fn_1_15BA4);
    lbl_1_bss_3414 = frandmod(3);
    lbl_1_bss_35EC = fn_1_FC80(lbl_1_bss_3414 + DATANUM(0x250000, 15));
    fn_800870D4(&collisionParam, 1U);
    collisionParam.yDeviate = 0.0f;
    collisionParam.speed = 1.0f;
    collisionParam.maxDot = 1.0f;
    fn_800870A4(&collisionParam, 1U);
    fn_800870D4(&collisionParam, 4U);
    collisionParam.speed = 0.4f;
    fn_800870A4(&collisionParam, 4U);
}

extern s32 lbl_1_bss_3404;

/* Raises the stage particles while their effect is active. */
void fn_1_186D0(HU3D_MODEL *model, HU3D_PARTICLE *manager, Mtx matrix)
{
    s16 index;
    f32 progress;
    HU3D_PARTICLE_DATA *currentParticle;

    currentParticle = manager->data;
    for (index = 0; index < 16;
         index++, currentParticle++) {
        currentParticle->scale = 0.0f;
    }
    if (lbl_1_bss_3404 == 1) {
        manager->count++;
    } else {
        manager->count = 0;
    }
    currentParticle = manager->data;
    if (manager->count != 0) {
        for (index = 0; index < 16;
             currentParticle++, index++) {
            currentParticle->pos.x = lbl_1_bss_34E4[index].x;
            if (manager->count <= 7) {
                progress = (f32) manager->count / 7.0f;
            } else {
                progress = 1.0f;
            }
            currentParticle->pos.y = 75.0f + 50.0f * progress;
            currentParticle->pos.z = lbl_1_bss_34E4[index].z;
            currentParticle->vel.x = 0.0f;
            currentParticle->vel.y = 0.0f;
            currentParticle->vel.z = 0.0f;
            currentParticle->color.r = currentParticle->color.g = currentParticle->color.b = 0xFF;
            currentParticle->scale = 50.0f;
        }
    }
}

extern f32 lbl_1_bss_3408;

extern f32 lbl_1_bss_3410;

extern void fn_1_1C53C(MCResponse_s *event);

void fn_1_1AED4(MCResponse_s *context);
/* Positions the stage slots for the selected lane. */
static inline void MikeActLayoutApply(s32 laneIndex)
{
    Point3d position;
    s32 allocationCount;
    s32 slotIndex;
    s32 direction;
    s32 index;

    lbl_1_bss_3400 = 0;
    index = 0;
    allocationCount = 0;
    for (; index < 24; index++) {
        slotIndex = lbl_1_bss_375C[index];
        if (lbl_1_bss_3400 > 0 ||
            lbl_1_data_1348[laneIndex][index].layoutId == -1) {
            position.x = (f32)(((lbl_1_bss_3400 / 3) * 100) - 900);
            position.y = 0.0f;
            position.z = (f32)(((lbl_1_bss_3400 % 3) * 300) - 300);
            direction = 0;
            lbl_1_bss_3400++;
        } else {
            position.x = (f32)(((lbl_1_data_1348[laneIndex][index].layoutId - 1) *
                                5000) - 1000 +
                               (lbl_1_data_1348[laneIndex][index].horizontalIndex * 100));
            position.y = 0.0f;
            position.z = (f32)((lbl_1_data_1348[laneIndex][index].depthIndex * 300) - 450);
            direction = lbl_1_data_1348[laneIndex][index].rotationIndex;
            fn_1_10F98((s16)slotIndex, 7);
        }
        if (direction == 0) {
            position.z += 150.0f;
        } else {
            position.x += 150.0f;
        }
        lbl_1_bss_1B8[slotIndex].callback_state = direction * 90;
        fn_80026790(lbl_1_bss_1B8[slotIndex].model_id, &position);
        fn_80026790(
            lbl_1_bss_1B8[slotIndex].linked_model_id,
            &position);
        fn_1_10F68((s16)slotIndex, &position);
        fn_80026808(lbl_1_bss_1B8[slotIndex].model_id, 0.0f,
                        (f32)(direction * 90), 0.0f);
        fn_80026808(
            lbl_1_bss_1B8[slotIndex].linked_model_id,
            0.0f, (f32)(direction * 90), 0.0f);
    }
}

void fn_1_18880(s32 laneIndex)
{
    MikeActLayoutApply(laneIndex);
}

extern MikeActStateRecord lbl_1_bss_1B8[256];
/* Moves the players into position and runs the stage sequence. */
void fn_1_18C54(void)
{
    Point3d playerPosition;
    f32 horizontalDistance;
    f32 depthDistance;
    f32 distance;
    f32 minimumDistance;
    f32 arrivalThreshold;
    s32 laneIndex;
    s32 groupIndex;
    s32 frameCount;
    s32 motionFrame;
    s32 targetLane;
    s32 randomSide;

    fn_80086BC8(lbl_1_bss_C4[0]->player->actor, &playerPosition);
    if (lbl_1_bss_3422 != 2 && playerPosition.x >= 36000.0f) {
        if (lbl_1_bss_3422 != 1 || lbl_1_bss_3420 <= 0) {
            for (laneIndex = 0; laneIndex < 16; laneIndex++) {
                fn_80026A44(lbl_1_bss_35C4[laneIndex], 1U);
            }
            fn_80026A44(lbl_1_bss_35F8, 1U);
            fn_800269E8(lbl_1_bss_35F8, 0x40000002U);
            fn_8002D414(lbl_1_bss_35F8, 0.0f);
        }

        for (laneIndex = 0; laneIndex < 16; laneIndex++) {
            if (lbl_1_bss_3404 == 0) {
                if (lbl_1_bss_35A4[laneIndex] > 0) {
                    lbl_1_bss_35A4[laneIndex]--;
                } else {
                    arrivalThreshold = 2.0f;
                    horizontalDistance = lbl_1_bss_3424[laneIndex].x -
                                         lbl_1_bss_34E4[laneIndex].x;
                    depthDistance = lbl_1_bss_3424[laneIndex].z -
                                    lbl_1_bss_34E4[laneIndex].z;
                    distance = sqrtf(horizontalDistance * horizontalDistance +
                                     depthDistance * depthDistance);
                    if (distance <= arrivalThreshold) {
                        minimumDistance = 40.0f;
                        lbl_1_bss_34E4[laneIndex].x =
                            lbl_1_bss_3424[laneIndex].x;
                        lbl_1_bss_34E4[laneIndex].z =
                            lbl_1_bss_3424[laneIndex].z;
                        lbl_1_bss_35A4[laneIndex] = (rand8() % 64) + 0x5A;
                        do {
                            lbl_1_bss_3424[laneIndex].x =
                                40400.0f + (180.0f * ((f32)(laneIndex % 4) - 1.5f)) +
                                (130.0f * (((f32)rand8() / 255.0f) - 0.5f));
                            lbl_1_bss_3424[laneIndex].y = 40.0f;
                            lbl_1_bss_3424[laneIndex].z =
                                (200.0f * ((f32)(laneIndex / 4) - 1.5f)) +
                                (150.0f * (((f32)rand8() / 255.0f) - 0.5f));
                            horizontalDistance =
                                lbl_1_bss_3424[laneIndex].x -
                                lbl_1_bss_34E4[laneIndex].x;
                            depthDistance = lbl_1_bss_3424[laneIndex].z -
                                            lbl_1_bss_34E4[laneIndex].z;
                            distance = sqrtf(horizontalDistance * horizontalDistance +
                                             depthDistance * depthDistance);
                        } while (distance <
                                 (minimumDistance -=
                                      minimumDistance > 1.0f));
                    } else {
                        lbl_1_bss_34E4[laneIndex].x +=
                            arrivalThreshold * (horizontalDistance / distance);
                        lbl_1_bss_34E4[laneIndex].z +=
                            arrivalThreshold * (depthDistance / distance);
                        fn_80026808(
                            lbl_1_bss_35C4[laneIndex], 0.0f,
                            180.0f * (atan2(horizontalDistance, depthDistance) /
                                      M_PI),
                            0.0f);
                    }
                    fn_80026790(lbl_1_bss_35C4[laneIndex],
                                     &lbl_1_bss_34E4[laneIndex]);
                }
            } else if (lbl_1_bss_3404 == 1) {
            } else if (lbl_1_bss_3404 == 2) {
                lbl_1_bss_34E4[laneIndex].y -=
                    (lbl_1_bss_34E4[laneIndex].y > 10.0f) ? 0.4f : 0.0f;
                fn_80026790(lbl_1_bss_35C4[laneIndex],
                                 &lbl_1_bss_34E4[laneIndex]);
            }
        }
    }

    switch (lbl_1_bss_3422) {
    case 0:
        fn_1_1A234();
        break;
    case 1:
        fn_1_1B2B8();
        break;
    }

    for (laneIndex = 0; laneIndex < 3; laneIndex++) {
        Point3d pushUp = {0.0f, 0.0f, 3.0f};
        Point3d pushBack = {0.0f, 5.0f, 0.0f};
        s32 cameraPositions[3][3] = {
            {0x789B0, 0x789B0, 0x789B0},
            {0x477C, 0x814C, 0x789B0},
            {0x4010, 0x814C, 0x789B0}
        };
        Point3d actorPosition;

        fn_80086BC8(lbl_1_bss_36FC[laneIndex].actor, &actorPosition);
        if (lbl_1_bss_36FC[laneIndex].state->event_state == 0 && actorPosition.x > -1000.0f &&
            lbl_1_bss_36FC[laneIndex].state->callback_state == 0x28) {
            fn_80086C8C(lbl_1_bss_36FC[laneIndex].actor, &pushUp);
        }
        if (lbl_1_bss_36FC[laneIndex].state->event_state == 0 && lbl_1_bss_36FC[laneIndex].state->callback_state < 0xA) {
            fn_80086C8C(lbl_1_bss_36FC[laneIndex].actor, &pushBack);
        }

        for (groupIndex = 0; groupIndex < 3; groupIndex++) {
            if (lbl_1_bss_36FC[laneIndex].position.x <
                    (f32)cameraPositions[laneIndex][groupIndex] &&
                    (f32)cameraPositions[laneIndex][groupIndex] <=
                    1000.0f + playerPosition.x &&
                800.0f + playerPosition.x <=
                    (f32)(cameraPositions[laneIndex][groupIndex] + 0x4B0)) {
                lbl_1_bss_36FC[laneIndex].position.x =
                    (f32)cameraPositions[laneIndex][groupIndex];
                fn_1_16718(
                    laneIndex,
                    cameraPositions[laneIndex][groupIndex]);
                HuAudFXPlay(0x54A);
            }
        }

        frameCount = lbl_1_bss_36FC[laneIndex].state->callback_state;
        switch (lbl_1_bss_36FC[laneIndex].state->event_state) {
        case 0:
            frameCount += frameCount < 40;
            fn_80026CBC(
                lbl_1_bss_36FC[laneIndex].linkedModelId,
                (frameCount > 10) ? 1.0f : (f32)frameCount / 10.0f);
            if (((s32)actorPosition.z == 0 &&
                 lbl_1_data_1838[0] != 0 &&
                 lbl_1_data_1254[3].x - 180.0f < actorPosition.x &&
                 actorPosition.x < 180.0f + lbl_1_data_1254[3].x) ||
                ((s32)actorPosition.z == 0 &&
                 lbl_1_data_1838[1] != 0 &&
                 lbl_1_data_1254[4].x - 180.0f < actorPosition.x &&
                 actorPosition.x < 180.0f + lbl_1_data_1254[4].x)) {
                lbl_1_bss_36FC[laneIndex].state->event_state = 1;
            }
            break;
        case 1:
            frameCount -= frameCount > 0;
            fn_80026CBC(lbl_1_bss_36FC[laneIndex].linkedModelId,
                              (f32)frameCount / 40.0f);
            break;
        }
            lbl_1_bss_36FC[laneIndex].state->callback_state = frameCount;
        }

    for (groupIndex = 0; groupIndex < 2; groupIndex++) {
        if (lbl_1_bss_363C[groupIndex][0].state == 1) {
            targetLane = (playerPosition.z <= -150.0f)
                             ? 0
                             : ((playerPosition.z > 150.0f) ? 2 : 1);
            randomSide = rand8() % 2;
            lbl_1_bss_363C[groupIndex][0].state = 2;
            lbl_1_bss_363C[groupIndex][0].flags = 0;
            HuAudFXPlay(0x54B);
            if (randomSide == 0) {
                fn_1_16718(targetLane,
                           900.0f + lbl_1_bss_363C
                                        [groupIndex][targetLane].position.x);
                HuAudFXPlay(0x54A);
            } else {
                lbl_1_bss_3624[groupIndex].stateRecord->position.x =
                    900.0f + lbl_1_bss_363C
                                 [groupIndex][targetLane].position.x;
                lbl_1_bss_3624[groupIndex].stateRecord->position.y = 0.0f;
                lbl_1_bss_3624[groupIndex].stateRecord->position.z =
                    (f32)((targetLane * 0x12C) - 0x12C);
                fn_8002D414(lbl_1_bss_3622, 0.0f);
                fn_80026A44(lbl_1_bss_3622, 1U);
                fn_80026A44(lbl_1_bss_3622, 0x40000002U);
                fn_80026790(lbl_1_bss_3622,
                                 &lbl_1_bss_3624[groupIndex].stateRecord->position);
                fn_80026790(lbl_1_bss_3624[groupIndex].primaryModelId,
                                 &lbl_1_bss_3624[groupIndex].stateRecord->position);
                fn_80026790(lbl_1_bss_3624[groupIndex].secondaryModelId,
                                 &lbl_1_bss_3624[groupIndex].stateRecord->position);
            }
            for (laneIndex = 0; laneIndex < 3; laneIndex++) {
                motionFrame = (laneIndex != targetLane)
                                  ? 0xD2
                                  : ((randomSide != 0) ? 0x46 : 0x8C);
                fn_8002D414(
                    lbl_1_bss_363C
                        [groupIndex][laneIndex].primaryModelId,
                    (f32)motionFrame);
                fn_8002D730(
                    lbl_1_bss_363C
                        [groupIndex][laneIndex].primaryModelId,
                    (f32)motionFrame,
                                      (f32)(motionFrame + 0x3B));
                fn_80026A44(
                    lbl_1_bss_363C
                        [groupIndex][laneIndex].primaryModelId,
                    0x40000001U);
                fn_80026A44(
                    lbl_1_bss_363C
                        [groupIndex][laneIndex].primaryModelId,
                    0x40000002U);
                if (laneIndex != targetLane) {
                    fn_8002D414(
                        lbl_1_bss_363C
                            [groupIndex][laneIndex].primaryModelId,
                        60.0f);
                    fn_8002D730(
                        lbl_1_bss_363C
                            [groupIndex][laneIndex].auxiliaryModelId,
                        60.0f, 120.0f);
                    fn_80026A44(
                        lbl_1_bss_363C
                            [groupIndex][laneIndex].auxiliaryModelId,
                        0x40000001U);
                    fn_80026A44(
                        lbl_1_bss_363C
                            [groupIndex][laneIndex].auxiliaryModelId,
                        0x40000002U);
                }
            }
        } else if (lbl_1_bss_363C[groupIndex][0].state == 2) {
            lbl_1_bss_363C[groupIndex][0].flags++;
            if (lbl_1_bss_363C[groupIndex][0].flags >= 0x3C) {
                lbl_1_bss_363C[groupIndex][0].state = 3;
                lbl_1_bss_363C[groupIndex][0].flags = 0;
            }
        }
    }

    for (laneIndex = 0; laneIndex < 6; laneIndex++) {
        if (playerPosition.x >= lbl_1_data_1940[laneIndex].x) {
            fn_80026A44(lbl_1_bss_360C[laneIndex], 0x40000002U);
        }
    }
}

void fn_1_1A230(void)
{
}

extern MikeActPlayerRecord *lbl_1_bss_C4[5];

extern MikeActPlayerRecord *lbl_1_bss_B8;

extern s16 lbl_1_bss_1AA;

extern u32 lbl_1_bss_341C;

/* Runs the transition triggered when the stage actor passes the fixed X position. */
void fn_1_1A234(void)
{
    s32 playerIndex;

    if (lbl_1_bss_B8 != NULL) {
      switch (lbl_1_bss_3420) {
    case 0:
        if (lbl_1_bss_F0->position.x > 10750.0f) {
            fn_1_49D0(lbl_1_bss_B8);
            if ((lbl_1_bss_B8->player->actor->colGroundAttr & 0x407F) != 0) {
                fn_1_45B0(lbl_1_bss_B8, 0x2000);
                fn_8007E818(lbl_1_bss_B8->player);
                fn_1_EA08(0);
                fn_1_1BF4();
                lbl_1_bss_3420 = 1;
                return;
            }
        }
        break;
    case 1:
        fn_1_665C(lbl_1_bss_B8, 1);
        if (fn_8007E8B4(lbl_1_bss_B8->player, &lbl_1_data_1864[0],
                             5.0f, 5.0f) != 0) {
        /* The original call passes this record where the engine function takes a player. */
        fn_8007EEBC((MGPLAYER *)lbl_1_bss_B8, 0);
            lbl_1_bss_B8->movement_angle = 90.0f;
            fn_80086CA8(lbl_1_bss_B8->player->actor,
                           lbl_1_bss_B8->movement_angle);
            fn_1_F5BC();
            lbl_1_bss_3420 = 2;
            return;
        }
        break;
    case 2:
        if (fn_1_1F70() != 0) {
            fn_1_4C84(&lbl_1_data_1864[1]);
            lbl_1_bss_3410 = 0.0f;
            lbl_1_bss_3408 = 0.0f;
            lbl_1_bss_341C = 0;
            lbl_1_bss_3420 = 3;
            fn_1_1BF4();
            return;
        }
        break;
    case 3: {
        if (lbl_1_bss_341C++ == 0xF0) {
            lbl_1_bss_341C = 0;
            lbl_1_bss_3420 = 4;
            return;
        }
        break;
    }
    case 4: {
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            fn_8007E8B4(lbl_1_bss_C4[playerIndex]->player,
                             &lbl_1_data_1894[playerIndex], 5.0f, 5.0f);
        }
        if (lbl_1_bss_341C++ == 0xF) {
            WipeCreate(2, 0, 30);
            lbl_1_bss_3420 = 5;
            return;
        }
        break;
    }
    case 5:
        if (WipeCheck() != 0) {
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                fn_8007E8B4(lbl_1_bss_C4[playerIndex]->player,
                                 &lbl_1_data_1894[playerIndex], 5.0f, 5.0f);
            }
            return;
        }
        fn_8008697C(lbl_1_bss_C4[0]->player, &lbl_1_data_1894[0]);
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            fn_8008697C(lbl_1_bss_C4[playerIndex]->player,
                           &lbl_1_data_1894[playerIndex]);
            lbl_1_bss_C4[playerIndex]->movement_angle = 90.0f;
            fn_80086CA8(lbl_1_bss_C4[playerIndex]->player->actor, 90.0f);
            fn_8007EEBC(lbl_1_bss_C4[playerIndex]->player, 0xA);
            fn_1_665C(lbl_1_bss_C4[playerIndex], 0x13);
        }
        lbl_1_bss_1AA = 5;
        fn_1_36E4(1);
        fn_8003E5A4(&lbl_1_data_1848);
        fn_1_F370(MESSNUM(MESS_MICGO, 19), 0);
        fn_1_1B40(1, fn_1_1AED4);
        fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 13), 1);
        lbl_1_bss_3418 = 0;
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            char *itemAnimation;

            itemAnimation = fn_8005C0F0(
                lbl_1_bss_C4[playerIndex]->character_no, 4, 0);
            if (itemAnimation) {
                fn_80026A44(lbl_1_bss_3606[playerIndex - 1], 1U);
                fn_8002741C(
                    lbl_1_bss_C4[playerIndex]->player->actor->mdlId,
                    itemAnimation, lbl_1_bss_3606[playerIndex - 1]);
            }
        }
        fn_80026760(lbl_1_bss_35FE, 11740.0f, 525.0f, 220.0f);
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            HuPrcVSleep();
        }
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            fn_8005C430(lbl_1_bss_C4[playerIndex]->character_no,
                             0x40000002U);
            fn_8005C16C(lbl_1_bss_C4[playerIndex]->character_no, 0.0f);
        }
        WipeCreate(1, 0, 30);
        lbl_1_bss_3420 = 6;
        return;
    case 6:
        if (WipeCheck() != 0) {
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                fn_8005C430(lbl_1_bss_C4[playerIndex]->character_no,
                                 0x40000002U);
                fn_8005C16C(lbl_1_bss_C4[playerIndex]->character_no, 0.0f);
            }
            return;
        }
        lbl_1_bss_3420 = 0xA;
        return;
    case 0xA:
        fn_1_1B07C();
        if (lbl_1_bss_3418 >= 0xA) {
            lbl_1_bss_3420 = 0x32;
            fn_80026A44(lbl_1_bss_35FE, 0x40000002U);
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            }
            return;
        }
        break;
    case 0x32:
        lbl_1_bss_341C = 0;
        lbl_1_bss_3420++;
        fn_1_1BF4();
        HuAudFXPlay(0x53F);
        return;
    case 0x33: {
        if (lbl_1_bss_341C++ == 0x78) {
            WipeCreate(2, 0, 30);
            lbl_1_bss_3420++;
            return;
        }
        break;
    }
    case 0x34:
        if (WipeCheck() == 0) {
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                fn_80027508(
                    lbl_1_bss_C4[playerIndex]->player->actor->mdlId);
            }
            fn_800268B0(lbl_1_bss_35FE, 0.0f, 0.0f, 0.0f);
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                fn_8007EEBC(lbl_1_bss_C4[playerIndex]->player, 0);
            }
            fn_1_1AFD0();
            WipeCreate(1, 0, 30);
            lbl_1_bss_3420++;
            return;
        }
        break;
    case 0x35:
        if (WipeCheck() == 0) {
            fn_8007E888(lbl_1_bss_B8->player);
            lbl_1_bss_3422 = 1;
            lbl_1_bss_3420 = 0;
            MikeActLayoutApply(lbl_1_bss_3422);
        }
        break;
      }
    }
}
/* Starts the guide animation when the microphone response matches. */
void fn_1_1AED4(MCResponse_s *context)
{
    s32 playerIndex;

    switch (lbl_1_bss_3420) {
    case 10:
        if (*context->result == 0) {
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                if (lbl_1_bss_3408 == 0.0f) {
                    DebugValue(lbl_1_bss_3418 == 0);
                }
                CharFXPlay(lbl_1_bss_C4[playerIndex]->character_no, 0x251);
            }
            lbl_1_bss_3408 = 1.0f;
        }
        break;
    }
}


void fn_1_1AFB0(s16 mode, s16 frameNo)
{
    fn_1_1F94();
}

/* Reset the selection display and hide the other players during the guide sequence. */
void fn_1_1AFD0(void) {
    s32 playerIndex;

    fn_1_1FD4();
    fn_1_EA08(1);
    playerIndex = 1;
    while (playerIndex < 4) {
        fn_80086E3C(lbl_1_bss_C4[playerIndex]->player);
        fn_1_45B0(lbl_1_bss_C4[playerIndex], 0x2000);
        fn_800269E8((s16)lbl_1_bss_C4[playerIndex]->player->actor->mdlId, 1U);
        playerIndex += 1;
    }
    fn_1_F598();
}
/* Advances the guide animation and updates the progress display. */
s32 fn_1_1B07C(void)
{
    s32 playerIndex;

    if (lbl_1_bss_3410 == 0.0f && lbl_1_bss_3408 > 0.0f) {
        HuAudFXPlay(1358);
    }
    lbl_1_bss_3410 += lbl_1_bss_3408;
    if (lbl_1_bss_3410 >= 30.0f) {
        lbl_1_bss_3410 = 0.0f;
        lbl_1_bss_3408 = 0.0f;
        lbl_1_bss_3418++;
    }
    if (lbl_1_bss_3408 != 0.0f || lbl_1_bss_3418 != 0) {
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            fn_8005C430(lbl_1_bss_C4[playerIndex]->character_no, 0x40000002U);
            fn_8005C16C(lbl_1_bss_C4[playerIndex]->character_no,
                (20.0f * lbl_1_bss_3410) / 30.0f);
        }
    }
    fn_8002D414(lbl_1_bss_35FE,
        600.0f * (((f32)lbl_1_bss_3418 + (lbl_1_bss_3410 / 30.0f)) / 10.0f));
    return 0;
}

extern f32 lbl_1_bss_340C;

extern s16 lbl_1_bss_3416;

/* Runs the transition at the next stage marker and repositions the other players. */
void fn_1_1B2B8(void)
{
    s32 playerIndex;

    if (lbl_1_bss_B8 == NULL) {
        return;
    }
    switch (lbl_1_bss_3420) {
    case 0:
        if (lbl_1_bss_F0->position.x >
            lbl_1_data_18E0[0].x - 150.0f) {
            fn_1_49D0(lbl_1_bss_B8);
            if ((lbl_1_bss_B8->player->actor->colGroundAttr & 0x407F) != 0) {
                fn_1_45B0(lbl_1_bss_B8, 0x2000);
                fn_8007E818(lbl_1_bss_B8->player);
                fn_1_EA08(0);
                fn_1_1BF4();
                lbl_1_bss_3420 = 1;
                return;
            }
        }
        break;
    case 1:
        fn_1_665C(lbl_1_bss_B8, 1);
        if (fn_8007E8B4(lbl_1_bss_B8->player, &lbl_1_data_18E0[0],
                             5.0f, 5.0f) != 0) {
            /* The original call passes this record where the engine function takes a player. */
            fn_8007EEBC((MGPLAYER *)lbl_1_bss_B8, 0);
            lbl_1_bss_B8->movement_angle = 90.0f;
            fn_80086CA8(lbl_1_bss_B8->player->actor,
                           lbl_1_bss_B8->movement_angle);
            fn_1_F5BC();
            lbl_1_bss_3420 = 2;
            return;
        }
        break;
    case 2:
        if (fn_1_1F70() != 0) {
            fn_1_4C84(&lbl_1_data_18E0[1]);
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                lbl_1_bss_C4[playerIndex]->movement_angle = 0.0f;
                fn_80086CA8(lbl_1_bss_C4[playerIndex]->player->actor,
                               lbl_1_bss_C4[playerIndex]->movement_angle);
                fn_80026808(
                    lbl_1_bss_C4[playerIndex]->player->actor->mdlId,
                    0.0f, lbl_1_bss_C4[playerIndex]->movement_angle, 0.0f);
                fn_1_665C(lbl_1_bss_C4[playerIndex], 0);
            }
            fn_1_1BF4();
            lbl_1_bss_3410 = 0.0f;
            lbl_1_bss_340C = 0.0f;
            lbl_1_bss_3408 = 0.0f;
            lbl_1_bss_341C = 0;
            lbl_1_bss_3416 = 0;
            lbl_1_bss_3420 = 3;
            return;
        }
        break;
    case 3:
        if (lbl_1_bss_341C++ == 0xF0) {
            lbl_1_bss_341C = 0;
            lbl_1_bss_3420 = 4;
            return;
        }
        break;
    case 4:
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            fn_8007E8B4(lbl_1_bss_C4[playerIndex]->player,
                             &lbl_1_data_1910[playerIndex], 5.0f, 5.0f);
        }
        if (lbl_1_bss_341C++ == 0xF) {
            WipeCreate(2, 0, 30);
            lbl_1_bss_3420 = 5;
            return;
        }
        break;
    case 5:
        if (WipeCheck() != 0) {
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                fn_8007E8B4(lbl_1_bss_C4[playerIndex]->player,
                                 &lbl_1_data_1910[playerIndex], 5.0f, 5.0f);
            }
            return;
        }
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            fn_8008697C(lbl_1_bss_C4[playerIndex]->player,
                           &lbl_1_data_1910[playerIndex]);
            lbl_1_bss_C4[playerIndex]->movement_angle = 180.0f;
            fn_80086CA8(lbl_1_bss_C4[playerIndex]->player->actor,
                           lbl_1_bss_C4[playerIndex]->movement_angle);
            fn_80026808(
                lbl_1_bss_C4[playerIndex]->player->actor->mdlId,
                0.0f,
                lbl_1_bss_C4[playerIndex]->movement_angle,
                0.0f);
            fn_8007EEBC(lbl_1_bss_C4[playerIndex]->player, 0xA);
            fn_1_665C(lbl_1_bss_C4[playerIndex], 0x11);
            fn_8005C430(lbl_1_bss_C4[playerIndex]->character_no,
                             0x40000002U);
            fn_8005C16C(lbl_1_bss_C4[playerIndex]->character_no, 0.0f);
        }
        lbl_1_bss_1AA = 5;
        fn_1_36E4(1);
        fn_8003E5A4(&lbl_1_data_18C4);
        fn_1_EACC();
        fn_1_F370(MESSNUM(MESS_MICGO, 20), 0);
        fn_1_1B40(1, fn_1_1C53C);
        fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 14), 1);
        lbl_1_bss_3418 = 0;
        WipeCreate(1, 0, 30);
        lbl_1_bss_3420 = 6;
        return;
    case 6:
        HuPrcVSleep();
        fn_80086E3C(lbl_1_bss_C4[1]->player);
        fn_80086E3C(lbl_1_bss_C4[2]->player);
        fn_80086E3C(lbl_1_bss_C4[3]->player);
        if (WipeCheck() != 0) {
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                fn_8005C430(lbl_1_bss_C4[playerIndex]->character_no,
                                 0x40000002U);
                fn_8005C16C(
                    lbl_1_bss_C4[playerIndex]->character_no, 0.0f);
            }
            return;
        }
        lbl_1_bss_3420 = 0x6E;
        return;
    case 0x6E:
        if (fn_1_1C880() == 0 && lbl_1_bss_3418 == 0xA) {
            fn_1_1E1C(lbl_1_bss_3414 + MESSNUM(MESS_MICGOSUPORT, 15), 1);
            fn_1_FE94(1);
            lbl_1_bss_341C = 0;
            fn_1_F370(MESSNUM(MESS_MICGO, 12), 0);
            lbl_1_bss_3420++;
            return;
        }
        break;
    case 0x6F:
        if (fn_1_1C880() == 0) {
            if (lbl_1_bss_341C++ == 0x14) {
                fn_1_F148();
                lbl_1_bss_3420 = 0x78;
                return;
            }
        }
        break;
    case 0x78:
        if (fn_1_1C880() == 0) {
            lbl_1_bss_3418 = fn_1_F138();
            if (lbl_1_bss_3418 == 0) {
                fn_1_F188();
                fn_1_F370(MESSNUM(MESS_MICGO, 14), 0);
                fn_1_FE94(0);
                lbl_1_bss_341C = 0;
                lbl_1_bss_3420 = 0x8C;
                HuAudFXPlay(1342);
                fn_1_1BF4();
                return;
            }
        }
        break;
    case 0x82:
        if (fn_1_1C880() == 0) {
            fn_1_F188();
            fn_1_FE94(0);
            fn_1_F370(MESSNUM(MESS_MICGO, 13), 0);
            fn_1_1BF4();
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                CharFXPlay(lbl_1_bss_C4[playerIndex]->character_no, 0x243);
            }
            HuAudFXPlay(1341);
            lbl_1_bss_3420++;
            return;
        }
        break;
    case 0x83:
        fn_1_1C880();
        return;
    case 0x8C:
        if (fn_1_1C880() == 0) {
            DebugValue(lbl_1_bss_341C++ == 0xB4);
            return;
        }
        break;
    case 0x96:
        fn_1_1BF4();
        fn_1_F188();
        fn_1_FE94(0);
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            fn_8007EEBC(lbl_1_bss_C4[playerIndex]->player, 0);
            fn_1_665C(lbl_1_bss_C4[playerIndex], 0);
            fn_8005C47C(lbl_1_bss_C4[playerIndex]->character_no,
                               0x40000002U);
            fn_8005C430(lbl_1_bss_C4[playerIndex]->character_no,
                             0x40000001U);
            lbl_1_bss_C4[playerIndex]->movement_angle = 90.0f;
            fn_80086CA8(lbl_1_bss_C4[playerIndex]->player->actor,
                           lbl_1_bss_C4[playerIndex]->movement_angle);
            fn_80026808(
                lbl_1_bss_C4[playerIndex]->player->actor->mdlId,
                0.0f,
                lbl_1_bss_C4[playerIndex]->movement_angle,
                0.0f);
        }
        fn_80026A44(lbl_1_bss_35FA, 0x40000002U);
        fn_80026A44(lbl_1_bss_35FA, 0x40000001U);
        fn_80026A44(lbl_1_bss_35F8, 0x40000002U);
        fn_80026A44(lbl_1_bss_35F8, 0x40000001U);
        fn_8002D414(lbl_1_bss_35F8, 0.0f);
        fn_8002D86C(lbl_1_bss_35F8, 1.0f);
        HuAudFXPlay(1363);
        lbl_1_bss_341C = 0;
        lbl_1_bss_3420++;
        return;
    case 0x97:
        if (lbl_1_bss_341C == 0x4B) {
            HuAudFXPlay(MSM_SE_GUIDE_TERESA_FAILURE);
        }
        for (playerIndex = 0; playerIndex < 16; playerIndex++) {
            if (lbl_1_bss_341C >= 0x2D && lbl_1_bss_341C < 0x4B) {
                lbl_1_bss_3404 = 1;
                fn_800269E8(lbl_1_bss_35C4[playerIndex], 0x40000002U);
            }
            if (lbl_1_bss_341C >= 0x4B && lbl_1_bss_341C < 0x87) {
                lbl_1_bss_3404 = 2;
                fn_80026CBC(
                    lbl_1_bss_35C4[playerIndex],
                    1.0f - ((f32)(lbl_1_bss_341C - 0x4B) / 60.0f));
            }
            if (lbl_1_bss_341C == 0x87) {
                fn_800268B0(lbl_1_bss_35C4[playerIndex],
                                  0.0f, 0.0f, 0.0f);
            }
        }
        if (lbl_1_bss_341C == 0x87) {
            HuAudFXPlay(1343);
        }
        if (lbl_1_bss_341C++ == 0xC3) {
            WipeCreate(2, 0, 30);
            lbl_1_bss_3420++;
            return;
        }
        break;
    case 0x98:
        if (WipeCheck() == 0) {
            fn_1_1C7D4();
            WipeCreate(1, 0, 30);
            lbl_1_bss_3420++;
            return;
        }
        break;
    case 0x99:
        if (WipeCheck() == 0) {
            fn_8007E888(lbl_1_bss_B8->player);
            lbl_1_bss_3422 = 2;
            lbl_1_bss_3420 = 0;
            MikeActLayoutApply(lbl_1_bss_3422);
        }
        break;
    }
}
/* Handles microphone responses for the current guide step. */
void fn_1_1C53C(MCResponse_s *event)
{
    s32 playerIndex;
    s32 microphoneSelection;
    s32 micState;

    if (HuMCMicGet() == 2) {
        micState = 0;
    } else {
        micState = 1;
    }
    microphoneSelection = micState;

    switch (lbl_1_bss_3420) {
    case 0x6E:
        if (*event->result == microphoneSelection) {
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                DebugValue(lbl_1_bss_3418 == 0);
                lbl_1_bss_C4[playerIndex]->movement_angle = 45.0f;
                fn_80086CA8(lbl_1_bss_C4[playerIndex]->player->actor,
                               lbl_1_bss_C4[playerIndex]->movement_angle);
                CharFXPlay(lbl_1_bss_C4[playerIndex]->character_no, 0x251);
            }
            if (lbl_1_bss_3408 == 0.0f) {
                fn_1_F0B8();
                lbl_1_bss_3418++;
            }
            lbl_1_bss_3408 = 60.0f;
        }
        break;
    case 0x8C:
        if (*event->result == microphoneSelection) {
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                CharFXPlay(lbl_1_bss_C4[playerIndex]->character_no, 0x251);
            }
            if (lbl_1_bss_3418 < 0xA) {
                lbl_1_bss_3418++;
                return;
            }
        }
        break;
    case 0x78:
        if (HuMCMicGet() == 2) {
            if (*event->result == 0) {
                lbl_1_bss_3420 = 0x82;
                return;
            }
        } else if (*event->result == lbl_1_bss_3414 + 2) {
            lbl_1_bss_3420 = 0x82;
        }
        break;
    }
}

void fn_1_1C7B4(s16 mode, s16 frameNo)
{
    fn_1_1F94();
}

/* Reset the selection display and hide the other players after the stage sequence. */
void fn_1_1C7D4(void) {
    fn_1_1AFD0();
}
/* Advances the timed movement for the guide's other players. */
s32 fn_1_1C880(void)
{
    s32 duration;
    s32 step;
    s32 playerIndex;
    f32 progress;
    f64 motionFraction;

    duration = (s32)lbl_1_bss_3408;
    step = (s32)lbl_1_bss_3410;
    if (0.0f != lbl_1_bss_3408) {
        if (lbl_1_bss_3420 != 0x82 && lbl_1_bss_3420 != 0x83) {
            lbl_1_bss_3408 = 60.0f;
        } else {
            lbl_1_bss_3408 = 5.0f;
        }
        lbl_1_bss_340C += 60.0f / lbl_1_bss_3408;
        if (lbl_1_bss_340C > 180.0f) {
            lbl_1_bss_340C -= 180.0f;
        }
        lbl_1_bss_3410 += 60.0f / lbl_1_bss_3408;
        if (step < 30 && lbl_1_bss_3410 >= 30.0f) {
            HuAudFXPlay(lbl_1_bss_3416 % 2 != 0 ? 0x54F : 0x550);
        }
        if (lbl_1_bss_3410 > 60.0f) {
            lbl_1_bss_3410 -= 60.0f;
            lbl_1_bss_3416++;
            if (lbl_1_bss_3420 == 0x6E) {
                lbl_1_bss_3408 = 0.0f;
            }
        }

        progress = 6.0f *
            (lbl_1_bss_3416 + lbl_1_bss_3410 / 60.0f) /
            43.0f;
        progress -= (s32)progress;
        if (progress < 0.5) {
            motionFraction = 2.0f * progress;
        } else {
            motionFraction = 1.0 -
                2.0 * (progress - 0.5);
        }
        progress = motionFraction;
        progress = progress * progress * progress * progress * progress * progress * progress * progress;
        fn_8002D414(lbl_1_bss_35F8, 30.0f * progress);
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            fn_8005C430(lbl_1_bss_C4[playerIndex]->character_no, ((1 << 30) | (1 << 1)));
            fn_8005C16C(lbl_1_bss_C4[playerIndex]->character_no, lbl_1_bss_3410);
            fn_8002D414(lbl_1_bss_3600[playerIndex - 1], lbl_1_bss_3410);
        }
        fn_8002D414(lbl_1_bss_35FC, lbl_1_bss_340C);
    } else {
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            fn_8005C430(lbl_1_bss_C4[playerIndex]->character_no, ((1 << 30) | (1 << 1)));
            fn_8005C16C(lbl_1_bss_C4[playerIndex]->character_no, 0.0f);
        }
    }
    if (lbl_1_bss_3416 >= 0x1E) {
        lbl_1_bss_3420 = 0x96;
        fn_8002D86C(lbl_1_bss_35FC,
                           60.0f / (f32)duration);
        return 1;
    }
    return 0;
}

s32 lbl_1_bss_375C[25];
MikeActArenaObject lbl_1_bss_36FC[3];
MikeActSceneEffectRow lbl_1_bss_363C[2][3];
MikeActScenePair lbl_1_bss_3624[3];
s16 lbl_1_bss_3622;
s16 lbl_1_bss_3618[5];
s16 lbl_1_bss_360C[6];
s16 lbl_1_bss_3606[3];
s16 lbl_1_bss_3600[3];
s16 lbl_1_bss_35FE;
s16 lbl_1_bss_35FC;
s16 lbl_1_bss_35FA;
s16 lbl_1_bss_35F8;
s16 lbl_1_bss_35F6;
s16 lbl_1_bss_35F4;
ANIMDATA *lbl_1_bss_35F0;
s16 lbl_1_bss_35EC;
s16 lbl_1_bss_35EA;
s16 lbl_1_bss_35E4[3];
s16 lbl_1_bss_35C4[16];
s16 lbl_1_bss_35A4[16];
Point3d lbl_1_bss_34E4[16];
Point3d lbl_1_bss_3424[16];
s16 lbl_1_bss_3422;
s16 lbl_1_bss_3420;
u32 lbl_1_bss_341C;
s16 lbl_1_bss_3418;
s16 lbl_1_bss_3416;
s16 lbl_1_bss_3414;
f32 lbl_1_bss_3410;
f32 lbl_1_bss_340C;
f32 lbl_1_bss_3408;
s32 lbl_1_bss_3404;
s32 lbl_1_bss_3400;
