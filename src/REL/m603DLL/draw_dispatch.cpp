#include "REL/framework/frame_time_inline.h"
#include "REL/framework/list_object.h"
#include "REL/m603DLL/draw_owner.h"
#include "REL/m603DLL/camera_group.h"
#include "REL/m603DLL/shadow_owner.h"
#include "REL/m603DLL/night.h"
#include <new>
extern "C" {
#include "dolphin/types.h"
#include "math.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"

}

class ModelDrawOwner : public ListNode<ModelDrawOwner> {
public:
    s32 frameCount;
    s32 cameraIndex;
    s16 modelId;
    u8 initializedByte;
    ModelDrawOwner(s32 cameraIndex);
    ~ModelDrawOwner();
};

void fn_1_39C4();
extern "C" {
DrawListOwner::DrawListOwner() {}
}
DrawListOwner::~DrawListOwner() { fn_1_39C4(); }
void fn_1_48CC();
extern "C" void fn_1_3A94();
void fn_1_499C();
void DrawListOwner::fn_1_3714() { fn_1_48CC(); }
extern "C" {
void DrawListOwner::flagEnabled(int bit) { flags &= ~(1UL << bit); }
void DrawListOwner::update() { fn_1_3A94(); fn_1_499C(); }

extern ListRoot<FadeWork> lbl_1_bss_70;
extern ListRoot<ModelDrawOwner> lbl_1_bss_84;
const u32 lbl_1_rodata_198[4] = {1, 2, 4, 8};
}
void fn_1_3AD4(HU3D_MODEL*, Mtx*);

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
    fn_80026B64(modelId, (u16)lbl_1_rodata_198[context]);

    if (NightActive()) {
        CameraOwnerBBC::getChecked().fn_1_F30(this->context);
        ShadowModelOwner::getChecked().fn_1_5914(this->context);
        new ModelDrawOwner(this->context);
    }
}

FadeWork::~FadeWork()
{
    fn_800262D0(modelId);
}
void fn_1_39C4() { while (!lbl_1_bss_70.empty()) delete lbl_1_bss_70.back(); }
void fn_1_3AC4(FadeWork *);
void fn_1_3A94() { lbl_1_bss_70.visit(fn_1_3AC4); }
void fn_1_3AC4(FadeWork *work) { work->progress = work->progress + 1; }
void fn_1_3AD4(HU3D_MODEL *model,Mtx *matrix) { fn_8002472C(); FadeWork *hookData=(FadeWork *)model->hookData; hookData->fn_1_3B1C(matrix); }

void FadeWork::fn_1_3B1C(void *context)
{
    s16 night = lbl_802C0278;
    if (!night) {
        fn_1_3B8C(context);
    } else {
        fn_1_43A0(context);
    }
}

void FadeWork::fn_1_3B8C(void *context)
{
    Mtx identity;
    f32 progress;
    s32 i;

    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, GX_PNMTX0);
    GXSetNumTevStages(1U);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1U);
    GXSetNumTexGens(0U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0U);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    progress = (f32)this->progress / 5.0f;
    if (progress > 1.0f) {
        progress = 2.0f - progress;
        if (this->active == 0) {
            this->active = 1;
            CameraOwnerBBC::getChecked().fn_1_F30(this->context);
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
void FadeWork::fn_1_43A0(void *context)
{
    Mtx identity;
    f32 progress;
    f32 alpha;
    s32 alphaByte;

    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

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


const u32 lbl_1_rodata_1D8[4] = {1, 2, 4, 8};
}
extern void fn_1_49DC(HU3D_MODEL *model, Mtx *matrix);
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
    fn_80026B64(modelId, (u16)lbl_1_rodata_1D8[cameraIndex]);
}
extern "C" {

ModelDrawOwner::~ModelDrawOwner() { fn_800262D0(modelId); }
}
void fn_1_48CC() { while (!lbl_1_bss_84.empty()) delete lbl_1_bss_84.back(); }
void fn_1_49CC(ModelDrawOwner *);
void fn_1_499C() { lbl_1_bss_84.visit(fn_1_49CC); }
void fn_1_49CC(ModelDrawOwner *owner) { owner->frameCount = owner->frameCount + 1; }
void fn_1_4A24(void*,void*); void fn_1_4A54(void*,void*);
void fn_1_49DC(HU3D_MODEL *model,Mtx *matrix) { fn_8002472C(); void *hookData=model->hookData; fn_1_4A24(hookData,matrix); }
void fn_1_4A24(void *first,void *second) { fn_1_4A54(first,second); }
extern "C" {
extern "C" {
extern s8 lbl_1_bss_8C;
Vec lbl_1_data_120[16] = {
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
    {0.0f,0.0f,-111.0f},
};
}
}

void fn_1_4A54(void *first,void *second)
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
    GXSetNumTevStages(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                  GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                   GX_LO_CLEAR);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    width = 66.92521f;
    height = 50.0f;
    innerWidth = 0.89f * width;
    innerHeight = 0.87f * height;
    if (lbl_1_bss_8C == 0) {
        lbl_1_data_120[0].x = -width;
        lbl_1_data_120[0].y = -height;
        lbl_1_data_120[1].x = width;
        lbl_1_data_120[1].y = -height;
        lbl_1_data_120[2].x = innerWidth;
        lbl_1_data_120[2].y = -innerHeight;
        lbl_1_data_120[3].x = -innerWidth;
        lbl_1_data_120[3].y = -innerHeight;
        lbl_1_data_120[4].x = width;
        lbl_1_data_120[4].y = -height;
        lbl_1_data_120[5].x = width;
        lbl_1_data_120[5].y = height;
        lbl_1_data_120[6].x = innerWidth;
        lbl_1_data_120[6].y = innerHeight;
        lbl_1_data_120[7].x = innerWidth;
        lbl_1_data_120[7].y = -innerHeight;
        lbl_1_data_120[8].x = -width;
        lbl_1_data_120[8].y = height;
        lbl_1_data_120[9].x = width;
        lbl_1_data_120[9].y = height;
        lbl_1_data_120[10].x = innerWidth;
        lbl_1_data_120[10].y = innerHeight;
        lbl_1_data_120[11].x = -innerWidth;
        lbl_1_data_120[11].y = innerHeight;
        lbl_1_data_120[12].x = -width;
        lbl_1_data_120[12].y = -height;
        lbl_1_data_120[13].x = -width;
        lbl_1_data_120[13].y = height;
        lbl_1_data_120[14].x = -innerWidth;
        lbl_1_data_120[14].y = innerHeight;
        lbl_1_data_120[15].x = -innerWidth;
        lbl_1_data_120[15].y = -innerHeight;
        lbl_1_bss_8C = 1;
    }

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
            GXPosition3f32(lbl_1_data_120[tableIndex].x,
                           lbl_1_data_120[tableIndex].y, -111.0f);
            GXColor4u8(0xFF, 0xFF, 0xFF, 0xFF);
            tableIndex++;
        }
    }
}


extern "C" {






ListRoot<FadeWork> lbl_1_bss_70;
ListRoot<ModelDrawOwner> lbl_1_bss_84;
s8 lbl_1_bss_8C;



}
