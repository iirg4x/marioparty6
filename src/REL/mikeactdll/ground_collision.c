/* Ground collisions, event transitions, and radial particle motion. */
#include "REL/mikeactdll/stage_collision.h"
#include "REL/mikeactdll/effect_spawner.h"
#include "REL/mikeactdll/sound.h"
#include "REL/mikeactdll/sequence_context.h"
#include "math.h"

#include "dolphin/gx.h"

#include "REL/mikeactdll/event_callback.h"

#include "game/object.h"

#include "game/mg/seqman.h"

#include "game/mg/actman.h"

#include "game/sprite.h"

#include "include/game/gamework.h"

#include "REL/mikeactdll/module_types.h"

#include "game/hu3d.h"

#include "game/process.h"

#include "datanum/effect.h"

#include "messdir_enum.h"

#include "game/mic.h"

#include "include/game/mg/actman.h"

#include "include/game/audio.h"

#include "include/game/gamemes.h"

#include "game/main.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/wipe.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

extern u32 lbl_1_bss_30;

extern s16 lbl_1_bss_33F0;

extern HUPROCESS *lbl_1_bss_6C;

extern s16 lbl_1_bss_33EE;

extern s16 lbl_1_bss_33EA;

extern s16 lbl_1_bss_33E8;

extern s16 lbl_1_bss_33C8;

extern MGACTOR *lbl_1_bss_33C4;

extern s16 lbl_1_bss_33E0;

extern s16 lbl_1_bss_33CC;

extern u32 lbl_1_bss_33C0;

extern Point3d lbl_1_data_FD8[4];

extern OM_CAMERA_VIEW lbl_1_data_F8C;

extern s16 lbl_1_bss_1AA;

extern s16 lbl_1_bss_33DE;

extern s16 lbl_1_bss_33CA;

extern f32 lbl_1_bss_33D0;

extern s16 lbl_1_bss_33D4;

extern f32 lbl_1_bss_33D8;

extern s16 lbl_1_bss_33DC;

extern Point3d lbl_1_data_F80;

extern ANIMDATA *lbl_1_bss_33F8;

typedef struct MikeActRiseActorWork_s {
    MGACTOR *actor;
    s16 state;
    u8 unknown_06[2]; /* No code names this field. */
    Point3d position;
    f32 verticalOffset;
    s16 playerIndex;
    s16 targetIndex;
    s16 modelId;
    s16 alternateMotionId;
    s16 motionId;
    s16 linkedModelId;
    s16 shadowModelId;
    s16 shadowTimer;
} MikeActRiseActorWork;

extern s16 lbl_1_bss_33F2;

extern s16 lbl_1_bss_33F4;

extern s16 lbl_1_bss_33FC;

extern Point3d lbl_1_data_E70[9];

extern s16 lbl_1_bss_33EC;

extern Point3d lbl_1_data_D50;

extern Point3d lbl_1_data_D7C[5];

extern Point3d lbl_1_data_C68;

extern Point3d lbl_1_data_C74;

extern GXColor lbl_1_data_C80;

extern Point3d lbl_1_data_C84[8];

extern Point3d lbl_1_data_CE4;

extern Point3d lbl_1_data_CF0[8];

extern Point3d lbl_1_data_EFC[4];

extern Point3d lbl_1_data_F74;

void fn_1_13AD8(void);

extern s16 lbl_1_bss_33F6;

extern MGACTOR_PARAM lbl_1_data_D5C;

extern MGACTOR_PARAM lbl_1_data_DB8;

extern Point3d lbl_1_data_DD8[10];

extern MGACTOR_PARAM lbl_1_data_E50;

extern Point3d lbl_1_data_F2C[6];

extern MGACTOR_PARAM lbl_1_data_EDC;

void fn_1_13F40(void);

void fn_1_14C14(void);

extern Point3d lbl_1_data_FA8[4];

extern u32 lbl_1_bss_33E4;

extern MikeActPlayerRecord *lbl_1_bss_B8;

void fn_1_14B48(s16 mode, s16 frameNo);

void fn_1_15328(HU3D_PARTICLE *motionList);

void fn_1_15610(HU3D_MODEL *model, HU3D_PARTICLE *motionList, Mtx mtx);

Point3d lbl_1_data_C68 = { 0.0f, 5000.0f, 0.0f };
Point3d lbl_1_data_C74 = { 0.1f, -1.0f, -0.4f };
GXColor lbl_1_data_C80 = { 255, 255, 255, 255 };
Point3d lbl_1_data_C84[8] = { { 21450.0f, 0.0f, -300.0f }, { 22450.0f, 0.0f, -300.0f }, { 21450.0f, 0.0f, 300.0f }, { 26050.0f, 0.0f, 300.0f }, { 39850.0f, 0.0f, -300.0f }, { 41050.0f, 0.0f, 0.0f }, { 39850.0f, 0.0f, 300.0f }, { 47350.0f, 0.0f, 0.0f } };
Point3d lbl_1_data_CE4 = { 29150.0f, 0.0f, 0.0f };
Point3d lbl_1_data_CF0[8] = { { 4000.0f, 0.0f, 0.0f }, { 5900.0f, 0.0f, -300.0f }, { 5900.0f, 0.0f, 300.0f }, { 22600.0f, 0.0f, 0.0f }, { 28000.0f, 0.0f, -300.0f }, { 30800.0f, 0.0f, -300.0f }, { 30800.0f, 0.0f, 300.0f }, { 40900.0f, 0.0f, 300.0f } };
Point3d lbl_1_data_D50 = { 10350.0f, 150.0f, 0.0f };
MGACTOR_PARAM lbl_1_data_D5C = { 10.0f, 149.0f, 101, 5, fn_1_12380, 0, 0, 0 };
Point3d lbl_1_data_D7C[5] = { { 7350.0f, 0.0f, 0.0f }, { 8650.0f, 0.0f, 0.0f }, { 20350.0f, 0.0f, -300.0f }, { 23850.0f, 0.0f, 0.0f }, { 46950.0f, 0.0f, 0.0f } };
MGACTOR_PARAM lbl_1_data_DB8 = { 100.0f, 160.0f, 13, 5, fn_1_12388, 32, 0, 0 };
Point3d lbl_1_data_DD8[10] = { { 25450.0f, 0.0f, 0.0f }, { 25750.0f, 0.0f, 0.0f }, { 26050.0f, 0.0f, 0.0f }, { 32550.0f, 0.0f, -300.0f }, { 30650.0f, 0.0f, 0.0f }, { 30950.0f, 0.0f, 0.0f }, { 31750.0f, 0.0f, 0.0f }, { 32550.0f, 0.0f, 300.0f }, { 38150.0f, 0.0f, 0.0f }, { 38450.0f, 0.0f, 0.0f } };
MGACTOR_PARAM lbl_1_data_E50 = { 2000.0f, 149.9f, 12, 5, fn_1_12390, 32, 0, 0 };
Point3d lbl_1_data_E70[9] = { { 20050.0f, 0.0f, 0.0f }, { 21150.0f, 0.0f, 0.0f }, { 20850.0f, 0.0f, 300.0f }, { 24150.0f, 0.0f, 300.0f }, { 28050.0f, 0.0f, 300.0f }, { 36950.0f, 0.0f, -300.0f }, { 35650.0f, 0.0f, 0.0f }, { 36950.0f, 0.0f, 0.0f }, { 35650.0f, 0.0f, 300.0f } };
MGACTOR_PARAM lbl_1_data_EDC = { 80.0f, 50.0f, 102, 5, fn_1_124FC, 34, 0, 0 };
Point3d lbl_1_data_EFC[4] = { { 17950.0f, 0.0f, 0.0f }, { 33450.0f, 0.0f, 0.0f }, { 34550.0f, 0.0f, 0.0f }, { 43350.0f, 0.0f, 0.0f } };
Point3d lbl_1_data_F2C[6] = { { 2130.0f, 150.0f, -750.0f }, { 13000.0f, 530.0f, -850.0f }, { 17000.0f, 180.0f, -600.0f }, { 22200.0f, 100.0f, -700.0f }, { 47300.0f, 100.0f, -700.0f }, { 50100.0f, 100.0f, -600.0f } };
Point3d lbl_1_data_F74 = { 45400.0f, 0.0f, 0.0f };
Point3d lbl_1_data_F80 = { 44850.0f, -18.5f, -300.0f };
OM_CAMERA_VIEW lbl_1_data_F8C = { { 10140.0f, 302.18f, 148.0f }, { -21.62f, -50.0f, 0.0f }, 3350.0f };
Point3d lbl_1_data_FA8[4] = { { 9700.0f, 0.0f, 0.0f }, { 9600.0f, 0.0f, -300.0f }, { 9800.0f, 0.0f, -300.0f }, { 10000.0f, 0.0f, -300.0f } };
Point3d lbl_1_data_FD8[4] = { { 9950.0f, 0.0f, 0.0f }, { 10150.0f, 0.0f, -300.0f }, { 10150.0f, 0.0f, 0.0f }, { 10150.0f, 0.0f, 300.0f } };

s32 fn_1_120E0(void *playerContext, void *eventTarget) { return 0; }

s32 fn_1_120E8(MGACTOR *actor)
{
    Point3d effectPosition;
    Point3d soundPosition;
    MikeActPlayerRecord *player;
    OMOBJ *effectObject;
    s16 modelIndex;
    s16 effectIndex;

    if (actor->param >= 10) {
        return;
    }
    player = lbl_1_bss_C4[actor->param];
    if ((actor->colGroundAttr & 4) != 0) {
        if (fn_1_45D8(player, 0x800) == 0) {
            fn_8005F77C(player->character_no, 4);
            fn_1_45B0(player, 0x800);
        }
        if (lbl_1_bss_30 > player->lastEffectTick + 0x1E && player->movement_speed > 5.5f) {
            player->lastEffectTick = lbl_1_bss_30;
            fn_80086BC8(actor, &effectPosition);
            modelIndex = fn_80036EBC(lbl_1_bss_33F0);
            fn_80026A44(modelIndex, 1U);
            fn_80026A44(modelIndex, 0x40000002U);
            fn_80026790(modelIndex, &effectPosition);
            fn_800269E8(modelIndex, 0x08000000U);
            fn_80026B8C(modelIndex, 7);
        }
    } else if (fn_1_45D8(player, 0x800) != 0) {
        fn_8005F77C(player->character_no, 0);
        fn_1_45C4(player, 0x800);
    }

    if ((actor->colGroundAttr & 0x210) != 0 && fn_1_45D8(player, 4) != 0) {
        effectIndex = fn_1_1110C((s16)actor->colMesh);
        if (effectIndex != -1) {
            fn_1_10FB4(effectIndex);
            fn_80026A44(lbl_1_bss_1B8[effectIndex].model_id, 0x40000002U);
            effectObject = fn_8003AB38(lbl_1_bss_6C, 0x2300, 0, 0, -1, fn_1_12690);
            effectObject->work[0] = effectIndex;
            fn_800267CC(lbl_1_bss_1B8[effectIndex].model_id, &effectPosition);
            fn_80086BC8(actor, &soundPosition);
            if (effectPosition.x - soundPosition.x > 0.0f) {
                soundPosition.x += 50.0f;
            } else {
                soundPosition.x -= 50.0f;
            }
            fn_1_30A0(&soundPosition, 0x547);
            fn_1_1277C(&soundPosition);
        }
    }
}

int fn_1_12380(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b) { return 0; }

int fn_1_12388(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b) { return 0; }

int fn_1_12390(COL_NARROW_PARAM *effectRequest, COL_NARROW_PARAM *playerRequest)
{
    HuVecF position;
    s16 playerNo;
    OMOBJ *effectObject;

    if (playerRequest->paramB >= 0 && playerRequest->paramB < 4 &&
        fn_1_45D8(lbl_1_bss_C4[playerRequest->paramB], 4) != 0) {
        playerNo = fn_1_1106C(effectRequest->paramA);
        if (playerNo != -1) {
            fn_1_10FB4(playerNo);
            fn_80026A44(
                lbl_1_bss_1B8[playerNo].model_id, 0x40000002U);
            effectObject = fn_8003AB38(lbl_1_bss_6C, 0x2300, 0U, 0U, -1, fn_1_12690);
            effectObject->work[0] = (u32)playerNo;
            PSVECSubtract(&effectRequest->point,
                          &playerRequest->point, &position);
            position.y = 0.0f;
            position.z = 0.0f;
            PSVECScale(&position, &position, 0.5f);
            PSVECAdd(&playerRequest->point, &position, &position);
            fn_1_30A0(&position, 0x546);
            fn_1_1277C(&position);
        }
    }
    return 0;
}

int fn_1_124FC(COL_NARROW_PARAM *event,
               COL_NARROW_PARAM *playerContext)
{
    HuVecF playerPosition;
    HuVecF statePosition;
    s16 playerNo;

    if (playerContext->paramB >= 0 && playerContext->paramB < 4) {
        playerNo = fn_1_1106C(event->paramA);
        if (lbl_1_bss_1B8[playerNo].event_state != 0) {
            return 0;
        }
        if (fn_1_45D8(lbl_1_bss_C4[playerContext->paramB], 4) != 0) {
            lbl_1_bss_1B8[playerNo].event_state = 1;
            fn_80086BC8(lbl_1_bss_1B8[playerNo].actor, &statePosition);
            fn_80086BC8(lbl_1_bss_C4[playerContext->paramB]->player->actor,
                          &playerPosition);
            lbl_1_bss_1B8[playerNo].callback_state =
                (s32)lbl_1_bss_C4[playerContext->paramB]->movement_angle;
        } else {
            fn_1_4A70(lbl_1_bss_C4[playerContext->paramB]);
            lbl_1_bss_1B8[playerNo].event_state = 2;
        }
    }

    return 0;
}

void fn_1_12690(OMOBJ *object)
{
    s16 modelId;

    switch (object->work[1]) {
    case 0:
        object->work[2]++;
        if ((u32)object->work[2] == 0x1E) {
            object->work[1] = 1;
            object->work[2] = 0;
        }
        break;
    case 1:
        object->work[2]++;
        modelId = lbl_1_bss_1B8[object->work[0]].model_id;
        if ((u32)(object->work[2] & 1) != 0) {
            fn_800269E8(modelId, 1U);
        } else {
            fn_80026A44(modelId, 1U);
        }
        if ((u32)object->work[2] == 0x1E) {
            fn_1_10E64((s16)object->work[0]);
            omDelObjEx(fn_8000DD9C(), object);
        }
        break;
    }
}

void fn_1_1277C(HuVecF *position)
{
    HU3D_MODELID model = fn_80036EBC(lbl_1_bss_33EE);
    MikeActEffectModelSetup(model, position);
}

void fn_1_12804(void)
{
    s16 linkedModelA;
    s16 linkedModelB;
    s16 modelA;
    s16 modelB;
    s32 index;

    lbl_1_bss_33FC = fn_800286D0(&lbl_1_data_C68, &lbl_1_data_C74,
                                       &lbl_1_data_C80);
    Hu3DGLightStaticSet(lbl_1_bss_33FC, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_33FC);
    fn_1_111A8(lbl_1_data_C84, 8);
    fn_1_11690(&lbl_1_data_CE4, 1);

    for (index = 0; index < 8; index++) {
        if (index == 0) {
            linkedModelA = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 38), 0x10000000, 2));
            linkedModelB = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 39), 0x10000000, 2));
            modelA = linkedModelA;
            modelB = linkedModelB;
        } else {
            modelA = fn_80025D4C(linkedModelA);
            modelB = fn_80025D4C(linkedModelB);
        }
        fn_80026B8C(modelA, 6);
        fn_1_10AE0(modelA, modelB, &lbl_1_data_CF0[index]);
    }

    for (index = 0; index < 5; index++) {
        if (index == 0) {
            linkedModelA = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 7), 0x10000000, 2));
            modelA = linkedModelA;
        } else {
            modelA = fn_80025D4C(linkedModelA);
        }
        fn_80026760(modelA, lbl_1_data_D7C[index].x - 240.0f,
                        lbl_1_data_D7C[index].y, lbl_1_data_D7C[index].z);
        fn_1_107EC(modelA);
    }

    for (index = 0; index < 4; index++) {
        if (index == 0) {
            linkedModelA = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 36), 0x10000000, 2));
            linkedModelB = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 37), 0x10000000, 2));
            modelA = linkedModelA;
            modelB = linkedModelB;
        } else {
            modelA = fn_80025D4C(linkedModelA);
            modelB = fn_80025D4C(linkedModelB);
        }
        fn_80026FFC(modelA);
        fn_800269E8(modelA, ((1 << 30) | (1 << 1)));
        fn_80026B8C(modelA, 6);
        fn_1_10AE0(modelA, modelB, &lbl_1_data_EFC[index]);
    }

    modelA = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 28), 0x10000000, 2));
    fn_80026B8C(modelA, 4);
    lbl_1_bss_33C8 = fn_1_10CFC(modelA, &lbl_1_data_D50);
    modelA = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 30), 0x10000000, 2));
    lbl_1_bss_33EC = fn_1_107EC(modelA);
    modelA = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 33), 0x10000000, 2));
    fn_80026FFC(modelA);
    fn_1_10CFC(modelA, &lbl_1_data_F74);
    lbl_1_bss_33D4 = modelA;
    lbl_1_bss_33D0 = 315.0f;
    fn_80026808(modelA, 0.0f, lbl_1_bss_33D0, 0.0f);
    modelA = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 34), 0x10000000, 2));
    fn_80026FFC(modelA);
    modelB = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 35), 0x10000000, 2));
    lbl_1_bss_33DC = fn_1_10AE0(modelA, modelB, &lbl_1_data_F80);
    lbl_1_bss_33D8 = 0.0f;
    lbl_1_bss_33F8 = HuSprAnimRead(
        fn_80008DD0(EFFECT_ANM_dust, 0x10000000, 2));
    HuSprAnimLock(lbl_1_bss_33F8);
    lbl_1_bss_33F0 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 40), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_33F0, (1 << 0));
    fn_800269E8(lbl_1_bss_33F0, ((1 << 30) | (1 << 1)));
    lbl_1_bss_33EE = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 3), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_33EE, (1 << 0));
    fn_800269E8(lbl_1_bss_33EE, ((1 << 30) | (1 << 1)));
    fn_800269E8(lbl_1_bss_33EE, (1 << 22));
    lbl_1_bss_33CC = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 31), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_33CC, (1 << 0));
    fn_800269E8(lbl_1_bss_33CC, ((1 << 30) | (1 << 1)));
    fn_80026B8C(lbl_1_bss_33CC, 7);
    fn_800269E8(lbl_1_bss_33CC, ((1 << 30) | (1 << 0)));
    fn_8002D730(lbl_1_bss_33CC, 0.0f, 45.0f);
    lbl_1_bss_33CA = fn_80036DF8(
        fn_80008DD0(DATANUM(0xa70000, 32), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_33CA, (1 << 27));
    fn_800269E8(lbl_1_bss_33CA, (1 << 0));
    fn_800269E8(lbl_1_bss_33CA, ((1 << 30) | (1 << 1)));
    fn_80026B8C(lbl_1_bss_33CA, 7);
    lbl_1_bss_33EA = 0;
    lbl_1_bss_33E8 = 0;
    MikeActPreActorCallbackSet(fn_1_13A8C);
    MikeActPostActorCallbackSet(fn_1_13AD8);
}

void fn_1_12E48(void)
{
    COL_ATTRPARAM collisionParams;
    MGACTOR *actor;
    s16 linkedModelId;
    s32 index;
    s16 modelId;

    modelId = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 0), 0x10000000, 2));
    fn_800268B0(modelId, 0.1f, 0.1f, 0.1f);
    fn_800269E8(modelId, (1 << 0));
    lbl_1_bss_33C4 = fn_80080B04(&lbl_1_data_D5C, modelId);
    fn_80086AA8(lbl_1_bss_33C4, &lbl_1_data_D50);
    fn_80080DD4(lbl_1_bss_33C4, 4U);

    for (index = 0; index < 10; index++) {
        if (index == 0) {
            linkedModelId = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 24), 0x10000000, 2));
            fn_800269E8(linkedModelId, ((1 << 30) | (1 << 1)));
            modelId = linkedModelId;
        } else {
            modelId = fn_80025D4C(linkedModelId);
        }
        fn_80026B8C(modelId, 7);
        actor = fn_80080B04(&lbl_1_data_E50, modelId);
        fn_80080DD4(actor, 4U);
        fn_1_1092C(actor, &lbl_1_data_DD8[index]);
    }

    for (index = 0; index < 5; index++) {
        if (index == 0) {
            linkedModelId = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 25), 0x10000000, 2));
            fn_800269E8(linkedModelId, ((1 << 30) | (1 << 1)));
            modelId = linkedModelId;
        } else {
            modelId = fn_80025D4C(linkedModelId);
        }
        fn_80026FFC(modelId);
        fn_80026B8C(modelId, 4);
        actor = fn_80080B04(&lbl_1_data_DB8, modelId);
        fn_80080DD4(actor, 4U);
        fn_1_1092C(actor, &lbl_1_data_D7C[index]);
    }

    fn_1_1381C();
    for (index = 0; index < 6; index++) {
        modelId = fn_80025A04(
            fn_80008DD0(DATANUM(0xa70000, 41), 0x10000000, 2));
        fn_1_10CFC(modelId, &lbl_1_data_F2C[index]);
        fn_80026B8C(modelId, 6);
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
    }

    fn_1_4518(fn_1_120E0);
    fn_1_45A0((void (*)(void *, s32))fn_1_120E8);
    lbl_1_bss_33DE = frandmod(3);
    lbl_1_bss_33F6 = fn_1_FC80(lbl_1_bss_33DE + MESSNUM(MESS_MICGOSUPORT, 9));

    fn_800870D4(&collisionParams, 1U);
    collisionParams.yDeviate = 0.0f;
    collisionParams.speed = 1.0f;
    collisionParams.maxDot = 1.0f;
    fn_800870A4(&collisionParams, 1U);
    fn_800870D4(&collisionParams, 4U);
    collisionParams.speed = 0.4f;
    fn_800870A4(&collisionParams, 4U);
    fn_80080D7C(lbl_1_bss_33EC, 1U);
}

void fn_1_131A4(OMOBJ *object)
{
    MikeActRiseActorWork *work;
    Point3d playerPosition;
    Point3d horizontalOffset;
    s32 playerIndex;

    work = object->data;
    switch (work->state) {
        case 0:
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if ((lbl_1_bss_C4[playerIndex]->flags & 2) == 0) {
                    fn_80086BC8(lbl_1_bss_C4[playerIndex]->player->actor,
                                  &playerPosition);
                    PSVECSubtract(&playerPosition, &work->position,
                                  &horizontalOffset);
                    horizontalOffset.y = 0.0f;
                    if (PSVECMag(&horizontalOffset) < 600.0f) {
                        fn_1_30A0(&work->position, 0x542);
                        fn_1_10E64(work->playerIndex);
                        work->linkedModelId = fn_80025D4C(lbl_1_bss_33F2);
                        fn_80026B8C(work->linkedModelId, 6);
                        work->playerIndex = fn_1_10CFC(work->linkedModelId,
                                                      &work->position);
                        fn_80026FFC(work->linkedModelId);
                        work->shadowModelId = fn_1_15818(&work->position);
                        work->shadowTimer = 0;
                        fn_80026A44(work->modelId, (1 << 0));
                        fn_8002C95C(work->modelId, work->alternateMotionId);
                        work->actor = fn_80080B04(&lbl_1_data_EDC,
                                                    work->modelId);
                        work->actor->gravity = 0.0f;
                        fn_80080DD4(work->actor, 4U);
                        work->targetIndex = fn_1_1092C(work->actor,
                                                      &work->position);
                        playerPosition = work->position;
                        playerPosition.y += work->verticalOffset;
                        fn_80026790(work->modelId, &playerPosition);
                        work->state = 1;
                        break;
                    }
                }
            }
            break;
        case 1:
            if (work->verticalOffset < -30.0f) {
                work->verticalOffset += 7.0f;
                playerPosition = work->position;
                playerPosition.y += work->verticalOffset;
                fn_80086AA8(work->actor, &playerPosition);
            } else if (fn_8002D75C(work->modelId) != 0) {
                fn_8002C95C(work->modelId, work->motionId);
                fn_800269E8(work->modelId, ((1 << 30) | (1 << 0)));
                work->state = 2;
            }
            break;
        case 2:
            if (lbl_1_bss_1B8[work->targetIndex].event_state != 0) {
                fn_80026A44(work->linkedModelId, ((1 << 30) | (1 << 1)));
                if (lbl_1_bss_1B8[work->targetIndex].event_state == 1) {
                    fn_1_30A0(&work->position, 0x548);
                    work->state = 3;
                    fn_80026E44(work->modelId);
                } else if (lbl_1_bss_1B8[work->targetIndex].event_state == 2) {
                    work->state = 4;
                    fn_8002C95C(work->modelId, work->alternateMotionId);
                    fn_8002D414(work->modelId, 19.0f);
                    fn_80026A44(work->modelId, ((1 << 30) | (1 << 0)));
                    fn_800269E8(work->modelId, ((1 << 30) | (1 << 2)));
                }
            }
            break;
        case 3: {
            work->position.x += 40.0 * sin(
                (M_PI *
                 (f64)lbl_1_bss_1B8[work->targetIndex].callback_state) /
                180.0);
            work->position.z += 40.0 * cos(
                (M_PI *
                 (f64)lbl_1_bss_1B8[work->targetIndex].callback_state) /
                180.0);
            work->position.y += 40.0f;
            fn_80086AA8(work->actor, &work->position);
            if (work->position.y >= 1200.0f) {
                horizontalOffset.y = 1.0f -
                    (0.04f * ((work->position.y - 1200.0f) / 40.0f));
                if (horizontalOffset.y <= 0.0f) {
                    horizontalOffset.y = 0.0f;
                    work->state = 6;
                }
                fn_800268B0(
                    lbl_1_bss_1B8[work->targetIndex].model_id,
                    horizontalOffset.y, horizontalOffset.y,
                    horizontalOffset.y);
            }
            break;
        }
        case 4:
            if (fn_8002D4E0(work->modelId) <= 10.0f) {
                if (work->shadowModelId == -1) {
                    work->shadowModelId = fn_1_15818(&work->position);
                    work->shadowTimer = 0;
                }
                if (work->verticalOffset > -100.0f) {
                    work->verticalOffset -= 7.0f;
                    playerPosition = work->position;
                    playerPosition.y += work->verticalOffset;
                    fn_80086AA8(work->actor, &playerPosition);
                } else {
                    work->state = 5;
                }
            }
            break;
        case 5:
            if (work->shadowModelId == -1) {
                work->state = 6;
            }
            break;
        case 6:
            fn_1_10E64(work->playerIndex);
            fn_1_10E64(work->targetIndex);
            omDelObjEx(fn_8000DD9C(), object);
            break;
    }
    if (work->shadowModelId != -1) {
        work->shadowTimer++;
        if (work->shadowTimer == 0x28) {
            fn_800262D0(work->shadowModelId);
            work->shadowModelId = -1;
        }
    }
}

void fn_1_1381C(void)
{
    s32 index;
    s16 modelId;
    OMOBJ *object;
    MikeActRiseActorWork *work;

    lbl_1_bss_33F4 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 26), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_33F4, (1 << 0));
    fn_800269E8(lbl_1_bss_33F4, (1 << 14));
    lbl_1_bss_33F2 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 27), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_33F2, (1 << 0));
    fn_800269E8(lbl_1_bss_33F2, (1 << 14));
    fn_800269E8(lbl_1_bss_33F2, ((1 << 30) | (1 << 1)));

    for (index = 0; index < 9; index++) {
        object = fn_8003AB38(lbl_1_bss_6C, 0x1F00, 0, 0, -1, fn_1_131A4);
        object->data = HuMemDirectMallocNum(HEAP_HEAP, 0x28,
                                             0x10000000);
        work = object->data;
        memset(work, 0, sizeof(*work));
        work->shadowModelId = -1;
        work->position = lbl_1_data_E70[index];
        modelId = fn_80025D4C(lbl_1_bss_33F4);
        fn_80026B8C(modelId, 4);
        work->linkedModelId = modelId;
        work->playerIndex = fn_1_10CFC(modelId, &work->position);

        modelId = fn_80025A04(
            fn_80008DD0(DATANUM(0xa70000, 42), 0x10000000, 2));
        fn_800269E8(modelId, (1 << 0));
        fn_800269E8(modelId, (1 << 14));
        fn_80026B8C(modelId, 4);
        work->verticalOffset = -100.0f;
        fn_80026760(modelId, work->position.x,
                        work->position.y + work->verticalOffset,
                        work->position.z);
        work->alternateMotionId = fn_80030458(
            modelId,
            fn_80008DD0(DATANUM(0xa70000, 43), 0x10000000, 2));
        work->motionId = fn_80030458(
            modelId,
            fn_80008DD0(DATANUM(0xa70000, 44), 0x10000000, 2));
        work->modelId = modelId;
    }
}

void fn_1_13A8C(void)
{
    switch (lbl_1_bss_33EA) {
    case 0:
        fn_1_13F40();
        break;
    case 1:
        fn_1_14C14();
        break;
    }
}

void fn_1_13AD8(void)
{
    Point3d modelPosition;
    Point3d position;

    if (lbl_1_bss_33EA == 0 && lbl_1_bss_33E8 >= 10) {
        fn_1_10F38(lbl_1_bss_33C8, &modelPosition);
        fn_80086BC8(lbl_1_bss_33C4, &position);
        position.y = 150.0f + position.y;
        position.z = 0.0f;
        fn_1_10F68(lbl_1_bss_33C8, &position);
        fn_80026790(lbl_1_bss_1B8[lbl_1_bss_33C8].model_id, &position);
        position.x -= modelPosition.x;
        fn_80026874(lbl_1_bss_1B8[lbl_1_bss_33C8].model_id, &modelPosition);
        modelPosition.z = (f32)((f64)modelPosition.z - 0.38197186342054884 * (f64)position.x);
        fn_80026838(lbl_1_bss_1B8[lbl_1_bss_33C8].model_id, &modelPosition);
    }
}

s32 fn_1_13C2C(void)
{
    s16 effectState;
    s32 playerNo;
    Point3d actorPosition;
    Point3d push;

    effectState = lbl_1_bss_33E0;
    if (effectState == 0 && lbl_1_bss_33E8 >= 40) {
        effectState = 1;
    }

    push.x = push.y = 0.0f;
    if (lbl_1_bss_33E8 == 30 || lbl_1_bss_33E8 == 31) {
        push.z = 20.0f;
    } else {
        push.z = 0.5f * (f32)effectState;
    }

    for (playerNo = 0; playerNo < 4; playerNo++) {
        fn_80086C8C(lbl_1_bss_C4[playerNo]->player->actor, &push);
    }
    fn_80086BC8(lbl_1_bss_C4[2]->player->actor, &push);
    fn_80086BC8(lbl_1_bss_33C4, &actorPosition);
    actorPosition.x = 200.0f + push.x;
    if (actorPosition.x >= 15150.0) {
        actorPosition.x = 15150.0f;
    }
    fn_80086B04(lbl_1_bss_33C4, &actorPosition);

    lbl_80266A4C.y = push.y + (lbl_1_data_F8C.center.y - lbl_1_data_FD8[2].y);
    lbl_80266A4C.x = push.x + (lbl_1_data_F8C.center.x - lbl_1_data_FD8[2].x);

    if (lbl_1_bss_33E8 == 30 || lbl_1_bss_33E8 == 31) {
        fn_80026A44(lbl_1_bss_33CC, (1 << 0));
        fn_80026A44(lbl_1_bss_33CC, ((1 << 30) | (1 << 1)));
        fn_80026790(lbl_1_bss_33CC, &actorPosition);
    }
    if (actorPosition.x >= 15050.0) {
        fn_1_3128(lbl_1_bss_33C0);
        lbl_1_bss_33E8 = 50;
        fn_8002D414(lbl_1_bss_33CC, 46.0f);
        fn_8002D730(lbl_1_bss_33CC, 46.0f, 75.0f);
        fn_80026A44(lbl_1_bss_33CC, ((1 << 30) | (1 << 0)));
        return 1;
    }
    return 0;
}

void fn_1_13F40(void)
{
    s32 playerNo;
    Point3d position;

    if (lbl_1_bss_B8 != NULL) {
    switch (lbl_1_bss_33E8) {
    case 0:
        if (lbl_1_bss_F0->position.x >
            lbl_1_data_FA8[0].x - 150.0f) {
            fn_1_49D0(lbl_1_bss_B8);
            if ((lbl_1_bss_B8->player->actor->colGroundAttr & 0x407FU) != 0) {
                fn_1_45B0(lbl_1_bss_B8, 0x2000);
                fn_8007E818(lbl_1_bss_B8->player);
                fn_1_EA08(0);
                fn_1_1BF4();
                lbl_1_bss_33E8 = 1;
            }
        }
        break;
    case 1:
        fn_1_665C(lbl_1_bss_B8, 1);
        if (fn_8007E8B4(lbl_1_bss_B8->player, &lbl_1_data_FA8[0],
                             5.0f, 5.0f) != 0) {
            /* The original call passes this record where the engine function takes a player. */
            fn_8007EEBC((MGPLAYER *)lbl_1_bss_B8, 0);
            lbl_1_bss_B8->movement_angle = 90.0f;
            fn_80086CA8(lbl_1_bss_B8->player->actor,
                           lbl_1_bss_B8->movement_angle);
            fn_1_F5BC();
            lbl_1_bss_33E8 = 2;
        }
        break;
    case 2:
        if (fn_1_1F70() != 0) {
            fn_1_4C84(&lbl_1_data_FA8[1]);
            fn_1_1BF4();
            lbl_1_bss_33E4 = 0;
            lbl_1_bss_33E8 = 3;
        }
        break;
    case 3:
        if (lbl_1_bss_33E4++ == 0xF0U) {
            lbl_1_bss_33E4 = 0;
            lbl_1_bss_33E8 = 4;
        }
        break;
    case 4:
        for (playerNo = 1; playerNo < 4; playerNo++) {
            fn_8007E8B4(lbl_1_bss_C4[playerNo]->player,
                             &lbl_1_data_FD8[playerNo], 5.0f, 5.0f);
        }
        if (lbl_1_bss_33E4++ == 0xFU) {
            WipeCreate(2, 0, 0x1E);
            lbl_1_bss_33E8 = 5;
        }
        break;
    case 5:
        if (WipeCheck() != 0) {
            for (playerNo = 1; playerNo < 4; playerNo++) {
                fn_8007E8B4(lbl_1_bss_C4[playerNo]->player,
                                 &lbl_1_data_FD8[playerNo], 5.0f, 5.0f);
            }
            break;
        }
        for (playerNo = 1; playerNo < 4; playerNo++) {
            fn_8008697C(lbl_1_bss_C4[playerNo]->player,
                           &lbl_1_data_FD8[playerNo]);
            lbl_1_bss_C4[playerNo]->movement_angle = 90.0f;
            fn_80086CA8(lbl_1_bss_C4[playerNo]->player->actor, 90.0f);
            fn_8007EEBC(lbl_1_bss_C4[playerNo]->player, 0xA);
            fn_1_665C(lbl_1_bss_C4[playerNo], 0x10);
            fn_800805CC(lbl_1_bss_C4[playerNo]->player->actor, 0x200U);
        }
        fn_800805CC(lbl_1_bss_B8->player->actor, 0x200U);
        lbl_1_bss_1AA = 5;
        fn_1_36E4(1);
        fn_8003E5A4(&lbl_1_data_F8C);
        fn_1_EACC();
        fn_1_F370(MESSNUM(MESS_MICGO, 15), 0);
        fn_1_1B40(1, fn_1_14970);
        fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 8), 1);
        lbl_1_bss_33E0 = 0;
        WipeCreate(1, 0, 0x1E);
        lbl_1_bss_33E8 = 6;
        break;
    case 6:
        if (WipeCheck() == 0) {
            lbl_1_bss_33E8 = 0xA;
        }
        break;
    case 0xA:
        if (fn_1_13C2C() == 0 && lbl_1_bss_33E0 == 0xA) {
            fn_1_1E1C(lbl_1_bss_33DE + MESSNUM(MESS_MICGOSUPORT, 9), 1);
            fn_1_FE94(1);
            lbl_1_bss_33E4 = 0;
            fn_1_F370(MESSNUM(MESS_MICGO, 12), 0);
            lbl_1_bss_33E8++;
        }
        break;
    case 0xB:
        if (fn_1_13C2C() == 0 && lbl_1_bss_33E4++ == 0x14U) {
            fn_1_F148();
            lbl_1_bss_33E8 = 0x14;
        }
        break;
    case 0x14:
        if (fn_1_13C2C() == 0) {
            lbl_1_bss_33E0 = fn_1_F138();
            if (lbl_1_bss_33E0 == 0) {
                fn_1_F188();
                fn_1_F370(MESSNUM(MESS_MICGO, 14), 0xB4);
                fn_1_FE94(0);
                lbl_1_bss_33E4 = 0;
                lbl_1_bss_33E8 = 0x28;
                fn_1_30E4(0x53E);
                fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 8), 1);
            }
        }
        break;
    case 0x1E:
        if (fn_1_13C2C() == 0) {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (playerNo != 0) {
                    CharFXPlay(lbl_1_bss_C4[playerNo]->character_no, 0x243);
                }
                fn_8005C390(lbl_1_bss_C4[playerNo]->character_no, 2.0f);
            }
            HuAudFXPitchSet(lbl_1_bss_33C0, 0x1FFF);
            fn_1_30E4(0x53D);
            fn_1_1BF4();
            fn_1_F188();
            fn_1_FE94(0);
            fn_1_F370(MESSNUM(MESS_MICGO, 13), 0);
            lbl_1_bss_33E8++;
        }
        break;
    case 0x1F:
        fn_1_13C2C();
        break;
    case 0x28:
        if (fn_1_13C2C() == 0 && lbl_1_bss_33E4++ == 0xB4U) {
            fn_1_F370(MESSNUM(MESS_MICGO, 15), 0);
        }
        break;
    case 0x32:
        for (playerNo = 1; playerNo < 4; playerNo++) {
            fn_8007EEBC(lbl_1_bss_C4[playerNo]->player, 0);
            fn_8008062C(lbl_1_bss_C4[playerNo]->player->actor, 0x200U);
        }
        for (playerNo = 0; playerNo < 4; playerNo++) {
            fn_8005C390(lbl_1_bss_C4[playerNo]->character_no, 1.0f);
        }
        fn_8008062C(lbl_1_bss_B8->player->actor, 0x200U);
        fn_1_1BF4();
        fn_1_F188();
        fn_1_FE94(0);
        lbl_1_bss_33E4 = 0;
        lbl_1_bss_33E8++;
        break;
    case 0x33:
        fn_80086BC8(lbl_1_bss_33C4, &position);
        if (position.y < 320.0f) {
            fn_1_30A0(&position, 0x544);
            fn_1_30E4(0x53F);
            position.y = 0.0f;
            fn_80026790(lbl_1_bss_33CA, &position);
            fn_80026A44(lbl_1_bss_33CA, (1 << 0));
            fn_80026A44(lbl_1_bss_33CA, ((1 << 30) | (1 << 1)));
            fn_1_35A0(0x1E);
            lbl_1_bss_33E8++;
        }
        break;
    case 0x34:
        if (lbl_1_bss_33E4++ == 0x5AU) {
            WipeCreate(2, 0, 0x1E);
            lbl_1_bss_33E8++;
        }
        break;
    case 0x35:
        if (WipeCheck() == 0) {
            fn_1_14B68();
            WipeCreate(1, 0, 0x1E);
            lbl_1_bss_33E8++;
        }
        break;
    case 0x36:
        if (WipeCheck() == 0) {
            fn_8007E888(lbl_1_bss_B8->player);
            lbl_1_bss_33EA++;
            lbl_1_bss_33E8 = 0;
        }
        break;
    }
    }
}

void fn_1_14970(MCResponse_s *context)
{
    s32 playerNo;
    s32 desiredSelection;
    s16 selection;

    switch (lbl_1_bss_33E8) {
    case 10:
        if (*context->result == 0) {
            if (lbl_1_bss_33E0 == 0) {
                lbl_1_bss_33C0 = fn_1_30A0(
                    &lbl_1_bss_1B8[lbl_1_bss_33C8].position, 0x543);
                for (playerNo = 1; playerNo < 4; playerNo++) {
                    CharFXPlay(lbl_1_bss_C4[playerNo]->character_no, 0x349);
                    fn_8005C47C(lbl_1_bss_C4[playerNo]->character_no,
                                       ((1 << 30) | (1 << 1)));
                }
            }
            fn_1_F0B8();
            lbl_1_bss_33E0++;
        }
        break;
    case 40:
        if (*context->result == 0 && lbl_1_bss_33E0 < 10) {
            lbl_1_bss_33E0++;
        }
        break;
    case 20:
        if (HuMCMicGet() == 2) {
            desiredSelection = 0;
        } else {
            desiredSelection = lbl_1_bss_33DE + 1;
        }
        selection = desiredSelection;
        if (*context->result == selection) {
            lbl_1_bss_33E8 = 30;
        }
        break;
    }
}

void fn_1_14B48(s16 mode, s16 frameNo)
{
    fn_1_1F94();
}

/* Reset the selection display and hide the other players during the guide sequence. */
void fn_1_14B68(void) {
    s32 playerIndex;

    fn_1_1FD4();
    fn_1_EA08(1);
    playerIndex = 1;
    while (playerIndex < 4) {
        fn_80086E3C(lbl_1_bss_C4[playerIndex]->player);
        fn_1_45B0(lbl_1_bss_C4[playerIndex], 0x2000);
        fn_800269E8(lbl_1_bss_C4[playerIndex]->player->actor->mdlId, 1U);
        playerIndex += 1;
    }
    fn_1_F598();
}

void fn_1_14C14(void)
{
    Point3d targetPosition;

    if (lbl_1_bss_B8 == NULL) {
        return;
    }
    switch (lbl_1_bss_33E8) {
    case 0:
        if (lbl_1_bss_F0->position.x > 44500.0f) {
            fn_1_49D0(lbl_1_bss_B8);
            if ((lbl_1_bss_B8->player->actor->colGroundAttr & 0x407FU) != 0) {
                fn_1_45B0(lbl_1_bss_B8, 0x2000);
                fn_8007E818(lbl_1_bss_B8->player);
                fn_1_10F98(lbl_1_bss_33DC, 0);
                fn_1_1BF4();
                fn_1_665C(lbl_1_bss_B8, 1);
                lbl_1_bss_33E8 = 1;
            }
        }
        break;
    case 1:
        targetPosition = lbl_1_data_F80;
        targetPosition.y = 0.0f;
        if (fn_8007E8B4(lbl_1_bss_B8->player, &targetPosition,
                             5.0f, 200.0f) != 0) {
            fn_8007EEBC(lbl_1_bss_B8->player, 1);
            lbl_1_bss_33E8 = 2;
        }
        break;
    case 2:
        targetPosition = lbl_1_data_F80;
        targetPosition.y = 0.0f;
        if (fn_8007E8B4(lbl_1_bss_B8->player, &targetPosition,
                             5.0f, 5.0f) != 0) {
            lbl_1_bss_B8->movement_angle = 90.0f;
            fn_80086CA8(lbl_1_bss_B8->player->actor,
                           lbl_1_bss_B8->movement_angle);
            fn_1_1B40(1, fn_1_151D4);
            fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 12), 1);
            fn_1_F370(MESSNUM(MESS_MICGO, 18), 0);
            fn_1_45B0(lbl_1_bss_B8, 0x2000);
            fn_1_665C(lbl_1_bss_B8, 0);
            fn_8007EEBC(lbl_1_bss_B8->player, 0xA);
            lbl_1_bss_33E8 = 3;
        }
        break;
    case 3:
        lbl_1_bss_33D0 = (f32)((f64)lbl_1_bss_33D0 -
                               0.1 * (f64)lbl_1_bss_B8->speedIndex);
        lbl_1_bss_33D8 += (f32)lbl_1_bss_B8->speedIndex;
        if (lbl_1_bss_33D0 <= 180.0f) {
            fn_1_F598();
            lbl_1_bss_33D0 = 180.0f;
            fn_1_1BF4();
            fn_1_3128(lbl_1_bss_33C0);
            lbl_1_bss_33E8 = 4;
        }
        fn_80026808(lbl_1_bss_33D4, 0.0f, lbl_1_bss_33D0, 0.0f);
        fn_80026808(lbl_1_bss_1B8[lbl_1_bss_33DC].model_id,
                        0.0f, 0.0f, lbl_1_bss_33D8);
        break;
    case 4:
        targetPosition = lbl_1_data_F80;
        targetPosition.y = 0.0f;
        targetPosition.z = 0.0f;
        if (fn_8007E8B4(lbl_1_bss_B8->player, &targetPosition,
                             5.0f, 5.0f) != 0) {
            fn_1_49D0(lbl_1_bss_B8);
            fn_1_45C4(lbl_1_bss_B8, 0x2000);
            fn_8007EEBC(lbl_1_bss_B8->player, 0);
            fn_1_10F98(lbl_1_bss_33DC, 7);
            fn_1_1B40(0, NULL);
            fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 1), 6);
            fn_1_30E4(0x53F);
            lbl_1_bss_33E8 = 5;
        }
        break;
    case 5:
        fn_8007E888(lbl_1_bss_B8->player);
        lbl_1_bss_33EA++;
        break;
    }
}

void fn_1_151D4(MCResponse_s *context)
{
    Point3d position;
    s16 microphoneChoice;

    microphoneChoice = HuMCMicGet() == 2 ? 0 : 4;
    if (*context->result == microphoneChoice && lbl_1_bss_B8->speedIndex < 4) {
        if (lbl_1_bss_B8->speedIndex == 0) {
            lbl_1_bss_33C0 = fn_1_30A0(
                &lbl_1_bss_1B8[lbl_1_bss_33C8].position, 0x545);
        }
        lbl_1_bss_B8->speedIndex++;
        fn_80086BC8(lbl_1_bss_B8->player->actor, &position);
        fn_1_30A0(&position, 0x539);
        if (lbl_1_bss_B8->speedIndex == 1) {
            fn_1_665C(lbl_1_bss_B8, 1);
            return;
        }
        fn_1_665C(lbl_1_bss_B8, 2);
    }
}

void fn_1_15328(HU3D_PARTICLE *motionList)
{
    s32 index;
    HU3D_PARTICLE_DATA *motion;

    motion = motionList->data;
    for (index = 0; index < motionList->maxCnt; index++, motion++) {
        motion->vel.x = (f32)sin(
            MTXDegToRad((360.0f * (f32)index /
             (f32)motionList->maxCnt)));
        motion->vel.y = (f32)(0.27240694444444447 *
                                         (10.0f +
                                          (f32)frandmod(15)));
        motion->vel.z = (f32)cos(
            MTXDegToRad((360.0f * (f32)index /
             (f32)motionList->maxCnt)));
        motion->scale = 100.0f;
        motion->pos.x = 100.0f *
            (0.01f *
            ((f32)(s32)(frand() & 0x3F) - 31.0f));
        motion->pos.y = 0.0f;
        motion->pos.z = 100.0f *
            (0.01f *
            ((f32)(s32)(frand() & 0x3F) - 31.0f));
        motion->color.a = 0;
        motion->time = 0;
        motion->parManId = (frand() & 0xF) + 0xF;
        motion->zRot = (f32)(MTXDegToRad(frandmod(0x168)));
    }
}

void fn_1_15610(HU3D_MODEL *model, HU3D_PARTICLE *motionList, Mtx mtx)
{
    s32 index;
    HU3D_PARTICLE_DATA *motion;
    f32 previousVerticalVelocity;
    f32 verticalDisplacement;

    motion = motionList->data;
    for (index = 0; index < motionList->maxCnt; index++, motion++) {
        if (motion->time != 2) {
            if (motion->time == 0) {
                motion->color.a += motion->parManId;
                if (motion->color.a >= 0xDC) {
                    motion->color.a = 0xDC;
                    motion->time++;
                }
            } else if (motion->color.a > 0x20) {
                motion->color.a -=
                    (u8)(0.5f * (f32)motion->parManId);
            } else {
                motion->color.a = 0;
                motion->time++;
            }

            motion->scale += 3.3333333333333335;
            previousVerticalVelocity = motion->vel.y;
            motion->vel.y += -0.27240694444444447;
            verticalDisplacement = 0.5f *
                (previousVerticalVelocity + motion->vel.y);
            motion->pos.x += 0.016666666666666666 *
                (100.0f * (2.0f * motion->vel.x));
            motion->pos.y += verticalDisplacement;
            motion->pos.z += 0.016666666666666666 *
                (100.0f * (2.0f * motion->vel.z));
        }
    }
}

s16 fn_1_15818(Point3d *position)
{
    s16 modelId;
    HU3D_PARTICLE *particleData;
    HU3D_PARTICLE_DATA *particle;
    s32 particleIndex;

    modelId = fn_8003216C(lbl_1_bss_33F8, 6);
    particleData = lbl_802C01C8[modelId].hookData;
    fn_800326D0(modelId, fn_1_15610);
    fn_800325B0(modelId, 0xF0U, 0xE0U, 0x80U);
    fn_80032620(modelId, 0.0f);
    fn_8003269C(modelId, 0U);
    fn_80026B8C(modelId, 7);
    fn_80026790(modelId, position);

    particle = particleData->data;
    for (particleIndex = 0; particleIndex < particleData->maxCnt;
         particleIndex++, particle++) {
        particle->vel.x = (f32)sin((f64)(
            MTXDegToRad(((360.0f * (f32)particleIndex) /
                            (f32)particleData->maxCnt))));
        particle->vel.y = (f32)(0.27240694444444447 * (f64)(
            10.0f + (f32)frandmod(0xF)));
        particle->vel.z = (f32)cos((f64)(
            MTXDegToRad(((360.0f * (f32)particleIndex) /
                            (f32)particleData->maxCnt))));
        particle->scale = 100.0f;
        particle->pos.x = (f32)(100.0f *
            (0.01f * ((f32)(s32)(frand() & 0x3F) - 31.0f)));
        particle->pos.y = 0.0f;
        particle->pos.z = (f32)(100.0f *
            (0.01f * ((f32)(s32)(frand() & 0x3F) - 31.0f)));
        particle->color.a = 0;
        particle->time = 0;
        particle->parManId = (s16)((frand() & 0xF) + 0xF);
        particle->zRot = (f32)(MTXDegToRad((f32)frandmod(0x168)));
    }
    return modelId;
}

s16 lbl_1_bss_33FC;
ANIMDATA *lbl_1_bss_33F8;
s16 lbl_1_bss_33F6;
s16 lbl_1_bss_33F4;
s16 lbl_1_bss_33F2;
s16 lbl_1_bss_33F0;
s16 lbl_1_bss_33EE;
s16 lbl_1_bss_33EC;
s16 lbl_1_bss_33EA;
s16 lbl_1_bss_33E8;
u32 lbl_1_bss_33E4;
s16 lbl_1_bss_33E0;
s16 lbl_1_bss_33DE;
s16 lbl_1_bss_33DC;
f32 lbl_1_bss_33D8;
s16 lbl_1_bss_33D4;
f32 lbl_1_bss_33D0;
s16 lbl_1_bss_33CC;
s16 lbl_1_bss_33CA;
s16 lbl_1_bss_33C8;
MGACTOR *lbl_1_bss_33C4;
u32 lbl_1_bss_33C0;
