/* Player model hooks, effect attachment, and horizontal distance checks. */

#include "REL/mikeactdll/model_hook_setup.h"
#include "REL/mikeactdll/position.h"
#include "game/audio.h"
#include "math.h"

#include "REL/mikeactdll/module_types.h"

#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "game/mg/actman.h"

#include "REL/mikeactdll/shared_types.h"
#include <string.h>

void fn_1_D994(void);

void fn_1_E208(void);

const Point3d lbl_1_rodata_278 = { -300.0f, 800.0f, 0.0f };

void fn_1_D950(void)
{
    fn_1_D994();
    fn_1_E208();
}

int fn_1_D974(COL_NARROW_PARAM *self, COL_NARROW_PARAM *comparisonData) {
    if (comparisonData->paramB > 10)
        return 1;
    return 0;
}

void fn_1_D990(void)
{
}

void fn_1_DB04(OMOBJ *object);

extern HUPROCESS *lbl_1_bss_6C;

extern s32 lbl_1_bss_E8;

MGACTOR_PARAM lbl_1_data_478 = { 2.0f, 50.0f, 10, 5, fn_1_D974, 0xA0, (int)fn_1_D990, 0 };

/* Creates the stage actor and places it at its initial position. */
void fn_1_D994(void)
{
    Point3d position;
    s16 modelId;
    OMOBJ *object;
    MGACTOR *actor;

    position = lbl_1_rodata_278;
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa70000, 0), 0x10000000, 2));
    fn_80026E44(modelId);
    fn_800268B0(modelId, 0.35f, 0.35f, 0.35f);
    object = fn_8003AB38(lbl_1_bss_6C, 0x2100, 0U, 0U, -1, fn_1_DB04);
    object->data = HuMemDirectMallocNum(HEAP_HEAP, 0x1C, 0x10000000);
    lbl_1_bss_F0 = object->data;
    memset(lbl_1_bss_F0, 0, 0x1CU);
    actor = fn_80080B04(&lbl_1_data_478, modelId);
    fn_80086CBC(actor, 10.0f);
    fn_80086AA8(actor, &position);
    lbl_1_bss_F0->actor = actor;
    lbl_1_bss_F0->owner = 0;
    lbl_1_bss_E8 = 1;
}

extern s16 lbl_1_bss_EC;
/* Positions the held item at its character model hook. */
void fn_1_DB04(OMOBJ *object)
{
    Point3d modelPosition;
    char *itemHook;
    f32 rotation;

    if (lbl_1_bss_F0->owner) {
        fn_80086BC8(
            lbl_1_bss_F0->owner->player->actor,
            &lbl_1_bss_F0->position);
        itemHook = fn_8005C0F0(
            lbl_1_bss_F0->owner->character_no,
            4, 0);
        Hu3DModelObjPosGet(
            lbl_1_bss_F0->owner->player->actor->mdlId,
            itemHook, &modelPosition);
    } else {
        fn_80086BC8(lbl_1_bss_F0->actor,
                      &lbl_1_bss_F0->position);
        fn_80086BC8(lbl_1_bss_F0->actor,
                      &modelPosition);
    }

    if (lbl_1_bss_EC != -1) {
        Hu3DParManPosSet(lbl_1_bss_EC, modelPosition.x,
                         40.0f + modelPosition.y,
                         modelPosition.z);
    }

    if (lbl_1_bss_E8 != 0) {
        if ((lbl_1_bss_F0->actor->colGroundAttr &
             0x7FU) != 0) {
            lbl_1_bss_E8 = 0;
            fn_80086CA8(lbl_1_bss_F0->actor,
                           0.0f);
            return;
        }
        fn_80086CB0(lbl_1_bss_F0->actor,
                       &rotation);
        rotation += 30.0f;
        fn_80086CA8(lbl_1_bss_F0->actor, rotation);
    }
}

void fn_1_DCEC(MikeActPlayerRecord *record)
{
    s16 modelId;
    char *hookName;

    if (lbl_1_bss_F0->owner == 0) {
        lbl_1_bss_F0->owner = record;
        fn_1_45B0(record, 1);
        fn_80086F44(lbl_1_bss_F0->actor);
        hookName = fn_8005C0F0(record->character_no, 4, 0);
        modelId = (s16) lbl_1_bss_F0->actor->mdlId;
        fn_8002741C((s16) record->player->actor->mdlId, hookName, modelId);
        fn_80026760(modelId, 0.0f, 0.0f, 0.0f);
        fn_1_4F3C(record);
        lbl_1_bss_F0->state = 1;
    }
}

void fn_1_DDFC(void)
{
    Point3d position;
    s16 modelId;
    char *hookName;

    if (lbl_1_bss_F0->owner != NULL) {
        lbl_1_bss_F0->state = 2;
        modelId = lbl_1_bss_F0->owner->player->actor->mdlId;
        hookName = fn_8005C0F0(lbl_1_bss_F0->owner->character_no, 4, 0);
        Hu3DModelObjPosGet(lbl_1_bss_F0->owner->player->actor->mdlId,
                           hookName, &position);
        fn_80086FA0(lbl_1_bss_F0->actor, &position);
        fn_80026790(lbl_1_bss_F0->actor->mdlId, &position);
        fn_80086CC4(lbl_1_bss_F0->actor,
            130.0f + 130.0f * ((-1.0f + sqrtf(1231.7693f)) / 2.0f));
        lbl_1_bss_E8 = 1;
        fn_80027508(modelId);
        lbl_1_bss_F0->position = position;
        fn_1_45C4(lbl_1_bss_F0->owner, 1);
        lbl_1_bss_F0->owner = 0;
        fn_1_4F3C(0);
    }
}

void fn_1_E0B8(s32 value)
{
    lbl_1_bss_E8 = value;
}

/* Measure horizontal distance from the current actor to a position. */
f32 MikeActHorizontalDistanceGet(Point3d *position) {
    Point3d actorPosition;

    fn_80086BC8(lbl_1_bss_F0->actor, &actorPosition);
    actorPosition.y = position->y;
    return PSVECDistance(position, &actorPosition);
}

s32 fn_1_E11C(MikeActPlayerRecord *turnState)
{
    HuVecF playerPosition;
    f32 distance;

    if (lbl_1_bss_F0->owner != 0) {
        return 0;
    }
    fn_80086BC8(turnState->player->actor, &playerPosition);
    distance = MikeActHorizontalDistanceGet(&playerPosition);
    return distance <= 105.0f;
}

MikeActPlayerRecord *fn_1_E1F8(void)
{
    return lbl_1_bss_C4[0];
}

extern struct Hu3DParmanParam_s lbl_1_data_498;

/* Loads and configures the stage particle manager. */
void fn_1_E208(void)
{
    ANIMDATA *animation;
    s16 particleManager;

    animation = HuSprAnimDataRead(0x210001);
    particleManager = fn_800334F4(animation, 0x64, &lbl_1_data_498);
    Hu3DParManAttrSet(particleManager, 0x64);
    fn_8003269C(Hu3DParManModelIDGet(particleManager), 1U);
    Hu3DParManRotSet(
        particleManager, 90.0f,
        0.0f, 0.0f);
    fn_80033E64(particleManager, 7);
    lbl_1_bss_EC = particleManager;
}

void fn_1_E2C8(void) {
    Hu3DParManAttrSet(lbl_1_bss_EC, 1);
    lbl_1_bss_EC = -1;
}

struct Hu3DParmanParam_s lbl_1_data_498 = { 30, 0, 3.3f, 30.0f, 0.0f, { 0.0f, -0.05f, 0.0f }, 2.0f, 1.0f, 20.0f, 0.98f, 2, { { 255, 255, 255, 255 }, { 255, 255, 64, 255 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }, { { 255, 128, 128, 0 }, { 255, 64, 32, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } } };

MikeActWorldContext *lbl_1_bss_F0;
s16 lbl_1_bss_EC;
s32 lbl_1_bss_E8;
