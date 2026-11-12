#include "REL/m604DLL/stage_effects.h"
#include "dolphin/types.h"
#include "REL/m604DLL/sprite_parameters.h"
#include "math.h"
#include "dolphin/gx.h"
#include "humath.h"
#include "dolphin/mtx.h"
extern "C" {
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/data.h"
const u32 lbl_1_rodata_210[4] = {0x3F0031, 0x3F0032, 0x3F0033, 0x3F0034};
}
void fn_1_5074(HU3D_MODEL *, Mtx *);
void fn_1_5314(LensFlare *, Mtx);

void fn_1_5074(HU3D_MODEL *model, Mtx *data) {
    LensFlare *hookData = static_cast<LensFlare *>(model->hookData);
    fn_1_5314(hookData, *data);
}

LensFlare::LensFlare() {
    s32 index;
    s16 handle = fn_800260F4(fn_1_5074);
    fn_80026B8C((s16)handle, 6);
    fn_80026B64((s16)handle, 0xF);
    HU3D_MODEL *model = &lbl_802C01C8[handle];
    model->hookData = this;
    modelId = (s16)handle;
    for (index=0; index<4; ++index) {
        animations[index] = HuSprAnimRead(fn_80008DD0(lbl_1_rodata_210[index],0x10000000,2));
        HuSprAnimLock(animations[index]);
    }
}

extern "C" void fn_800262D0(short);

LensFlare::~LensFlare() {
    fn_800262D0(modelId);
    for (int spriteIndex = 0; spriteIndex < 4; spriteIndex++) {
        HuSprAnimKill(animations[spriteIndex]);
    }
}

void LensFlare::flagEnabled(int bitIndex) {
    flags &= ~(1 << bitIndex);
}

struct SpritePositionCopy {
    Vec value;
    SpritePositionCopy() {}
    SpritePositionCopy(const Vec &source) : value(source) {}
};

void fn_1_5B38(LensFlare *, f32, SpritePositionCopy, f32, u32, GXTexMapID);
Vec fn_1_5F04(LensFlare *, SpritePositionCopy, Mtx);

extern "C" {
extern const s32 lbl_1_rodata_220[4];

const s32 lbl_1_rodata_220[4] = {0, 1, 2, 3};
const FlareSpriteParameters lbl_1_rodata_230[8] = {
    {8.0f, -1.0f, 0xFFFFFFFF, GX_TEXMAP0},
    {8.0f, -0.8f, 0x30FFFF80, GX_TEXMAP2},
    {7.0f, -0.7f, 0x60C080FF, GX_TEXMAP1},
    {5.0f, 0.1f, 0x50FFFFFF, GX_TEXMAP1},
    {10.0f, 0.2f, 0x30FFFFC0, GX_TEXMAP2},
    {15.0f, 0.7f, 0x60FFFFFF, GX_TEXMAP1},
    {20.0f, 0.8f, 0x40FFC0FF, GX_TEXMAP2},
    {12.0f, 1.0f, 0x30C0FFFF, GX_TEXMAP1}
};

}

void fn_1_5298(LensFlare *work)
{
    for (s32 index = 1; index < 4; ++index) {
        HuSprTexLoad(work->animations[index], 0,
                     (s16)lbl_1_rodata_220[index],
                     GX_REPEAT, GX_REPEAT, GX_LINEAR);
    }
}

void fn_1_5314(LensFlare *work, Mtx cameraTransform)
{
    static Vec lbl_1_data_1D0 = { -100.0f, 4200.0f, 0.0f };
    Vec direction;
    HU3D_CAMERA *camera = &Hu3DCamera[lbl_802C01A6];
    Vec cameraPosition = camera->pos;
    lbl_1_data_1D0.y = 4200.0f + (0.8f * cameraPosition.y);
    lbl_1_data_1D0.x = cameraPosition.x - 100.0f;
    cameraPosition.x += 500.0f;

    PSVECSubtract(&cameraPosition, &lbl_1_data_1D0, &direction);
    PSVECNormalize(&direction, &direction);

    Mtx identity;
    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, GX_PNMTX0);

    Vec projectedDirection;
    PSMTXMultVecSR(cameraTransform, &direction, &projectedDirection);
    f32 depthFactor = projectedDirection.z;
    if (depthFactor < 0.0f) {
        return;
    }

    HuSprTexLoad(work->animations[0], 0, 0,
                     GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetZMode(1, GX_ALWAYS, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetZCompLoc(0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetAlphaUpdate(1);

    SpritePositionCopy spritePosition;
    Vec screenPosition;
    f32 baseScale = 120.0f * lbl_1_rodata_230[0].verticalScale;

    spritePosition.value.x = projectedDirection.x * baseScale;
    spritePosition.value.y = projectedDirection.y * baseScale;
    spritePosition.value.z = -100.0f;
    fn_1_5B38(work, 1.0f, spritePosition, 0.5f * lbl_1_rodata_230[0].width,
              lbl_1_rodata_230[0].color,
              (GXTexMapID)lbl_1_rodata_230[0].textureMap);

    f32 fieldOfView = camera->fov;
    f32 fovCosine = (f32)cos((f64)(MTXDegToRad(fieldOfView)));
    depthFactor = (depthFactor - fovCosine) / (1.0f - fovCosine);
    PSVECScale(&projectedDirection, &projectedDirection, 1.0f - fovCosine);

    if (!(depthFactor < 0.0f)) {
        SpritePositionCopy worldPosition(lbl_1_data_1D0);
        screenPosition = fn_1_5F04(work, worldPosition, cameraTransform);
        s32 screenX = (s32)screenPosition.x;
        s32 screenY = (s32)screenPosition.y;
        fn_1_5298(work);
        for (s32 index = 1; index < 8; index++) {
            f32 scale = 120.0f * lbl_1_rodata_230[index].verticalScale;
            spritePosition.value.x = projectedDirection.x * scale;
            spritePosition.value.y = projectedDirection.y * scale;
            spritePosition.value.z = -100.0f;
            fn_1_5B38(work, depthFactor, spritePosition,
                      0.5f * lbl_1_rodata_230[index].width,
                      lbl_1_rodata_230[index].color,
                      (GXTexMapID)lbl_1_rodata_230[index].textureMap);
        }
        if (!(depthFactor < 0.9f)) {
            f32 alphaFactor = (depthFactor - 0.9f) / 0.2f;
            GXClearVtxDesc();
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
            GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
            f32 alpha = 80.0f * alphaFactor;
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(-79.0f, -240.0f, -100.0f);
            GXColor4u8(0xFF, 0xFF, 0xFF, (s32)alpha);
            GXPosition3f32(-79.0f, 240.0f, -100.0f);
            GXColor4u8(0xFF, 0xFF, 0xFF, (s32)alpha);
            GXPosition3f32(79.0f, 240.0f, -100.0f);
            GXColor4u8(0xFF, 0xFF, 0xFF, (s32)alpha);
            GXPosition3f32(79.0f, -240.0f, -100.0f);
            GXColor4u8(0xFF, 0xFF, 0xFF, (s32)alpha);
        }
    }
}

void fn_1_5B38(LensFlare *, f32 alphaScale, SpritePositionCopy position,
                           f32 halfSize, u32 packedColor,
                           GXTexMapID textureMap)
{
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, textureMap, GX_COLOR0A0);

    u8 red = (u8)packedColor;
    u8 redForColor = red;
    u8 green = (u8)(packedColor >> 8);
    u8 greenForColor = green;
    u8 blue = (u8)(packedColor >> 16);
    u8 blueForColor = blue;
    u8 alpha = (u8)(packedColor >> 24);
    s32 scaledAlpha = (s32)(alphaScale * alpha);
    f32 size = halfSize * alphaScale;
    position.value.y += 15.0f;

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXPosition3f32(position.value.x - size, position.value.y - size, position.value.z);
    GXColor4u8(redForColor, greenForColor, blueForColor, (u8)scaledAlpha);
    GXTexCoord2f32(0.0f, 0.0f);

    GXPosition3f32(position.value.x - size, position.value.y + size, position.value.z);
    GXColor4u8(redForColor, greenForColor, blueForColor, (u8)scaledAlpha);
    GXTexCoord2f32(0.0f, 1.0f);

    GXPosition3f32(position.value.x + size, position.value.y + size, position.value.z);
    GXColor4u8(redForColor, greenForColor, blueForColor, (u8)scaledAlpha);
    GXTexCoord2f32(1.0f, 1.0f);

    GXPosition3f32(position.value.x + size, position.value.y - size, position.value.z);
    GXColor4u8(redForColor, greenForColor, blueForColor, (u8)scaledAlpha);
    GXTexCoord2f32(1.0f, 0.0f);
}

Vec fn_1_5F04(LensFlare *, SpritePositionCopy worldPosition, Mtx cameraTransform)
{
    HU3D_CAMERA *camera;
    Vec cameraPosition;
    Vec projectedPosition;
    f64 sine;
    f32 verticalScale;
    f32 horizontalScale;

    camera = &Hu3DCamera[lbl_802C01A6];
    PSMTXMultVec(cameraTransform, &worldPosition.value, &cameraPosition);

    verticalScale = 142.2f *
        (cameraPosition.z *
         (sin((M_PI *
               (f64)(camera->fov / 2.0f)) / 180.0) /
          cos((M_PI *
               (f64)(camera->fov / 2.0f)) / 180.0)));
    horizontalScale = cameraPosition.z *
        (sin((M_PI *
              (f64)(camera->fov / 2.0f)) / 180.0) /
         cos((M_PI *
              (f64)(camera->fov / 2.0f)) / 180.0));

    projectedPosition.x = 79.0f +
        cameraPosition.x * (79.0f / -verticalScale);
    projectedPosition.y = 240.0f +
        cameraPosition.y * (240.0f / horizontalScale);
    projectedPosition.z = 0.0f;
    return projectedPosition;
}

void LensFlare::update()
{
}
