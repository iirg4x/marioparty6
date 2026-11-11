#include "REL/m648DLL/particle_entry.h"
#include "humath.h"
float RandomFloat(float, float);

float RandomFloat(void);

unsigned long RandomUnsigned(void);

extern "C" {
#include "dolphin/os.h"
}

#include "REL/m648DLL/positioned_model_owner.h"

extern "C" {
#include "dolphin/types.h"
}

extern "C" {
#include "dolphin/gx.h"
}

extern "C" {
#include "dolphin/mtx.h"
}

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/data.h"
}

extern "C" {
#include "game/memory.h"
}

extern "C" {
#include "game/sprite.h"
}

extern "C" {
#include "REL/framework/assertion.h"
}

extern "C" {
#include "REL/m648DLL/camera_view.h"
}

extern "C" {
#include <math.h>
}

#include "REL/framework/list_object.h"

extern "C" {
extern ListRoot<WhiteParticles> lbl_1_bss_5AC;
extern ListRoot<ColorParticles> lbl_1_bss_5C0;
}

#include "REL/m648DLL/managers.h"

extern "C" {
Vec lbl_1_bss_288[0x21];
}

extern "C" {
Vec lbl_1_bss_414[0x21];
}

extern "C" {
static const s32 animationDataNumbers8FA4[2] = {
    DATANUM(0x460000, 25), DATANUM(0x460000, 27)
};
}

void fn_1_9BA4(void);

void fn_1_A12C(void);

void fn_1_970C();
void fn_1_9C74();
void fn_1_A218();

ANIMDATA *fn_1_93AC(u32 entryIndex);

void fn_1_97C0(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

void fn_1_9D28(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

void fn_1_963C(void);

void fn_1_973C(WhiteParticles *);

extern "C" {
#include "dolphin.h"
}

void fn_1_9CA4(ColorParticles *);

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/gamework.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

void fn_1_A0E8(HU3D_MODEL *model, Mtx *data);

extern ListRoot<PositionedModelOwner> lbl_1_bss_5D4;

void fn_1_A248(PositionedModelOwner *);

ParticleAnimationOwner::ParticleAnimationOwner() {
    s32 animationIndex;
    s32 directionIndex;
    for (animationIndex = 0; animationIndex < 2; ++animationIndex) {
        animations[animationIndex] =
            HuSprAnimRead(fn_80008DD0(animationDataNumbers8FA4[animationIndex],
                                              0x10000000, 2));
        HuSprAnimLock(animations[animationIndex]);
    }

    for (directionIndex = 0; directionIndex < 0x21; ++directionIndex) {
        f32 angle = 360.0f * ((f32)(directionIndex & 0x1F) / 32.0f) - 180.0f;
        f64 radians;

        angle += 90.0f;
        if (angle > 180.0f) {
            angle -= 360.0f;
        }
        radians = (M_PI * (f64)angle) / 180.0;
        radians = (f64)(f32)radians;
        lbl_1_bss_414[directionIndex].x = (f32)(0.7 * cos(radians));
        lbl_1_bss_414[directionIndex].y = (f32)(0.7 * sin(radians));
        lbl_1_bss_414[directionIndex].z = 0.0f;
        lbl_1_bss_288[directionIndex].x = (f32)cos(radians);
        lbl_1_bss_288[directionIndex].y = (f32)sin(radians);
        lbl_1_bss_288[directionIndex].z = 0.0f;
    }
}

void fn_1_963C(void);

ParticleAnimationOwner::~ParticleAnimationOwner() {
    fn_1_963C();
    fn_1_9BA4();
    fn_1_A12C();
    for (s32 spriteIndex = 0; spriteIndex < 2; spriteIndex++) HuSprAnimKill(animations[spriteIndex]);
}

ANIMDATA *fn_1_93AC(u32 entryIndex)
{
    return ParticleAnimationOwner::getChecked().animations[entryIndex];
}

void ParticleAnimationOwner::flagEnabled(int request) { flags &= ~(1UL << request); }

void ParticleAnimationOwner::update() { fn_1_970C(); fn_1_9C74(); fn_1_A218(); }

WhiteParticles::WhiteParticles(Vec *position)
{
    ANIMDATA *particleAnimation;

    insert4(&lbl_1_bss_5AC);
    particleAnimation = fn_1_93AC(0);
    modelId = fn_8003216C(particleAnimation, 1);
    fn_80026790(modelId, position);
    fn_80026B8C(modelId, 5);
    fn_800324E0(modelId, 100.0f);
    fn_800326D0(modelId, fn_1_97C0);
    fn_80032620(modelId, 0.0f);
    fn_8003269C(modelId, 0);
    state = 0;
}

WhiteParticles::~WhiteParticles()
{
    fn_800262D0(modelId);
}

void fn_1_963C() {
    while(!lbl_1_bss_5AC.empty()) delete lbl_1_bss_5AC.back();
}

void fn_1_970C()
{
    lbl_1_bss_5AC.visit(fn_1_973C);
}

void fn_1_973C(WhiteParticles *owner)
{
    HU3D_PARTICLE *particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[owner->modelId].hookData);

    if (particle->dataCnt < 0) {
        delete owner;
    }
}

void fn_1_97C0(HU3D_MODEL *model, HU3D_PARTICLE *owner, Mtx matrix)
{
    HU3D_PARTICLE_DATA *end = owner->data + owner->maxCnt;
    HU3D_PARTICLE_DATA *initializingEntry;
    HU3D_PARTICLE_DATA *entry;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 updatedCount;

    if (owner->count == 0) {
        for (initializingEntry = owner->data; initializingEntry != end;
             ++initializingEntry) {
            initializingEntry->vel.x = 2.5f *
                                RandomFloat(-1.0f,
                                           1.0f);
            initializingEntry->vel.y = 2.5f *
                                RandomFloat(-1.0f,
                                           1.0f);
            initializingEntry->vel.z = 2.5f *
                                RandomFloat(-1.0f,
                                           1.0f);
            initializingEntry->pos.x = initializingEntry->pos.y =
                initializingEntry->pos.z =
                0.0f;
            initializingEntry->color.r = 0x80;
            initializingEntry->color.g = 0xFF;
            initializingEntry->color.b = 0xFF;
            initializingEntry->color.a = 0x80;
        }
    }

    updatedCount = 0;
    for (entry = owner->data; entry != end; ++entry) {
        entry->vel.y = (f32)(entry->vel.y - 0.27240694444444447);
        PSVECScale(&entry->vel, &entry->vel, 0.8f);
        PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
        entry->color.r = 0xFF;
        green = entry->color.g;
        green += 0x20;
        if (green > 0xFF) {
            green = 0xFF;
        }
        entry->color.g = (u8)green;
        blue = entry->color.b;
        blue += 0x20;
        if (blue > 0xFF) {
            blue = 0xFF;
        }
        entry->color.b = (u8)blue;
        alpha = entry->color.a;
        alpha -= 8;
        if (alpha < 0) {
            alpha = 0;
            entry->scale = 0.0f;
            owner->dataCnt = -1;
        }
        entry->color.a = (u8)alpha;
        ++updatedCount;
    }

    owner->emitCnt = (s16)updatedCount;
    DCStoreRangeNoSync(owner->data, owner->maxCnt * 0x48);
}

ColorParticles::ColorParticles(Vec *position)
{
    ANIMDATA *particleAnimation;

    insert4(&lbl_1_bss_5C0);
    particleAnimation = fn_1_93AC(1);
    modelId = fn_8003216C(particleAnimation, 8);
    fn_80026790(modelId, position);
    fn_80026B8C(modelId, 5);
    fn_800324E0(modelId, 40.0f);
    fn_800326D0(modelId, fn_1_9D28);
    fn_80032620(modelId, 0.0f);
    fn_8003269C(modelId, 0);
    state = 0;
}

ColorParticles::~ColorParticles()
{
    fn_800262D0(modelId);
}

void fn_1_9BA4() {
    while(!lbl_1_bss_5C0.empty()) delete lbl_1_bss_5C0.back();
}

void fn_1_9C74()
{
    lbl_1_bss_5C0.visit(fn_1_9CA4);
}

void fn_1_9CA4(ColorParticles *owner)
{
    HU3D_PARTICLE *particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[owner->modelId].hookData);

    if (particle->dataCnt < 0) {
        delete owner;
    }
}

extern "C" {
const GXColor lbl_1_rodata_2B8[8] = {
    {0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xC0, 0xFF},
    {0xFF, 0xC0, 0xFF, 0xFF},
    {0xFF, 0xC0, 0xC0, 0xFF},
    {0xC0, 0xFF, 0xFF, 0xFF},
    {0xC0, 0xFF, 0xC0, 0xFF},
    {0xC0, 0xC0, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF},
};
}
void fn_1_9D28(HU3D_MODEL *model, HU3D_PARTICLE *owner, Mtx matrix)
{
    HU3D_PARTICLE_DATA *end = owner->data + owner->maxCnt;
    HU3D_PARTICLE_DATA *initializingEntry;
    HU3D_PARTICLE_DATA *entry;
    const GXColor *color;
    s32 alpha;

    if (owner->count == 0) {
        for (initializingEntry = owner->data; initializingEntry != end;
             ++initializingEntry) {
            initializingEntry->vel.x = 2.5f *
                RandomFloat(-1.0f, 1.0f);
            initializingEntry->vel.y = 2.5f *
                RandomFloat(-1.0f, 1.0f);
            initializingEntry->vel.z = RandomFloat(0.0f,
                                                       4.0f);
            initializingEntry->pos.x = initializingEntry->pos.y =
                initializingEntry->pos.z = 0.0f;
            color = &lbl_1_rodata_2B8[RandomUnsigned() & 7];
            initializingEntry->color = *color;
            initializingEntry->zRot = (f32)(M_PI *
                (f64)(360.0f * RandomFloat()) / 180.0);
        }
    }

    for (entry = owner->data; entry != end; ++entry) {
        PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
        alpha = entry->color.a;
        alpha -= owner->emitCnt;
        if (alpha < 0) {
            alpha = 0;
            entry->scale = 0.0f;
            owner->dataCnt = -1;
        }
        entry->color.a = (u8)alpha;
        entry->zRot = (f32)(M_PI *
            (5.0 + 180.0 *
             ((f64)entry->zRot / M_PI)) / 180.0);
    }

    owner->emitCnt += 1;
    DCStoreRangeNoSync(owner->data, owner->maxCnt * 0x48);
}

PositionedModelOwner::PositionedModelOwner(const Vec *value)
    : model(fn_1_A0E8), position(*value)
{
    insert4(&lbl_1_bss_5D4);
    model.setHookData(this);
    model.setLayer(5);
    updatesRemaining = 0xF;
}

PositionedModelOwner::~PositionedModelOwner() {}

void fn_1_A0E8(HU3D_MODEL *model, Mtx *data) {
    PositionedModelOwner *hookData = static_cast<PositionedModelOwner *>(model->hookData);
    hookData->fn_1_A2F8(*data);
}

void fn_1_A12C() {
    while(!lbl_1_bss_5D4.empty()) delete lbl_1_bss_5D4.back();
}

void fn_1_A218()
{
    lbl_1_bss_5D4.visitMoved(fn_1_A248);
}

void fn_1_A248(PositionedModelOwner *object) { object->fn_1_A270(); }

void PositionedModelOwner::fn_1_A270() {
    updatesRemaining = updatesRemaining - 1;
    if (updatesRemaining < 0) {
        delete this;
    }
}

void PositionedModelOwner::fn_1_A2F8(Mtx viewMatrix)
{
    Mtx inverseView;
    Mtx translatedView;
    s32 alpha;
    f32 radius;
    s32 index;

    PSMTXInverse(viewMatrix, inverseView);
    inverseView[0][3] = inverseView[1][3] = inverseView[2][3] = 0.0f;
    PSMTXTrans(translatedView, position.x, position.y, position.z);
    PSMTXConcat(viewMatrix, translatedView, translatedView);
    PSMTXConcat(translatedView, inverseView, translatedView);
    GXLoadPosMtxImm(translatedView, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(GX_DISABLE, GX_ALWAYS, GX_DISABLE);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    alpha = updatesRemaining * 4;
    radius = 1.5f * (f32)(0x10 - updatesRemaining);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0x42);
    for (index = 0; index < 0x21; ++index) {
        GXPosition3f32(radius * lbl_1_bss_288[index].x,
                       radius * lbl_1_bss_288[index].y, 0.0f);
        GXColor4u8(0xFF, 0xFF, 0xFF, alpha);
        GXPosition3f32(radius * lbl_1_bss_414[index].x,
                       radius * lbl_1_bss_414[index].y, 0.0f);
        GXColor4u8(0, 0xFF, 0xFF, 0);
    }
}

extern "C" {
ListRoot<WhiteParticles> lbl_1_bss_5AC;
ListRoot<ColorParticles> lbl_1_bss_5C0;
}
ListRoot<PositionedModelOwner> lbl_1_bss_5D4;
