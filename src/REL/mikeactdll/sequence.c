/* Stage sequences, player transitions, and linked particle effects. */
#include "REL/mikeactdll/sequence.h"
#include "REL/mikeactdll/microphone_init.h"
#include "REL/mikeactdll/stage_collision.h"
#include "REL/mikeactdll/sound.h"
#include "REL/mikeactdll/sequence_context.h"
#include "game/audio.h"
#include "string.h"

#include "math.h"

#include "REL/mikeactdll/module_types.h"

#include "REL/mikeactdll/mic_selection_context.h"

#include "datanum/effect.h"

#include "messdir_enum.h"

#include "game/mic.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "REL/mikeactdll/effect_spawner.h"

#include "game/memory.h"

#include "game/object.h"

#include "game/frand.h"

#include "REL/mikeactdll/visibility_target.h"

#include "game/process.h"

#include <stdlib.h>

typedef struct {
    s16 positionIndexA;
    s16 positionIndexB;
    s16 hookedModels[4];
    s16 state;
    s16 playerIndex;
    s16 frameCount;
    u8 unknown_12[2]; /* No code names this field. */
    f32 rotation;
    f32 targetPosition;
    f32 previousPosition;
    s32 firstSoundHandle;
    s32 secondSoundHandle;
} MikeActEffectController;

extern s16 lbl_1_bss_37D2;

extern s16 lbl_1_bss_37D4;

extern s16 lbl_1_bss_37D6;

extern s16 lbl_1_bss_37D8[3];

extern s16 lbl_1_bss_37DE;

extern OMOBJ *lbl_1_bss_37E0[3];

extern s16 lbl_1_bss_37EC;

extern s16 lbl_1_bss_37EE[2];

extern s16 lbl_1_bss_37F6;

extern s16 lbl_1_bss_37F8;

extern s16 lbl_1_bss_37FA;

extern s16 lbl_1_bss_37FC;

extern s16 lbl_1_bss_37FE;

extern s16 lbl_1_bss_3800;

extern s16 lbl_1_bss_3808;

extern s16 lbl_1_bss_380A;

extern Point3d lbl_1_data_1A68;

extern Point3d lbl_1_data_1A74;

extern GXColor lbl_1_data_1A80;

extern Point3d lbl_1_data_1A84[3];

extern Point3d lbl_1_data_1AA8;

extern Point3d lbl_1_data_1B88[2];

extern Point3d lbl_1_data_1C0C[2];

extern char *lbl_1_data_1C54[4];

extern Point3d lbl_1_data_1CC4;

extern Point3d lbl_1_data_1CD0;

extern char *lbl_1_data_1D88[3];

extern s16 lbl_1_bss_37C8;

extern s16 lbl_1_bss_37F2;

extern s16 lbl_1_bss_37F4;

extern Point3d lbl_1_data_1AB4[15];

extern MGACTOR_PARAM lbl_1_data_1B68;

extern Point3d lbl_1_data_1BA0[2];

extern Point3d lbl_1_data_1BB8[6];

extern s16 lbl_1_data_1C00[6];

extern Point3d lbl_1_data_1E10;

extern Point3d lbl_1_data_1E1C;

extern Point3d lbl_1_data_1C64[4];
extern Point3d lbl_1_data_1C94[4];

extern Point3d lbl_1_data_1CDC[5];
extern OM_CAMERA_VIEW lbl_1_data_1D18;

s16 fn_1_1DC68(HuVecF *position);

void fn_1_1DD38(s16 playerNo, HuVecF *eventPosition, HuVecF *playerPosition);

extern s16 lbl_1_bss_1AA;

extern MikeActPlayerRecord *lbl_1_bss_B8;

void fn_1_1E2F0(void);

void fn_1_1F18C(void);

extern s16 lbl_1_bss_37D0;

extern u32 lbl_1_bss_3804;


extern Point3d lbl_1_data_1D94[4];

extern Point3d lbl_1_data_1DC4[4];

extern OM_CAMERA_VIEW lbl_1_data_1DF4;

extern s16 lbl_1_bss_37C0;

extern s32 lbl_1_bss_37C4;

extern s16 lbl_1_bss_37CA;

extern s16 lbl_1_bss_37CC;

extern s16 lbl_1_bss_37CE;

static inline s16 createLinkedEffectModel(Point3d *position)
{
    s16 modelId;

    modelId = fn_80036EBC(lbl_1_bss_3802);
    fn_80026A44(modelId, (1 << 0));
    fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
    fn_800269E8(modelId, (1 << 27));
    fn_800269E8(modelId, (1 << 22));
    fn_80026B8C(modelId, 7);
    fn_80026790(modelId, position);
    return modelId;
}

extern MikeActPlayerRecord *lbl_1_bss_C4[5];

void fn_1_204F0(HuVecF *position);

extern s32 lbl_1_bss_88;

extern f32 lbl_1_data_42C[3];

void fn_1_21590(MikeActVisibilityTarget *work, s16 useDownHook);

Point3d lbl_1_data_1A68 = { 0.0f, 5000.0f, 0.0f };
Point3d lbl_1_data_1A74 = { 0.1f, -1.0f, -0.4f };
GXColor lbl_1_data_1A80 = { 255, 255, 255, 255 };
Point3d lbl_1_data_1A84[3] = { { 14620.0f, 300.0f, -300.0f }, { 17165.0f, 300.0f, -300.0f }, { 43875.0f, 0.0f, 0.0f } };
Point3d lbl_1_data_1AA8 = { 32875.0f, 300.0f, 0.0f };
Point3d lbl_1_data_1AB4[15] = { { 1950.0f, 0.0f, -300.0f }, { 1150.0f, 0.0f, 300.0f }, { 5130.0f, 300.0f, 0.0f }, { 8367.5f, 300.0f, 0.0f }, { 15630.0f, 0.0f, 0.0f }, { 30130.0f, 300.0f, 0.0f }, { 35130.0f, 0.0f, -300.0f }, { 37125.0f, 0.0f, -300.0f }, { 37380.0f, 0.0f, -300.0f }, { 36125.0f, 0.0f, 0.0f }, { 38625.0f, 0.0f, 0.0f }, { 34630.0f, 0.0f, 300.0f }, { 37875.0f, 0.0f, 300.0f }, { 45135.0f, 300.0f, -300.0f }, { 45135.0f, 300.0f, 300.0f } };
MGACTOR_PARAM lbl_1_data_1B68 = { 180.0f, 100.0f, 12, 5, fn_1_1DEE0, 0, 0, 0 };
Point3d lbl_1_data_1B88[2] = { { 6750.0f, 300.0f, 0.0f }, { 31750.0f, 300.0f, 0.0f } };
Point3d lbl_1_data_1BA0[2] = { { 5627.5f, 300.0f, 0.0f }, { 30627.5f, 300.0f, 0.0f } };
Point3d lbl_1_data_1BB8[6] = { { 17000.0f, 0.0f, 150.0f }, { 18250.0f, 0.0f, 150.0f }, { 39500.0f, 0.0f, 150.0f }, { 40500.0f, 0.0f, -150.0f }, { 41500.0f, 0.0f, 150.0f }, { 42500.0f, 0.0f, -150.0f } };
s16 lbl_1_data_1C00[6] = { 3, 3, 1, 1, 2, 2 };
Point3d lbl_1_data_1C0C[2] = { { 9850.0f, 300.0f, 0.0f }, { 24850.0f, 300.0f, 0.0f } };
char *lbl_1_data_1C54[4] = { "itemhook_w1", "itemhook_w2", "itemhook_w3", "itemhook_w4" };
Point3d lbl_1_data_1C64[4] = { { 20800.0f, 0.0f, 300.0f }, { 20000.0f, 0.0f, -300.0f }, { 20200.0f, 0.0f, -300.0f }, { 20400.0f, 0.0f, -300.0f } };
Point3d lbl_1_data_1C94[4] = { { 20500.0f, 0.0f, 0.0f }, { 20560.0f, 20.0f, -230.0f }, { 21040.0f, 20.0f, -230.0f }, { 20800.0f, 20.0f, 10.0f } };
Point3d lbl_1_data_1CC4 = { 20800.0f, 0.0f, -220.0f };
Point3d lbl_1_data_1CD0 = { 21500.0f, 0.0f, 0.0f };
Point3d lbl_1_data_1CDC[5] = { { 21500.0f, 0.0f, 0.0f }, { 21200.0f, 0.0f, 300.0f }, { 21900.0f, 0.0f, 270.0f }, { 21850.0f, 0.0f, -180.0f }, { 21400.0f, 0.0f, 300.0f } };
OM_CAMERA_VIEW lbl_1_data_1D18 = { { 21200.0f, 100.0f, -150.0f }, { -30.62f, 0.0f, 0.0f }, 4500.0f };
char *lbl_1_data_1D58[3] = { "itemhook_c1", "itemhook_c2", "itemhook_c3" };
char *lbl_1_data_1D88[3] = { "itemhook_h1", "itemhook_h2", "itemhook_h3" };
Point3d lbl_1_data_1D94[4] = { { 46000.0f, 300.0f, 0.0f }, { 45900.0f, 300.0f, -300.0f }, { 46100.0f, 300.0f, -300.0f }, { 46300.0f, 300.0f, -300.0f } };
Point3d lbl_1_data_1DC4[4] = { { 46150.0f, 300.0f, 0.0f }, { 46400.0f, 300.0f, -300.0f }, { 46400.0f, 300.0f, 0.0f }, { 46400.0f, 300.0f, 300.0f } };
OM_CAMERA_VIEW lbl_1_data_1DF4 = { { 46591.0f, 578.49f, 53.08f }, { -32.62f, -52.0f, 0.0f }, 3716.0f };
Point3d lbl_1_data_1E10 = { 46575.0f, 0.0f, 0.0f };
Point3d lbl_1_data_1E1C = { 46500.0f, 0.0f, 0.0f };

void fn_1_1CDEC(void)
{
    s32 hookIndex;
    s32 playerIndex;
    MikeActEffectController *work;
    s16 lightId;
    s16 mainModel;
    s16 otherModel;
    s16 linkedModel;
    s16 attachedModel;
    OMOBJ *object;
    s16 effectModel;
    s32 modelIndex;

    lightId = fn_800286D0(&lbl_1_data_1A68, &lbl_1_data_1A74,
                                &lbl_1_data_1A80);
    Hu3DGLightStaticSet(lightId, 1);
    Hu3DGLightInfinitytSet(lightId);
    fn_1_111A8(lbl_1_data_1A84, 3);
    fn_1_11690(&lbl_1_data_1AA8, 1);

    mainModel = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 102), 0x10000000, 2));
    fn_80026B8C(mainModel, 4);
    otherModel = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 106), 0x10000000, 2));
    lbl_1_bss_37DE = fn_1_10AE0(mainModel, otherModel, &lbl_1_data_1CC4);

    for (modelIndex = 0; modelIndex < 3; modelIndex++) {
        effectModel = fn_80025A04(
            fn_80008DD0(DATANUM(0xa70000, 103), 0x10000000, 2));
        fn_8002741C(mainModel, lbl_1_data_1D88[modelIndex], effectModel);
        fn_800269E8(effectModel, ((1 << 30) | (1 << 1)));
        fn_800269E8(effectModel, ((1 << 30) | (1 << 2)));
        fn_8002D730(effectModel, 70.0f, 100.0f);
        fn_8002D414(effectModel, 100.0f);
        lbl_1_bss_37D8[modelIndex] = effectModel;
    }

    mainModel = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 91), 0x10000000, 2));
    fn_80026B8C(mainModel, 6);
    fn_800269E8(mainModel, ((1 << 30) | (1 << 0)));
    fn_8002D730(mainModel, 0.0f, 120.0f);
    fn_1_10CFC(mainModel, &lbl_1_data_1CD0);
    lbl_1_bss_37D2 = mainModel;

    lbl_1_bss_37D6 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 104), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_37D6, (1 << 0));
    fn_800269E8(lbl_1_bss_37D6, ((1 << 30) | (1 << 1)));
    lbl_1_bss_37D4 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 105), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_37D4, (1 << 0));
    fn_800269E8(lbl_1_bss_37D4, ((1 << 30) | (1 << 1)));

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        if (playerIndex == 0) {
            linkedModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 89), 0x10000000, 2));
            fn_800269E8(linkedModel, ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 1)));
            attachedModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 90), 0x10000000, 2));
            mainModel = linkedModel;
            otherModel = attachedModel;
        } else {
            mainModel = fn_80025D4C(linkedModel);
            otherModel = fn_80025D4C(attachedModel);
        }
        fn_80026FFC(mainModel);
        lbl_1_bss_37EE[playerIndex] =
            fn_1_10AE0(mainModel, otherModel, &lbl_1_data_1B88[playerIndex]);
    }

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        object = fn_8003AB38(lbl_1_bss_6C, 0x2300, 0, 0, -1, fn_1_20808);
        lbl_1_bss_37E0[playerIndex] = object;
        object->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*work), 0x10000000);
        work = object->data;
        memset(work, 0, sizeof(*work));

        if (playerIndex == 0) {
            attachedModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 101), 0x10000000, 2));
            otherModel = attachedModel;
        } else {
            otherModel = fn_80025D4C(attachedModel);
        }
        linkedModel = fn_80025A04(
            fn_80008DD0(DATANUM(0xa70000, 98), 0x10000000, 2));
        work->positionIndexA = fn_1_10AE0(linkedModel, otherModel,
                                 &lbl_1_data_1C0C[playerIndex]);
        mainModel = fn_80025A04(
            fn_80008DD0(DATANUM(0xa70000, 99), 0x10000000, 2));
        work->positionIndexB = fn_1_10CFC(mainModel,
                                 &lbl_1_data_1C0C[playerIndex]);
        fn_800269E8(mainModel, ((1 << 30) | (1 << 1)));
        fn_80026E44(mainModel);
        for (hookIndex = 0; hookIndex < 4; hookIndex++) {
            mainModel = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 100), 0x10000000, 2));
            fn_8002741C(linkedModel, lbl_1_data_1C54[hookIndex], mainModel);
            work->hookedModels[hookIndex] = mainModel;
        }
        work->targetPosition = 4000.0f + lbl_1_data_1C0C[playerIndex].x;
        work->state = 0;
    }

    lbl_1_bss_3802 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 3), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_3802, (1 << 0));
    fn_800269E8(lbl_1_bss_3802, ((1 << 30) | (1 << 1)));
    fn_800269E8(lbl_1_bss_3802, (1 << 22));
    lbl_1_bss_3800 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 81), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_3800, (1 << 0));
    fn_80026808(lbl_1_bss_3800, 0.0f, 270.0f, 0.0f);
    lbl_1_bss_37FE = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 83), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_37FE, (1 << 0));
    lbl_1_bss_37FC = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 84), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_37FC, (1 << 0));
    fn_800269E8(lbl_1_bss_37FC, ((1 << 30) | (1 << 1)));
    lbl_1_bss_37FA = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 82), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_37FA, ((1 << 30) | (1 << 1)));
    fn_800269E8(lbl_1_bss_37FA, (1 << 0));
    lbl_1_bss_37F8 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 96), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_37F8, (1 << 0));
    lbl_1_bss_37F6 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 95), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_37F6, (1 << 0));
    fn_800269E8(lbl_1_bss_37F6, ((1 << 30) | (1 << 1)));
    lbl_1_bss_37EC = 0;
    lbl_1_bss_380A = 0;
    lbl_1_bss_3808 = 0;
    MikeActPreActorCallbackSet(fn_1_1E29C);
}

/* Sets up the third stage's models, effect objects and collision attributes. */
void fn_1_1D594(void)
{
    COL_ATTRPARAM collisionParams;
    MikeActVisibilityTarget *placementWork;
    OMOBJ *object;
    MGACTOR *actor;
    s16 modelA;
    s16 modelB;
    s16 baseModelA;
    s32 itemIndex;
    s16 baseModelB;
    s16 slot;
    s32 placementIndex;
    MikeActEffectController *effectWork;

    lbl_1_bss_37F4 = frandmod(3);
    lbl_1_bss_37F2 = fn_1_FC80(lbl_1_bss_37F4 + DATANUM(0x250000, 21));

    for (placementIndex = 0; placementIndex < 0xF; placementIndex++) {
        if (placementIndex == 0) {
            baseModelA = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 85), 0x10000000, 2));
            modelA = baseModelA;
            baseModelB = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 86), 0x10000000, 2));
            modelB = baseModelB;
        } else {
            modelA = fn_80025D4C(baseModelA);
            modelB = fn_80025D4C(baseModelB);
        }
        fn_800269E8(modelA, ((1 << 30) | (1 << 1)));
        fn_80026B8C(modelA, 4);
        fn_800269E8(modelB, ((1 << 30) | (1 << 0)));
        fn_80026B8C(modelB, 6);
        actor = fn_80080B04(&lbl_1_data_1B68, modelA);
        fn_80080DD4(actor, 4U);
        slot = fn_1_1092C(actor, &lbl_1_data_1AB4[placementIndex]);
        lbl_1_bss_1B8[slot].event_state = fn_1_10CFC(modelB, &lbl_1_data_1AB4[placementIndex]);
        lbl_1_bss_1B8[slot].callback_state = 0;
        object = fn_8003AB38(lbl_1_bss_6C, 0x2300, 0U, 0U, -1, fn_1_2116C);
        object->work[0] = (u32)slot;
        object->work[1] = 0;
        object->work[2] = 0;
    }

    for (placementIndex = 0; placementIndex < 6; placementIndex++) {
        object = fn_8003AB38(lbl_1_bss_6C, 0x2300, 0U, 0U, -1, fn_1_21304);
        object->data = HuMemDirectMallocNum(HEAP_HEAP, 0x388, 0x10000000);
        placementWork = object->data;
        memset(placementWork, 0, 0x388U);
        placementWork->soundHandle = -1;
        if (placementIndex == 0) {
            baseModelA = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 93), 0x10000000, 2));
            modelA = baseModelA;
        } else {
            modelA = fn_80025D4C(baseModelA);
        }
        fn_80026B8C(modelA, 4);
        placementWork->mode = lbl_1_data_1C00[placementIndex];
        if (placementWork->mode == 3) {
            placementWork->mode = (s16)(frandmod(2) + 1);
        }
        placementWork->positionIndex = fn_1_10CFC(modelA, &lbl_1_data_1BB8[placementIndex]);
        for (itemIndex = 0; itemIndex < 20; itemIndex++) {
            modelA = fn_80025D4C(lbl_1_bss_37F8);
            fn_80026B8C(modelA, 6);
            placementWork->items[itemIndex].model_id = modelA;
            placementWork->items[itemIndex].active = 0;
        }
        if (placementIndex == 0) {
            baseModelB = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 94), 0x10000000, 2));
            modelA = baseModelB;
        } else {
            modelA = fn_80025D4C(baseModelB);
        }
        fn_80026B8C(modelA, 4);
        placementWork->playerIndex = fn_1_10CFC(modelA, &lbl_1_data_1BB8[placementIndex]);
    }

    for (placementIndex = 0; placementIndex < 2; placementIndex++) {
        if (placementIndex == 0) {
            baseModelA = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 88), 0x10000000, 2));
            fn_800269E8(baseModelA, ((1 << 30) | (1 << 1)));
            modelA = baseModelA;
        } else {
            modelA = fn_80025D4C(baseModelA);
        }
        fn_80026B8C(modelA, 4);
        fn_80026FFC(modelA);
        slot = fn_1_10CFC(modelA, &lbl_1_data_1BA0[placementIndex]);
        object = fn_8003AB38(lbl_1_bss_6C, 0x2300, 0U, 0U, -1, fn_1_1E0C4);
        object->work[0] = (u32)placementIndex;
        object->work[1] = (u32)slot;
        object->work[2] = 0;
        object->work[3] = 0;
    }

    modelA = fn_80025A04(fn_80008DD0(DATANUM(0xa70000, 87), 0x10000000, 2));
    fn_800269E8(modelA, ((1 << 30) | (1 << 1)));
    fn_80026B8C(modelA, 6);
    lbl_1_bss_37C8 = fn_1_10CFC(modelA, &lbl_1_data_1E10);

    modelA = fn_80025A04(fn_80008DD0(DATANUM(0xa70000, 92), 0x10000000, 2));
    fn_800269E8(modelA, ((1 << 30) | (1 << 0)));
    fn_80026790(modelA, &lbl_1_data_1E1C);
    for (placementIndex = 0; placementIndex < 2; placementIndex++) {
        fn_1_10F98(lbl_1_bss_37EE[placementIndex], 0);
    }
    for (placementIndex = 0; placementIndex < 2; placementIndex++) {
        effectWork = lbl_1_bss_37E0[placementIndex]->data;
        fn_1_10F98(effectWork->positionIndexA, 0);
    }
    fn_1_45A0((void (*)(void *, s32))fn_1_1DCF4);
    fn_800870D4(&collisionParams, 1U);
    collisionParams.yDeviate = 0.0f;
    collisionParams.speed = 1.0f;
    collisionParams.maxDot = 1.0f;
    fn_800870A4(&collisionParams, 1U);
    fn_800870D4(&collisionParams, 4U);
    collisionParams.speed = 0.4f;
    fn_800870A4(&collisionParams, 4U);
}

void fn_1_1DB7C(OMOBJ *object)
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
            fn_800269E8(modelId, (1 << 0));
        } else {
            fn_80026A44(modelId, (1 << 0));
        }
        if ((u32)object->work[2] == 0x1E) {
            fn_1_10E64((s16)object->work[0]);
            omDelObjEx(fn_8000DD9C(), object);
        }
        break;
    }
}

s16 fn_1_1DC68(HuVecF *position)
{
    HU3D_MODELID model = fn_80036EBC(lbl_1_bss_3802);
    MikeActEffectModelSetup(model, position);
    return model;
}

/* Set horizontal velocity while an actor is on the moving ground surface. */
void fn_1_1DCF4(MGACTOR *actor) {
    if (actor->param < 0xA) {
        if ((actor->colGroundAttr & 0x40) != 0) {
            actor->vel.x = 10.0f;
            return;
        }
        actor->vel.x = 0.0f;
    }
}

void fn_1_1DD38(s16 playerNo, HuVecF *eventPosition, HuVecF *playerPosition)
{
    HuVecF effectPosition;
    OMOBJ *callbackObject;
    s16 modelId;

    fn_1_10FB4(playerNo);
    fn_80026A44(lbl_1_bss_1B8[playerNo].model_id, 0x40000002U);
    callbackObject = fn_8003AB38(lbl_1_bss_6C, 0x2300, 0U, 0U, -1, fn_1_1DB7C);
    callbackObject->work[0] = playerNo;
    PSVECSubtract(eventPosition, playerPosition, &effectPosition);
    effectPosition.y = 0.0f;
    effectPosition.z = 0.0f;
    PSVECScale(&effectPosition, &effectPosition, 0.5f);
    PSVECAdd(playerPosition, &effectPosition, &effectPosition);
    modelId = fn_80036EBC(lbl_1_bss_3802);
    fn_80026A44(modelId, 1U);
    fn_80026A44(modelId, 0x40000002U);
    fn_800269E8(modelId, 0x08000000U);
    fn_800269E8(modelId, 0x00400000U);
    fn_80026B8C(modelId, 7);
    fn_80026790(modelId, &effectPosition);
    fn_1_30A0(&effectPosition, 0x546);
    lbl_1_bss_1B8[playerNo].callback_state = 1;
    fn_1_10E64((s16)lbl_1_bss_1B8[playerNo].event_state);
}

/* Handles a collision event for a valid record and spawns the linked effect
 * halfway between the two event points along X. */
int fn_1_1DEE0(COL_NARROW_PARAM *eventSource, COL_NARROW_PARAM *eventContext)
{

    HuVecF effectPosition;
    OMOBJ *callbackObject;
    s16 recordIndex;

    if (eventContext->paramB >= 0 && eventContext->paramB < 4) {
        recordIndex = fn_1_1106C(eventSource->paramA);
        if (recordIndex != -1) {
            fn_1_4A70(lbl_1_bss_C4[eventContext->paramB]);
            fn_1_10FB4(recordIndex);
            fn_80026A44(lbl_1_bss_1B8[recordIndex].model_id,
                               ((1 << 30) | (1 << 1)));
            callbackObject = fn_8003AB38(lbl_1_bss_6C, 0x2300, 0U, 0U, -1,
                                        fn_1_1DB7C);
            callbackObject->work[0] = recordIndex;
            PSVECSubtract(&eventSource->point, &eventContext->point,
                          &effectPosition);
            effectPosition.y = 0.0f;
            effectPosition.z = 0.0f;
            PSVECScale(&effectPosition, &effectPosition, 0.5f);
            PSVECAdd(&eventContext->point, &effectPosition, &effectPosition);
            createLinkedEffectModel(&effectPosition);
            fn_1_30A0(&effectPosition, 0x546);
            lbl_1_bss_1B8[recordIndex].callback_state = 1;
            fn_1_10E64((s16)lbl_1_bss_1B8[recordIndex].event_state);
        }
    }
    return 0;
}

void fn_1_1E0C4(OMOBJ *object)
{
    Point3d soundPosition;
    Point3d actorPosition;
    Point3d modelPosition;
    Point3d screenPosition;
    s16 recordIndex;

    recordIndex = lbl_1_bss_37EE[object->work[0]];
    if (object->work[2] != 0) {
        modelPosition = lbl_1_bss_1B8[recordIndex].position;
        modelPosition.x += 750.0f;
        modelPosition.z = -450.0f;
        Hu3D3Dto2D(&modelPosition, 1, &screenPosition);
        if (screenPosition.x < 0.0f) {
            fn_1_3128(object->work[3]);
            omDelObjEx(fn_8000DD9C(), object);
        }
        return;
    }

    if (object->work[2] == 0 &&
        (lbl_1_bss_C4[0]->player->actor->colGroundAttr & 0x407FU) != 0) {
        fn_1_10F38((s16)object->work[1], &soundPosition);
        fn_80086BC8(lbl_1_bss_C4[0]->player->actor, &actorPosition);
        if (PSVECDistance(&soundPosition, &actorPosition) < 100.0f) {
            fn_1_30A0(&soundPosition, 0x555);
            fn_80026A44(lbl_1_bss_1B8[object->work[1]].model_id,
                               ((1 << 30) | (1 << 1)));
            fn_80026A44(lbl_1_bss_1B8[recordIndex].model_id,
                               ((1 << 30) | (1 << 1)));
            fn_1_10F98(recordIndex, 7);
            object->work[2] = 1;
            object->work[3] = fn_1_30E4(0x556);
        }
    }
}

void fn_1_1E29C(void)
{
    switch (lbl_1_bss_380A) {
    case 0:
        fn_1_1E2F0();
        break;
    case 1:
        fn_1_1F18C();
        break;
    case 2:
    case 3:
    default:
        break;
    }
}

/* Advances the first player transition and releases the other players at its end. */
void fn_1_1E2F0(void)
{
    s32 playerNo;
    s16 modelId;

    if (lbl_1_bss_B8 != NULL) {
        switch (lbl_1_bss_3808) {
        case 0:
            if (!(lbl_1_bss_F0->position.x <
                  lbl_1_data_1C64[0].x - 150.0f)) {
                fn_1_1BF4();
                fn_1_49D0(lbl_1_bss_B8);
                if ((lbl_1_bss_B8->player->actor->colGroundAttr & 0x407FU) != 0) {
                    fn_1_45B0(lbl_1_bss_B8, 0x2000);
                    fn_8007E818(lbl_1_bss_B8->player);
                    fn_1_36E4(1);
                    lbl_1_bss_3808 = 1;
                }
            }
            break;
        case 1:
            fn_1_665C(lbl_1_bss_B8, 1);
            if (fn_8007E8B4(lbl_1_bss_B8->player, &lbl_1_data_1C64[0],
                                 5.0f, 5.0f) != 0) {
                /* The original call passes this record where the engine function takes a player. */
                fn_8007EEBC((MGPLAYER *)lbl_1_bss_B8, 0);
                lbl_1_bss_B8->movement_angle = 180.0f;
                fn_80086CA8(lbl_1_bss_B8->player->actor,
                               lbl_1_bss_B8->movement_angle);
                fn_1_F5BC();
                lbl_1_bss_3808 = 2;
            }
            break;
        case 2:
            if (fn_1_1F70() != 0) {
                fn_1_4C84(&lbl_1_data_1C94[1]);
                for (playerNo = 1; playerNo < 4; playerNo++) {
                    lbl_1_bss_C4[playerNo]->player->actor->gravity = 0.0f;
                    fn_800805CC(lbl_1_bss_C4[playerNo]->player->actor, 2U);
                }
                fn_1_1BF4();
                lbl_1_bss_3804 = 0;
                lbl_1_bss_3808 = 3;
            }
            break;
        case 3:
            if (lbl_1_bss_3804++ == 0xF0U) {
                lbl_1_bss_3804 = 0;
                fn_8003EA54(&lbl_1_data_1D18, 30);
                for (playerNo = 1; playerNo < 4; playerNo++) {
                    lbl_1_bss_C4[playerNo]->movement_angle = 0.0f;
                    fn_80086CA8(lbl_1_bss_C4[playerNo]->player->actor,
                                   lbl_1_bss_C4[playerNo]->movement_angle);
                    fn_8007EEBC(lbl_1_bss_C4[playerNo]->player, 0xA);
                }
                for (playerNo = 0; playerNo < 3; playerNo++) {
                    fn_80026A44(lbl_1_bss_37D8[playerNo], ((1 << 30) | (1 << 1)));
                }
                lbl_1_bss_3808 = 4;
            }
            break;
        case 4:
            if (fn_8003EB94(1U) != 0) {
                lbl_1_bss_3808 = 5;
            }
            break;
        case 5:
            fn_1_F370(MESSNUM(MESS_MICGO, 21), 0);
            fn_1_1B40(1, fn_1_1ED4C);
            fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 18), 2);
            for (playerNo = 1; playerNo < 4; playerNo++) {
                fn_1_665C(lbl_1_bss_C4[playerNo], 0x14);
            }
            lbl_1_bss_37D0 = 0;
            lbl_1_bss_3804 = 0;
            lbl_1_bss_3808 = 0xA;
            break;
        case 0xA:
            if (lbl_1_bss_3804 == 0xC) {
                fn_80026A44(lbl_1_bss_37D2, ((1 << 30) | (1 << 0)));
                fn_8002D730(lbl_1_bss_37D2, 0.0f, 180.0f);
                fn_1_1BF4();
                lbl_1_bss_3804 = 0;
                lbl_1_bss_3808++;
            }
            if (lbl_1_bss_37D0 != 0) {
                lbl_1_bss_37D0--;
                if (lbl_1_bss_37D0 == 0) {
                    fn_1_30E4(0x55C);
                }
            }
            break;
        case 0xB:
            if (lbl_1_bss_3804++ == 0x14U) {
                lbl_1_bss_3804 = 0;
                lbl_1_bss_3808++;
            }
            break;
        case 0xC:
            if (lbl_1_bss_3804 % 10 == 0) {
                fn_1_30E4(0x55C);
                modelId = fn_80036EBC(lbl_1_bss_37D4);
                fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
                fn_80026A44(modelId, (1 << 0));
                fn_80026B8C(modelId, 6);
                fn_80026790(modelId, &lbl_1_data_1CDC[lbl_1_bss_3804 / 10]);
            }
            lbl_1_bss_3804++;
            if (lbl_1_bss_3804 > 0x28U) {
                for (playerNo = 0; playerNo < 3; playerNo++) {
                    fn_8002D414(lbl_1_bss_37D8[playerNo], 80.0f);
                    fn_8002D730(lbl_1_bss_37D8[playerNo], 70.0f, 100.0f);
                }
                for (playerNo = 1; playerNo < 4; playerNo++) {
                    fn_1_665C(lbl_1_bss_C4[playerNo], 0);
                }
                lbl_1_bss_3804 = 0;
                lbl_1_bss_3808++;
            }
            break;
        case 0xD:
            if (lbl_1_bss_3804++ >= 0x19U) {
                lbl_1_bss_3804 = 0;
                lbl_1_bss_3808++;
            }
            break;
        case 0xE:
            if (fn_8002D75C(lbl_1_bss_37D2) != 0) {
                for (playerNo = 1; playerNo < 4; playerNo++) {
                    fn_1_665C(lbl_1_bss_C4[playerNo], 0xD);
                }
                fn_1_3608();
                lbl_1_bss_3808++;
            }
            break;
        case 0xF:
            if (fn_8003EB94(1U) != 0 &&
                fn_8005C240(lbl_1_bss_C4[1]->character_no) != 0) {
                lbl_1_bss_3808++;
            }
            break;
        case 0x10:
            fn_1_30E4(0x53F);
            fn_1_49D0(lbl_1_bss_B8);
            fn_1_45C4(lbl_1_bss_B8, 0x2000);
            fn_1_1F5C();
            fn_1_1B40(0, 0);
                fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 1), 6);
            fn_1_36E4(0);
            for (playerNo = 1; playerNo < 4; playerNo++) {
                fn_1_45B0(lbl_1_bss_C4[playerNo], 0x2000);
            }
            fn_1_F598();
            lbl_1_bss_3808++;
            break;
        case 0x11:
            if (lbl_1_bss_3804 == 0) {
                fn_8007E888(lbl_1_bss_B8->player);
            }
            if (lbl_1_bss_3804++ == 0x3CU) {
                for (playerNo = 1; playerNo < 4; playerNo++) {
                    fn_8007EEBC(lbl_1_bss_C4[playerNo]->player, 0);
                    fn_1_665C(lbl_1_bss_C4[playerNo], 0);
                }
                lbl_1_bss_3808++;
            }
            break;
        case 0x12:
            if (lbl_1_bss_F0->position.x > 22000.0f) {
                for (playerNo = 1; playerNo < 4; playerNo++) {
                    fn_80086E3C(lbl_1_bss_C4[playerNo]->player);
                    fn_1_45B0(lbl_1_bss_C4[playerNo], 0x2000);
                    fn_800269E8((s16)lbl_1_bss_C4[playerNo]->player->actor->mdlId, (1 << 0));
                }
                lbl_1_bss_380A++;
                lbl_1_bss_3808 = 0;
            }
            break;
        }
    }
}

void fn_1_1ED4C(MCResponse_s *context)
{
    s32 playerNo;
    s16 modelId;

    switch (lbl_1_bss_3808) {
    case 10:
    if ((lbl_1_bss_3804 & 1) != 0) {
        if (*context->result == 1) {
            for (playerNo = 1; playerNo < 4; playerNo++) {
                CharFXPlay(lbl_1_bss_C4[playerNo]->character_no, 0x251);
                fn_8005C16C(lbl_1_bss_C4[playerNo]->character_no,
                                  30.0f);
                fn_8005C3DC(lbl_1_bss_C4[playerNo]->character_no,
                                      30.0f, 60.0f);
                fn_8005C47C(lbl_1_bss_C4[playerNo]->character_no,
                                   ((1 << 30) | (1 << 1)));
                fn_8005C47C(lbl_1_bss_C4[playerNo]->character_no,
                                   ((1 << 30) | (1 << 0)));
                fn_8002D414(lbl_1_bss_37D8[playerNo - 1],
                                  30.0f);
                fn_8002D730(lbl_1_bss_37D8[playerNo - 1],
                                      30.0f, 60.0f);
            }
            fn_1_30E4(0x563);
            fn_1_30E4(0x55B);
            lbl_1_bss_37D0 = 0x23;
            modelId = fn_80036EBC(lbl_1_bss_37D6);
            fn_80026A44(modelId, (1 << 0));
            fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
            fn_800269E8(modelId, (1 << 27));
            fn_80026B8C(modelId, 7);
            fn_80026790(modelId, &lbl_1_data_1CC4);
            fn_1_F370(MESSNUM(MESS_MICGO, 21), 0);
            lbl_1_bss_3804++;
        }
    } else if (*context->result == 0) {
        if (lbl_1_bss_3804 == 0) {
            for (playerNo = 1; playerNo < 4; playerNo++) {
                fn_80026A44(lbl_1_bss_37D8[playerNo - 1],
                                   ((1 << 30) | (1 << 2)));
                fn_80026A44(lbl_1_bss_37D8[playerNo - 1],
                                   ((1 << 30) | (1 << 1)));
                fn_80026A44(lbl_1_bss_37D8[playerNo - 1],
                                   ((1 << 30) | (1 << 0)));
            }
        }
        for (playerNo = 1; playerNo < 4; playerNo++) {
            fn_8005C16C(lbl_1_bss_C4[playerNo]->character_no, 0.0f);
            fn_8005C3DC(lbl_1_bss_C4[playerNo]->character_no,
                                  0.0f, 30.0f);
            fn_8005C47C(lbl_1_bss_C4[playerNo]->character_no,
                               ((1 << 30) | (1 << 1)));
            fn_8005C47C(lbl_1_bss_C4[playerNo]->character_no,
                               ((1 << 30) | (1 << 0)));
            fn_8002D414(lbl_1_bss_37D8[playerNo - 1],
                              0.0f);
            fn_8002D730(lbl_1_bss_37D8[playerNo - 1],
                                  0.0f, 30.0f);
        }
        fn_1_30E4(0x55A);
            fn_1_F370(MESSNUM(MESS_MICGO, 22), 0);
        lbl_1_bss_3804++;
    }
        break;
    }
}

/* Advances the second player transition, including its screen wipes. */
void fn_1_1F18C(void)
{
    s32 playerNo;

    if (lbl_1_bss_B8 != NULL) {
    switch (lbl_1_bss_3808) {
    case 0:
        if (lbl_1_bss_F0->position.x <
            lbl_1_data_1D94[0].x - 150.0f) {
            break;
        }
        fn_1_49D0(lbl_1_bss_B8);
        if ((lbl_1_bss_B8->player->actor->colGroundAttr & 0x407FU) == 0) {
            break;
        }
        fn_1_1BF4();
        fn_1_45B0(lbl_1_bss_B8, 0x2000);
        fn_8007E818(lbl_1_bss_B8->player);
        fn_1_EA08(0);
        lbl_1_bss_3808 = 1;
        break;
    case 1:
        fn_1_665C(lbl_1_bss_B8, 1);
        if (fn_8007E8B4(lbl_1_bss_B8->player, &lbl_1_data_1D94[0],
                             5.0f, 5.0f) != 0) {
            /* The original call passes this record where the engine function takes a player. */
            fn_8007EEBC((MGPLAYER *)lbl_1_bss_B8, 0);
            lbl_1_bss_B8->movement_angle = 90.0f;
            fn_80086CA8(lbl_1_bss_B8->player->actor,
                           lbl_1_bss_B8->movement_angle);
            fn_1_F5BC();
            lbl_1_bss_3808 = 2;
        }
        break;
    case 2:
        if (fn_1_1F70() != 0) {
            fn_1_4C84(&lbl_1_data_1D94[1]);
            fn_1_1BF4();
            lbl_1_bss_3804 = 0;
            lbl_1_bss_3808 = 3;
        }
        break;
    case 3:
        if ((u32)lbl_1_bss_3804++ == 0xF0U) {
            lbl_1_bss_3804 = 0;
            lbl_1_bss_3808 = 4;
            for (playerNo = 1; playerNo < 4; playerNo++) {
                fn_1_665C(lbl_1_bss_C4[playerNo], 1);
                fn_8007EEBC(lbl_1_bss_C4[playerNo]->player, 0);
            }
        }
        break;
    case 4:
        for (playerNo = 1; playerNo < 4; playerNo++) {
            fn_8007E8B4(lbl_1_bss_C4[playerNo]->player,
                             &lbl_1_data_1DC4[playerNo], 5.0f, 5.0f);
        }
        if ((u32)lbl_1_bss_3804++ == 0xFU) {
            WipeCreate(2, 0, 30);
            lbl_1_bss_3808 = 5;
        }
        break;
    case 5:
        if (WipeCheck() != 0) {
            for (playerNo = 1; playerNo < 4; playerNo++) {
                fn_8007E8B4(lbl_1_bss_C4[playerNo]->player,
                                 &lbl_1_data_1DC4[playerNo], 5.0f, 5.0f);
            }
            break;
        }
        for (playerNo = 1; playerNo < 4; playerNo++) {
            fn_8008697C(lbl_1_bss_C4[playerNo]->player,
                           &lbl_1_data_1DC4[playerNo]);
            lbl_1_bss_C4[playerNo]->movement_angle = 90.0f;
            fn_80086CA8(lbl_1_bss_C4[playerNo]->player->actor, 90.0f);
        }
        fn_8008697C(lbl_1_bss_C4[0]->player, lbl_1_data_1DC4);
        lbl_1_bss_1AA = 3;
        fn_1_36E4(1);
        fn_8003E5A4(&lbl_1_data_1DF4);
        fn_1_EACC();
        fn_1_F370(MESSNUM(MESS_MICGO, 23), 0);
        fn_1_1B40(1, fn_1_1FFE8);
        fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 20), 1);
        WipeCreate(1, 0, 30);
        lbl_1_bss_37CE = 0;
        lbl_1_bss_37CC = 0;
        lbl_1_bss_37CA = 0;
        lbl_1_bss_3808 = 6;
        break;
    case 6:
        if (WipeCheck() == 0) {
            lbl_1_bss_37C4 = 0;
            lbl_1_bss_37C0 = 0;
            lbl_1_bss_3804 = 0;
            lbl_1_bss_3808 = 0xA;
        }
        break;
    case 0xA:
        if (fn_1_1FB7C() == 0 && lbl_1_bss_37CE == 0xA) {
            fn_1_1E1C(lbl_1_bss_37F4 + MESSNUM(MESS_MICGOSUPORT, 21), 1);
            fn_1_FE94(1);
            lbl_1_bss_3804 = 0;
            fn_1_F370(MESSNUM(MESS_MICGO, 12), 0);
            lbl_1_bss_3808++;
        }
        break;
    case 0xB:
        if (fn_1_1FB7C() == 0) {
            if ((u32)lbl_1_bss_3804++ >= 0x14U) {
                fn_1_F148();
                lbl_1_bss_3808 = 0x14;
            }
        }
        break;
    case 0x14:
        if (fn_1_1FB7C() == 0 && fn_1_F138() == 0) {
            fn_1_F188();
            fn_1_F370(MESSNUM(MESS_MICGO, 14), 0);
            fn_1_FE94(0);
            fn_1_30E4(0x53E);
            fn_1_1BF4();
            lbl_1_bss_3804 = 0;
            lbl_1_bss_3808 = 0x28;
        }
        break;
    case 0x1E:
        if (fn_1_1FB7C() == 0) {
            fn_1_30E4(0x53D);
            fn_1_1BF4();
            fn_1_F188();
            fn_1_FE94(0);
            fn_1_F370(MESSNUM(MESS_MICGO, 13), 0);
            lbl_1_bss_3808++;
        }
        break;
    case 0x1F:
        if (fn_1_1FB7C() != 0) {
            for (playerNo = 1; playerNo < 4; playerNo++) {
                CharFXPlay(lbl_1_bss_C4[playerNo]->character_no, 0x243);
            }
        }
        break;
    case 0x28:
        fn_1_1FB7C();
        break;
    case 0x32:
        fn_1_30E4(0x55E);
        fn_80026A44(lbl_1_bss_1B8[lbl_1_bss_37C8].model_id,
                           ((1 << 30) | (1 << 1)));
        fn_1_30E4(0x53F);
        for (playerNo = 1; playerNo < 4; playerNo++) {
            fn_8007EEBC(lbl_1_bss_C4[playerNo]->player, 0);
        }
        fn_1_1BF4();
        fn_1_F188();
        fn_1_FE94(0);
        lbl_1_bss_3804 = 0;
        lbl_1_bss_3808++;
        break;
    case 0x33:
        if (lbl_1_bss_3804 == 0x50U) {
            fn_1_30E4(0x565);
            fn_1_35A0(0x1E);
        }
        if ((u32)lbl_1_bss_3804++ == 0x96U) {
            WipeCreate(2, 0, 30);
            lbl_1_bss_3808++;
        }
        break;
    case 0x34:
        if (WipeCheck() == 0) {
            fn_1_1FD4();
            fn_1_EA08(1);
            for (playerNo = 1; playerNo < 4; playerNo++) {
                fn_8005C390(lbl_1_bss_C4[playerNo]->character_no,
                                   1.0f);
                fn_1_69EC(lbl_1_bss_C4[playerNo], 0);
                fn_80086E3C(lbl_1_bss_C4[playerNo]->player);
                fn_1_45B0(lbl_1_bss_C4[playerNo], 0x2000);
                fn_8005C430(lbl_1_bss_C4[playerNo]->character_no,
                                 ((1 << 30) | (1 << 0)));
                fn_800269E8(
                    lbl_1_bss_C4[playerNo]->player->actor->mdlId, (1 << 0));
            }
            fn_1_F598();
            WipeCreate(1, 0, 30);
            lbl_1_bss_3808++;
        }
        break;
    case 0x35:
        if (WipeCheck() == 0) {
            fn_8007E888(lbl_1_bss_B8->player);
            lbl_1_bss_380A++;
            lbl_1_bss_3808 = 0;
        }
        break;
    }
    }
}

s32 fn_1_1FB7C(void)
{
    Point3d position;
    s16 active;
    s32 playerNo;

    active = lbl_1_bss_37CE;
    if (active == 0 && lbl_1_bss_3808 >= 40) {
        active = 1;
    }
    if (active == 0) {
        return;
    }
    if (fn_8005C1B8(lbl_1_bss_C4[1]->character_no) >=
            10.0f &&
        lbl_1_bss_37CA == 0) {
        fn_1_30E4(0x55D);
        lbl_1_bss_37CC++;
        lbl_1_bss_37C0 = 0;
        lbl_1_bss_37C4 = 1;
        lbl_1_bss_37CA = 1;
    }

    if (lbl_1_bss_37CE == 10 &&
        fn_8005C240(lbl_1_bss_C4[1]->character_no)) {
        lbl_1_bss_37CA = 0;
        for (playerNo = 1; playerNo < 4; playerNo++) {
            if (lbl_1_bss_3808 == 31) {
                fn_8005C390(lbl_1_bss_C4[playerNo]->character_no,
                                   4.5f);
            } else {
                fn_8005C390(lbl_1_bss_C4[playerNo]->character_no,
                                   1.0f);
            }
            CharFXPlay(lbl_1_bss_C4[playerNo]->character_no, 0x251);
            fn_1_69EC(lbl_1_bss_C4[playerNo], 0x15);
            fn_8005C47C(lbl_1_bss_C4[playerNo]->character_no,
                               ((1 << 30) | (1 << 0)));
        }
    }

    if (lbl_1_bss_37C4 != 0) {
        if (lbl_1_bss_37C0 == 0) {
            for (playerNo = 1; playerNo < 4; playerNo++) {
                s16 effectModelId;

                position = lbl_1_data_1DC4[playerNo];
                position.x += 100.0f;
                position.y += 50.0f;
                effectModelId = createLinkedEffectModel(&position);
                fn_80026808(effectModelId, 0.0f, lbl_80266A40.y, 0.0f);
            }
        }
        fn_1_10F38(lbl_1_bss_37C8, &position);
        if (lbl_1_bss_37C0++ >= 8) {
            lbl_1_bss_37C0 = 0;
            lbl_1_bss_37C4 = 0;
            fn_80026790(
                lbl_1_bss_1B8[lbl_1_bss_37C8].model_id, &position);
        } else {
            position.x += (f32)frandmod(10) - 5.0f;
            fn_80026790(
                lbl_1_bss_1B8[lbl_1_bss_37C8].model_id, &position);
        }
    }

    if (lbl_1_bss_37CC >= 50) {
        lbl_1_bss_3808 = 50;
        return 1;
    }
    return 0;
}

void fn_1_1FFE8(MCResponse_s *context)
{
    s16 desiredSelection;
    s32 playerNo;

    desiredSelection = HuMCMicGet() == 2 ? 0 : 2;
    switch (lbl_1_bss_3808) {
    case 10:
        if (*context->result == desiredSelection) {
            fn_1_F0B8();
            lbl_1_bss_37CE++;
            lbl_1_bss_37CA = 0;
            for (playerNo = 1; playerNo < 4; playerNo++) {
                fn_1_69EC(lbl_1_bss_C4[playerNo], 0x15);
                CharFXPlay(lbl_1_bss_C4[playerNo]->character_no, 0x251);
                fn_8005C47C(lbl_1_bss_C4[playerNo]->character_no,
                                   0x40000001U);
            }
        }
        break;
    case 20:
        if (HuMCMicGet() == 2) {
            if (*context->result == 0) {
                lbl_1_bss_3808 = 0x1E;
            }
        } else if (*context->result == lbl_1_bss_37F4 + 3) {
            lbl_1_bss_3808 = 0x1E;
        }
        break;
    }
}

/* Moves the spawner, updates its particles and removes it when they expire. */
void fn_1_20188(OMOBJ *object)
{
    MikeActEffectSpawner *spawner;
    Point3d modelPosition;
    Point3d playerPosition;
    s32 particleIndex;
    s32 allParticlesInactive;
    s16 modelId;

    spawner = object->data;
    fn_800267CC(spawner->modelId, &modelPosition);
    for (particleIndex = 0; particleIndex < 32; particleIndex++) {
        if (spawner->particles[particleIndex].active != 0) {
            spawner->particles[particleIndex].scale +=
                0.01 + (f64)((f32)frandmod(0x64) / 10000.0f);
            fn_800268B0(
                spawner->particles[particleIndex].modelId,
                spawner->particles[particleIndex].scale,
                spawner->particles[particleIndex].scale,
                spawner->particles[particleIndex].scale);
            spawner->particles[particleIndex].transparency -= 0.005f;
            if (spawner->particles[particleIndex].transparency <= 0.0f) {
                spawner->particles[particleIndex].active = 0;
                fn_800262D0(spawner->particles[particleIndex].modelId);
            } else {
                fn_80026CBC(
                    spawner->particles[particleIndex].modelId,
                    spawner->particles[particleIndex].transparency);
            }
        }
    }

    switch (spawner->state) {
    case 0:
        modelPosition.x -= 5.0f;
        fn_80026790(spawner->modelId, &modelPosition);
        spawner->riseSpeed += 5.0f;
        if (spawner->riseSpeed >= 2000.0f) {
            spawner->state = 2;
            return;
        }
        fn_1_205EC(spawner);
        modelPosition.x += 75.0f;
        fn_80086BC8(lbl_1_bss_C4[0]->player->actor, &playerPosition);
        playerPosition.y += 75.0f;
        if (PSVECDistance(&modelPosition, &playerPosition) < 150.0f) {
            spawner->state = 1;
        }
        break;
    case 1:
        modelId = fn_80036EBC(lbl_1_bss_37FC);
        fn_80026A44(modelId, 1U);
        fn_80026A44(modelId, 0x40000002U);
        fn_800269E8(modelId, 0x08000000U);
        fn_80026B8C(modelId, 7);
        fn_80026790(modelId, &modelPosition);
        fn_1_30A0(&modelPosition, 0x561);
        fn_1_4A70(lbl_1_bss_C4[0]);
        spawner->state = 2;
        return;
    case 2:
        fn_800262D0(spawner->modelId);
        spawner->state = 3;
        /* The state-3 cleanup immediately follows the model kill. */
    case 3:
        allParticlesInactive = 1;
        for (particleIndex = 0; particleIndex < 32; particleIndex++) {
            if (spawner->particles[particleIndex].active == 1) {
                allParticlesInactive = 0;
                break;
            }
        }
        if (allParticlesInactive != 0) {
            omDelObjEx(fn_8000DD9C(), object);
        }
    }
}

void fn_1_204F0(HuVecF *position)
{
    OMOBJ *object;
    MikeActEffectSpawner *effectData;

    object = fn_8003AB38(lbl_1_bss_6C, 0x1F00, 0, 0, -1, fn_1_20188);
    object->data = HuMemDirectMallocNum(HEAP_HEAP, 0x18C, 0x10000000);
    effectData = object->data;
    memset(effectData, 0, 0x18C);
    effectData->state = 0;
    effectData->riseSpeed = 0.0f;
    effectData->modelId = fn_80025D4C(lbl_1_bss_3800);
    fn_80026E44(effectData->modelId);
    fn_80026A44(effectData->modelId, (1 << 0));
    fn_80026790(effectData->modelId, position);
    fn_80026808(effectData->modelId, 0.0f, 270.0f, 0.0f);
}

/* Starts a particle in the first free slot after the spawn cooldown. */
void fn_1_205EC(MikeActEffectSpawner *spawner)
{
    s32 particleIndex;
    Point3d position;

    if (spawner->cooldown > 0) {
        spawner->cooldown--;
        return;
    }
    fn_800267CC(spawner->modelId, &position);
    for (particleIndex = 0; particleIndex < 32; particleIndex++) {
        if (spawner->particles[particleIndex].active == 0) {
            spawner->particles[particleIndex].active = 1;
            spawner->cooldown = frandmod(5) + 5;
            spawner->particles[particleIndex].modelId = fn_80025D4C(lbl_1_bss_37FE);
            position.x += 75.0f;
            fn_80026790(spawner->particles[particleIndex].modelId, &position);
            spawner->particles[particleIndex].scale =
                0.4f + (f32)frandmod(0xC8) / 1000.0f;
            fn_800268B0(spawner->particles[particleIndex].modelId,
                spawner->particles[particleIndex].scale,
                spawner->particles[particleIndex].scale,
                spawner->particles[particleIndex].scale);
            spawner->particles[particleIndex].transparency =
                (f32)(0.9 +
                    (f64)((f32)frandmod(0xA) / 100.0f));
            fn_80026CBC(spawner->particles[particleIndex].modelId,
                spawner->particles[particleIndex].transparency);
            fn_80026A44(spawner->particles[particleIndex].modelId, 1U);
            fn_80026B8C(spawner->particles[particleIndex].modelId, 7);
            return;
        }
    }
}

void fn_1_20808(OMOBJ *object)
{
    MikeActEffectController *controller;
    Point3d attachmentPosition;
    Point3d playerPosition;
    Point3d firstPosition;
    Point3d secondPosition;
    s32 hookIndex;
    OMOBJ *effectObject;
    MikeActEffectSpawner *effectData;
    s16 modelId;

    controller = object->data;
    fn_80086BC8(lbl_1_bss_C4[0]->player->actor, &playerPosition);
    fn_1_10F38(controller->positionIndexA, &firstPosition);
    fn_1_10F38(controller->positionIndexB, &secondPosition);
    controller->previousPosition = firstPosition.x;

    if ((controller->state > 0) && (controller->state <= 8)) {
        if ((1000.0f + playerPosition.x) > firstPosition.x) {
            firstPosition.x = 1000.0f + playerPosition.x;
            if (firstPosition.x >= controller->targetPosition) {
                firstPosition.x = controller->targetPosition;
            }
            if (controller->firstSoundHandle == -1) {
                controller->firstSoundHandle = fn_1_30A0(&firstPosition, 0x557);
            }
            secondPosition.x = firstPosition.x;
            controller->rotation = (f32)((f64)controller->rotation +
                (0.9549296585513721 *
                    (f64)(firstPosition.x - controller->previousPosition)));
            for (hookIndex = 0; hookIndex < 4; hookIndex++) {
                fn_80026808(controller->hookedModels[hookIndex], 0.0f,
                                controller->rotation, 0.0f);
            }
        } else if (controller->firstSoundHandle != -1) {
            fn_1_3128(controller->firstSoundHandle);
            controller->firstSoundHandle = -1;
        }

        if ((controller->state < 6) &&
            (lbl_1_bss_F0->owner == NULL)) {
            controller->frameCount = 0;
            controller->state = 6;
        }
    }

    switch (controller->state) {
    case 0:
        if (!(playerPosition.x > firstPosition.x) &&
            (playerPosition.x > (firstPosition.x - 1000.0f))) {
            lbl_1_bss_88 = 1;
            fn_8002D730(
                lbl_1_bss_1B8[controller->positionIndexB].model_id, 1.0f,
                60.0f);
            fn_8002D414(
                lbl_1_bss_1B8[controller->positionIndexB].model_id, 1.0f);
            fn_80026A44(
                lbl_1_bss_1B8[controller->positionIndexB].model_id,
                0x40000002U);
            controller->state = 1;
            controller->secondSoundHandle = -1;
            controller->firstSoundHandle = -1;
            fn_1_10F98(controller->positionIndexA, 0);
            fn_1_30A0(&secondPosition, 0x55F);
        }
        break;
    case 1:
        modelId = lbl_1_bss_1B8[controller->positionIndexB].model_id;
        if (fn_8002D75C(
                lbl_1_bss_1B8[controller->positionIndexB].model_id) != 0) {
            controller->state = 2;
        }
        break;
    case 2:
        controller->playerIndex = lbl_1_bss_C4[0]->lane;
        controller->frameCount = 0;
        controller->state = 3;
        controller->secondSoundHandle = fn_1_30A0(&secondPosition, 0x560);
        /* fall through */
    case 3:
        if (firstPosition.x == controller->targetPosition) {
            controller->state = 6;
        } else {
            controller->frameCount++;
            if (secondPosition.z < lbl_1_data_42C[controller->playerIndex]) {
                secondPosition.z += 15.0f;
                if (secondPosition.z >= lbl_1_data_42C[controller->playerIndex]) {
                    secondPosition.z = lbl_1_data_42C[controller->playerIndex];
                    controller->state = 4;
                }
            } else {
                secondPosition.z -= 15.0f;
                if (secondPosition.z <= lbl_1_data_42C[controller->playerIndex]) {
                    secondPosition.z = lbl_1_data_42C[controller->playerIndex];
                    controller->state = 4;
                }
            }
        }
        break;
    case 4:
        if (controller->secondSoundHandle != -1) {
            fn_1_3128(controller->secondSoundHandle);
            controller->secondSoundHandle = -1;
        }
        if (firstPosition.x == controller->targetPosition) {
            controller->state = 6;
        } else {
            fn_8002D414(
                lbl_1_bss_1B8[controller->positionIndexB].model_id, 70.0f);
            fn_8002D730(
                lbl_1_bss_1B8[controller->positionIndexB].model_id, 70.0f,
                95.0f);
            Hu3DModelObjPosGet(
                lbl_1_bss_1B8[controller->positionIndexB].model_id,
                "itemhook_cn", &attachmentPosition);
            fn_1_30A0(&attachmentPosition, 0x558);
            effectObject = fn_8003AB38(
                lbl_1_bss_6C, 0x1F00, 0, 0, -1, fn_1_20188);
            effectObject->data = HuMemDirectMallocNum(
                HEAP_HEAP, 0x18C, 0x10000000U);
            effectData = effectObject->data;
            memset(effectData, 0, 0x18CU);
            effectData->state = 0;
            effectData->riseSpeed = 0.0f;
            effectData->modelId = fn_80025D4C(lbl_1_bss_3800);
            fn_80026E44(effectData->modelId);
            fn_80026A44(effectData->modelId, 1U);
            fn_80026790(effectData->modelId, &attachmentPosition);
            fn_80026808(effectData->modelId, 0.0f, 270.0f, 0.0f);
            {
                modelId = fn_80036EBC(lbl_1_bss_37FA);
                fn_80026A44(modelId, 1U);
                fn_80026A44(modelId, 0x40000002U);
                fn_80026B8C(modelId, 6);
                fn_80026808(modelId, 0.0f, 90.0f, 0.0f);
                attachmentPosition.x -= 80.0f;
                fn_80026790(modelId, &attachmentPosition);
            }
            controller->state = 5;
        }
        break;
    case 5:
        if ((fn_8002D75C(
                lbl_1_bss_1B8[controller->positionIndexB].model_id) != 0) &&
            (firstPosition.x == controller->targetPosition)) {
            controller->state = 6;
        } else if (controller->frameCount++ >= 0xD2) {
            controller->state = 2;
        }
        break;
    case 6:
        fn_1_30A0(&secondPosition, 0x567);
        fn_8002D730(
            lbl_1_bss_1B8[controller->positionIndexB].model_id, 100.0f,
            125.0f);
        fn_8002D414(
            lbl_1_bss_1B8[controller->positionIndexB].model_id, 100.0f);
        if (controller->secondSoundHandle == -1) {
            controller->secondSoundHandle = fn_1_30A0(&secondPosition, 0x560);
        }
        controller->state = 7;
        break;
    case 7:
        if (secondPosition.z < 0.0f) {
            secondPosition.z += 15.0f;
            if (secondPosition.z > 0.0f) {
                secondPosition.z = 0.0f;
                controller->state = 8;
            }
        } else {
            secondPosition.z -= 15.0f;
            if (secondPosition.z < 0.0f) {
                secondPosition.z = 0.0f;
                controller->state = 8;
            }
        }
        break;
    case 8:
        if (controller->secondSoundHandle != -1) {
            fn_1_3128(controller->secondSoundHandle);
            controller->secondSoundHandle = -1;
        }
        if (controller->firstSoundHandle != -1) {
            fn_1_3128(controller->firstSoundHandle);
            controller->firstSoundHandle = -1;
        }
        if (firstPosition.x == controller->targetPosition) {
            lbl_1_bss_88 = 0;
            fn_1_10F98(controller->positionIndexA, 7);
            fn_80026790(
                lbl_1_bss_1B8[controller->positionIndexA].linked_model_id,
                &firstPosition);
            ++controller->state;
        }
        break;
    case 99:
        break;
    }

    fn_1_10F68(controller->positionIndexA, &firstPosition);
    fn_1_10F68(controller->positionIndexB, &secondPosition);
    fn_80026790(
        lbl_1_bss_1B8[controller->positionIndexA].model_id, &firstPosition);
    fn_80026790(
        lbl_1_bss_1B8[controller->positionIndexB].model_id, &secondPosition);
}

void fn_1_2116C(OMOBJ *object)
{
    HuVecF targetPosition;
    HuVecF screenPosition;
    HuVecF playerPosition;

    if (lbl_1_bss_1B8[object->work[0]].callback_state != 0) {
        if (object->work[1] == 1) {
            fn_1_3128(object->work[2]);
        }
        omDelObjEx(fn_8000DD9C(), object);
        return;
    }

    targetPosition = lbl_1_bss_1B8[object->work[0]].position;
    fn_80086BC8(lbl_1_bss_C4[0]->player->actor, &playerPosition);
    if (playerPosition.x < targetPosition.x - 1500.0f ||
        1500.0f + targetPosition.x < playerPosition.x) {
        return;
    }

    Hu3D3Dto2D(&targetPosition, 1, &screenPosition);
    if (screenPosition.x > -20.0f && screenPosition.x < 596.0f) {
        if (object->work[1] == 0) {
            object->work[1] = 1;
            object->work[2] = fn_1_30A0(&lbl_1_bss_1B8[object->work[0]].position, 0x554);
        }
    } else if (object->work[1] == 1) {
        object->work[1] = 0;
        fn_1_3128(object->work[2]);
        object->work[2] = 0;
    }
}

void fn_1_21304(OMOBJ *controller)
{
    Point3d screenPosition;
    Point3d targetPosition;
    Point3d playerPosition;
    MikeActVisibilityTarget *target;

    target = controller->data;
    fn_80086BC8(lbl_1_bss_C4[0]->player->actor, &playerPosition);
    fn_1_10F38(target->positionIndex, &targetPosition);
    if (playerPosition.x < targetPosition.x - 1500.0f ||
        1500.0f + targetPosition.x < playerPosition.x) {
        return;
    }

    screenPosition = targetPosition;
    if (target->mode != 2) {
        screenPosition.z -= 300.0f;
    }
    Hu3D3Dto2D(&screenPosition, 1, &screenPosition);
    if (screenPosition.x > -20.0f && screenPosition.x < 596.0f) {
        if (target->soundHandle == -1 && target->paused == 0) {
            target->soundHandle = fn_1_30A0(&targetPosition, 0x562);
        }
    } else if (target->soundHandle != -1) {
        fn_1_3128(target->soundHandle);
        target->soundHandle = -1;
    }

    switch (target->state) {
    case 0:
        if (playerPosition.x > targetPosition.x - 1500.0f) {
            target->state = 1;
        }
        break;
    case 1:
        if (playerPosition.x > 1500.0f + targetPosition.x) {
            target->state = 3;
            return;
        }
        fn_1_217BC(target);
        if (target->updateCount++ >= 6 && target->paused == 0) {
            target->updateCount = 0;
            if (target->mode == 0) {
                fn_1_21590(target, 0);
                fn_1_21590(target, 1);
            } else if (target->mode == 1) {
                fn_1_21590(target, 0);
            } else {
                fn_1_21590(target, 1);
            }
        }
        fn_1_21954(target);
        return;
    case 2:
        fn_1_217BC(target);
        return;
    case 3:
        fn_1_217BC(target);
        break;
    }
}

/* Starts an item effect at the selected model hook. */
void fn_1_21590(MikeActVisibilityTarget *work, s16 useDownHook)
{
    Point3d position;
    f32 verticalOffset;
    s32 itemIndex;
    MikeActModelEffect *item;

    item = NULL;
    itemIndex = 0;
    while (itemIndex < 20) {
        if (work->items[itemIndex].active == 0) {
            item = &work->items[itemIndex];
            break;
        }
        itemIndex++;
    }
    if (item != NULL) {
        if (useDownHook != 0) {
            Hu3DModelObjPosGet(lbl_1_bss_1B8[work->positionIndex].model_id,
                "itemhook_down", &position);
        } else {
            Hu3DModelObjPosGet(lbl_1_bss_1B8[work->positionIndex].model_id,
                "itemhook_up", &position);
        }
        position.y += 5.0f;
        item->position = position;
        item->active = 1;
        item->frame = 0;
        item->scale = 0.3f;
        item->movement.x = 0.1f * (f32)(frandmod(7) - 4);
        item->movement.y = 0.0f;
        if (useDownHook != 0) {
            verticalOffset = 5.0f;
        } else {
            verticalOffset = -5.0f;
        }
        item->movement.z = verticalOffset;
        item->transparency = 0.9f;
        fn_80026790(item->model_id, &item->position);
        fn_800268B0(item->model_id, item->scale, item->scale, item->scale);
        fn_80026CBC(item->model_id, item->transparency);
        fn_80026B8C(item->model_id, 6);
        fn_80026A44(item->model_id, 1U);
        work->activeItemCount++;
    }
}

void fn_1_217BC(MikeActVisibilityTarget *manager)
{
    MikeActModelEffect *effect;
    HuVecF movement;
    s32 index;

    for (index = 0; index < 20; index++) {
        effect = &manager->items[index];
        if (effect->active != 0) {
            if (effect->frame < 10) {
                movement.x = effect->movement.x;
                movement.y = effect->movement.y;
                movement.z = effect->movement.z / 2.0f;
                PSVECAdd(&effect->position, &movement, &effect->position);
            } else {
                PSVECAdd(&effect->position, &effect->movement, &effect->position);
            }
            fn_80026790(effect->model_id, &effect->position);
            if (effect->frame < 10) {
                manager->items[index].scale *= 1.1;
            } else {
                manager->items[index].scale *= 1.02;
            }
            fn_800268B0(effect->model_id, effect->scale, effect->scale, effect->scale);
            effect->transparency = (f32)((f64)effect->transparency - 0.014545454545454545);
            fn_80026CBC(effect->model_id, effect->transparency);
            if (effect->frame++ >= 0x37) {
                effect->frame = 0;
                fn_800269E8(effect->model_id, 1U);
                effect->active = 0;
                manager->activeItemCount--;
            }
        }
    }
}

void fn_1_21954(MikeActVisibilityTarget *trigger)
{
    HuVecF playerPosition;
    HuVecF targetPosition;
    s16 effectModelId;

    if (trigger->triggered != 0) {
        return;
    }

    fn_80086BC8(lbl_1_bss_C4[0]->player->actor, &playerPosition);
    fn_1_10F38(trigger->playerIndex, &targetPosition);
    if (PSVECDistance(&playerPosition, &targetPosition) < 100.0f) {
        if (trigger->soundHandle != -1) {
            fn_1_3128(trigger->soundHandle);
        }
        fn_1_10E64(trigger->playerIndex);
        effectModelId = fn_80036EBC(lbl_1_bss_37F6);
        fn_80026A44(effectModelId, 1U);
        fn_80026A44(effectModelId, 0x40000002U);
        fn_800269E8(effectModelId, 0x08000000U);
        fn_80026B8C(effectModelId, 7);
        fn_80026790(effectModelId, &targetPosition);
        fn_1_30A0(&targetPosition, 0x566);
        trigger->triggered = 1;
        trigger->paused = 1;
        trigger->state = 2;
        fn_1_4A70(lbl_1_bss_C4[0]);
        return;
    }

    if (trigger->paused == 0 && !(playerPosition.y > 50.0f + targetPosition.y)) {
        if (playerPosition.x < targetPosition.x - 70.0f ||
            playerPosition.x > targetPosition.x + 70.0f) {
            return;
        }
        switch (trigger->mode) {
        case 0:
            if (playerPosition.z > targetPosition.z - 295.0f &&
                playerPosition.z < targetPosition.z) {
                fn_1_4A70(lbl_1_bss_C4[0]);
                trigger->paused = 1;
            }
            if (playerPosition.z < targetPosition.z + 295.0f &&
                playerPosition.z > targetPosition.z) {
                fn_1_4A70(lbl_1_bss_C4[0]);
                trigger->paused = 1;
            }
            break;
        case 1:
            if (playerPosition.z > targetPosition.z - 295.0f &&
                playerPosition.z < targetPosition.z) {
                fn_1_4A70(lbl_1_bss_C4[0]);
                trigger->paused = 1;
            }
            break;
        case 2:
            if (playerPosition.z < targetPosition.z + 295.0f &&
                playerPosition.z > targetPosition.z) {
                fn_1_4A70(lbl_1_bss_C4[0]);
                trigger->paused = 1;
            }
            break;
        }
        if (trigger->paused != 0) {
            fn_1_3128(trigger->soundHandle);
        }
    }
}

s16 lbl_1_bss_380A;
s16 lbl_1_bss_3808;
u32 lbl_1_bss_3804;
s16 lbl_1_bss_3802;
s16 lbl_1_bss_3800;
s16 lbl_1_bss_37FE;
s16 lbl_1_bss_37FC;
s16 lbl_1_bss_37FA;
s16 lbl_1_bss_37F8;
s16 lbl_1_bss_37F6;
s16 lbl_1_bss_37F4;
s16 lbl_1_bss_37F2;
s16 lbl_1_bss_37EE[2];
s16 lbl_1_bss_37EC;

OMOBJ *lbl_1_bss_37E0[3];
s16 lbl_1_bss_37DE;
s16 lbl_1_bss_37D8[3];
s16 lbl_1_bss_37D6;
s16 lbl_1_bss_37D4;
s16 lbl_1_bss_37D2;
s16 lbl_1_bss_37D0;
s16 lbl_1_bss_37CE;
s16 lbl_1_bss_37CC;
s16 lbl_1_bss_37CA;
s16 lbl_1_bss_37C8;
s32 lbl_1_bss_37C4;
s16 lbl_1_bss_37C0;
