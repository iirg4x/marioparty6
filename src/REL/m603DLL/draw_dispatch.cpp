#include "REL/framework/frame_time_inline.h"
#include "REL/framework/list_object.h"
#include "REL/m603DLL/draw_owner.h"
#include "REL/m603DLL/camera_group.h"
#include "REL/m603DLL/shadow_owner.h"
#include <new>
extern "C" {
#include "dolphin/types.h"
#include "math.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "game/gamework.h"

}

class ModelDrawOwner : public ListNode<ModelDrawOwner> {
public:
    s32 frameCount;
    s32 cameraIndex;
    s16 modelId;
    u8 initializedByte;
    ModelDrawOwner(s32 cameraIndex);
    ~ModelDrawOwner();
    void fn_1_4A24(Mtx *matrix);
    void fn_1_4A54(Mtx *matrix);
};

/* Initialize the draw-list request owner. */
DrawListOwner::DrawListOwner() {}

/* Delete all shutter and flash effects. */
DrawListOwner::~DrawListOwner()
{
    fn_1_39C4();
}
/* Delete all frame overlays. */
void DrawListOwner::fn_1_3714()
{
    fn_1_48CC();
}

/* Clear the requested flag bit. */
void DrawListOwner::flagEnabled(int bit)
{
    flags &= ~(1UL << bit);
}
/* Advance the effect and frame-overlay lists. */
void DrawListOwner::update()
{
    fn_1_3A94();
    fn_1_499C();
}

ListRoot<FadeWork> lbl_1_bss_70;
ListRoot<ModelDrawOwner> lbl_1_bss_84;
const u32 shutterCameraBits[4] = {1, 2, 4, 8};

/* Create a camera-specific shutter or flash drawing hook. */
FadeWork::FadeWork(s32 context)
{
    HU3D_MODEL *model;

    progress = 0;
    this->context = context;
    active = 0;
    insert4(&lbl_1_bss_70);
    modelId = fn_800260F4(fn_1_3AD4);
    model = &lbl_802C01C8[modelId];
    model->hookData = this;
    fn_80026B8C(modelId, 6);
    fn_80026B64(modelId, (u16)shutterCameraBits[context]);

    if (GWMgNightFGet() != 0) {
        CameraOwner::getChecked().fn_1_F30(this->context);
        ShadowModelOwner::getChecked().fn_1_5914(this->context);
        new ModelDrawOwner(this->context);
    }
}

/* Release the effect drawing model. */
FadeWork::~FadeWork()
{
    fn_800262D0(modelId);
}
/* Delete every shutter and flash effect. */
void fn_1_39C4()
{
    while (!lbl_1_bss_70.empty()) delete lbl_1_bss_70.back();
}
/* Visit the effect list to advance its frame counters. */
void fn_1_3A94()
{
    lbl_1_bss_70.visit(fn_1_3AC4);
}
/* Advance one effect frame counter. */
void fn_1_3AC4(FadeWork *work)
{
    work->progress = work->progress + 1;
}
/* Dispatch the model drawing hook to its effect object. */
void fn_1_3AD4(HU3D_MODEL *model,Mtx *matrix)
{
    fn_8002472C();
    FadeWork *hookData = static_cast<FadeWork *>(model->hookData);
    hookData->fn_1_3B1C(matrix);
}

/* Choose shutter drawing by day and flash drawing by night. */
void FadeWork::fn_1_3B1C(Mtx *matrix)
{
    if (GWMgNightFGet() == 0) {
        fn_1_3B8C(matrix);
    } else {
        fn_1_43A0(matrix);
    }
}

/* Draw the shutter triangles and trigger the camera at their midpoint. */
void FadeWork::fn_1_3B8C(Mtx *matrix)
{
    Mtx identity;
    f32 progress;

    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, GX_PNMTX0);
    configureColorDrawing();

    progress = (f32)this->progress / 5.0f;
    if (progress > 1.0f) {
        progress = 2.0f - progress;
        if (this->active == 0) {
            this->active = 1;
            CameraOwner::getChecked().fn_1_F30(this->context);
            ShadowModelOwner::getChecked().fn_1_5914(this->context);
            new ModelDrawOwner(this->context);
        }
    }
    if (progress < 0.0f) {
        delete this;
        return;
    }

    progress = 2.0f * (70.0f * progress);
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3U);
    GXPosition3f32(-70.0f, -70.0f, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);
    GXPosition3f32(-70.0f, 70.0f, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);
    GXPosition3f32(-70.0f + progress, 70.0f, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);

    GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3U);
    GXPosition3f32(-70.0f, 70.0f, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);
    GXPosition3f32(70.0f, 70.0f, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);
    GXPosition3f32(70.0f, 70.0f - progress, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);

    GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3U);
    GXPosition3f32(70.0f, 70.0f, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);
    GXPosition3f32(70.0f, -70.0f, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);
    GXPosition3f32(70.0f - progress, -70.0f, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);

    GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3U);
    GXPosition3f32(70.0f, -70.0f, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);
    GXPosition3f32(-70.0f, -70.0f, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);
    GXPosition3f32(-70.0f, -70.0f + progress, -110.0f);
    GXColor4u8(0, 0, 0, 0xFF);
}
/* Draw a fading white camera flash. */
void FadeWork::fn_1_43A0(Mtx *matrix)
{
    Mtx identity;
    f32 progress;
    f32 alpha;
    s32 alphaByte;

    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, GX_PNMTX0);
    configureColorDrawing();

    progress = (f32)this->progress / 8.0f;
    alpha = 1.0f - progress;
    if (alpha < 0.0f) {
        delete this;
        return;
    }

    alphaByte = (s32)(255.0f * alpha);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-70.0f, -70.0f, -110.0f);
    GXColor4u8(0xFF, 0xFF, 0xFF, alphaByte);
    GXPosition3f32(70.0f, -70.0f, -110.0f);
    GXColor4u8(0xFF, 0xFF, 0xFF, alphaByte);
    GXPosition3f32(70.0f, 70.0f, -110.0f);
    GXColor4u8(0xFF, 0xFF, 0xFF, alphaByte);
    GXPosition3f32(-70.0f, 70.0f, -110.0f);
    GXColor4u8(0xFF, 0xFF, 0xFF, alphaByte);
}
extern "C" {

const u32 frameCameraBits[4] = {1, 2, 4, 8};
}
/* Create a frame overlay for one camera. */
ModelDrawOwner::ModelDrawOwner(s32 cameraIndex)
{
    HU3D_MODEL *model;

    frameCount = 0;
    this->cameraIndex = cameraIndex;
    initializedByte = 0;
    insert4(&lbl_1_bss_84);
    modelId = fn_800260F4(fn_1_49DC);
    model = &lbl_802C01C8[modelId];
    model->hookData = this;
    fn_80026B8C(modelId, 6);
    fn_80026B64(modelId, (u16)frameCameraBits[cameraIndex]);
}

/* Release the frame-overlay drawing model. */
ModelDrawOwner::~ModelDrawOwner()
{
    fn_800262D0(modelId);
}

/* Delete every frame overlay. */
void fn_1_48CC()
{
    while (!lbl_1_bss_84.empty()) delete lbl_1_bss_84.back();
}
/* Visit the frame-overlay list to advance its counters. */
void fn_1_499C()
{
    lbl_1_bss_84.visit(fn_1_49CC);
}
/* Advance one frame-overlay counter. */
void fn_1_49CC(ModelDrawOwner *owner)
{
    owner->frameCount = owner->frameCount + 1;
}
/* Dispatch the model drawing hook to its frame overlay. */
void fn_1_49DC(HU3D_MODEL *model, Mtx *matrix)
{
    fn_8002472C();
    ModelDrawOwner *hookData = static_cast<ModelDrawOwner *>(model->hookData);
    hookData->fn_1_4A24(matrix);
}

/* Draw this camera frame overlay. */
void ModelDrawOwner::fn_1_4A24(Mtx *matrix)
{
    fn_1_4A54(matrix);
}

/* Draw the tinted photo area and its white frame. */
void ModelDrawOwner::fn_1_4A54(Mtx *matrix)
{
    Mtx positionMatrix;
    f32 width;
    f32 height;
    f32 innerWidth;
    f32 innerHeight;
    s32 tableIndex;
    s32 batch;
    s32 vertex;

    PSMTXIdentity(positionMatrix);
    GXLoadPosMtxImm(positionMatrix, GX_PNMTX0);
    configureColorDrawing();

    width = 66.92521f;
    height = 50.0f;
    innerWidth = 0.89f * width;
    innerHeight = 0.87f * height;
    static Vec frameVertices[16] = {
        {-width, -height, -111.0f},
        {width, -height, -111.0f},
        {innerWidth, -innerHeight, -111.0f},
        {-innerWidth, -innerHeight, -111.0f},
        {width, -height, -111.0f},
        {width, height, -111.0f},
        {innerWidth, innerHeight, -111.0f},
        {innerWidth, -innerHeight, -111.0f},
        {-width, height, -111.0f},
        {width, height, -111.0f},
        {innerWidth, innerHeight, -111.0f},
        {-innerWidth, innerHeight, -111.0f},
        {-width, -height, -111.0f},
        {-width, height, -111.0f},
        {-innerWidth, innerHeight, -111.0f},
        {-innerWidth, -innerHeight, -111.0f}
    };

    tableIndex = 0;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-width, -height, -111.0f);
    GXColor4u8(0xFF, 0xC0, 0x40, 0x20);
    GXPosition3f32(width, -height, -111.0f);
    GXColor4u8(0xFF, 0xC0, 0x40, 0x20);
    GXPosition3f32(width, height, -111.0f);
    GXColor4u8(0xFF, 0xC0, 0x40, 0x20);
    GXPosition3f32(-width, height, -111.0f);
    GXColor4u8(0xFF, 0xC0, 0x40, 0x20);

    for (batch = 0; batch < 4; batch++) {
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        for (vertex = 0; vertex < 4; vertex++) {
            GXPosition3f32(frameVertices[tableIndex].x,
                           frameVertices[tableIndex].y, -111.0f);
            GXColor4u8(0xFF, 0xFF, 0xFF, 0xFF);
            tableIndex++;
        }
    }
}
