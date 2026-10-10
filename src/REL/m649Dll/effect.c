/* Draws Stamp By Me's routed-prop shadows, moving markers, and score effects. */
#include "dolphin/math.h"

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

#include "datadir_enum.h"

#include "game/hu3d.h"

#include "string.h"

#include <stdlib.h>

#include <string.h>

typedef struct M649EffectSlotsView_s {
    void *entries[1];
} M649EffectSlotsView;

typedef struct M649ShadowRecord_s {
    s32 active, camera; /* Draw flag and camera index selecting the shadow model. */
    float radius; /* Shadow fan radius in world units. */
    Vec *position, *offset; /* Live shadow center and the offset added when drawing it. */
} M649ShadowRecord;

typedef struct M649EffectOwnerWorkView_s {
    ANIMDATA *animations[2];
} M649EffectOwnerWorkView;

typedef struct M649Point_s { Vec position; s16 active, hit; } M649Point;

typedef struct M649Motion_s {
    s32 id, timer; /* Prop identity and its startup delay in frames. */
    float position, speed, targetSpeed; /* Travel distance and current/desired speed. */
    s32 mode, group, character, nextPoint; /* Marker mode, color group, character, and next path
                                            * slot. */
    M649Point points[64];
} M649Motion;

typedef struct M649EffectOwner_s { ANIMDATA *animations[2]; } M649EffectOwner;

typedef struct M649EffectRecordView_s {
    s32 visible, generation, phase, spawnDelay, frame; /* Draw flag, trail depth, and update
                                                        * state. */
    float size; /* Radius of the rendered score effect in world units. */
    float *speed; /* Live travel speed of the prop that created this effect. */
    Vec position, trailPosition; /* Moving effect position and rising trail center. */
    GXColor color; /* Trail color and fading alpha. */
} M649EffectRecordView;

typedef struct M649EffectRecordView_s_099FC {
    s32 visible, generation, phase, spawnDelay, frame; /* Draw flag, trail depth, and update
                                                        * state. */
    float size; /* Radius of the rendered score effect in world units. */
    float *speed; /* Live travel speed of the prop that created this effect. */
    Vec position, trailPosition; /* Moving effect position and rising trail center. */
    GXColor color; /* Trail color and fading alpha. */
} M649EffectRecordView_099FC;

typedef struct M649EffectRecord_s {
    s32 visible, generation, phase, spawnDelay, frame; /* Draw flag, trail depth, and update
                                                        * state. */
    float size; /* Radius of the rendered score effect in world units. */
    float *speed; /* Live travel speed of the prop that created this effect. */
    Vec position, trailPosition; /* Moving effect position and rising trail center. */
    GXColor color; /* Trail color and fading alpha. */
} M649EffectRecord;

typedef struct M649EffectSlotsView_s_0A018 {
    M649ShadowRecord *entries[1];
} M649EffectSlotsView_0A018;

typedef struct M649EffectRecordView_s_0A018 {
    s32 visible, generation, phase, spawnDelay, frame; /* Draw flag, trail depth, and update
                                                        * state. */
    float size; /* Radius of the rendered score effect in world units. */
    float *speed; /* Live travel speed of the prop that created this effect. */
    Vec position, trailPosition; /* Moving effect position and rising trail center. */
    GXColor color; /* Trail color and fading alpha. */
} M649EffectRecordView_0A018;

void fn_1_9014(OMOBJ *obj);

void fn_1_904C(OMOBJ *object);

void fn_1_941C(OMOBJ *object);

void fn_1_9934(OMOBJ *object);

/* static */
extern OMOBJ *lbl_1_bss_5A0;

extern HUPROCESS *lbl_1_bss_5A4;

extern void fn_1_9024(OMOBJ *node);

void fn_1_99FC(OMOBJ *object);

void fn_1_910C(HU3D_MODEL *model, Mtx *view);

extern s32 fn_1_1218(s32 camera);

static signed long lbl_1_data_538[2] = {3, 3};

extern s32 fn_1_1210(s32 resource);

void fn_1_9500(HU3D_MODEL *model, Mtx *view);

void fn_1_9C14(HU3D_MODEL *model, Mtx *view);

void fn_1_A168(s32 type, Vec *position, float *speed, s32 mode);

/* Creates the effect owner and its shadow, marker, and score-effect models at setup. */
void fn_1_8F58(HUPROCESS *process) {
    HUPROCESS *unusedProcessPointer;
    OMOBJ *effectObject;
    void *ownerData;
    lbl_1_bss_5A4 = process;
    effectObject = omAddObjEx(lbl_1_bss_5A4, 16, 24U, 0U, - 1, fn_1_9014);
    lbl_1_bss_5A0 = effectObject;
    ownerData = HuMemDirectMallocNum(HEAP_HEAP, 8, HU_MEMNUM_OVL);
    effectObject->data = ownerData;
    memset(ownerData, 0, 8);
    fn_1_904C(effectObject);
    fn_1_941C(effectObject);
    fn_1_9934(effectObject);
}

void fn_1_9010(void)
{
}

void fn_1_9014(OMOBJ *obj)
{
    obj->objFunc = fn_1_9024;
}

/* Runs each frame for the effect owner and calls fn_1_99FC to advance score effects. */
extern void fn_1_9024(OMOBJ *node)
{
    fn_1_99FC(node);
}

/* Creates four camera-specific shadow models during effect-owner setup. */
void fn_1_904C(OMOBJ *object)
{
    s32 i;
    s16 modelId;
    HU3D_MODEL *model;
    M649EffectSlotsView *work;
    for (i = 0; i < 4; i++) {
        modelId = Hu3DHookFuncCreate(fn_1_910C);
        object->mdlId[i] = modelId;
        Hu3DModelCameraSet(modelId, fn_1_1218(i));
        Hu3DModelLayerSet(modelId, 3);
        model = &Hu3DData[modelId];
        work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*work), HU_MEMNUM_OVL);
        model->hookData = work;
        memset(work, 0, sizeof(*work));
    }
}

/* Draws an active shadow as a translucent fan through the hook registered by fn_1_904C. */
void fn_1_910C(HU3D_MODEL *model, Mtx *view)
{
    M649ShadowRecord *record;
    s32 i, j;
    M649ShadowRecord **records = model->hookData;
    float angle, radius, x, z;
    Mtx matrix;
    Vec position;
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    for (i = 0; i < 1; i++) {
        record = records[i];
        if (record != NULL && record->active) {
            radius = record->radius;
            VECAdd(record->position, record->offset, &position);
            MTXTrans(matrix, position.x, position.y, position.z);
            MTXConcat(*view, matrix, matrix);
            GXLoadPosMtxImm(matrix, GX_PNMTX0);
            GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 18);
            GXPosition3f32((0.0f), (0.0f), (0.0f));
            GXColor1u32(128);
            for (j = 0, angle = (0.0f); j < 16; j++, angle += (22.5)) {
                z = radius * cos((3.141592653589793) * angle / (180.0));
                x = radius * sin((3.141592653589793) * angle / (180.0));
                GXPosition3f32(x, (0.0f), z);
                GXColor1u32(48);
            }
            GXPosition3f32((0.0f), (0.0f), radius);
            GXColor1u32(48);
            GXEnd();
        }
    }
}

/* Loads the two marker textures and creates the sixteen marker models at setup. */
void fn_1_941C(OMOBJ *object)
{
    s32 i, j;
    s16 modelId;
    M649EffectOwnerWorkView *work = lbl_1_bss_5A0->data;
    HU3D_MODEL *model;
    for (i = 0; i < 2; i++) {
        work->animations[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            fn_1_1210(lbl_1_data_538[i]), HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (j = 0; j < 16; j++) {
        modelId = Hu3DHookFuncCreate(fn_1_9500);
        object->mdlId[j + 4] = modelId;
        Hu3DModelLayerSet(modelId, 5);
        model = &Hu3DData[modelId];
        model->hookData = NULL;
    }
}

static const GXColor lbl_1_rodata_320[4] = {
    { 255, 0, 0, 128 }, { 0, 0, 255, 128 }, { 16, 208, 16, 192 }, { 224, 144, 0, 192 }
};

/* Draws active moving markers through the hook registered by fn_1_941C. */
void fn_1_9500(HU3D_MODEL *model, Mtx *view)
{
    M649Motion *motion = model->hookData;
    M649Point *point;
    s32 i;
    M649EffectOwner *owner;
    float size, z;
    Mtx matrix;
    if (motion == NULL) { return; }
    if (motion->mode < 0) { return; }
    owner = lbl_1_bss_5A0->data;
    size = (35.0f);
    HuSprTexLoad(owner->animations[motion->mode], 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 255);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetNumChans(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetNumTevStages(1);
    GXSetTevColor(GX_TEVREG0, lbl_1_rodata_320[motion->group]);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    for (i = 0; i < 64; i++) {
        point = &motion->points[i];
        if (point->active) {
            z = point->position.z - motion->position;
            if (z < (-480.0) || z > (480.0)) { continue; }
            MTXTrans(matrix, point->position.x, point->position.y, z);
            MTXConcat(*view, matrix, matrix);
            GXLoadPosMtxImm(matrix, GX_PNMTX0);
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(-size, (0.0f), -size);
            GXTexCoord2f32((0.0f), (0.0f));
            GXPosition3f32(size, (0.0f), -size);
            GXTexCoord2f32((1.0f), (0.0f));
            GXPosition3f32(size, (0.0f), size);
            GXTexCoord2f32((1.0f), (1.0f));
            GXPosition3f32(-size, (0.0f), size);
            GXTexCoord2f32((0.0f), (1.0f));
            GXEnd();
        }
    }
}

/* Creates four camera-specific model hooks with 24 score-effect records each at setup. */
void fn_1_9934(OMOBJ *object)
{
    s32 i;
    s16 modelId;
    HU3D_MODEL *model;
    M649EffectRecordView *records;
    s32 size;
    for (i = 0; i < 4; i++) {
        modelId = Hu3DHookFuncCreate(fn_1_9C14);
        object->mdlId[i + 20] = modelId;
        Hu3DModelCameraSet(modelId, fn_1_1218(i));
        Hu3DModelLayerSet(modelId, 6);
        size = sizeof(*records) * 24;
        records = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
        memset(records, 0, size);
        model = &Hu3DData[modelId];
        model->hookData = records;
    }
}

static GXColor lbl_1_data_540[7] = {
    {255, 0, 0, 200}, {255, 128, 0, 200}, {255, 255, 0, 200}, {0, 255, 0, 200},
    {0, 128, 255, 200}, {0, 0, 255, 200}, {128, 0, 255, 200}
};

/* Advances each score effect once per frame, spawning its trail and fading it out. */
void fn_1_99FC(OMOBJ *object)
{
    M649EffectRecordView_099FC *record;
    HU3D_MODEL *model;
    s16 modelId;
    s32 i, j;
    for (i = 0; i < 4; i++) {
        modelId = object->mdlId[i + 20];
        model = &Hu3DData[modelId];
        record = model->hookData;
        for (j = 0; j < 24; j++, record++) {
            switch (record->phase) {
            case 0:
                break;
            case 1:
                record->phase++;
                record->spawnDelay = 4;
                record->frame = 8;
                record->visible = 1;
                record->size = (30.0f);
                record->trailPosition = record->position;
                record->color = lbl_1_data_540[record->generation % 7];
                /* Initialize the trail, then advance its first frame immediately. */
            case 2:
                if (record->generation != 0 && --record->spawnDelay <= 0) {
                    fn_1_A168(i, &record->position, record->speed, record->generation - 1);
                    record->generation = 0;
                }
                record->position.z -= *record->speed;
                record->trailPosition.z -= *record->speed;
                record->trailPosition.y += (4.0);
                record->size += (1.0);
                /* The unsigned alpha wraps after fading out, which retires the effect. */
                if ((record->color.a -= 6) >= 240) {
                    record->phase = 0;
                    record->visible = 0;
                }
                break;
            }
        }
    }
}

static const Vec lbl_1_rodata_360[10] = {
    {0.0f, 0.0f, -1.0f}, {0.195928f, 0.0f, -0.396994f},
    {0.951057f, 0.0f, -0.309017f}, {0.391856f, 0.0f, 0.206011f},
    {0.587785f, 0.0f, 0.809017f}, {0.0f, 0.0f, 0.436339f},
    {-0.587785f, 0.0f, 0.809017f}, {-0.391856f, 0.0f, 0.206011f},
    {-0.951057f, 0.0f, -0.309017f}, {-0.195928f, 0.0f, -0.396994f}
};

/* Draws visible score-effect trails through the hook registered by fn_1_9934. */
void fn_1_9C14(HU3D_MODEL *model, Mtx *view)
{
    M649EffectOwnerWorkView *work = lbl_1_bss_5A0->data;
    const Vec *point;
    M649EffectRecord *record = model->hookData;
    s32 j;
    Vec *position;
    s32 segments;
    u32 color;
    s32 i;
    s32 vertices;
    float angle, size;
    Mtx matrix;
    vertices = 10;
    segments = 16;
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 255);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    for (i = 0; i < 24; i++, record++) {
        if (record->phase && record->visible) {
            position = &record->trailPosition;
            size = record->size;
            color = *(u32 *)&record->color;
            MTXScale(matrix, size, (1.0f), size);
            MTXTransApply(matrix, matrix, position->x, position->y, position->z);
            MTXConcat(*view, matrix, matrix);
            GXLoadPosMtxImm(matrix, GX_PNMTX0);
            GXSetLineWidth(48, GX_TO_ZERO);
            GXBegin(GX_LINESTRIP, GX_VTXFMT0, vertices + 1);
            for (j = 0; j < vertices; j++) {
                point = &lbl_1_rodata_360[j];
                GXPosition3f32(point->x, point->y, point->z);
                GXColor1u32(color);
            }
            point = lbl_1_rodata_360;
            GXPosition3f32(point->x, point->y, point->z);
            GXColor1u32(color);
            GXEnd();
            GXSetLineWidth(36, GX_TO_ZERO);
            GXBegin(GX_LINESTRIP, GX_VTXFMT0, segments + 1);
            for (j = 0, angle = (0.0f); j < segments + 1;
                 j++, angle += (360.0f) / segments) {
                GXPosition3f32(sin((3.141592653589793) * angle / (180.0)),
                    (0.0f), cos((3.141592653589793) * angle / (180.0)));
                GXColor1u32(color);
            }
            GXEnd();
        }
    }
}

/* Attaches a queued shadow effect to its selected model when the caller submits it. */
void fn_1_A018(void *effect)
{
    M649ShadowRecord **entry;
    s32 i;
    OMOBJ *object;
    HU3D_MODEL *model;
    s16 modelId;
    M649EffectSlotsView_0A018 *work;
    if (((M649ShadowRecord *)effect) != NULL) {
        object = lbl_1_bss_5A0;
        modelId = object->mdlId[((M649ShadowRecord *)effect)->camera];
        model = &Hu3DData[modelId];
        work = model->hookData;
        entry = work->entries;
        for (i = 0; i < 1; i++, entry++) {
            if (*entry == NULL) {
                *entry = ((M649ShadowRecord *)effect);
                break;
            }
        }
    }
}

/* Attaches a moving marker to its model and selects that marker's camera. */
void fn_1_A0BC(M649Motion *motion)
{
    HU3D_MODEL *model;
    OMOBJ *object;
    s16 modelId;
    if (motion != NULL) {
        object = lbl_1_bss_5A0;
        modelId = object->mdlId[motion->id + 4];
        model = &Hu3DData[modelId];
        if (model->hookData == NULL) {
            model->hookData = motion;
            Hu3DModelCameraSet(modelId, fn_1_1218(motion->group));
        }
    }
}

/* Adds a score-effect record to the first free slot for the selected camera. */
void fn_1_A168(s32 cameraIndex, Vec *position, float *speed, s32 effectType)
{
    M649EffectRecordView_0A018 *record;
    s32 i;
    OMOBJ *object = lbl_1_bss_5A0;
    HU3D_MODEL *model;
    s16 modelId;
    modelId = object->mdlId[cameraIndex + 20];
    model = &Hu3DData[modelId];
    record = model->hookData;
    for (i = 0; i < 24; i++, record++) {
        if (record->phase == 0) {
            record->phase++;
            record->generation = effectType;
            record->speed = speed;
            record->position = *position;
            break;
        }
    }
}
