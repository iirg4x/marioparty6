// Coin, dollar and togezo models, effects and collision queries.

extern "C" {
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
#include "humath.h"
#include <math.h>
#include "game/hu3d.h"
}

#include "REL/framework/frame_durations.h"
#include "REL/framework/tri_colli.h"
#include <new>
#include "REL/m623DLL/item_effects_state.h"
#include "datadir_enum.h"
#include "REL/framework/singleton.h"
#include "REL/framework/model.h"
#include "REL/framework/list_object.h"
#include <memory>
#include <vector>
#include <cstdio>
#include "REL/m623DLL/scene_collision.h"
#include "REL/m623DLL/item_effects_owner.h"
#include "REL/m623DLL/particle_object.h"

extern Point3d lbl_1_bss_4DC[33];

void fn_1_7FA0(HU3D_MODEL *model, Mtx *matrix);

void fn_1_7FEC(HU3D_MODEL *model, Mtx *matrix);

// The ten coin hooks in each arrangement.
char *lbl_1_data_2A8[5][10] = {
    { "null3", "null4", "null11", "null9", "null8",
      "null7", "null6", "null5", "null2", "null10" },
    { "null31", "null32", "null33", "null34", "null35",
      "null36", "null37", "null38", "null39", "null40" },
    { "null48", "null49", "null50", "null51", "null52",
      "null53", "null54", "null55", "null56", "null57" },
    { "null65", "null66", "null67", "null68", "null69",
      "null70", "null71", "null72", "null73", "null74" },
    { "null82", "null83", "null84", "null85", "null86",
      "null87", "null88", "null89", "null90", "null91" }
};

// The two dollar hooks in each arrangement.
char *lbl_1_data_3B8[5][2] = {
    { "null12", "null13" },
    { "null46", "null47" },
    { "null63", "null64" },
    { "null80", "null81" },
    { "null97", "null98" }
};

// The five spiny hooks in each arrangement.
char *lbl_1_data_490[5][5] = {
    { "null22", "null23", "null24", "null25", "null26" },
    { "null41", "null42", "null43", "null44", "null45" },
    { "null58", "null59", "null60", "null61", "null62" },
    { "null75", "null76", "null77", "null78", "null79" },
    { "null92", "null93", "null94", "null95", "null96" }
};

// Models whose hooks give the coin, dollar and spiny positions for each arrangement.
const u32 lbl_1_rodata_2C8[5][3] = {
    { DATANUM(0x520000, 35), DATANUM(0x520000, 36), DATANUM(0x520000, 37) },
    { DATANUM(0x520000, 38), DATANUM(0x520000, 39), DATANUM(0x520000, 40) },
    { DATANUM(0x520000, 41), DATANUM(0x520000, 42), DATANUM(0x520000, 43) },
    { DATANUM(0x520000, 44), DATANUM(0x520000, 45), DATANUM(0x520000, 46) },
    { DATANUM(0x520000, 47), DATANUM(0x520000, 48), DATANUM(0x520000, 49) }
};

// Creates the item models and reads their hook positions for both views.
ItemEffectsOwner::ItemEffectsOwner(int type)
    : collision(fn_1_1A2C()), coinModel(DATANUM(0x520000, 29)), dollarModel(DATANUM(0x520000, 31)),
      spinyModel(DATANUM(0x520000, 30)), coinLayout(lbl_1_rodata_2C8[type][0]),
      dollarLayout(lbl_1_rodata_2C8[type][1]), spinyLayout(lbl_1_rodata_2C8[type][2])
{
    HU3D_MODEL *firstHookModel;
    HU3D_MODEL *secondHookModel;
    Point3d position;
    s16 initialModelId;
    s16 nameModel;
    s16 effectModel;
    s32 stateIndex;
    s32 coinIndex;
    s32 dollarIndex;
    s32 spinyIndex;
    s32 circleIndex;
    f32 angle;
    f32 radians;

    initialModelId = coinModel;
    std::printf("TYPE %d\n", type);
    nameModel = coinLayout;
    effectModel = coinModel;
    stateIndex = 0;
    coinIndex = 0;
    for (; coinIndex < 10; ++stateIndex, ++coinIndex) {
        Hu3DModelObjPosGet(nameModel, lbl_1_data_2A8[type][coinIndex], &position);
        std::printf("coin %f %f %f\n", position.x, position.y, position.z);
        setupEffect(&records[0][stateIndex], 1, effectModel, &position, 0, 100.0f);
        setupEffect(&records[1][stateIndex], 1, effectModel, &position, 1, 100.0f);
    }
    nameModel = dollarLayout;
    effectModel = dollarModel;
    dollarIndex = 0;
    for (; dollarIndex < 2; ++stateIndex, ++dollarIndex) {
        Hu3DModelObjPosGet(nameModel, lbl_1_data_3B8[type][dollarIndex], &position);
        std::printf("dollar %f %f %f\n", position.x, position.y, position.z);
        setupEffect(&records[0][stateIndex], 2, effectModel, &position, 0, 20.0f);
        setupEffect(&records[1][stateIndex], 2, effectModel, &position, 1, 20.0f);
    }
    nameModel = spinyLayout;
    effectModel = spinyModel;
    spinyIndex = 0;
    for (; spinyIndex < 5; ++stateIndex, ++spinyIndex) {
        Hu3DModelObjPosGet(nameModel, lbl_1_data_490[type][spinyIndex], &position);
        std::printf("togezo %f %f %f\n", position.x, position.y, position.z);
        setupEffect(&records[0][stateIndex], 3, effectModel, &position, 0, 20.0f);
        setupEffect(&records[1][stateIndex], 3, effectModel, &position, 1, 20.0f);
    }
    coinModel.setAttribute(1);
    dollarModel.setAttribute(1);
    spinyModel.setAttribute(1);
    coinLayout.setAttribute(1);
    dollarLayout.setAttribute(1);
    spinyLayout.setAttribute(1);
    for (circleIndex = 0; circleIndex < 33; ++circleIndex) {
        angle = (360.0f * ((f32) (circleIndex & 0x1F) / 32.0f)) - 180.0f;
        angle += 90.0f;
        if (angle > 180.0f) {
            angle -= 360.0f;
        }
        radians = (f32) ((M_PI * (f64) angle) / 180.0);
        lbl_1_bss_4DC[circleIndex].x = (f32) (120.0 * cos((f64) radians));
        lbl_1_bss_4DC[circleIndex].y = 0.0f;
        lbl_1_bss_4DC[circleIndex].z = (f32) (120.0 * sin((f64) radians));
    }
    hookIds[0] = fn_800260F4(fn_1_7FA0);
    fn_80026B8C(hookIds[0], 4);
    fn_80026B64(hookIds[0], 0x1U);
    fn_80026760(hookIds[0], 0.0f, 0.0f, 0.0f);
    firstHookModel = &lbl_802C01C8[hookIds[0]];
    firstHookModel->hookData = this;
    hookIds[1] = fn_800260F4(fn_1_7FEC);
    fn_80026B8C(hookIds[1], 4);
    fn_80026B64(hookIds[1], 0x2U);
    fn_80026760(hookIds[1], 0.0f, 0.0f, 0.0f);
    secondHookModel = &lbl_802C01C8[hookIds[1]];
    secondHookModel->hookData = this;
}

// Releases this owner's effect records and resources.
ItemEffectsOwner::~ItemEffectsOwner()
{
    for (int index = 0; index < 17; index++) {
        fn_800262D0(records[0][index].model);
        fn_800262D0(records[1][index].model);
    }
    fn_800262D0(hookIds[0]);
    fn_800262D0(hookIds[1]);
}

// Initializes an item and its model after sampling the terrain below its hook.
void ItemEffectsOwner::setupEffect(ItemEffectsState *state, int mode, short model,
                                   Point3d *position, int viewIndex, f32 heightOffset)
{
    short linkedModel;
    linkedModel = state->model = fn_80025D4C(model);
    state->active = 1;
    state->motionState = 0;
    state->fade = 1.0f;
    state->mode = mode;
    state->unk08 = viewIndex;
    std::vector<PlaneRecord> results;
    Vec groundPosition;
    groundPosition.y = fn_1_1ACC(groundPosition.x = position->x, groundPosition.z = position->z);

    state->basePosition.x = groundPosition.x;
    state->basePosition.y = groundPosition.y;
    state->basePosition.z = groundPosition.z;
    state->basePosition.y += 20.0f;
    state->position.x = groundPosition.x;
    state->position.y = groundPosition.y;
    state->position.z = groundPosition.z;
    state->position.y += heightOffset;
    state->rotationY = 90.0f;
    state->rotationZ = 0.0f;
    fn_800269E8(linkedModel, 0x4000U);
    fn_80026A44(linkedModel, 0x1U);
    fn_80026760(linkedModel, state->position.x, state->position.y, state->position.z);
    fn_800268B0(linkedModel, 1.5f, 1.5f, 1.5f);
    fn_80026808(linkedModel, 0.0f, state->rotationY, 0.0f);
    fn_80026B8C(linkedModel, 5);
    if (viewIndex == 0) {
        fn_80026B64(linkedModel, 0x1U);
    } else {
        fn_80026B64(linkedModel, 0x2U);
    }
}

// Tests horizontal separation against the supplied radius.
// Nothing in this minigame calls this. The first parameter is unused.
int fn_1_704C(void *, const Vec *first, const Vec *second, f32 distance)
{
    f32 xForAxisCheck;
    f32 zForAxisCheck;
    f32 xDifference;
    f32 absoluteX;
    f32 zDifference;
    f32 absoluteZ;

    xDifference = first->x - second->x;
    xForAxisCheck = xDifference;
    if (xForAxisCheck < 0.0f) {
        absoluteX = -xForAxisCheck;
    } else {
        absoluteX = xForAxisCheck;
    }
    if (absoluteX > distance) {
        return 0;
    }

    zDifference = first->z - second->z;
    zForAxisCheck = zDifference;
    if (zForAxisCheck < 0.0f) {
        absoluteZ = -zForAxisCheck;
    } else {
        absoluteZ = zForAxisCheck;
    }
    if (absoluteZ > distance) {
        return 0;
    }

    if (xDifference * xDifference + zDifference * zDifference > distance * distance) {
        return 0;
    }
    return 1;
}

// Resets an effect record after a hit and creates its particle object.
void ItemEffectsOwner::fn_1_715C(ItemEffectsState *state, int viewIndex)
{
    if (viewIndex != 1 || state->mode != 3) {
        state->active = 0;
        state->frame = 0;
        if (state->mode == 3) {
            state->motionState = 1;
        } else {
            fn_800269E8(state->model, 0x1U);
            state->motionState = 2;
        }

        new ItemHitParticles(&state->position, viewIndex);
    }
}

// Handles the first active item in range and returns its kind.
int fn_1_7210(int viewIndex, const Vec *position, f32 radius, Vec *result)
{
    ItemEffectsOwner *owner;
    ItemEffectsState *record;
    int index;
    f32 xForAxisCheck;
    f32 zForAxisCheck;
    f32 zDifference;
    f32 xDifference;
    f32 xDistance;
    f32 zDistance;
    u8 withinRange;

    owner = &ItemEffectsOwner::getChecked();
    result->x = result->y = result->z = 0.0f;
    for (index = 0; index < 17; ++index) {
        record = &owner->records[viewIndex][index];
        if (record->active != 0) {
            xForAxisCheck = xDifference = position->x - record->position.x;
            if (xForAxisCheck < 0.0f) {
                xDistance = -xForAxisCheck;
            } else {
                xDistance = xForAxisCheck;
            }
            if (xDistance > radius) {
                withinRange = 0;
            } else {
                zForAxisCheck = zDifference = position->z - record->position.z;
                if (zForAxisCheck < 0.0f) {
                    zDistance = -zForAxisCheck;
                } else {
                    zDistance = zForAxisCheck;
                }
                if (zDistance > radius) {
                    withinRange = 0;
                } else if (xDifference * xDifference + zDifference * zDifference >
                           radius * radius) {
                    withinRange = 0;
                } else {
                    withinRange = 1;
                }
            }
            if (withinRange != 0) {
                owner->fn_1_715C(&owner->records[viewIndex][index], viewIndex);

                result->x = record->position.x;
                result->y = record->position.y;
                result->z = record->position.z;
                return record->mode;
            }
        }
    }
    return 0;
}

// Finds the nearest active spiny within 5,000 units along X.
u8 fn_1_7534(int viewIndex, const Vec *position, Vec *result)
{
    ItemEffectsOwner *owner;
    ItemEffectsState *closest;
    u8 found;
    int index;
    f32 difference;
    f32 closestDistance;
    f32 candidateDistance;
    f32 distance;

    owner = &ItemEffectsOwner::getChecked();
    found = 0;
    closest = 0;
    closestDistance = 5000.0f;
    result->x = result->y = result->z = 0.0f;
    for (index = 0; index < 17; ++index) {
        ItemEffectsState *record = &owner->records[viewIndex][index];
        if (record->active != 0 && record->mode == 3) {
            difference = position->x - record->position.x;
            if (difference < 0.0f) {
                distance = -difference;
            } else {
                distance = difference;
            }
            if ((candidateDistance = distance) < 5000.0f && closestDistance > candidateDistance) {
                found = 1;
                closestDistance = candidateDistance;
                closest = record;
            }
        }
    }
    if (found != 0) {
        result->x = closest->position.x;
        result->y = closest->position.y;
        result->z = closest->position.z;
    }
    return found;
}

// Finds the nearest active coin or dollar within 5,000 units along X.
u8 fn_1_76E0(int viewIndex, const Vec *position, Vec *result)
{
    ItemEffectsOwner *owner;
    ItemEffectsState *closest;
    u8 found;
    int index;
    f32 difference;
    f32 closestDistance;
    f32 candidateDistance;
    f32 distance;

    owner = &ItemEffectsOwner::getChecked();
    found = 0;
    closest = 0;
    closestDistance = 5000.0f;
    result->x = result->y = result->z = 0.0f;
    for (index = 0; index < 17; ++index) {
        ItemEffectsState *record = &owner->records[viewIndex][index];
        if (record->active != 0 && record->mode != 3) {
            difference = position->x - record->position.x;
            if (difference < 0.0f) {
                distance = -difference;
            } else {
                distance = difference;
            }
            if ((candidateDistance = distance) < 5000.0f && closestDistance > candidateDistance) {
                found = 1;
                closestDistance = candidateDistance;
                closest = record;
            }
        }
    }
    if (found != 0) {
        result->x = closest->position.x;
        result->y = closest->position.y;
        result->z = closest->position.z;
    }
    return found;
}

// Advances one effect record's motion, fade and model state.
void ItemEffectsOwner::fn_1_788C(ItemEffectsState *state)
{
    s16 modelId = state->model;
    f32 fade;

    if (state->frame > 32) {
        fn_800269E8(modelId, 0x1U);
        state->motionState = 2;
        return;
    }

    switch (state->mode) {
    case 1:
        state->rotationY += 40.0f;
        if (state->rotationY > 180.0f) {
            state->rotationY -= 360.0f;
        }
        break;
    case 3:
        state->rotationZ += 40.0f;
        if (state->rotationZ > 180.0f) {
            state->rotationZ -= 360.0f;
        }
        break;
    }

    fade = (f32) state->frame / 32.0f;
    if (fade > 1.0f) {
        fade = 1.0f;
    }
    fade = 1.0f - (fade * fade);
    state->position.y += 50.0f;
    fn_80026760(modelId, state->position.x, state->position.y, state->position.z);
    fn_80026808(modelId, state->rotationZ, state->rotationY, 0.0f);
    fn_80026CBC(modelId, fade);

    fade = (f32) state->frame / 16.0f;
    if (fade > 1.0f) {
        fade = 1.0f;
    }
    state->fade = 1.0f - fade;
}

// Updates every effect record in both views.
void ItemEffectsOwner::update()
{
    int itemIndex;
    int viewIndex;

    for (itemIndex = 0; itemIndex < 17; itemIndex++) {
        for (viewIndex = 0; viewIndex < 2; viewIndex++) {
            ItemEffectsState *state = &records[viewIndex][itemIndex];

            if (state->motionState == 1) {
                fn_1_788C(&records[viewIndex][itemIndex]);
            } else if (state->mode == 1) {
                state->rotationY += 5.0f;
                if (state->rotationY > 180.0f) {
                    state->rotationY -= 360.0f;
                }
                fn_80026808(state->model, 0.0f, state->rotationY, 0.0f);
            }
            state->frame++;
        }
    }
}

// Acknowledges the item effect request.
void ItemEffectsOwner::flagEnabled(int request)
{
    flags &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    }
}

void fn_1_8038(ItemEffectsOwner *, Mtx matrix, s32);

// Draws the item circles in the first view.
void fn_1_7FA0(HU3D_MODEL *model, Mtx *matrix)
{
    fn_8002472C();
    ItemEffectsOwner *owner = static_cast<ItemEffectsOwner *>(model->hookData);
    fn_1_8038(owner, *matrix, 0);
}

// Draws the item circles in the second view.
void fn_1_7FEC(HU3D_MODEL *model, Mtx *matrix)
{
    fn_8002472C();
    ItemEffectsOwner *owner = static_cast<ItemEffectsOwner *>(model->hookData);
    fn_1_8038(owner, *matrix, 1);
}

void fn_1_8070(ItemEffectsOwner *owner, Mtx matrix, s32 viewIndex);

// Passes the selected view to the item circle renderer.
void fn_1_8038(ItemEffectsOwner *owner, Mtx matrix, s32 viewIndex)
{
    fn_1_8070(owner, matrix, viewIndex);
}

// Draws transformed circles beneath the items in the selected view.
void fn_1_8070(ItemEffectsOwner *owner, Mtx matrix, s32 viewIndex)
{
    s32 objectIndex;

    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    for (objectIndex = 0; objectIndex < 17; ++objectIndex) {
        ItemEffectsState *record = &owner->records[viewIndex][objectIndex];
        s32 alpha;

        if (record->motionState != 2) {
            s32 vertexIndex;
            f32 baseX;
            f32 baseY;
            f32 baseZ;

            GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 34);
            baseZ = record->basePosition.z;
            baseY = record->basePosition.y;
            baseX = record->basePosition.x;
            GXPosition3f32(baseX, baseY, baseZ);
            alpha = (s32) (128.0f * record->fade);
            GXColor4u8(0x20, 0x20, 0x40, alpha);

            for (vertexIndex = 0; vertexIndex < 33; ++vertexIndex) {
                GXPosition3f32(record->basePosition.x + lbl_1_bss_4DC[vertexIndex].x,
                               record->basePosition.y,
                               record->basePosition.z + lbl_1_bss_4DC[vertexIndex].z);
                GXColor4u8(0x20, 0x20, 0x80, 0);
            }
        }
    }
}

// The positions used to draw the circle fans beneath items.
Point3d lbl_1_bss_4DC[33];
