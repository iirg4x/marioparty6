/* Stage actor records, collision maps, and visible block selection. */
#include "REL/mikeactdll/stage_collision.h"
#include "REL/mikeactdll/sound.h"
#include "game/audio.h"
#include "REL/mikeactdll/module_types.h"

#include "string.h"
#include "REL/mikeactdll/module.h"

extern s16 lbl_1_bss_1AE[3];

extern s16 lbl_1_bss_1A8;

extern s16 lbl_1_bss_1AA;

extern s16 lbl_1_bss_1AC;

extern u16 lbl_1_bss_34;

extern s16 lbl_1_bss_1B4;

extern s16 lbl_1_bss_31BA[256];

extern s16 lbl_1_bss_33BA;

extern s16 lbl_1_bss_33BE;

static const s32 lbl_1_rodata_368[3] = {DATANUM(0xa70000, 21), DATANUM(0xa70000, 45), DATANUM(0xa70000, 76)};
static const s32 lbl_1_rodata_374[3] = {DATANUM(0xa70000, 22), DATANUM(0xa70000, 46), DATANUM(0xa70000, 77)};
static const s32 lbl_1_rodata_380[3] = {DATANUM(0xa70000, 23), DATANUM(0xa70000, 47), DATANUM(0xa70000, 78)};
static const s32 lbl_1_rodata_38C[3] = {0, 2, 2};

typedef void (*MikeActSetupCallback)(void);

extern MikeActSetupCallback lbl_1_data_958[3];

extern MikeActSetupCallback lbl_1_data_964[3];

void fn_1_105FC(void);

MikeActSetupCallback lbl_1_data_958[3] = { fn_1_12804, fn_1_1698C, fn_1_1CDEC };
MikeActSetupCallback lbl_1_data_964[3] = { fn_1_12E48, fn_1_17B5C, fn_1_1D594 };
char *lbl_1_data_B28[55] = { "blk01_A", "blk01_B", "blk01_C", "blk01_D", "blk01_E", "blk02_A", "blk02_B", "blk02_C", "blk02_D", "blk02_E", "blk03_A", "blk03_B", "blk03_C", "blk03_D", "blk03_E", "blk04_A", "blk04_B", "blk04_C", "blk04_D", "blk04_E", "blk05_A", "blk05_B", "blk05_C", "blk05_D", "blk05_E", "blk06_A", "blk06_B", "blk06_C", "blk06_D", "blk06_E", "blk07_A", "blk07_B", "blk07_C", "blk07_D", "blk07_E", "blk08_A", "blk08_B", "blk08_C", "blk08_D", "blk08_E", "blk09_A", "blk09_B", "blk09_C", "blk09_D", "blk09_E", "blk10_A", "blk10_B", "blk10_C", "blk10_D", "blk10_E", "blk11_A", "blk11_B", "blk11_C", "blk11_D", "blk11_E" };

extern const Point3d lbl_1_rodata_3A0;
/* Loads the collision stage and starts the mode audio. */
void fn_1_101C0(s16 mode)
{
    s16 modelId;
    s32 index;

    HuAudAUXSet(lbl_1_rodata_38C[mode], -1);
    lbl_1_bss_1AC = 0x37;
    lbl_1_bss_33BA = 0;
    memset(lbl_1_bss_1B8, 0, 0x3000U);
    lbl_1_bss_33BE = fn_80025A04(
        fn_80008DD0(lbl_1_rodata_368[mode], 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_33BE, 3);
    fn_80026FFC(lbl_1_bss_33BE);
    modelId = fn_80025A04(
        fn_80008DD0(lbl_1_rodata_374[mode], 0x10000000, 2));
    fn_1_107EC(modelId);
    modelId = fn_80025A04(
        fn_80008DD0(lbl_1_rodata_380[mode], 0x10000000, 2));
    switch (mode) {
    case 0:
        break;
    case 1:
        modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa70000, 48), 0x10000000,
                                                       2));
        modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa70000, 49), 0x10000000,
                                                       2));
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_80027138(lbl_1_bss_33BE, "side03_C");
        fn_80027138(lbl_1_bss_33BE, "side03_D");
        fn_80027138(lbl_1_bss_33BE, "wall09_B");
        break;
    case 2:
        fn_80026B8C(modelId, 1);
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa70000, 79), 0x10000000,
                                                       2));
        fn_80026B8C(modelId, 0);
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa70000, 80), 0x10000000,
                                                       2));
        fn_80026B8C(modelId, 2);
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_80027138(lbl_1_bss_33BE, "gate");
        fn_80027138(lbl_1_bss_33BE, "tile_wall1");
        fn_80027138(lbl_1_bss_33BE, "tile_wall2");
        fn_80027138(lbl_1_bss_33BE, "tile_wall3");
        fn_80027138(lbl_1_bss_33BE, "tile_wall4");
        fn_80027138(lbl_1_bss_33BE, "tile_wall5");
        fn_80027138(lbl_1_bss_33BE, "tile_wall6");
        break;
    }
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa70000, 1), 0x10000000,
                                                   2));
    fn_80026760(modelId, -800.0f, 0.0f, 0.0f);
    fn_1_107EC(modelId);
    fn_1_105FC();
    lbl_1_data_958[mode]();
    fn_800804E8(lbl_1_bss_31BA, lbl_1_bss_33BA, 0x80);
    for (index = 0; index < lbl_1_bss_33BA; index++) {
        fn_80080D7C(index, 7U);
    }
    fn_80080D7C(1, 1U);
    lbl_1_data_964[mode]();
    lbl_1_bss_1AA = 2;
    lbl_1_bss_1A8 = -2;
    fn_1_11B44();
}

const Point3d lbl_1_rodata_3A0 = { 49000.0f, 50.0f, -500.0f };
/* Places an animated stage model at the requested position. */
void fn_1_105FC(void)
{
    Point3d position;
    f32 verticalOffset;
    s16 modelId;

    position = lbl_1_rodata_3A0;
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa70000, 8), 0x10000000, 2));
    lbl_1_bss_1AE[0] = fn_80030458(
        modelId, fn_80008DD0(DATANUM(0xa70000, 9), 0x10000000, 2));
    lbl_1_bss_1AE[1] = fn_80030458(
        modelId, fn_80008DD0(DATANUM(0xa70000, 10), 0x10000000, 2));
    lbl_1_bss_1AE[2] = fn_80030458(
        modelId, fn_80008DD0(DATANUM(0xa70000, 11), 0x10000000, 2));
    fn_80026E44(modelId);
    if (lbl_1_bss_34 == 2) {
        verticalOffset = 300.0f;
    } else {
        verticalOffset = 0.0f;
    }
    position.y += verticalOffset;
    lbl_1_bss_1B4 = fn_1_10CFC(modelId, &position);
    fn_8002C95C(modelId, lbl_1_bss_1AE[1]);
    fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
}

extern s16 lbl_1_bss_31B8;
/* Changes the selected stage piece's motion. */
void fn_1_10770(s16 selectionNo)
{
    fn_8002CCC0(
        lbl_1_bss_1B8[lbl_1_bss_1B4].model_id,
        lbl_1_bss_1AE[selectionNo],
        0.0f, 10.0f, ((1 << 30) | (1 << 0)));
}
/* Hides the model and records it in the stage work. */
s16 fn_1_107EC(s16 modelId)
{
    lbl_1_bss_31BA[lbl_1_bss_33BA++] = modelId;
    fn_800269E8(modelId, (1 << 0));
    return lbl_1_bss_33BA - 1;
}

/* Reserves a free collision entry and returns its index. */
s16 fn_1_10868(void)
{
    s16 index;

    for (index = 0; index < 256; index++) {
        if ((lbl_1_bss_1B8[index].flags & 1) == 0) {
            memset(&lbl_1_bss_1B8[index], 0, sizeof(lbl_1_bss_1B8[index]));
            lbl_1_bss_1B8[index].flags = 1;
            lbl_1_bss_31B8++;
            return index;
        }
    }
    return -1;
}

s16 fn_1_1092C(MGACTOR *actor, Point3d *position)
{
    s16 activeIndex;

    activeIndex = fn_1_10868();
    if (activeIndex == -1) {
        return -1;
    }
    lbl_1_bss_1B8[activeIndex].actor = actor;
    lbl_1_bss_1B8[activeIndex].kind = 0;
    lbl_1_bss_1B8[activeIndex].position = *position;
    lbl_1_bss_1B8[activeIndex].model_id = actor->mdlId;
    fn_80026790(actor->mdlId, position);
    fn_80086AA8(actor, position);
    fn_80086F44(actor);
    fn_800269E8(lbl_1_bss_1B8[activeIndex].model_id, (1 << 0));
    return activeIndex;
}

s16 fn_1_10AE0(s16 sourceModel, s16 targetModel, Point3d *position)
{
    s16 activeIndex;

    activeIndex = fn_1_10868();
    if (activeIndex == -1) {
        return -1;
    }
    lbl_1_bss_1B8[activeIndex].kind = 1;
    lbl_1_bss_1B8[activeIndex].position = *position;
    lbl_1_bss_1B8[activeIndex].model_id = sourceModel;
    lbl_1_bss_1B8[activeIndex].linked_model_id = targetModel;
    lbl_1_bss_1B8[activeIndex].collision_map_index = fn_1_107EC(targetModel);
    fn_80026790(sourceModel, position);
    fn_80026790(targetModel, position);
    fn_800269E8(sourceModel, (1 << 0));
    lbl_1_bss_1B8[activeIndex].collision_mask = 7;
    return activeIndex;
}

s16 fn_1_10CFC(s16 modelId, Point3d *position)
{
    s16 activeIndex;

    activeIndex = fn_1_10868();
    if (activeIndex == -1) {
        return -1;
    }
    lbl_1_bss_1B8[activeIndex].kind = 2;
    lbl_1_bss_1B8[activeIndex].position = *position;
    lbl_1_bss_1B8[activeIndex].model_id = modelId;
    fn_80026790(modelId, position);
    fn_800269E8(modelId, (1 << 0));
    return activeIndex;
}

void fn_1_10E64(s16 playerNo)
{
    MikeActStateRecord *stateRecord;

    stateRecord = &lbl_1_bss_1B8[playerNo];
    if (stateRecord->flags & 1) {
        switch (stateRecord->kind) {
        case 0:
            fn_80080F38(stateRecord->actor);
            fn_800262D0(stateRecord->model_id);
            break;
        case 1:
            fn_800262D0(stateRecord->model_id);
            fn_80080D7C(stateRecord->collision_map_index, 0U);
            break;
        case 2:
            fn_800262D0(stateRecord->model_id);
            break;
        }
        lbl_1_bss_31B8--;
        memset(stateRecord, 0, sizeof(*stateRecord));
    }
}

void fn_1_10F38(s16 playerNo, Point3d *out) {
    *out = lbl_1_bss_1B8[playerNo].position;
}

void fn_1_10F68(s16 playerNo, const Point3d *in) {
    lbl_1_bss_1B8[playerNo].position = *in;
}

void fn_1_10F98(s16 index, s32 collisionMask)
{
    lbl_1_bss_1B8[index].collision_mask = collisionMask;
}

void fn_1_10FB4(s16 index)
{
    MikeActStateRecord *stateRecord = &lbl_1_bss_1B8[index];
    lbl_1_bss_1B8[index].flags |= 2;
    switch (stateRecord->kind) {
    case 0:
        fn_80086F44(lbl_1_bss_1B8[index].actor);
        return;
    case 1:
        fn_80080D7C(stateRecord->collision_map_index, 0U);
    case 2:
        ;
    }
}

/* Finds the active collision entry for an actor number. */
s16 fn_1_1106C(u32 value)
{
    s16 index;

    for (index = 0; index < 256; index++) {
        if (lbl_1_bss_1B8[index].flags != 0 &&
            lbl_1_bss_1B8[index].kind == 0 &&
            value == (u32)lbl_1_bss_1B8[index].actor->no) {
            return index;
        }
    }
    return -1;
}

/* Finds the active collision entry for a collision map index. */
s16 fn_1_1110C(s16 state)
{
    s16 index;

    for (index = 0; index < 256; index++) {
        if (lbl_1_bss_1B8[index].flags != 0 &&
            lbl_1_bss_1B8[index].kind == 1 &&
            lbl_1_bss_1B8[index].collision_map_index == state) {
            return index;
        }
    }
    return -1;
}

extern s16 lbl_1_bss_33BC;

void fn_1_111A8(Point3d *positions, s32 count)
{
    s16 modelId;
    s32 positionIndex;
    s16 activeIndex;
    s16 linkedModelId;
    s16 sourceModelId;
    s16 sourceLinkedModelId;

    modelId = fn_80025A04(
        fn_80008DD0(DATANUM(0xa70000, 5), 0x10000000, 2));
    fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
    fn_800269E8(modelId, (1 << 0));
    lbl_1_bss_33BC = modelId;

    for (positionIndex = 0; positionIndex < count; positionIndex++) {
        if (positionIndex == 0) {
            sourceModelId = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 4), 0x10000000, 2));
            sourceLinkedModelId = fn_80025A04(
                fn_80008DD0(DATANUM(0xa70000, 6), 0x10000000, 2));
            modelId = sourceModelId;
            linkedModelId = sourceLinkedModelId;
        } else {
            modelId = fn_80025D4C(sourceModelId);
            linkedModelId = fn_80025D4C(sourceLinkedModelId);
        }
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_80026B8C(modelId, 4);
        fn_80026FFC(modelId);

        activeIndex = fn_1_10868();
        if (activeIndex != -1) {
            lbl_1_bss_1B8[activeIndex].kind = 1;
            lbl_1_bss_1B8[activeIndex].position = positions[positionIndex];
            lbl_1_bss_1B8[activeIndex].model_id = modelId;
            lbl_1_bss_1B8[activeIndex].linked_model_id = linkedModelId;
            lbl_1_bss_1B8[activeIndex].collision_map_index = fn_1_107EC(linkedModelId);
            fn_80026790(modelId, &positions[positionIndex]);
            fn_80026790(linkedModelId, &positions[positionIndex]);
            fn_800269E8(modelId, (1 << 0));
            lbl_1_bss_1B8[activeIndex].collision_mask = 7;
        }
    }
}

void fn_1_118CC(OMOBJ *object);

extern HUPROCESS *lbl_1_bss_6C;

void fn_1_114A8(s16 collisionMap)
{
    s16 stateIndex;
    s16 modelId;
    MikeActStateRecord *modelRecord;

    stateIndex = fn_1_1110C(collisionMap);

    if (stateIndex != -1) {
        modelRecord = &lbl_1_bss_1B8[stateIndex];
        modelId = fn_80036EBC(lbl_1_bss_33BC);
        fn_800269E8(modelId, (1 << 27));
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_80026A44(modelId, (1 << 0));
        fn_80026B8C(modelId, 6);
        fn_80026790(modelId, &modelRecord->position);
        fn_80026FFC(modelId);

        fn_1_10E64(stateIndex);
    }
}

void fn_1_11690(Point3d *positions, s32 count)
{
    s32 positionIndex;
    s16 modelId;
    s16 originalModelId;
    s16 recordIndex;
    s16 stateIndex;
    OMOBJ *object;

    for (positionIndex = 0; positionIndex < count; positionIndex++) {
        if (positionIndex == 0) {
            originalModelId = fn_80025A04(fn_80008DD0(DATANUM(0xa70000, 2), 0x10000000, 2));
            modelId = originalModelId;
        } else {
            modelId = fn_80025D4C(originalModelId);
        }
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_80026B8C(modelId, 6);
        fn_8002D730(modelId, 0.0f, 60.0f);
        object = fn_8003AB38(lbl_1_bss_6C, 0x2F00, 0, 0, -1, fn_1_118CC);
        stateIndex = fn_1_10868();
        if (stateIndex == -1) {
            recordIndex = -1;
        } else {
            lbl_1_bss_1B8[stateIndex].kind = 2;
            lbl_1_bss_1B8[stateIndex].position = positions[positionIndex];
            lbl_1_bss_1B8[stateIndex].model_id = modelId;
            fn_80026790(modelId, &positions[positionIndex]);
            fn_800269E8(modelId, (1 << 0));
            recordIndex = stateIndex;
        }
        object->work[0] = (u32)recordIndex;
        object->work[1] = 0;
    }
}

void fn_1_118CC(OMOBJ *object)
{
    Point3d position;
    switch ((s32)object->work[1]) {
    case 0:
        if (lbl_1_bss_B8) {
            fn_80086BC8(lbl_1_bss_B8->player->actor, &position);
            PSVECSubtract(&lbl_1_bss_1B8[object->work[0]].position, &position, &position);
            position.y = 0.0f;
            if (PSVECMag(&position) < 100.0f) {
                object->work[1] = 1;
                fn_1_4D6C(lbl_1_bss_B8, 1);
                fn_1_5BC8(lbl_1_bss_B8, 1);
                fn_80026A44(lbl_1_bss_1B8[object->work[0]].model_id, ((1 << 30) | (1 << 0)));
                fn_8002D730(lbl_1_bss_1B8[object->work[0]].model_id, 0.0f, 120.0f);
                fn_80086BC8(lbl_1_bss_B8->player->actor, &position);
                fn_1_317C(0x6A);
                fn_1_30A0(&position, 0x54C);
            }
        }
        break;
    case 1:
        if (fn_8002D75C(lbl_1_bss_1B8[object->work[0]].model_id) != 0) {
            fn_1_10E64((s16)object->work[0]);
            omDelObjEx(fn_8000DD9C(), object);
        }
        break;
    case 2:
        break;
    }
}

/* Shows stage sections around the current position and updates collision entries. */
void fn_1_11B44(void)
{
    Point3d position;
    s32 lowerIndex;
    s32 upperIndex;
    s32 index;
    HSF_OBJECT *modelObject;

    for (index = 0; index < lbl_1_bss_1AC; index++) {
        modelObject = fn_80026BF4(lbl_1_bss_33BE, lbl_1_data_B28[index]);
        modelObject->mesh.base.scale.x = modelObject->mesh.base.scale.y =
            modelObject->mesh.base.scale.z = 0.0f;
    }

    upperIndex = (s32)((1000.0f + lbl_80266A4C.x) / 1000.0f) + lbl_1_bss_1AA;
    lowerIndex = (s32)((1000.0f + lbl_80266A4C.x) / 1000.0f) + lbl_1_bss_1A8;
    if (upperIndex >= lbl_1_bss_1AC) {
        upperIndex = lbl_1_bss_1AC - 1;
    }
    if (lowerIndex <= 0) {
        lowerIndex = 0;
    }
    for (index = lowerIndex; index <= upperIndex; index++) {
        modelObject = fn_80026BF4(lbl_1_bss_33BE, lbl_1_data_B28[index]);
        modelObject->mesh.base.scale.x = modelObject->mesh.base.scale.y =
            modelObject->mesh.base.scale.z = 1.0f;
    }

    for (index = 0; index < 256; index++) {
        if ((lbl_1_bss_1B8[index].flags & 1) == 0 ||
            (lbl_1_bss_1B8[index].flags & 2) != 0) {
            continue;
        }
        switch (lbl_1_bss_1B8[index].kind) {
        case 0:
            fn_80086BC8(lbl_1_bss_1B8[index].actor, &position);
            if (position.x >= (1000.0f * (f32)lowerIndex) - 1000.0f &&
                position.x < 1000.0f * (f32)upperIndex) {
                fn_80086FA0(lbl_1_bss_1B8[index].actor, &position);
                fn_80026A44(lbl_1_bss_1B8[index].model_id, (1 << 0));
            } else {
                fn_80086F44(lbl_1_bss_1B8[index].actor);
                fn_800269E8(lbl_1_bss_1B8[index].model_id, (1 << 0));
            }
            break;
        case 1:
            position = lbl_1_bss_1B8[index].position;
            if (position.x >= (1000.0f * (f32)lowerIndex) - 1000.0f &&
                position.x < 1000.0f * (f32)upperIndex) {
                fn_80026A44(lbl_1_bss_1B8[index].model_id, (1 << 0));
                fn_80080D7C(lbl_1_bss_1B8[index].collision_map_index,
                                     lbl_1_bss_1B8[index].collision_mask);
            } else {
                fn_800269E8(lbl_1_bss_1B8[index].model_id, (1 << 0));
                fn_80080D7C(lbl_1_bss_1B8[index].collision_map_index, 0U);
            }
            break;
        case 2:
            position = lbl_1_bss_1B8[index].position;
            if (position.x >= (1000.0f * (f32)lowerIndex) - 1000.0f &&
                position.x < 1000.0f * (f32)upperIndex) {
                fn_80026A44(lbl_1_bss_1B8[index].model_id, (1 << 0));
            } else {
                fn_800269E8(lbl_1_bss_1B8[index].model_id, (1 << 0));
            }
            break;
        }
    }
}

s16 lbl_1_bss_33BE;
s16 lbl_1_bss_33BC;
s16 lbl_1_bss_33BA;
s16 lbl_1_bss_31BA[256];
s16 lbl_1_bss_31B8;
MikeActStateRecord lbl_1_bss_1B8[256];
s16 lbl_1_bss_1B4;
s16 lbl_1_bss_1AE[3];
s16 lbl_1_bss_1AC;
s16 lbl_1_bss_1AA;
s16 lbl_1_bss_1A8;
