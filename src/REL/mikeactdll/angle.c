/* Player movement, motion selection, jumping, and event callbacks. */
#include "REL/mikeactdll/sound.h"
#include "REL/mikeactdll/model_hook_setup.h"
#include "REL/mikeactdll/position.h"
#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/mtx.h"

#include "game/audio.h"

#include "REL/mikeactdll/event_callback.h"

#include "REL/mikeactdll/module_types.h"

#include "game/hu3d.h"

#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "game/mic.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "humath.h"

#include "messdir_enum.h"

extern f32 lbl_1_data_42C[3];

extern HUPROCESS *lbl_1_bss_6C;

extern u16 lbl_1_bss_34;

extern MGACTOR_PARAM lbl_1_data_128;

extern MGPLAYER_MODE_FUNC lbl_1_data_148[18];

extern MGPLAYER_MODE_FUNC lbl_1_data_190[18];

extern MGPLAYER_MODE_FUNC lbl_1_data_1D8[18];

extern unsigned int lbl_1_data_220[];

extern unsigned int lbl_1_data_25C[];

extern unsigned int lbl_1_data_2A0[];

extern HuVecF lbl_1_data_2E0[4];

void fn_1_4FB0(OMOBJ *object);

void fn_1_40A0(s16 playerIndex)
{
    OMOBJ *object;
    MikeActPlayerRecord *record;
    MGACTOR_PARAM actorParam;
    s16 modelId;

    object = fn_8003AB38(lbl_1_bss_6C, 0x1000, 0, 0, -1, fn_1_4FB0);
    object->data = HuMemDirectMallocNum(HEAP_HEAP, 0x5C, 0x10000000);
    record = object->data;
    memset(record, 0, 0x5C);
    lbl_1_bss_C4[playerIndex] = record;
    record->player_no = playerIndex;
    record->character_no = lbl_80265728[playerIndex].charNo;
    record->actionState = 0;
    record->targetLane = -1;
    record->callback = fn_1_5E98;

    actorParam = lbl_1_data_128;
    actorParam.param = playerIndex;
    switch (lbl_1_bss_34) {
    case 0:
        record->player = fn_80080728(playerIndex, &actorParam, 4, 1,
                                        0xFFFFFFE3U, lbl_1_data_220);
        fn_800806DC(record->player, lbl_1_data_148);
        break;
    case 1:
        record->player = fn_80080728(playerIndex, &actorParam, 4, 1,
                                        0xFFFFFFE3U, lbl_1_data_25C);
        fn_800806DC(record->player, lbl_1_data_190);
        break;
    case 2:
        record->player = fn_80080728(playerIndex, &actorParam, 4, 1,
                                        0xFFFFFFE3U, lbl_1_data_2A0);
        fn_800806DC(record->player, lbl_1_data_1D8);
        break;
    }

    fn_8008697C(record->player, &lbl_1_data_2E0[playerIndex]);
    modelId = record->player->actor->mdlId;
    fn_80026790(modelId, &lbl_1_data_2E0[playerIndex]);
    fn_80026E44(modelId);
    fn_80026B8C(modelId, 5);
    fn_80080DD4(record->player->actor, 7);
    fn_80080520(record->player, 0, 0, 0, 0);
    if (playerIndex == 0) {
        fn_80086CA8(record->player->actor, 90.0f);
    } else {
        fn_80086CA8(record->player->actor, 0.0f);
    }
    fn_1_45E4(record);
}

int fn_1_4378(COL_NARROW_PARAM *playerContext,
               COL_NARROW_PARAM *eventTarget)
{
    HuVecF separation;
    HuVecF adjustedPosition;
    MikeActPlayerRecord *playerRecord;
    s16 direction;

    playerRecord = lbl_1_bss_C4[playerContext->paramB];
    if (lbl_1_bss_C0 != 0) {
        if (lbl_1_bss_C0(playerContext, eventTarget) != 0) {
            return 1;
        }
    }
    if (fn_1_45D8(playerRecord, 0x2000) != 0) {
        return 0;
    }
    if (eventTarget->paramB >= 0xB && eventTarget->paramB < 0x64) {
        if (playerContext->colResult == 0 || playerContext->colResult == 2) {
            PSVECSubtract(&eventTarget->point, &playerContext->point, &separation);
            separation.y = 0.0f;
            PSVECScale(&separation, &separation, 1.01f);
            PSVECSubtract(&eventTarget->point, &separation, &adjustedPosition);
            playerContext->point.x = adjustedPosition.x;
            playerContext->point.z = adjustedPosition.z;
            direction = fn_1_477C(playerRecord);
            if (direction != -1 && playerRecord->targetLane == -1) {
                playerContext->point.z = lbl_1_data_42C[direction];
            }
        }
        return 1;
    }
    if (eventTarget->paramB == 0xA) {
        return 1;
    }
    return 0;
}

extern unsigned int lbl_1_data_220[15];

extern unsigned int lbl_1_data_25C[17];

extern unsigned int lbl_1_data_2A0[16];

void fn_1_4518(MikeActEventCallback callback)
{
    lbl_1_bss_C0 = callback;
}

extern void (*lbl_1_bss_BC)(void *, s32);

void fn_1_4528(MGACTOR *context, int callbackArgument)
{
    MikeActPlayerRecord *playerContext;

    playerContext = lbl_1_bss_C4[context->param];
    if (lbl_1_bss_BC != 0) {
        lbl_1_bss_BC(context, callbackArgument);
    }
}

void fn_1_45A0(void (*callback)(void *, s32))
{
    lbl_1_bss_BC = callback;
}

void fn_1_45B0(MikeActPlayerRecord *record, s32 flags)
{
    record->flags = (s16)(record->flags | flags);
}

void fn_1_45C4(MikeActPlayerRecord *record, s32 flags)
{
    record->flags = (s16)(record->flags & ~flags);
}

s32 fn_1_45D8(MikeActPlayerRecord *record, s32 flags)
{
    return record->flags & flags;
}

void fn_1_45E4(MikeActPlayerRecord *record)
{
    HuVecF actorPosition;

    fn_80086BC8(record->player->actor, &actorPosition);
    if (actorPosition.z < (lbl_1_data_42C[0] + lbl_1_data_42C[1]) / 2.0f) {
        record->lane = 0;
    } else if (actorPosition.z < (lbl_1_data_42C[1] + lbl_1_data_42C[2]) / 2.0f) {
        record->lane = 1;
    } else {
        record->lane = 2;
    }
    record->targetLane = -1;
}

void fn_1_46B4(MikeActPlayerRecord *record)
{
    HuVecF actorPosition;

    fn_80086BC8(record->player->actor, &actorPosition);
    if (actorPosition.z < (lbl_1_data_42C[0] + lbl_1_data_42C[1]) / 2.0f) {
        record->lane = 0;
    } else if (actorPosition.z < (lbl_1_data_42C[1] + lbl_1_data_42C[2]) / 2.0f) {
        record->lane = 1;
    } else {
        record->lane = 2;
    }
}

s16 fn_1_477C(MikeActPlayerRecord *actorOwner)
{
    HuVecF actorPosition;
    s32 index;

    fn_80086BC8(actorOwner->player->actor, &actorPosition);
    for (index = 0; index < 3; index++) {
        if (actorPosition.z > lbl_1_data_42C[index] - 15.0f &&
            actorPosition.z < 15.0f + lbl_1_data_42C[index]) {
            return index;
        }
    }
    return -1;
}

f32 fn_1_483C(MikeActPlayerRecord *actorOwner, HuVecF *position)
{
    HuVecF actorPosition;
    HuVecF offset;

    fn_80086BC8(actorOwner->player->actor, &actorPosition);
    PSVECSubtract(position, &actorPosition, &offset);
    return (f32)(180.0 * (atan2((f64)offset.x, (f64)offset.z) / M_PI));
}

typedef struct MikeActActorDirectionContext_s {
    MGPLAYER *player;
    u8 unknown_04[0x10]; /* No code names this field. */
    f32 angle;
} MikeActActorDirectionContext;

void fn_1_48B8(MikeActActorDirectionContext *context, HuVecF *position)
{
    HuVecF offset;
    HuVecF actorPosition;
    f32 angleDegrees;
    f32 outputAngle;

    fn_80086BC8(context->player->actor, &actorPosition);
    PSVECSubtract(position, &actorPosition, &offset);
    angleDegrees = 180.0 *
        (atan2((f64)offset.x, (f64)offset.z) / M_PI);
    outputAngle = angleDegrees;
    context->angle = outputAngle;
}

MikeActPlayerRecord *fn_1_495C(s16 characterNo)
{
    s32 index;
    MikeActPlayerRecord *record;

    for (index = 0; index < 4; index++) {
        record = lbl_1_bss_C4[index];
        if ((record->flags & 2) == 0 && record->character_no == characterNo) {
            return record;
        }
    }
    return NULL;
}

void fn_1_49D0(MikeActPlayerRecord *record)
{
    record->flags = (s16)(record->flags & ~0x100C);
    fn_1_5AD8(lbl_1_bss_B8, 0);
}

s32 fn_1_4A20(void)
{
    MikeActPlayerRecord *record;

    if (lbl_1_bss_B8) {
        record = lbl_1_bss_B8;
        if ((record->flags & 0xC) != 0) {
            return 1;
        }
    }
    return 0;
}

void fn_1_4A70(MikeActPlayerRecord *record)
{
    Point3d soundPosition;

    if ((record->flags & 4) == 0 &&
        fn_8008718C(record->player, 2, -90.0f, -1) != 0) {
        fn_1_46B4(record);
        record->targetLane = -1;
        fn_1_5AD8(record, 0);
        fn_1_5C0C(record, 0);
        fn_1_1BF4();
        fn_80086BC8(record->player->actor, &soundPosition);
        fn_1_30A0(&soundPosition, 0x53A);
        if (record == lbl_1_bss_B8) {
            fn_1_DDFC();
        }
        if (HuMCMicGet() == 2) {
            fn_8006A030(record->player_no, 0x14, 7, 3);
        }
    }
}

void fn_1_4BE4(MikeActPlayerRecord *record, Point3d *position)
{
    record->flags = (s16)(record->flags & ~0x100C);
    fn_1_5AD8(lbl_1_bss_B8, 0);
    fn_80086F00(record->player, position);
    fn_80087158(record->player, 1U);
    fn_8007EEBC(record->player, 3);
    record->flags = (s16)(record->flags | 0x2000);
    fn_80026A44(record->player->actor->mdlId, 1U);
}

/* static */

void fn_1_4C84(Point3d *positions) {
    s32 player_index;
    MikeActPlayerRecord *player;

    fn_1_30A0(&positions[1], 0x540);
    player_index = 1;
    while (player_index < 4) {
        player = lbl_1_bss_C4[player_index];
        player->flags &= ~0x100C;
        fn_1_5AD8(lbl_1_bss_B8, 0);
        fn_80086F00(player->player, &positions[player_index - 1]);
        fn_80087158(player->player, 1U);
        fn_8007EEBC(player->player, 3);
        player->flags |= 0x2000;
        fn_80026A44(player->player->actor->mdlId, 1U);
        player_index += 1;
    }
}

void fn_1_4D6C(MikeActPlayerRecord *record, s32 enabled)
{
    s16 modelId;
    HU3D_MODEL *model;
    HSF_DATA *modelData;
    HSF_MATERIAL *material;
    s32 index;
    modelId = (s16) record->player->actor->mdlId;
    model = &lbl_802C01C8[modelId];
    modelData = lbl_802C01C8[modelId].hsf;
    material = modelData->material;
    for (index = 0; index < modelData->materialNum; index++, material++) {
        material->refAlpha = (f32) (enabled ? 1 : 0);
        material->hiliteScale = (f32) (enabled ? 1 : 0);
    }
}

void fn_1_4E78(OMOBJ *object)
{
    Point3d actorPosition;

    if (lbl_1_bss_B8) {
        fn_80086BC8(lbl_1_bss_B8->player->actor, &actorPosition);
        actorPosition.x -= 20.0f;
        actorPosition.y += 60.0f +
            fn_8005C4C8(lbl_1_bss_B8->character_no);
        actorPosition.z -= 10.0f;
        omSetTra(object, actorPosition.x, actorPosition.y, actorPosition.z);
    }
}

void fn_1_4FAC(s32 enabled);

void fn_1_4F3C(MikeActPlayerRecord *record)
{
    if (record != 0) {
        fn_1_4FAC(1);
        if (HuMCMicGet() == 2) {
            fn_8006A030(record->player_no, 0x14, 4, 4);
        }
    } else {
        fn_1_4FAC(0);
    }
    lbl_1_bss_B8 = record;
}

void fn_1_4FAC(s32 enabled)
{
}

void fn_1_50C8(MikeActPlayerRecord *record);

void fn_1_5468(MikeActPlayerRecord *record);

void fn_1_52D8(MikeActPlayerRecord *record);

void fn_1_4FB0(OMOBJ *object)
{
    MikeActPlayerRecord *record = object->data;
    HuVecF actorPosition;

    record->buttons_down = (record->buttons = 0);
    if ((record->flags & 0x2000) != 0) {
        return;
    }
    if (record->callback != 0) {
        record->callback(record);
    }
    fn_1_50C8(record);
    fn_1_5468(record);
    if ((record->player->actor->colGroundAttr & 0xBF80) != 0) {
        fn_80086BC8(record->player->actor, &actorPosition);
        if (PSVECDistance(&actorPosition, &record->previous_position) < 2.0f) {
            record->stationaryFrames += 1;
            if (record->stationaryFrames >= 0x5A) {
                fn_1_5AD8(record, 0);
                fn_1_5C0C(record, 0);
            }
        }
    } else {
        record->stationaryFrames = 0;
    }
    fn_80086BC8(record->player->actor, &record->previous_position);
    fn_1_52D8(record);
}

extern f32 lbl_1_data_438[3];

extern f32 lbl_1_data_444[6];

void fn_1_50C8(MikeActPlayerRecord *record)
{
    Point3d currentPosition;
    s16 targetLane;

    if (record != lbl_1_bss_B8) {
        return;
    }
    targetLane = fn_1_477C(record);
        if (record->targetLane != -1) {
            if (targetLane != record->targetLane) {
                fn_80086BC8(record->player->actor, &currentPosition);
                if (currentPosition.z < lbl_1_data_42C[record->targetLane]) {
                    record->movement_angle = lbl_1_data_444[record->movementState];
                    return;
                } else {
                    record->movement_angle = lbl_1_data_438[record->movementState];
                    return;
                }
            }
            record->lane = targetLane;
            record->targetLane = -1;
            if (record->movementState == 0) {
                fn_1_5AD8(record, 0);
                fn_80086CA8(record->player->actor, record->movement_angle);
                return;
            }
            if (record->movementState == 2) {
                record->movement_angle = -90.0f;
                fn_80086CA8(record->player->actor, record->movement_angle);
                return;
            }
            record->movement_angle = 90.0f;
            fn_80086CA8(record->player->actor, record->movement_angle);
            return;
        }
    if (record->actionState == 3) {
        return;
    }
}

void fn_1_52D8(MikeActPlayerRecord *record)
{
    HuVecF movement;
    Mtx cameraMatrix;
    f32 speed;
    s8 padX;
    s8 padZ;

    speed = record->movement_speed;
    if (speed > 0.0f) {
        if (record->flags & 0xC) {
            speed = 20.0f;
            fn_8005C390(record->character_no, 2.0f);
        } else {
            fn_8005C390(record->character_no, 1.0f);
        }
    }

    movement.x = (f32)((f64)speed * sin((f64)record->movement_angle * M_PI / 180.0));
    movement.y = 0.0f;
    movement.z = (f32)((f64)speed * cos((f64)record->movement_angle * M_PI / 180.0));
    fn_1_3D50(cameraMatrix);
    PSMTXMultVec(cameraMatrix, &movement, &movement);
    padX = (s8)(5.6f * movement.x);
    padZ = (s8)(5.6f * movement.z);
    fn_80080520(record->player,
                   padX,
                   padZ,
                   record->buttons_down,
                   record->buttons);
}

void fn_1_5468(MikeActPlayerRecord *record)
{
    HU3D_MODEL *model;
    HSF_DATA *hsf;
    HSF_MATERIAL *material;
    s16 modelId;
    s32 materialIndex;
    HuVecF actorPosition;

    if (record->timer != 0) {
        record->timer--;
        if (record->timer == 0) {
            record->flags &= ~8;
            fn_1_5B4C(record, record->movementState);
        }
    }
    if (record->duration != 0) {
        record->duration--;
        if (record->duration == 0) {
            fn_1_31C8();
            record->flags &= ~4;
            modelId = record->player->actor->mdlId;
            model = &lbl_802C01C8[modelId];
            hsf = lbl_802C01C8[modelId].hsf;
            material = hsf->material;
            for (materialIndex = 0; materialIndex < hsf->materialNum; materialIndex++, material++) {
                material->refAlpha = 0.0f;
                material->hiliteScale = 0.0f;
            }
        }
    }
    {
        MGACTOR *actor = record->player->actor;
        if (actor->colGroundAttr == 8) {
            fn_1_5AD8(record, 1);
            fn_80086BC8(actor, &actorPosition);
            fn_1_30A0(&actorPosition, 0x54C);
            fn_1_114A8(actor->colMesh);
            fn_1_5BC8(record, 0);
        }
    }
}

extern MikeActPlayerRecord *fn_1_E1F8(void);

void fn_1_6AA8(MGPLAYER *);
void fn_1_6AAC(MGPLAYER *);
void fn_1_74D8(MGPLAYER *);
void fn_1_9014(MGPLAYER *);
void fn_1_9F00(MGPLAYER *);
void fn_1_A8F4(MGPLAYER *);
void fn_1_AD6C(MGPLAYER *);
void fn_1_B244(MGPLAYER *);
void fn_1_B754(MGPLAYER *);
void fn_1_BCF4(MGPLAYER *);
int fn_1_4378(COL_NARROW_PARAM *, COL_NARROW_PARAM *);
void fn_1_4528(MGACTOR *, int);
MGACTOR_PARAM lbl_1_data_128 = { 150.0f, 50.0f, 0, 0, fn_1_4378, 0, 0, fn_1_4528 };
MGPLAYER_MODE_FUNC lbl_1_data_148[18] = { fn_1_6AAC, fn_1_74D8, fn_1_9014, fn_1_A8F4, fn_1_AD6C, fn_1_B244, fn_1_B754, fn_1_9F00, fn_1_BCF4, 0, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8 };
MGPLAYER_MODE_FUNC lbl_1_data_190[18] = { fn_1_6AAC, fn_1_74D8, fn_1_9014, fn_1_A8F4, fn_1_AD6C, fn_1_B244, fn_1_B754, fn_1_9F00, fn_1_BCF4, 0, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8 };
MGPLAYER_MODE_FUNC lbl_1_data_1D8[18] = { fn_1_6AAC, fn_1_74D8, fn_1_9014, fn_1_A8F4, fn_1_AD6C, fn_1_B244, fn_1_B754, fn_1_9F00, fn_1_BCF4, 0, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8, fn_1_6AA8 };
unsigned int lbl_1_data_220[15] = { 0x930000, 0x930001, 0x930002, 0x930003, 0x930004, 0x930011, 0x930016, 0x8E002B, 0x8E002C, 0x930018, 0x930026, 0x930036, 0x930020, 0x93004C, 0 };
unsigned int lbl_1_data_25C[17] = { 0x930000, 0x930001, 0x930002, 0x930003, 0x930004, 0x930011, 0x930016, 0x8E002B, 0x8E002C, 0x930018, 0x930026, 0x930036, 0x930020, 0x93002D, 0x8E0073, 0x8E0074, 0 };
unsigned int lbl_1_data_2A0[16] = { 0x930000, 0x930001, 0x930002, 0x930003, 0x930004, 0x930011, 0x930016, 0x8E002B, 0x8E002C, 0x930018, 0x930026, 0x930036, 0x930020, 0x93001C, 0x930005, 0 };
HuVecF lbl_1_data_2E0[4] = { { -650.0f, 0.0f, 0.0f }, { -1300.0f, 0.0f, -300.0f }, { -1100.0f, 0.0f, -300.0f }, { -900.0f, 0.0f, -300.0f } };
MikeActMotionEntry lbl_1_data_310[0x16] = { { 0, 65535, 5.0f, 0x40000001 }, { 1, 65535, 5.0f, 0x40000001 }, { 2, 65535, 5.0f, 0x40000001 }, { 3, 65535, 1.0f, 0 }, { 4, 65535, 5.0f, 0 }, { 0, 7, 5.0f, 0x40000001 }, { 1, 7, 5.0f, 0x40000001 }, { 2, 8, 5.0f, 0x40000001 }, { 3, 8, 1.0f, 0 }, { 4, 7, 5.0f, 0 }, { 5, 65535, 5.0f, 0 }, { 6, 65535, 5.0f, 0 }, { 9, 65535, 5.0f, 0 }, { 10, 65535, 5.0f, 0 }, { 11, 65535, 5.0f, 0 }, { 12, 65535, 5.0f, 0 }, { 13, 65535, 0.0f, 0x40000003 }, { 13, 65535, 1.0f, 0x40000001 }, { 14, 65535, 5.0f, 0x40000001 }, { 15, 65535, 5.0f, 0x40000001 }, { 13, 65535, 10.0f, 0x40000002 }, { 14, 65535, 0.0f, 0 } };
f32 lbl_1_data_418[5] = { 0.0f, 4.99f, 7.5f, 10.0f, 12.5f };
f32 lbl_1_data_42C[3] = { -300.0f, 0.0f, 300.0f };
f32 lbl_1_data_438[3] = { 180.0f, 135.0f, -135.0f };
f32 lbl_1_data_444[6] = { 0.0f, 45.0f, -45.0f, 90.0f, 90.0f, -90.0f };

void fn_1_55E8(MikeActPlayerRecord *record, s16 action)
{
    HuVecF actorPosition;

    if (fn_1_1C5C() == 2) {
        record = fn_1_E1F8();
        lbl_1_bss_B8 = record;
        fn_1_5C0C(record, 3);
        return;
    }
    if (record != NULL && (record->flags & 0x2000) == 0) {
    switch (action) {
    case 0:
        record->stationaryFrames = 0;
        fn_80086BC8(record->player->actor, &actorPosition);
        if (record->movementState != 1) {
            if (record->movementState != 2) {
                fn_1_30A0(&actorPosition, 0x539);
            }
            fn_1_45E4(record);
            record->movement_angle = 90.0f;
            fn_1_5AD8(record, 1);
            fn_1_5C0C(record, 0);
            if (fn_1_477C(record) == -1) {
                record->targetLane = record->lane;
            }
        } else if (record->speedIndex != 4) {
            record->speedIncreaseCount++;
            if (record->speedIncreaseCount == 1) {
                fn_1_30A0(&actorPosition, 0x539);
                record->speedIndex++;
                record->speedIncreaseCount = 0;
                if (record->timer == 0) {
                    fn_1_5B4C(record, 1);
                }
            }
        }
        break;
    case 1:
        if (record->lane != 0) {
            record->stationaryFrames = 0;
            if (record->movementState == 0) {
                fn_1_5B4C(record, 1);
            }
            record->targetLane = record->lane - 1;
        }
        break;
    case 2:
        if (record->lane != 2) {
            record->stationaryFrames = 0;
            if (record->movementState == 0) {
                fn_1_5B4C(record, 1);
            }
            record->targetLane = record->lane + 1;
        }
        break;
    case 3:
        record->stationaryFrames = 0;
        fn_1_46B4(record);
        record->targetLane = -1;
        record->movement_angle = -90.0f;
        fn_1_5AD8(record, 2);
        fn_1_5C0C(record, 0);
        if (fn_1_477C(record) == -1) {
            record->targetLane = record->lane;
        }
        break;
    case 4:
        fn_1_5C0C(record, 1);
        break;
    case 5:
        if (record->actionState != 1) {
            fn_1_5AD8(record, 0);
            fn_1_5C0C(record, 0);
        }
        break;
    case 6:
        break;
    }
    }
}

void fn_1_5AD8(MikeActPlayerRecord *record, s16 state)
{
    record->movementState = state;
    if (state == 0) {
        record->timer = 0;
        record->speedIndex = 0;
        record->speedIncreaseCount = 0;
    } else {
        record->motionFrameCount = 0;
    }
    fn_1_5B4C(record, state);
}

/* Select movement speed and flags for the requested player state. */
void fn_1_5B4C(MikeActPlayerRecord *record, s16 state) {
    switch (state) {
    case 0:
        record->movement_speed = 0.0f;
        record->speedIndex = 0;
        record->flags = record->flags & ~0x8;
        break;
    case 1:
    case 2:
        if (record->speedIndex == 0) {
            record->speedIndex = 1;
        }
        record->movement_speed = lbl_1_data_418[record->speedIndex];
        break;
    default:
        break;
    }
}

void fn_1_5BC8(MikeActPlayerRecord *record, s32 enabled)
{
    if (enabled != 0) {
        record->duration = 0x1E0;
        record->flags = (s16)(record->flags | 4);
        return;
    }
    record->timer += 0x3C;
    record->flags = (s16)(record->flags | 8);
}

s32 fn_1_5C0C(MikeActPlayerRecord *work, s16 requestedState)
{
    if (work->actionState == requestedState) {
        return 0;
    }

    if (work->actionState == 0) {
        switch (requestedState) {
        case 1:
            if ((work->player->actor->colGroundAttr & 2) != 0 && work->movementState == 0) {
                work->movement_angle = 90.0f;
                work->movementState = 1;
                work->motionFrameCount = 0;
                if (work->speedIndex == 0) {
                    work->speedIndex = 1;
                }
                work->movement_speed = lbl_1_data_418[work->speedIndex];
                work->flags |= 0x1000;
            }
            work->callback = fn_1_5F48;
            break;
        case 3:
            if (lbl_1_bss_F0->owner != 0) {
                return;
            }
            if (work->player->stunType == 2 || work->player->stunType == 6) {
                return;
            }
            fn_1_1BF4();
            work->callback = fn_1_6000;
            work->movementState = 1;
            work->motionFrameCount = 0;
            if (work->speedIndex == 0) {
                work->speedIndex = 1;
            }
            work->movement_speed = lbl_1_data_418[work->speedIndex];
            work->flags |= 0x1000;
            break;
        case 2:
            if (fn_1_E11C(work) != 0) {
                work->stationaryFrames = 0;
                work->callback = fn_1_60E4;
                work->flags |= 0x1000;
            }
            break;
        default:
            break;
        }
    } else if (requestedState != 0) {
        return;
    } else {
        if ((work->flags & 0x1000) != 0) {
            work->flags &= ~0x1000;
            work->movementState = 0;
            work->timer = 0;
            work->speedIndex = 0;
            work->speedIncreaseCount = 0;
            work->movement_speed = 0.0f;
            work->speedIndex = 0;
            work->flags &= ~8;
        }
        work->callback = fn_1_5E98;
    }

    work->actionState = requestedState;
    work->transition_value = 0;
    work->transition_counter = 0;
    return;
}

void fn_1_5E98(MikeActPlayerRecord *turnState)
{
    if (turnState->player->motNo == 0 || turnState->player->motNo == 5) {
        if ((u32)turnState->motionFrameCount < 30U) {
            turnState->motionFrameCount++;
        }
    } else {
        turnState->motionFrameCount = 0U;
    }
    if (90.0f != turnState->movement_angle && (u32)turnState->motionFrameCount >= 30U) {
        turnState->movement_angle = 90.0f;
        fn_80086CA8(turnState->player->actor, turnState->movement_angle);
    }
}

void fn_1_5F48(MikeActPlayerRecord *turnState)
{
    switch (turnState->transition_counter) {
    case 0:
        turnState->buttons_down = 0x100;
        turnState->transition_counter = 1;
        return;
    case 1:
        turnState->buttons = 0x100;
        if (turnState->transition_value++ != 5) {
            return;
        }
        turnState->transition_counter = 2;
        return;
    case 2:
        turnState->buttons = 0x100;
        if (turnState->player->actor->colGroundAttr & 0x407F) {
            fn_1_5C0C(turnState, 0);
        }
    }
}

void fn_1_6000(MikeActPlayerRecord *context)
{
    HuVecF currentPosition;
    HuVecF offset;
    HuVecF actorPosition;
    f32 angleDegrees;
    f32 outputAngle;

    if (fn_1_E11C(context)) {
        fn_1_5C0C(context, 0);
        fn_1_5C0C(context, 2);
    } else {
        fn_80086BC8(lbl_1_bss_F0->actor, &currentPosition);
        fn_80086BC8(context->player->actor, &actorPosition);
        PSVECSubtract(&currentPosition, &actorPosition, &offset);
        angleDegrees = (f32) (180.0 *
            (atan2((f64) offset.x, (f64) offset.z) / M_PI));
        outputAngle = angleDegrees;
        context->movement_angle = outputAngle;
    }
}

void fn_1_60E4(MikeActPlayerRecord *record) {
    fn_8007EEBC(record->player, 8);
    fn_1_5C0C(record, 0);
}

void fn_1_62D4(MGPLAYER *context, s16 motionIndex);

extern MikeActPlayerRecord *lbl_1_bss_C4[5];

void fn_1_6124(MGPLAYER *context, s16 motionIndex)
{
    s16 modelId;
    u32 attributes;
    s16 overlayIndex;
    f32 speed;
    MikeActPlayerRecord *playerRecord;

    if (context->motNo != motionIndex) {
        playerRecord = lbl_1_bss_C4[context->actor->param];
        context->motNo = (u16)motionIndex;
        modelId = (s16)context->actor->mdlId;
        attributes = lbl_1_data_310[motionIndex].attributes;
        overlayIndex = (s16)lbl_1_data_310[motionIndex].overlayIndex;
        speed = lbl_1_data_310[motionIndex].speed;
        fn_80026A44(modelId, 0x40000201U);
        if (overlayIndex != -1) {
            fn_8005C2C8(context->charNo,
                               context->omObj->mtnId[lbl_1_data_310[motionIndex].motionIndex], 0.0f,
                               speed, attributes);
            fn_8002CAA8(modelId, context->omObj->mtnId[overlayIndex]);
            fn_8005C430(context->charNo, 0x40000200U);
            return;
        }
        fn_8002CAF0(modelId);
        fn_8005C2C8(context->charNo,
                           context->omObj->mtnId[lbl_1_data_310[motionIndex].motionIndex], 0.0f,
                           speed, attributes);
    }
}

void fn_1_62D4(MGPLAYER *context, s16 motionIndex)
{
    MikeActPlayerRecord *playerRecord;

    playerRecord = lbl_1_bss_C4[context->actor->param];
    if (motionIndex <= 4 && fn_1_45D8(playerRecord, 1)) {
        fn_1_6124(context, motionIndex + 5);
    } else {
        fn_1_6124(context, motionIndex);
    }
}

void fn_1_665C(MikeActPlayerRecord *record, s16 motionIndex)
{
    fn_1_62D4(record->player, motionIndex);
}

void fn_1_69EC(MikeActPlayerRecord *record, s16 selectionNo)
{
    fn_8005BE80(
        record->player->charNo,
        record->player->omObj->mtnId[
            lbl_1_data_310[selectionNo].motionIndex]);
}

s32 fn_1_6A50(MikeActPlayerRecord *record)
{
    if ((fn_8005C240(record->character_no) != 0) &&
        (fn_8005C284(record->character_no) < 0)) {
        return 1;
    }
    return 0;
}

void fn_1_6AA8(MGPLAYER *player)
{
}

static inline f32 mikeactAbs(f32 value)
{
    return fabs(value);
}

void fn_1_6AAC(MGPLAYER *player)
{
    f32 pushSpeedSquared;

    if ((player->actor->colGroundAttr & 0x407F) == 0) {
        return;
    }

    if ((fn_8008717C(player, 1U) == 0) &&
        ((lbl_802C0008[player->padNo] & 0x100) != 0)) {
        if ((player->actionFlag & 2) != 0) {
            fn_8007EEBC(player, 1);
        }
    } else if ((player->actionFlag & 1) != 0) {
        pushSpeedSquared = (player->actor->push.x * player->actor->push.x) +
                           (player->actor->push.z * player->actor->push.z);
        if (pushSpeedSquared > 25.0f) {
            fn_1_62D4(player, 2);
        } else if (pushSpeedSquared > 0.001) {
            fn_1_62D4(player, 1);
        } else {
            fn_1_62D4(player, 0);
        }
    } else {
        if (mikeactAbs(player->actor->velY) > 2000.0f) {
            fn_8007EEBC(player, 2);
        }
    }
}

/* Synchronize the character and model positions with the player actor. */
void fn_1_747C(MGPLAYER *player) {
    Point3d position;

    fn_80086BC8(player->actor, &position);
    fn_80026760((s16)player->actor->mdlId, position.x, position.y, position.z);
    fn_8005FC84(player->charNo, &position);
}

static inline void MikeActMotionStart(MGPLAYER *context,
                                    s16 motionIndex)
{
    s16 overlayIndex;
    s16 modelId;
    u32 attributes;
    f32 speed;
    MikeActPlayerRecord *playerRecord;

    if (context->motNo != motionIndex) {
        playerRecord = lbl_1_bss_C4[context->actor->param];
        context->motNo = (u16)motionIndex;
        modelId = (s16)context->actor->mdlId;
        attributes = lbl_1_data_310[motionIndex].attributes;
        overlayIndex = (s16)lbl_1_data_310[motionIndex].overlayIndex;
        speed = lbl_1_data_310[motionIndex].speed;
        fn_80026A44(modelId, ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 9)));
        if (overlayIndex != -1) {
            fn_8005C2C8(context->charNo,
                               context->omObj->mtnId[lbl_1_data_310[motionIndex].motionIndex],
                               0.0f, speed, attributes);
            fn_8002CAA8(modelId,
                                 context->omObj->mtnId[overlayIndex]);
            fn_8005C430(context->charNo, ((1 << 30) | (1 << 9)));
            return;
        }
        fn_8002CAF0(modelId);
        fn_8005C2C8(context->charNo,
                           context->omObj->mtnId[lbl_1_data_310[motionIndex].motionIndex],
                           0.0f, speed, attributes);
    }
}

static inline void MikeActMotionSelect(MGPLAYER *context, s16 motionIndex)
{
    MikeActPlayerRecord *playerRecord;

    playerRecord = lbl_1_bss_C4[context->actor->param];
    if (motionIndex <= 4 && fn_1_45D8(playerRecord, 1)) {
        MikeActMotionStart(context, motionIndex + 5);
    } else {
        MikeActMotionStart(context, motionIndex);
    }
}

/* Update the player jump, input direction, and landing transition. */
void fn_1_74D8(MGPLAYER *player) {
    Point3d stickInput;
    Point3d groundInput;
    MGACTOR *actor;
    MikeActPlayerRecord *record;
    f32 stickMagnitudeSquared;
    f32 groundMagnitudeSquared;
    f32 jumpFactor;
    f32 jumpVelocity;

    actor = player->actor;
    record = lbl_1_bss_C4[player->playerNo];
    switch (player->subMode) {                        /* irregular */
    case 0:
        CharFXPlay(record->character_no, 0x251);
        MikeActMotionSelect(player, 3);
        if (player->actor->gravity) {
            jumpFactor = 1.0f + (8.0f * (25000.0f / player->actor->gravity));
            jumpVelocity = player->actor->gravity +
                      (player->actor->gravity * ((-1.0f + sqrtf(jumpFactor)) / 2.0f));
        } else {
            jumpVelocity = 0.0f;
        }
        actor->velY = jumpVelocity;
        fn_80087124(player, 0x10U);
        player->timer = 1.0f;
        player->subMode = 1;
        return;
    case 1:
        actor->velY = (player->actor->gravity ? player->actor->gravity + (player->actor->gravity * ((-1.0f + sqrtf((1.0f + (8.0f * (25000.0f / player->actor->gravity))))) / 2.0f)) : 0.0f) - (player->timer * actor->gravity);
        if (player->timer >= 10.0f) {
            player->timer = 0.0f;
            player->subMode = 2;
        } else if (((((u32) (actor->colGroundAttr & 0x7F) != 0) && (actor->colNorm.y > 0.0f)) || ((u32) (actor->colGroundAttr & 0x4000) != 0)) && (actor->velY < 0.001)) {
            stickInput.x = lbl_802BFFFC[player->padNo] /
                           5.6f;
            stickInput.y = 0.0f;
            stickInput.z = lbl_802BFFF8[player->padNo] /
                           5.6f;
            stickMagnitudeSquared = (stickInput.z * stickInput.z) +
                   ((stickInput.x * stickInput.x) + (stickInput.y * stickInput.y));
            if (stickMagnitudeSquared < 25.0f) {
                MikeActMotionSelect(player, 4);
                fn_80087158(player, 0x300U);
                player->timer = 10.0f;
                player->subMode = 3;
            } else {
                fn_8007EEBC(player, 0);
                MikeActMotionSelect(player, 0);
            }
        }
        player->timer += 1.0f;
        return;
    case 2:
        MikeActMotionSelect(player, 3);
        if (((((u32) (actor->colGroundAttr & 0x7F) != 0) && (actor->colNorm.y > 0.0f)) || ((u32) (actor->colGroundAttr & 0x4000) != 0)) && (actor->velY < 0.001)) {
            groundInput.x = lbl_802BFFFC[player->padNo] /
                            5.6f;
            groundInput.y = 0.0f;
            groundInput.z = lbl_802BFFF8[player->padNo] /
                            5.6f;
            groundMagnitudeSquared = (groundInput.z * groundInput.z) +
                   ((groundInput.x * groundInput.x) + (groundInput.y * groundInput.y));
            if (groundMagnitudeSquared < 25.0f) {
                MikeActMotionSelect(player, 4);
                fn_80087158(player, 0x300U);
                player->timer = 10.0f;
                player->subMode = 3;
                return;
            }
            fn_8007EEBC(player, 0);
            MikeActMotionSelect(player, 0);
            if ((u32) (fn_80062094(player->charNo) & 0x10) == 0) {
                fn_1_747C(player);
                return;
            }
        }
        break;
    case 3:
        if ((player->timer <= 9.0f) && (fn_80087148(player, 2U) == 0)) {
            fn_80087168(player, 0x300U);
        }
        if (player->timer > 0.0f) {
            player->timer -= 1.0f;
            return;
        }
        fn_8007EEBC(player, 0);
        MikeActMotionSelect(player, 0);
        break;
    }
}

void fn_1_9014(MGPLAYER *player)
{
    MGACTOR *actor;
    Point3d stick;
    f32 stickMagnitudeSquared;

    actor = player->actor;
    switch (player->subMode) {
    case 0:
        fn_1_62D4(player, 3);
        fn_80087124(player, 0x10U);
        if ((actor->colGroundAttr & 0x407F) == 0) {
            return;
        }
        stick.x = lbl_802BFFFC[player->padNo] / 5.6f;
        stick.y = 0.0f;
        stick.z = lbl_802BFFF8[player->padNo] / 5.6f;
        stickMagnitudeSquared =
            stick.z * stick.z + (stick.x * stick.x + stick.y * stick.y);
        if (stickMagnitudeSquared < 25.0f) {
            fn_1_62D4(player, 4);
            fn_80087158(player, 0x300U);
            player->timer = 10.0f;
            player->subMode = 1;
            return;
        }
        fn_8007EEBC(player, 0);
        fn_1_62D4(player, 0);
        if ((fn_80062094(player->charNo) & 0x10) == 0) {
            fn_1_747C(player);
        }
        break;
    case 1:
        if (player->timer <= 9.0f && fn_80087148(player, 2U) == 0) {
            fn_80087168(player, 0x300U);
        }
        if (player->timer > 0.0f) {
            if (fn_8008717C(player, 1U) == 0 &&
                (lbl_802C0008[player->padNo] & 0x100) != 0) {
                if (fn_80087148(player, 2U) == 0) {
                    fn_80087168(player, 0x300U);
                }
                if (player->timer >= 6.0f &&
                    (fn_80062094(player->charNo) & 0x10) == 0) {
                    fn_1_747C(player);
                }
                fn_8007EEBC(player, 1);
            }
            player->timer -= 1.0f;
            return;
        }
        fn_8007EEBC(player, 0);
        fn_1_62D4(player, 0);
        break;
    }
}

f32 fn_1_9E68(f32 angle)
{
    angle = (f32)fmod(angle, 360.0);
    if (angle < -180.0f) {
        angle += 360.0f;
    } else if (angle > 180.0f) {
        angle -= 360.0f;
    }
    return angle;
}

void fn_1_9F00(MGPLAYER *player)
{
    MGACTOR *actor = player->actor;

    switch (player->subMode) {
    case 0:
        player->stunAngle = fn_1_9E68(player->stunAngle);
        actor->rotY = fn_1_9E68(actor->rotY);
        {
            Mtx matrix;
            Point3d direction;
            Point3d actorDirection;
            Point3d stunDirection;

            direction.x = 0.0f;
            direction.y = 0.0f;
            direction.z = 1.0f;

            PSMTXRotRad(matrix, 0x59, MTXDegToRad(player->actor->rotY));
            PSMTXMultVec(matrix, &direction, &actorDirection);
            PSMTXRotRad(matrix, 0x59, MTXDegToRad(player->stunAngle));
            PSMTXMultVec(matrix, &direction, &stunDirection);
        }
        fn_1_62D4(player, 0xA);
        actor->rotY = fn_1_9E68(180.0f + player->stunAngle);
        player->timer = 45.0f;
        fn_80087158(player, 0x300U);
        player->stunType = 6;
        player->subMode = 1;
        break;
    case 1:
        if (player->timer > 0.0f) {
            actor->push.x = 0.0f;
            actor->push.y = 0.0f;
            if (player->timer > 25.0f) {
                actor->push.z = -(player->timer / 1.4f) *
                                ((player->timer - 25.0f) / 20.0f);
            }
            player->timer -= 1.0f;
            break;
        }
        fn_1_62D4(player, 0xB);
        player->timer = 30.0f;
        player->subMode = 2;
        break;
    case 2:
        if (player->timer > 0.0f) {
            player->timer -= 1.0f;
            break;
        }
        {
            MikeActPlayerRecord *playerRecord;

        fn_1_1B40(2, 0);
        fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 0), 1);
        player->stunType = 5;
        fn_80087168(player, 0x300U);
            playerRecord = lbl_1_bss_C4[player->playerNo];
            playerRecord->flags &= ~0x2000;
        fn_80087168(player, 1U);
        fn_8007EEBC(player, 0);
        fn_1_62D4(player, 0);
        }
        break;
    }
}

void fn_1_A8F4(MGPLAYER *player)
{
    s16 soundIds[11] = { 1387, 1386, 1388, 795, 1389, 1391, 1390, 1384, 723, 651, 1385, };
    Point3d position;
    MikeActPlayerRecord * targetRecord;
    f32 progress;
    position = player->actor->pos;
    position.y += 40.0f;
    switch (player->subMode)
    {
    case 0 :
        player->timer = 0.0f;
        player->subMode = 1;
        break;
    case 1 :
        player->timer += 1.0f;
        if (player->timer == 60.0f)
        {
            fn_8005DEB0(player->charNo, 0x26, 0);
            HuAudFXPlay(soundIds[player->charNo]);
            fn_1_62D4(player, 0xD);
            player->subMode = 2;
        }
        break;
    case 2 :
        player->timer += 1.0f;
        targetRecord = lbl_1_bss_C4[player->playerNo];
        if (fn_1_6A50(targetRecord))
        {
            fn_8007EEBC(player, 0);
            fn_1_C318(player->playerNo - 1, 0, 0);
            fn_8005DEB0(player->charNo, 0x26, 1);
            return;
        }
        break;
    }
    progress = player->timer / 60.0f;
    position.y += 400.0f * (progress * progress);
    fn_1_C2E4(player->playerNo - 1, & position, 1);
    progress = progress < 0.25f ? 0.0f
             : progress > 0.875f ? 1.0f
             : (progress - 0.25f) / 0.625f;
    fn_800268B0((s16)player->actor->mdlId, progress * progress,
                      1.0f, progress * progress);
}

void fn_1_AD6C(MGPLAYER *player)
{
    MikeActPlayerRecord *playerRecord;

    switch (player->subMode) {
    case 0:
        fn_1_62D4(player, 0xC);
        player->subMode = 1;
        break;
    case 1:
        playerRecord = lbl_1_bss_C4[player->playerNo];
        if (fn_1_6A50(playerRecord)) {
            fn_1_62D4(player, 0);
            fn_8007EEBC(player, 0);
        }
        break;
    }
}

void fn_1_B244(MGPLAYER *player)
{
    MikeActPlayerRecord *playerRecord;
    s16 elapsed;

    switch (player->subMode) {
    case 0:
        fn_1_62D4(player, 0xD);
        player->subMode = 1;
        break;
    case 1:
        playerRecord = lbl_1_bss_C4[player->playerNo];
        if (fn_1_6A50(playerRecord)) {
            player->work[0] = 0;
            player->subMode = 2;
        }
        break;
    case 2:
        if (player->work[0]++ >= 0) {
            fn_1_62D4(player, 0);
            fn_8007EEBC(player, 0);
        }
        break;
    }
}

void fn_1_B754(MGPLAYER *player)
{
    Point3d position;

    switch (player->subMode) {
    case 0:
        fn_1_62D4(player, 0xE);
        fn_1_DDFC();
        fn_1_E0B8(0);
        fn_80086CC4(lbl_1_bss_F0->actor, 0.0f);
        fn_80086CBC(lbl_1_bss_F0->actor, 0.0f);
        fn_800805CC(lbl_1_bss_F0->actor, 6U);
        player->work[0] = 0;
        player->subMode = 1;
        break;
    case 1:
        if (player->work[0]++ >= 0x28) {
            player->subMode = 2;
        } else {
            fn_80086BC8(lbl_1_bss_F0->actor, &position);
            position.y += 5.0f;
            fn_80086AA8(lbl_1_bss_F0->actor, &position);
        }
        break;
    case 2:
        if (player->work[0]++ >= 0xB4) {
            fn_1_62D4(player, 0);
            fn_8007EEBC(player, 0);
        }
        break;
    }
    fn_800267CC((s16)lbl_1_bss_F0->actor->mdlId, &position);
}

void fn_1_BCF4(MGPLAYER *player)
{
    MikeActPlayerRecord *record = lbl_1_bss_C4[player->playerNo];
    MikeActPlayerRecord *actorRecord;

    switch (player->subMode) {
    case 0:
        record->flags |= 0x2000;
        fn_80087158(player, 1U);
        fn_1_62D4(player, 0xF);
        player->work[0] = 0;
        player->subMode = 1;
        break;
    case 1:
        if (player->work[0]++ == 0xF) {
            fn_1_30A0(&lbl_1_bss_F0->position, 0x537);
            fn_1_DCEC(record);
            player->subMode = 2;
        }
        if (fn_1_6A50(record)) {
            fn_1_62D4(player, 0);
            fn_8007EEBC(player, 0);
        }
        break;
    case 2:
        if (fn_1_6A50(record)) {
            fn_1_1B40(0, 0);
            fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 1), 6);
            fn_8007EEBC(player, 0);
            fn_80087168(player, 1U);
            record->flags &= ~0x2000;
        }
        break;
    }
}

MikeActPlayerRecord *lbl_1_bss_C4[5];
MikeActEventCallback lbl_1_bss_C0;
void (*lbl_1_bss_BC)(void *, s32);
MikeActPlayerRecord *lbl_1_bss_B8;
