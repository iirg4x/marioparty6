/* Native shared-pool reconstruction from recovered math/effect functions.
 * All storage is emitted by their real literal/cast use, never filler. */
#define _MATH_H
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "math.h"
#include "REL/mdminidll/readonly_scalars.h"
#include "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"

/* Consumed views of existing readonly f64 values; no storage is defined. */
extern const f64 lbl_1_rodata_290;
extern const f64 lbl_1_rodata_2A0;
extern const f64 lbl_1_rodata_2F8;



/* Shares the compiler's 90.0f pool with the scalar sine operation above. */



#define _MATH_H
#include "game/hu3d.h"

/* Existing four-byte readonly scalar; original declaration/name unknown.
 * This consumed view defines no storage. */
extern const f32 lbl_1_rodata_288;



#define _MATH_H
#include "game/main.h"

/* Target-backed readonly f32 scalar; original declaration/name unknown.
 * This consumed view defines no storage. */
extern const f32 lbl_1_rodata_2A8;



#define _MATH_H
#include "game/main.h"
#include "game/hsfex.h"
#include "game/sprite.h"
#include "REL/mdminidll/readonly_scalars.h"

/* Only the consumed prefix is typed here. Retail accesses the 2x3 matrix,
 * color, and two adjacent floats at the target-owned indirect-texture state;
 * no extent or meaning is claimed for any following bytes. */
typedef struct MDMinIndirectTextureConsumedView {
    f32 matrix[2][3];
    GXColor color;
    f32 translate_y;
    f32 translate_y_delta;
} MDMinIndirectTextureConsumedView;

extern ANIMDATA *lbl_1_bss_880;
extern ANIMDATA *lbl_1_bss_884;
extern void *lbl_1_bss_888;
extern const f32 lbl_1_rodata_2AC;
extern const f32 lbl_1_rodata_2B0;
extern const f32 lbl_1_rodata_2B4;
extern const f32 lbl_1_rodata_2B8;



#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "REL/mdminidll/layer_effect_state.h"

/* Target-backed scalar views; original declarations are unknown. */
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_298;
extern const f32 lbl_1_rodata_2A8;
extern const f32 lbl_1_rodata_2BC;
extern const f32 lbl_1_rodata_2C0;



#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"

#include "REL/mdminidll/layer_effect_state.h"

extern OMOBJ *lbl_1_bss_87C;
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_298;
extern const f32 lbl_1_rodata_2C4;
extern void fn_1_1F508(s16 layerNo);
#pragma section code_type ".text.pool_1f9f4"
extern void fn_1_1F9F4(OMOBJ *obj);
#pragma section code_type ".text"



#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "REL/mdminidll/layer_effect_state.h"

extern ANIMDATA *lbl_1_bss_880;
extern ANIMDATA *lbl_1_bss_884;
extern void *lbl_1_bss_888;
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_298;
extern const f32 lbl_1_rodata_2C8;
#pragma section code_type ".text.pool_1f574"
extern void fn_1_1F574(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
#pragma section code_type ".text"



#define _MATH_H
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"

extern void *lbl_1_bss_888;
extern s16 lbl_1_bss_88C;
/* Existing retail readonly scalar views, not new storage. */
extern const f32 lbl_1_rodata_2CC;
extern const f32 lbl_1_rodata_2D0;
extern const f32 lbl_1_rodata_2D4;
extern const f32 lbl_1_rodata_2D8;
extern const f32 lbl_1_rodata_2DC;
extern const f32 lbl_1_rodata_2E0;





#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "game/data.h"

/* The retail owner is exactly 10 readonly bytes. Its values are copied to a
 * local 5 x s16 view and passed as Hu3DParticleCreate's s16 maxCnt. */
typedef struct MDMinidllHalfwordTemplate {
    s16 values[5];
} MDMinidllHalfwordTemplate;

extern const MDMinidllHalfwordTemplate lbl_1_rodata_398;
extern const f32 lbl_1_rodata_288; /* retail 0.0f */
extern const f32 lbl_1_rodata_2A8; /* retail 1.0f */
extern const f32 lbl_1_rodata_328; /* retail 200.0f */
extern const f32 lbl_1_rodata_360; /* retail 275.0f */

extern s32 lbl_1_data_998[9];
extern HUPROCESS *lbl_1_bss_878;
extern OMOBJ *lbl_1_bss_87C;
extern ANIMDATA *lbl_1_bss_938[9];
extern HU3D_MODELID lbl_1_bss_8E4;
extern HU3D_MODELID lbl_1_bss_8DC[4];
extern HU3D_MODELID lbl_1_bss_8DA;
extern HU3D_MODELID lbl_1_bss_8D2[4];
extern HU3D_MODELID lbl_1_bss_8CE[2];
extern HU3D_MODELID lbl_1_bss_8CA[2];
extern HU3D_MODELID lbl_1_bss_88E[6][5];

#pragma section code_type ".text.pool_1fbe8"
void fn_1_1FBE8(OMOBJ *obj);
#pragma section code_type ".text.pool_20b74"
void fn_1_20B74(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
#pragma section code_type ".text"
#pragma section code_type ".text.particle_rays_2105c"
void fn_1_2105C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
#pragma section code_type ".text"
#pragma section code_type ".text.particle_spiral_2185c"
void fn_1_2185C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
#pragma section code_type ".text"
#pragma section code_type ".text.particle_scatter_221e0"
void fn_1_221E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
#pragma section code_type ".text"
#pragma section code_type ".text.particle_byte_update"
void fn_1_22EBC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
#pragma section code_type ".text"
#pragma section code_type ".text.pool_23700"
void fn_1_23700(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
#pragma section code_type ".text.pool_24524"
void fn_1_24524(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
#pragma section code_type ".text"





#define _MATH_H
#include "game/hu3d.h"
#include "game/animdata.h"
#include "REL/mdminidll/readonly_scalars.h"

extern const f32 lbl_1_rodata_328;
extern s16 lbl_1_bss_8DC[4];
extern ANIMDATA *lbl_1_bss_938[9];
extern void fn_1_2105C(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);



#define _MATH_H
#include "game/main.h"
#include "game/hu3d.h"
#include "game/frand.h"
#include "REL/mdminidll/readonly_scalars.h"

/* Extracted from the immutable candidate source snapshot.  Keep this helper
 * before its caller so MWCC sees the same auto-inline opportunity. */


extern const f32 lbl_1_rodata_2D8;
extern const f32 lbl_1_rodata_300;
extern const f32 lbl_1_rodata_350;
extern const f32 lbl_1_rodata_358;
extern const f32 lbl_1_rodata_394;



f32 fn_1_1F3AC(f32 farg0, f32 farg1, f32 farg2, f32 farg3)
{
    if (farg2 <= 0.0f) {
        return farg0;
    }
    if (farg2 >= farg3) {
        return farg1;
    }
    return (f32) ((f64) farg0 +
        ((f64) (farg1 - farg0) *
         sin((3.141592653589793 *
              (f64) ((90.0f / farg3) * farg2)) /
             180.0)));
}

#pragma section code_type ".text.pool_1f494"
f32 fn_1_1F494(f32 farg0, f32 farg1, f32 farg2, f32 farg3)
{
    if (farg2 <= 0.0f) {
        return farg0;
    }
    if (farg2 >= farg3) {
        return farg1;
    }
    return farg0 + ((farg2 / farg3) * (farg1 - farg0));
}

#pragma section code_type ".text.pool_1f4d8"
f32 fn_1_1F4D8(f32 farg0, f32 farg1, f32 farg2)
{
    if (farg0 == farg1) {
        return farg1;
    }
    return (farg1 + (farg0 * (farg2 - 1.0f))) / farg2;
}

#pragma section code_type ".text.pool_1f574"
void fn_1_1F574(HU3D_DRAW_OBJ *arg0, HSF_MATERIAL *arg1)
{
    Mtx spEC;
    Mtx spBC;
    Mtx sp8C;
    Mtx sp5C;
    Mtx sp2C;
    GXTexObj spC;
    MDMinIndirectTextureConsumedView *var_r31;
    HU3D_CAMERA *var_r30;

    var_r31 = *(MDMinIndirectTextureConsumedView **)((u8 *)arg0->model + 288);
    var_r30 = Hu3DCamera;
    GXInitTexObj(&spC, lbl_1_bss_888, 320U, 240U, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 0U);
    GXInitTexObjLOD(&spC, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0U, 0U, GX_ANISO_1);
    GXLoadTexObj(&spC, GX_TEXMAP1);
    HuSprTexLoad(lbl_1_bss_880, 0, 2, 1, 1, 1);
    HuSprTexLoad(lbl_1_bss_884, 0, 3, 1, 1, 1);
    GXSetNumTexGens(3U);
    GXSetNumTevStages(3U);
    C_MTXLightPerspective(sp5C, var_r30->fov, 1.2f, 0.5f, (-0.5f), 0.5f, 0.5f);
    PSMTXInverse(Hu3DCameraMtx, sp2C);
    PSMTXConcat(sp2C, arg0->matrix, spEC);
    PSMTXConcat(sp5C, Hu3DCameraMtx, sp8C);
    PSMTXConcat(sp8C, spEC, sp8C);
    GXLoadTexMtxImm(sp8C, 33U, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, 33U, 0U, 125U);
    PSMTXTrans(spEC, 0.0f,
               (var_r31->translate_y += var_r31->translate_y_delta),
               0.0f);
    PSMTXScale(spBC, 0.8f, 0.8f, 1.0f);
    PSMTXConcat(spBC, spEC, sp8C);
    GXLoadTexMtxImm(sp8C, 30U, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, 30U, 0U, 125U);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, 60U, 0U, 125U);
    GXSetTevColor(GX_TEVREG0, var_r31->color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetNumIndStages(1U);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP2);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 1U, 0U, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, var_r31->matrix, -1);
}

#pragma section code_type ".text.pool_1f9f4"
void fn_1_1F9F4(OMOBJ *arg0)
{
    MDMinidllLayerEffectStateView *state =
        (MDMinidllLayerEffectStateView *) lbl_1_bss_8E8;

    state->field_00 = state->field_0C = 0.0f;
    state->field_04 = state->field_10 = 0.0f;
    state->field_08 = state->field_14 =
        (0.09f * state->field_24) / 90.0f;
    state->field_20 =
        ((-0.02f) * state->field_24) / 90.0f;
    if ((state->field_24 -= 1.0f) < 0.0f) {
        Hu3DModelAttrSet(*arg0->mdlId, 1U);
        Hu3DLayerHookReset(1);
        arg0->objFunc = NULL;
    }
}

#pragma section code_type ".text.pool_1faf4"
void fn_1_1FAF4(s16 arg0, Point3d *arg1)
{
    OMOBJ *obj = lbl_1_bss_87C;

    if (obj) {
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)[arg0].field_1C =
            0.0f;
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)[arg0].field_24 =
            90.0f;
        Hu3DModelPosSet(obj->mdlId[arg0], arg1->x, arg1->y, (-840.0f));
        Hu3DModelAttrReset(obj->mdlId[arg0], 1U);
        Hu3DLayerHookSet(1, fn_1_1F508);
        obj->objFunc = fn_1_1F9F4;
    }
}

#pragma section code_type ".text.pool_1fbe8"
void fn_1_1FBE8(OMOBJ *arg0)
{
    HU3D_MODEL *model;

    model = NULL;
    omSetStatBit(arg0, 256U);
    lbl_1_bss_880 = HuSprAnimRead(HuDataSelHeapReadNum(9830488, 268435456, HEAP_MODEL));
    lbl_1_bss_884 = HuSprAnimRead(HuDataSelHeapReadNum(9830488, 268435456, HEAP_MODEL));
    lbl_1_bss_888 = HuMemDirectMallocNum(
        HEAP_MODEL, GXGetTexBufferSize(320U, 240U, 4U, 0U, 0U), 268435456U);
    *arg0->mdlId = Hu3DModelCreate(HuDataSelHeapReadNum(9830487, 268435456, HEAP_MODEL));
    Hu3DModelPosSet(*arg0->mdlId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(*arg0->mdlId, 90.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(*arg0->mdlId, 6.0f, 6.0f, 6.0f);
    Hu3DModelLayerSet(*arg0->mdlId, 1);
    Hu3DModelAttrSet(*arg0->mdlId, 1U);
    Hu3DModelMatHookSet(*arg0->mdlId, fn_1_1F574);
    memset(lbl_1_bss_8E8, 0, 40U);
    model = &Hu3DData[*arg0->mdlId];
    model->hookData = lbl_1_bss_8E8;
    arg0->objFunc = NULL;
}

#pragma section code_type ".text.pool_20068"
void fn_1_20068(s16 layerNo)
{
    Mtx44 sp58;
    Mtx sp28;
    GXTexObj sp8;
    s16 var_r31;
    f32 var_f31;

    C_MTXOrtho(sp58, 0.0f, 480.0f,
        0.0f, 640.0f, 0.0f, 10.0f);
    GXSetProjection(sp58, GX_ORTHOGRAPHIC);
    GXInitTexObj(&sp8, lbl_1_bss_888, 320U, 240U, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(&sp8, GX_LINEAR, GX_LINEAR, 0.0f,
        0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&sp8, GX_TEXMAP0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0U);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0U);
    GXSetNumTexGens(1U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60U, GX_FALSE, 125U);
    GXSetNumChans(1U);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0U, GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTevStages(1U);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    PSMTXIdentity(sp28);
    GXLoadPosMtxImm(sp28, 0U);
    var_f31 = fn_1_1F494(0.0f, 1.0f, (f32) lbl_1_bss_88C, 90.0f);
    lbl_1_bss_88C += 1;
    if (lbl_1_bss_88C > 90) {
        lbl_1_bss_88C = 90;
    }
    GXBegin(GX_QUADS, GX_VTXFMT0, 16U);
    var_r31 = 0;
    while (var_r31 < 4) {
        GXWGFifo.f32 = (f32) (var_r31 * -1);
        GXWGFifo.f32 = (f32) (var_r31 * -1);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (128.0f * var_f31);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = (f32) (var_r31 + 640);
        GXWGFifo.f32 = (f32) (var_r31 * -1);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (128.0f * var_f31);
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = (f32) (var_r31 + 640);
        GXWGFifo.f32 = (f32) (var_r31 + 480);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (128.0f * var_f31);
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = (f32) (var_r31 * -1);
        GXWGFifo.f32 = (f32) (var_r31 + 480);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (255.0f - (8.0f * var_f31));
        GXWGFifo.u8 = (u8) (s32) (128.0f * var_f31);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = 1.0f;
        var_r31 += 1;
    }
    GXSetTexCopySrc(0U, 0U, 640U, 480U);
    GXSetTexCopyDst(320U, 240U, GX_TF_RGB565, GX_TRUE);
    GXCopyTex(lbl_1_bss_888, GX_FALSE);
    Hu3DZClear();
}

#pragma section code_type ".text.pool_20b74"
#pragma section const_type ".rodata.particle_zero_2f8"
void fn_1_20B74(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    s16 var_r29;
    HU3D_PARTICLE_DATA *data;

    if (particle->dataCnt <= 0) {
        model->attr |= 1;
        return;
    }
    var_r29 = 0;
    data = particle->data;
    while (var_r29 < particle->maxCnt) {
        if (data->time == 1) {
            data->scale = fn_1_1F3AC(0.0f, data->accel.x, data->vel.x, data->vel.y);
            data->color.a = fn_1_1F3AC(data->accel.y, 0.0f, data->vel.x, data->vel.y);
            if ((data->vel.x += lbl_1_rodata_2A8) > data->vel.y) {
                data->time = 0;
                data->scale = lbl_1_rodata_288;
                particle->dataCnt -= 1;
            }
        }
        var_r29 += 1;
        data += 1;
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

#pragma section code_type ".text.pool_215a4"
#pragma section const_type ".rodata.particle_origin_328"
void fn_1_215A4(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_8DC[var_r31] = Hu3DParticleCreate(lbl_1_bss_938[1], 16);
        Hu3DModelPosSet(lbl_1_bss_8DC[var_r31], 200.0f * (f32) var_r31,
                        200.0f, lbl_1_rodata_288);
        Hu3DModelScaleSet(lbl_1_bss_8DC[var_r31], lbl_1_rodata_2A8,
                          lbl_1_rodata_2A8, lbl_1_rodata_2A8);
        Hu3DModelLayerSet(lbl_1_bss_8DC[var_r31], 7);
        Hu3DModelAttrSet(lbl_1_bss_8DC[var_r31], 1U);
        Hu3DParticleScaleSet(lbl_1_bss_8DC[var_r31], lbl_1_rodata_2A8);
        Hu3DParticleHookSet(lbl_1_bss_8DC[var_r31], fn_1_2105C);
        Hu3DParticleBlendModeSet(lbl_1_bss_8DC[var_r31], 1U);
        var_r31 += 1;
    }
}

#pragma section code_type ".text.pool_23700"
#pragma section const_type ".rodata.particle_inverse"
#define _MATH_H
#include "game/main.h"
#include "game/hu3d.h"
#include "game/frand.h"
#include "REL/mdminidll/readonly_scalars.h"

/* Extracted from the immutable candidate source snapshot.  Keep this helper
 * before its caller so MWCC sees the same auto-inline opportunity. */


extern const f32 lbl_1_rodata_2D8;
extern const f32 lbl_1_rodata_300;
extern const f32 lbl_1_rodata_350;
extern const f32 lbl_1_rodata_358;
extern const f32 lbl_1_rodata_394;

extern const f32 lbl_1_rodata_364;





void fn_1_23700(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    s16 var_r29;
    s16 var_r28;
    f32 var_f31;
    f32 var_f30;
    HU3D_PARTICLE_DATA *var_r31;

    var_r28 = 0;
    if (particle->count == 0) {
        var_r29 = 0;
        var_r31 = particle->data;
        while (var_r29 < particle->maxCnt) {
            var_r31->time = 0;
            var_r29 += 1;
            var_r31 += 1;
        }
        particle->dataCnt = 1;
        particle->pos.x = lbl_1_rodata_2D8;
        particle->pos.y = lbl_1_rodata_2D8;
        particle->pos.z = lbl_1_rodata_2D8;
    }
    var_r29 = 0;
    var_r31 = particle->data;
    while (var_r29 < particle->maxCnt) {
        if ((var_r31->time == 0) && (particle->dataCnt == 1) && (var_r28 <= 3)) {
            var_r28 += 1;
            var_r31->time = 1;
            var_r31->vel.x = lbl_1_rodata_288;
            var_r31->vel.y = (f32) (frandmod(30) + 60);
            var_r31->accel.x = (f32) (frandmod(100) - 50);
            var_r31->accel.y = (f32) (-frandmod(100) - 50);
            var_r31->accel.z = (f32) (frandmod(100) - 50);
            PSVECNormalize(&var_r31->accel, &var_r31->accel);
            var_r31->accel.x *= lbl_1_rodata_358;
            var_r31->accel.y *= lbl_1_rodata_364;
            var_r31->accel.z *= lbl_1_rodata_358;
            var_r31->pos.x = (particle->unk_10.x + (f32) frandmod(100)) - lbl_1_rodata_350;
            var_r31->pos.y = (particle->unk_10.y + (f32) frandmod(100)) - lbl_1_rodata_350;
            var_r31->pos.z = (particle->unk_10.z + (f32) frandmod(100)) - lbl_1_rodata_350;
            var_f30 = (f32) frandmod(128);
            var_f31 = particle->pos.x + var_f30;
            if (var_f31 > lbl_1_rodata_2D8) {
                var_f31 = lbl_1_rodata_2D8;
            }
            var_r31->color.r = var_f31;
            var_f31 = particle->pos.y + var_f30;
            if (var_f31 > lbl_1_rodata_2D8) {
                var_f31 = lbl_1_rodata_2D8;
            }
            var_r31->color.g = var_f31;
            var_f31 = particle->pos.z + var_f30;
            if (var_f31 > lbl_1_rodata_2D8) {
                var_f31 = lbl_1_rodata_2D8;
            }
            var_r31->color.b = var_f31;
            var_r31->color.a = 0;
        } else if (var_r31->time == 1) {
            var_r31->pos.x += var_r31->accel.x;
            var_r31->pos.y += var_r31->accel.y;
            var_r31->pos.z += var_r31->accel.z;
            if ((s32) (rand8() % 5) == 0) {
                var_r31->zRot = lbl_1_rodata_300 * (f32) frandmod(360);
                var_r31->color.a = frandmod(127) + 128;
            }
            var_f30 = fn_1_1F494(1.0f, 0.0f, var_r31->vel.x, var_r31->vel.y);
            var_r31->scale = var_f30 * (f32) frandmod(90);
            if ((var_r31->vel.x += lbl_1_rodata_2A8) > var_r31->vel.y) {
                var_r31->time = 0;
                var_r31->scale = lbl_1_rodata_288;
                var_r31->color.a = 0;
            }
        }
        var_r29 += 1;
        var_r31 += 1;
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * 72);
}

#pragma section code_type ".text.pool_24524"
#pragma section const_type ".rodata.particle_inverse"
void fn_1_24524(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    s16 var_r29;
    s16 var_r28;
    f32 var_f31;
    f32 var_f30;
    HU3D_PARTICLE_DATA *var_r31;

    var_r28 = 0;
    if (particle->count == 0) {
        var_r29 = 0;
        var_r31 = particle->data;
        while (var_r29 < particle->maxCnt) {
            var_r31->time = 0;
            var_r29 += 1;
            var_r31 += 1;
        }
        particle->dataCnt = 1;
        particle->pos.x = lbl_1_rodata_2D8;
        particle->pos.y = lbl_1_rodata_2D8;
        particle->pos.z = lbl_1_rodata_2D8;
    }
    var_r29 = 0;
    var_r31 = particle->data;
    while (var_r29 < particle->maxCnt) {
        if ((var_r31->time == 0) && (particle->dataCnt == 1) && (var_r28 < 1)) {
            var_r28 += 1;
            var_r31->time = 1;
            var_r31->vel.x = lbl_1_rodata_288;
            var_r31->vel.y = (f32) (frandmod(30) + 30);
            var_r31->accel.x = (f32) (frandmod(100) - 50);
            var_r31->accel.y = (f32) (-frandmod(100) - 50);
            var_r31->accel.z = (f32) (frandmod(100) - 50);
            PSVECNormalize(&var_r31->accel, &var_r31->accel);
            var_r31->accel.x *= lbl_1_rodata_358;
            var_r31->accel.z *= lbl_1_rodata_358;
            var_r31->colorIdx = (f32) (frandmod(10) + 5);
            var_r31->pos.x = (particle->unk_10.x + (f32) frandmod(100)) - lbl_1_rodata_350;
            var_r31->pos.y = particle->unk_10.y;
            var_r31->pos.z = (particle->unk_10.z + (f32) frandmod(100)) - lbl_1_rodata_350;
            var_f30 = (f32) frandmod(32);
            var_f31 = particle->pos.x + var_f30;
            if (var_f31 > lbl_1_rodata_2D8) {
                var_f31 = lbl_1_rodata_2D8;
            }
            var_r31->color.r = (u8) var_f31;
            var_f31 = particle->pos.y + var_f30;
            if (var_f31 > lbl_1_rodata_2D8) {
                var_f31 = lbl_1_rodata_2D8;
            }
            var_r31->color.g = (u8) var_f31;
            var_f31 = particle->pos.z + var_f30;
            if (var_f31 > lbl_1_rodata_2D8) {
                var_f31 = lbl_1_rodata_2D8;
            }
            var_r31->color.b = (u8) var_f31;
            var_r31->color.a = 0;
        } else if (var_r31->time == 1) {
            var_r31->pos.y += var_r31->colorIdx;
            var_r31->colorIdx += var_r31->accel.y;
            if ((s32) (rand8() % 5) == 0) {
                var_r31->zRot = lbl_1_rodata_300 * (f32) frandmod(360);
                var_r31->color.a = frandmod(127) + 128;
            }
            var_f30 = fn_1_1F494(1.0f, 0.0f, var_r31->vel.x, var_r31->vel.y);
            var_r31->scale = lbl_1_rodata_394 * var_f30;
            if ((var_r31->vel.x += lbl_1_rodata_2A8) > var_r31->vel.y) {
                var_r31->time = 0;
                var_r31->scale = lbl_1_rodata_288;
                var_r31->color.a = 0;
            }
        }
        var_r29 += 1;
        var_r31 += 1;
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * 72);
}

#pragma section code_type ".text.pool_24d80"
#pragma section const_type ".rodata"
#pragma section const_type ".rodata.particle_scatter_221e0"
void fn_1_24D80(HUPROCESS *process)
{
    MDMinidllHalfwordTemplate localTemplate;
    s16 var_r31;
    s16 var_r30;
    s16 var_r29;
    s16 var_r28;
    s16 var_r27;
    s16 var_r26;
    s16 var_r25;

    lbl_1_bss_878 = process;
    lbl_1_bss_87C = omAddObjEx(lbl_1_bss_878, 4096, 1U, 0U, -1, fn_1_1FBE8);
    var_r25 = 0;
    while (var_r25 < 9) {
        lbl_1_bss_938[var_r25] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_998[var_r25], 268435456, HEAP_MODEL));
        var_r25 += 1;
    }
    lbl_1_bss_8E4 = Hu3DParticleCreate(lbl_1_bss_938[0], 8);
    Hu3DModelPosSet(lbl_1_bss_8E4, lbl_1_rodata_288, lbl_1_rodata_288, lbl_1_rodata_288);
    Hu3DModelScaleSet(lbl_1_bss_8E4, lbl_1_rodata_2A8, lbl_1_rodata_2A8, lbl_1_rodata_2A8);
    Hu3DModelLayerSet(lbl_1_bss_8E4, 7);
    Hu3DModelAttrSet(lbl_1_bss_8E4, 1U);
    Hu3DParticleScaleSet(lbl_1_bss_8E4, lbl_1_rodata_2A8);
    Hu3DParticleHookSet(lbl_1_bss_8E4, fn_1_20B74);
    Hu3DParticleBlendModeSet(lbl_1_bss_8E4, 1U);
    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_8DC[var_r31] = Hu3DParticleCreate(lbl_1_bss_938[1], 16);
        Hu3DModelPosSet(lbl_1_bss_8DC[var_r31], lbl_1_rodata_328 * (f32) var_r31,
            lbl_1_rodata_328, lbl_1_rodata_288);
        Hu3DModelScaleSet(lbl_1_bss_8DC[var_r31], lbl_1_rodata_2A8,
            lbl_1_rodata_2A8, lbl_1_rodata_2A8);
        Hu3DModelLayerSet(lbl_1_bss_8DC[var_r31], 7);
        Hu3DModelAttrSet(lbl_1_bss_8DC[var_r31], 1U);
        Hu3DParticleScaleSet(lbl_1_bss_8DC[var_r31], lbl_1_rodata_2A8);
        Hu3DParticleHookSet(lbl_1_bss_8DC[var_r31], fn_1_2105C);
        Hu3DParticleBlendModeSet(lbl_1_bss_8DC[var_r31], 1U);
        var_r31 += 1;
    }
    lbl_1_bss_8DA = Hu3DParticleCreate(lbl_1_bss_938[2], 1000);
    Hu3DModelPosSet(lbl_1_bss_8DA, lbl_1_rodata_288, 275.0f, lbl_1_rodata_288);
    Hu3DModelScaleSet(lbl_1_bss_8DA, lbl_1_rodata_2A8, lbl_1_rodata_2A8, lbl_1_rodata_2A8);
    Hu3DModelLayerSet(lbl_1_bss_8DA, 7);
    Hu3DModelAttrSet(lbl_1_bss_8DA, 1U);
    Hu3DParticleScaleSet(lbl_1_bss_8DA, lbl_1_rodata_2A8);
    Hu3DParticleHookSet(lbl_1_bss_8DA, fn_1_2185C);
    var_r29 = 0;
    while (var_r29 < 4) {
        lbl_1_bss_8D2[var_r29] = Hu3DParticleCreate(lbl_1_bss_938[2], 64);
        Hu3DModelPosSet(lbl_1_bss_8D2[var_r29], lbl_1_rodata_288,
            lbl_1_rodata_288, lbl_1_rodata_288);
        Hu3DModelScaleSet(lbl_1_bss_8D2[var_r29], lbl_1_rodata_2A8,
            lbl_1_rodata_2A8, lbl_1_rodata_2A8);
        Hu3DModelLayerSet(lbl_1_bss_8D2[var_r29], 7);
        Hu3DModelAttrSet(lbl_1_bss_8D2[var_r29], 1U);
        Hu3DParticleScaleSet(lbl_1_bss_8D2[var_r29], lbl_1_rodata_2A8);
        Hu3DParticleHookSet(lbl_1_bss_8D2[var_r29], fn_1_221E0);
        Hu3DParticleBlendModeSet(lbl_1_bss_8D2[var_r29], 1U);
        var_r29 += 1;
    }
    var_r28 = 0;
    while (var_r28 < 2) {
        lbl_1_bss_8CE[var_r28] = Hu3DParticleCreate(lbl_1_bss_938[0], 10);
        Hu3DModelPosSet(lbl_1_bss_8CE[var_r28], lbl_1_rodata_288,
            lbl_1_rodata_288, lbl_1_rodata_288);
        Hu3DModelScaleSet(lbl_1_bss_8CE[var_r28], lbl_1_rodata_2A8,
            lbl_1_rodata_2A8, lbl_1_rodata_2A8);
        Hu3DModelAttrSet(lbl_1_bss_8CE[var_r28], 1U);
        Hu3DModelLayerSet(lbl_1_bss_8CE[var_r28], 2);
        Hu3DParticleHookSet(lbl_1_bss_8CE[var_r28], fn_1_22EBC);
        Hu3DParticleBlendModeSet(lbl_1_bss_8CE[var_r28], 1U);
        var_r28 += 1;
    }
    var_r27 = 0;
    while (var_r27 < 2) {
        lbl_1_bss_8CA[var_r27] = Hu3DParticleCreate(lbl_1_bss_938[3], 256);
        Hu3DModelPosSet(lbl_1_bss_8CA[var_r27], lbl_1_rodata_288,
            lbl_1_rodata_288, lbl_1_rodata_288);
        Hu3DModelScaleSet(lbl_1_bss_8CA[var_r27], lbl_1_rodata_2A8,
            lbl_1_rodata_2A8, lbl_1_rodata_2A8);
        Hu3DModelAttrSet(lbl_1_bss_8CA[var_r27], 1U);
        Hu3DModelLayerSet(lbl_1_bss_8CA[var_r27], 2);
        Hu3DParticleHookSet(lbl_1_bss_8CA[var_r27], fn_1_23700);
        Hu3DParticleBlendModeSet(lbl_1_bss_8CA[var_r27], 1U);
        var_r27 += 1;
    }
    localTemplate = lbl_1_rodata_398;
    for (var_r26 = 0; var_r26 < 6; var_r26 += 1) {
        for (var_r30 = 0; var_r30 < 5; var_r30 += 1) {
            lbl_1_bss_88E[var_r26][var_r30] = Hu3DParticleCreate(
                lbl_1_bss_938[var_r30 + 4], localTemplate.values[var_r30]);
            Hu3DModelPosSet(lbl_1_bss_88E[var_r26][var_r30],
                lbl_1_rodata_288, lbl_1_rodata_288, lbl_1_rodata_288);
            Hu3DModelScaleSet(lbl_1_bss_88E[var_r26][var_r30],
                lbl_1_rodata_2A8, lbl_1_rodata_2A8, lbl_1_rodata_2A8);
            Hu3DModelAttrSet(lbl_1_bss_88E[var_r26][var_r30], 1U);
            Hu3DModelLayerSet(lbl_1_bss_88E[var_r26][var_r30], 2);
            Hu3DParticleHookSet(lbl_1_bss_88E[var_r26][var_r30], fn_1_24524);
            Hu3DParticleBlendModeSet(lbl_1_bss_88E[var_r26][var_r30], 1U);
        }
    }
}

#pragma section code_type ".text.particle_color"
#pragma section const_type ".rodata"
#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"

/* Only these two fields of the 0x28-byte record are consumed here; the
 * intervening bytes remain an unknown target-backed view. */
typedef struct MDMinidllParticleMotionView {
    u8 unknown_00_1B[28];
    f32 unk1C;
    u8 unknown_20_23[4];
    f32 unk24;
} MDMinidllParticleMotionView;

extern OMOBJ *lbl_1_bss_87C;
extern s16 lbl_1_bss_8E4;
extern const GXColor lbl_1_rodata_3A2;
extern const f32 lbl_1_rodata_3A8;
extern const f32 lbl_1_rodata_3AC;

void fn_1_1F508(s16 layerNo);
void fn_1_1F9F4(OMOBJ *obj);

void fn_1_259FC(Point3d *arg0)
{
    GXColor savedColor;
    GXColor sourceColor;
    GXColor *colorBuffer;
    HU3D_PARTICLE_DATA *var_r31;
    OMOBJ *var_r29;
    HU3D_PARTICLE *var_r28;
    HU3D_MODEL *var_r27;

    var_r29 = lbl_1_bss_87C;
    if (var_r29) {
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)->field_1C = lbl_1_rodata_288;
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)->field_24 = lbl_1_rodata_298;
        Hu3DModelPosSet(*var_r29->mdlId, arg0->x, arg0->y, lbl_1_rodata_2C4);
        Hu3DModelAttrReset(*var_r29->mdlId, 1U);
        Hu3DLayerHookSet(1, fn_1_1F508);
        var_r29->objFunc = fn_1_1F9F4;
    }
    sourceColor = lbl_1_rodata_3A2;
    colorBuffer = &sourceColor;
    savedColor = *colorBuffer;
    var_r27 = &Hu3DData[lbl_1_bss_8E4];
    var_r28 = var_r27->hookData;
    var_r31 = &var_r28->data[5];
    var_r31->time = 1;
    var_r31->pos.x = arg0->x;
    var_r31->pos.y = arg0->y;
    var_r31->pos.z = arg0->z;
    var_r31->scale = lbl_1_rodata_288;
    var_r31->color = savedColor;
    var_r31->vel.x = lbl_1_rodata_288;
    var_r31->vel.y = lbl_1_rodata_3A8;
    var_r31->accel.x = lbl_1_rodata_3AC;
    var_r31->accel.y = (f32) savedColor.a;
    var_r28->dataCnt += 1;
    var_r27->attr &= 4294967294;
}


#pragma section code_type ".text.particle_activate"
#define _MATH_H
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"

extern s16 lbl_1_bss_8E4;

void fn_1_20A74(s16 arg0, Point3d *arg1, GXColor *arg2, f32 farg0, f32 farg1)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_8E4];
    particle = model->hookData;
    data = &particle->data[arg0];
    data->time = 1;
    data->pos.x = arg1->x;
    data->pos.y = arg1->y;
    data->pos.z = arg1->z;
    data->scale = lbl_1_rodata_288;
    data->color.r = arg2->r;
    data->color.g = arg2->g;
    data->color.b = arg2->b;
    data->color.a = arg2->a;
    data->vel.x = lbl_1_rodata_288;
    data->vel.y = farg0;
    data->accel.x = farg1;
    data->accel.y = (f32) arg2->a;
    particle->dataCnt++;
    model->attr &= 4294967294U;
}

#pragma section code_type ".text.particle_byte_update"
#define _MATH_H
#include "game/hu3d.h"

/* The retail owner is 12 bytes; this function copies only the consumed
 * ten-byte pair table. The trailing bytes remain deliberately unknown. */
typedef struct MDMinidllParticleBytePair {
    u8 bytes[2];
} MDMinidllParticleBytePair;

typedef struct MDMinidllParticleByteTemplate {
    MDMinidllParticleBytePair pairs[5];
} MDMinidllParticleByteTemplate;

typedef struct MDMinidllParticleByteOwner {
    MDMinidllParticleByteTemplate consumed;
    u8 unknown_10[2];
} MDMinidllParticleByteOwner;

extern const MDMinidllParticleByteOwner lbl_1_rodata_378;
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_2D4;
extern const f32 lbl_1_rodata_384;

void fn_1_22EBC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    MDMinidllParticleByteTemplate sp8;
    s16 var_r29;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;
    f32 var_f25;
    f32 var_f24;
    f32 var_f23;
    f32 var_f22;
    f32 var_f21;
    f32 var_f20;
    HU3D_PARTICLE_DATA *var_r31;

    sp8 = lbl_1_rodata_378.consumed;
    if (particle->count == 0) {
        var_r29 = 0;
        var_r31 = particle->data;
        while (var_r29 < particle->maxCnt) {
            var_r31->time = 0;
            var_r29 += 1;
            var_r31 += 1;
        }
    }
    var_r29 = 0;
    var_r31 = particle->data;
    while (var_r29 < particle->maxCnt) {
        if (var_r31->time == 1) {
            var_r31->pos.x = lbl_1_rodata_288;
            var_r31->pos.y = lbl_1_rodata_288;
            var_r31->pos.z = lbl_1_rodata_288;
            var_r31->vel.x = (f32) sp8.pairs[var_r29 % 5].bytes[1];
            var_r31->vel.y = (f32) sp8.pairs[var_r29 % 5].bytes[0];
            var_f31 = var_r31->vel.x;
            var_f26 = (f32) var_r31->color.a;
            if (var_f26 == var_f31) {
                var_f27 = var_f31;
            } else {
                var_f27 = (var_f31 + (var_f26 * lbl_1_rodata_384)) / lbl_1_rodata_2D4;
            }
            var_r31->color.a = (u8) var_f27;
            var_f30 = var_r31->vel.y;
            var_f24 = var_r31->scale;
            if (var_f24 == var_f30) {
                var_f25 = var_f30;
            } else {
                var_f25 = (var_f30 + (var_f24 * lbl_1_rodata_384)) / lbl_1_rodata_2D4;
            }
            var_r31->scale = var_f25;
        } else if (var_r31->time == 2) {
            var_r31->pos.x = lbl_1_rodata_288;
            var_r31->pos.y = lbl_1_rodata_288;
            var_r31->pos.z = lbl_1_rodata_288;
            var_r31->vel.x = lbl_1_rodata_288;
            var_r31->vel.y = lbl_1_rodata_288;
            var_f29 = var_r31->vel.x;
            var_f22 = (f32) var_r31->color.a;
            if (var_f22 == var_f29) {
                var_f23 = var_f29;
            } else {
                var_f23 = (var_f29 + (var_f22 * lbl_1_rodata_384)) / lbl_1_rodata_2D4;
            }
            var_r31->color.a = (u8) var_f23;
            var_f28 = var_r31->vel.y;
            var_f20 = var_r31->scale;
            if (var_f20 == var_f28) {
                var_f21 = var_f28;
            } else {
                var_f21 = (var_f28 + (var_f20 * lbl_1_rodata_384)) / lbl_1_rodata_2D4;
            }
            var_r31->scale = var_f21;
        } else {
            var_r31->color.a = 0;
            var_r31->scale = lbl_1_rodata_288;
        }
        var_r29 += 1;
        var_r31 += 1;
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * 72);
}

#pragma section code_type ".text.hook_update_pair"
#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_8CA[2];

/* Consumed hookData view: offsets 0, 4..12, and 16..24 are observed.
 * Natural ABI alignment accounts for the two bytes after enabled. */
typedef struct MDMinModelHookDataView {
    s16 enabled;
    f32 channels[3];
    Point3d position;
} MDMinModelHookDataView;

void fn_1_23574(s16 modelIndex, Point3d *position, u8 *channels)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_8CA[modelIndex]];
    MDMinModelHookDataView *hookData = model->hookData;

    hookData->enabled = 1;
    if (channels != NULL) {
        hookData->channels[0] = (f32) channels[0];
        hookData->channels[1] = (f32) channels[1];
        hookData->channels[2] = (f32) channels[2];
    }
    if (position != NULL) {
        hookData->position.x = position->x;
        hookData->position.y = position->y;
        hookData->position.z = position->z;
    }
    Hu3DModelAttrReset(lbl_1_bss_8CA[modelIndex], 1U);
}

#pragma section code_type ".text.hook_update_grid"
#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_88E[6][5];
extern const f64 lbl_1_rodata_2F0;

/* Consumed hookData layout established by the target stores: opaque header
 * bytes are retained; no meaning is assigned to the float slots. */
typedef struct MDMinigHookDataView {
    s16 unknown_00;
    u8 unknown_02[2];
    f32 unknown_04;
    f32 unknown_08;
    f32 unknown_0C;
    f32 unknown_10;
    f32 unknown_14;
    f32 unknown_18;
} MDMinigHookDataView;

void fn_1_2433C(s16 arg0, Point3d *arg1, u8 *arg2)
{
    s16 var_r28;
    HU3D_MODEL *var_r27;
    MDMinigHookDataView *var_r31;

    var_r28 = 0;
    while (var_r28 < 5) {
        var_r27 = &Hu3DData[lbl_1_bss_88E[arg0][var_r28]];
        var_r31 = var_r27->hookData;
        var_r31->unknown_00 = 1;
        if (arg2 != NULL) {
            var_r31->unknown_04 = (f32) arg2[0];
            var_r31->unknown_08 = (f32) arg2[1];
            var_r31->unknown_0C = (f32) arg2[2];
        }
        if (arg1 != NULL) {
            var_r31->unknown_10 = arg1->x;
            var_r31->unknown_14 = arg1->y;
            var_r31->unknown_18 = arg1->z;
        }
        Hu3DModelAttrReset(lbl_1_bss_88E[arg0][var_r28], 1U);
        var_r28 += 1;
    }
}

/* Keep the recovered providers visible to the pair-control caller. The
 * compiler inlines their operations with the providers' own local lifetimes. */
#pragma section code_type ".text.particle_controls"
extern s16 lbl_1_bss_8CE[];

void fn_1_22D30(s16 slot, HuVecF *pos, GXColor *color)
{
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle;
    s16 i;
    HU3D_MODEL *model;

    model = &Hu3DData[lbl_1_bss_8CE[slot]];
    particle = model->hookData;
    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = 1;
        if (color != NULL) {
            data->color.r = color->r;
            data->color.g = color->g;
            data->color.b = color->b;
        }
        i += 1;
        data++;
    }
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_8CE[slot], pos);
    }
    Hu3DModelAttrReset(lbl_1_bss_8CE[slot], 1U);
}

void fn_1_22E34(s16 slot)
{
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    s16 i;
    HU3D_MODEL *model;

    model = &Hu3DData[lbl_1_bss_8CE[slot]];
    particle = model->hookData;
    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = 2;
        i += 1;
        data++;
    }
}

#pragma section code_type ".text.particle_stop"
void fn_1_236AC(s16 index)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    model = &Hu3DData[lbl_1_bss_8CA[index]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* Eight retail bytes consumed as two four-byte color rows. The original
 * declaration is unknown; this view defines no replacement storage. */
typedef struct MDMinidllColorTemplate {
    u8 rows[2][4];
} MDMinidllColorTemplate;
extern const MDMinidllColorTemplate lbl_1_rodata_38C;

#pragma section code_type ".text.particle_pair_controls"
void fn_1_23EC4(s16 index, HuVecF *position, s16 kind)
{
    MDMinidllColorTemplate localColors;
    localColors = lbl_1_rodata_38C;
    if (kind == 1) {
        fn_1_22D30(index, position, (GXColor *)localColors.rows[index]);
        fn_1_23574(index, position, localColors.rows[index]);
        return;
    } else if (kind == 0) {
        fn_1_22E34(index);
        fn_1_236AC(index);
        return;
    } else if (kind == 2) {
        Hu3DModelAttrSet(lbl_1_bss_8CE[index], 1U);
        Hu3DModelAttrSet(lbl_1_bss_8CA[index], 1U);
    }
}

#pragma section code_type ".text.particle_place"
#define _MATH_H
#include "game/main.h"
#include "game/hu3d.h"

extern s16 lbl_1_bss_8DC[4];

/* Reconstructed mixed formal order preserves the observed r3/r4/r5 and f1
 * contracts, including the scalar-before-position stack homes. */
void fn_1_20F74(s16 arg0, f32 farg0, Point3d *arg1, GXColor *arg2)
{
    HU3D_MODEL *var_r29;
    HU3D_PARTICLE *var_r28;
    HU3D_PARTICLE_DATA *var_r31;

    var_r29 = &Hu3DData[lbl_1_bss_8DC[arg0]];
    var_r28 = var_r29->hookData;
    Hu3DModelPosSetV(lbl_1_bss_8DC[arg0], arg1);
    var_r31 = var_r28->data;
    var_r31->time = 1;
    var_r31->color.r = arg2->r;
    var_r31->color.g = arg2->g;
    var_r31->color.b = arg2->b;
    var_r31->color.a = arg2->a;
    var_r31->time = 0;
    var_r31->parManId = (s16) farg0;
    var_r28->dataCnt = 1;
    var_r29->attr &= ~1U;
}

#pragma section code_type ".text.particle_palette_activation"
#pragma section const_type ".rodata.particle_palette_activation"
extern const f32 lbl_1_rodata_310;
void fn_1_25BF4(s16 arg0, Point3d *arg1, s16 arg2)
{
    Point3d sp10;
    GXColor palette[5] = {
        { 254, 77, 75, 255 },
        { 50, 127, 200, 255 },
        { 199, 175, 0, 255 },
        { 52, 192, 63, 255 },
        { 159, 93, 200, 255 },
    };
    GXColor color1;
    GXColor color2;

    sp10.x = arg1->x;
    sp10.y = arg1->y;
    sp10.z = 10.0f + arg1->z;
    color1 = palette[arg2];
    fn_1_20A74(arg0, &sp10, &color1, 10.0f, lbl_1_rodata_310);
    color2 = palette[arg2];
    fn_1_20F74(arg0, 10.0f, &sp10, &color2);
}

#pragma section code_type ".text.particle_position"
#pragma section const_type ".rodata"
#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_8D2[];

/* This parameter order reproduces the observed stack homes while preserving
 * the fixed-prototype EABI inputs: index r3, lifetime f1, position r4. */
void fn_1_22114(s16 index, f32 lifetime, Point3d *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *particleData;

    model = &Hu3DData[lbl_1_bss_8D2[index]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_8D2[index], position);
    particleData = particle->data;
    particleData->time = 0;
    particleData->parManId = (s16) lifetime;
    particle->dataCnt = 1;
    model->attr &= 4294967294U;
}

#pragma section code_type ".text.particle_palette_burst"
#pragma section const_type ".rodata.particle_palette_burst"
void fn_1_25EF0(s16 arg0, Point3d *arg1, s16 arg2, s16 arg3)
{
    GXColor palette[5] = {
        { 254, 77, 75, 255 },
        { 50, 127, 200, 255 },
        { 199, 175, 0, 255 },
        { 52, 192, 63, 255 },
        { 159, 93, 200, 255 },
    };
    Point3d position;

    position.x = arg1->x;
    position.y = arg1->y;
    position.z = arg1->z;

    Hu3DZClearLayerSet(7);
    if (arg3 == 0) {
        {
            GXColor color = palette[arg2];
            fn_1_20A74(arg0, &position, &color, 20.0f, 300.0f);
        }
        {
            fn_1_22114(arg0, 30.0f, &position);
        }
        return;
    }
    {
        GXColor color = palette[arg2];
        fn_1_20A74(arg0, &position, &color, 60.0f, 300.0f);
    }
    {
        GXColor color = palette[arg2];
        fn_1_20F74(arg0, 30.0f, &position, &color);
    }
}

#pragma section code_type ".text.particle_row_stop"
#pragma section const_type ".rodata"
#define _MATH_H
#include "game/hu3d.h"
/* Six rows of five handles, established by the creation and teardown loops. */
extern s16 lbl_1_bss_88E[6][5];

void fn_1_244A4(s16 index)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    i = 0;
    while (i < 5) {
        model = &Hu3DData[lbl_1_bss_88E[index][i]];
        particle = model->hookData;
        particle->dataCnt = 0;
        i += 1;
    }
}

#pragma section code_type ".text.model_row_visibility"
#pragma section const_type ".rodata"
#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_88E[6][5];

void fn_1_24288(s16 row, s16 show)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        if (show) {
            Hu3DModelAttrReset(lbl_1_bss_88E[row][i], 1U);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_88E[row][i], 1U);
        }
    }
}

#pragma section code_type ".text.particle_palette_rows"
#pragma section const_type ".rodata.particle_palette_rows"
/* Readonly seven-row color template observed at retail rodata+0x3E4.
 * The fourth component is copied but not consumed by the three-channel hook. */
void fn_1_26454(s16 row, Point3d *position, s16 mode, s16 colorIndex)
{
    u8 colors[7][4] = {
        {254, 77, 75, 0},
        {50, 127, 200, 0},
        {199, 175, 0, 0},
        {52, 192, 63, 0},
        {159, 93, 200, 0},
        {255, 114, 46, 0},
        {109, 207, 246, 0},
    };

    if (mode == 1) {
        fn_1_2433C(row, position, colors[colorIndex]);
        return;
    }
    if (mode == 0) {
        fn_1_244A4(row);
        return;
    }
    if (mode == 2) {
        fn_1_24288(row, 0);
    }
}

#pragma section code_type ".text.particle_spiral_2185c"
#pragma section const_type ".rodata.particle_spiral_2185c"
void fn_1_2185C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    extern const f32 lbl_1_rodata_32C;
    extern const f32 lbl_1_rodata_330;
    extern const f32 lbl_1_rodata_2A8;
    extern const f32 lbl_1_rodata_308;
    extern const f32 lbl_1_rodata_310;
    s32 var_r29;
    s16 var_r28;
    s16 var_r27;
    f32 var_f31;
    HU3D_PARTICLE_DATA *var_r31;

    var_r27 = 0;
    var_f31 = 125.0f;
    if (particle->dataCnt == 1) {
        particle->emitCnt -= 2;
        if (particle->emitCnt < 0) {
            particle->emitCnt += 360.0f;
        }
        if (particle->count > 100U) {
            var_f31 -= (f32) (particle->count - 100);
            if (var_f31 < 1.0f) {
                var_f31 = 1.0f;
            }
        }
        var_r28 = 0;
        var_r31 = particle->data;
        while (var_r28 < particle->maxCnt) {
            if (var_r31->time == 0) {
                if (var_r27 < 7) {
                    var_r29 = (s16) (rand8() + 128);
                    var_r29 = (s16) (u8) var_r29;
                    var_r31->color.r = 255;
                    var_r29 = (s16) (rand8() + 128);
                    var_r29 = (s16) (u8) var_r29;
                    var_r31->color.g = 255;
                    var_r29 = (s16) (rand8() % 204);
                    var_r29 = (s16) (u8) var_r29;
                    var_r31->color.b = (u8) var_r29;
                    if ((s32) (var_r28 % 4) == 0) {
                        var_r31->color.b = 255;
                    }
                    var_r31->color.a = 255;
                    var_r31->scale = 80.0f;
                    var_r31->vel.x = (45.0f * (f32) (var_r28 % 8)) + (f32) particle->emitCnt;
                    var_r31->vel.z = 750.0f - (f32) (particle->count * 2);
                    if (var_r31->vel.z < lbl_1_rodata_2A8) {
                        var_r31->vel.z = 1.0f;
                    }
                    var_r31->accel.x = 0.0f;
                    var_r31->accel.y = (f32) (frandmod(30) + 90);
                    var_r31->pos.x = (f32) ((f64) var_r31->vel.z * -sin((3.141592653589793 * (f64) var_r31->vel.x) / 180.0));
                    var_r31->pos.y = (f32) ((f64) var_r31->vel.z * cos((3.141592653589793 * (f64) var_r31->vel.x) / 180.0));
                    var_r31->pos.z = 800.0f;
                    var_r31->time = 1;
                    var_r27 += 1;
                }
            } else if (var_r31->time == 1) {
                var_r31->pos.x = (f32) ((f64) var_r31->vel.z * -sin((3.141592653589793 * (f64) var_r31->vel.x) / 180.0));
                var_r31->pos.y = (f32) ((f64) var_r31->vel.z * cos((3.141592653589793 * (f64) var_r31->vel.x) / 180.0));
                var_r31->pos.z = fn_1_1F494(800.0f, -2000.0f, var_r31->accel.x, var_r31->accel.y);
                var_r31->zRot += lbl_1_rodata_308;
                var_r31->scale = fn_1_1F494(150.0f, 50.0f, var_r31->accel.x, var_r31->accel.y);
                var_r31->vel.x += 2.0f;
                var_r31->vel.z = fn_1_1F494(lbl_1_rodata_310, var_f31, var_r31->accel.x, var_r31->accel.y);
                if (var_r31->accel.x > (var_r31->accel.y - 10.0f)) {
                    var_r31->color.a = fn_1_1F494(255.0f, 0.0f, var_r31->accel.x - (var_r31->accel.y - 10.0f), 10.0f);
                }
                if ((var_r31->accel.x += lbl_1_rodata_2A8) > var_r31->accel.y) {
                    var_r31->time = 0;
                    var_r31->scale = 0.0f;
                }
            }
            var_r28 += 1;
            var_r31 += 1;
        }
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * 72);
}

#pragma section code_type ".text.particle_scatter_221e0"
#pragma section const_type ".rodata.particle_scatter_221e0"
void fn_1_221E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    Point3d sp30;
    Point3d sp24;
    s16 var_r28;
    s16 var_r27;
    f32 var_f31;
    HU3D_PARTICLE_DATA *var_r30;
    HU3D_PARTICLE_DATA *var_r31;

    if (particle->dataCnt == 1) {
        var_r28 = 0;
        var_r31 = particle->data;
        while (var_r28 < particle->maxCnt) {
            sp30.x = (f32) (frandmod(100) - 50);
            sp30.y = (f32) (frandmod(100) - 50);
            sp30.z = (f32) (frandmod(100) - 50);
            PSVECNormalize(&sp30, &sp24);
            var_f31 = 200.0f;
            var_r31->vel.x = sp24.x * var_f31;
            var_r31->vel.y = sp24.y * var_f31;
            var_r31->vel.z = sp24.z * var_f31;
            var_r27 = rand8() + 128;
            var_r27 &= 255;
            var_r31->color.r = (u8) var_r27;
            var_r27 = rand8() + 128;
            var_r27 &= 255;
            var_r31->color.g = (u8) var_r27;
            var_r27 = (s16) ((s32) rand8() % 128);
            var_r27 &= 255;
            var_r31->color.b = (u8) var_r27;
            var_r31->color.a = 204;
            var_r31->pos.x = var_r31->accel.x = (f32) (frandmod(4) - 2);
            var_r31->pos.y = var_r31->accel.y = (f32) (frandmod(4) - 2);
            var_r31->pos.z = var_r31->accel.z = (f32) (frandmod(4) - 2);
            var_r31->scale = 0.0f;
            var_r28 += 1;
            var_r31 += 1;
        }
        particle->dataCnt = 2;
    } else if (particle->dataCnt == 2) {
        var_r30 = particle->data;
        var_r28 = 0;
        var_r31 = particle->data;
        while (var_r28 < particle->maxCnt) {
            var_r31->scale = fn_1_1F3AC(0.0f, 125.0f, var_r30->time, 5.0f);
            var_r28 += 1;
            var_r31 += 1;
        }
        if ((var_r30->time += lbl_1_rodata_2A8) > 15) {
            var_r30->time = 0;
            particle->dataCnt = 3;
        }
    } else if (particle->dataCnt == 3) {
        var_r30 = particle->data;
        var_r28 = 0;
        var_r31 = particle->data;
        while (var_r28 < particle->maxCnt) {
            var_r31->pos.x = fn_1_1F3AC(var_r31->accel.x, var_r31->vel.x, var_r30->time, var_r30->parManId);
            var_r31->pos.y = fn_1_1F3AC(var_r31->accel.y, var_r31->vel.y, var_r30->time, var_r30->parManId);
            var_r31->pos.z = fn_1_1F3AC(var_r31->accel.z, var_r31->vel.z, var_r30->time, var_r30->parManId);
            var_r31->scale = fn_1_1F494(50.0f, 0.0f, var_r30->time, var_r30->parManId);
            var_r28 += 1;
            var_r31 += 1;
        }
        if ((var_r30->time += lbl_1_rodata_2A8) > var_r30->parManId) {
            particle->dataCnt = 0;
            model->attr |= 1;
        }
    }
}

#pragma section code_type ".text.particle_rays_2105c"
#pragma section const_type ".rodata.particle_rays_2105c"
void fn_1_2105C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    s16 var_r28;
    HU3D_PARTICLE_DATA *var_r29;
    HU3D_PARTICLE_DATA *var_r31;

    if (particle->dataCnt <= 0) {
        model->attr |= 1;
        return;
    }
    if (particle->dataCnt == 1) {
        var_r29 = particle->data;
        var_r28 = 0;
        var_r31 = particle->data;
        while (var_r28 < particle->maxCnt) {
            var_r31->attr |= 4;
            var_r31->zRot = 0.017453292f * (f32) frandmod(360);
            var_r31->vel.z = (f32) (frandmod(25) + 50);
            var_r31->vel.x = (f32) ((f64) var_r31->vel.z * -sin((3.141592653589793 * (f64) (57.29578f * var_r31->zRot)) / 180.0));
            var_r31->vel.y = (f32) ((f64) var_r31->vel.z * cos((3.141592653589793 * (f64) (57.29578f * var_r31->zRot)) / 180.0));
            var_r31->scale = 0.1f * var_r31->vel.z;
            var_r31->scaleY = 3.0f * var_r31->vel.z;
            var_r31->color.r = var_r29->color.r;
            var_r31->color.g = var_r29->color.g;
            var_r31->color.b = var_r29->color.b;
            var_r31->color.a = var_r29->color.a;
            var_r31->pos.x = var_r31->vel.x;
            var_r31->pos.y = var_r31->vel.y;
            var_r31->pos.z = 0.0f;
            var_r28 += 1;
            var_r31 += 1;
        }
        particle->dataCnt = 2;
    } else if (particle->dataCnt == 2) {
        var_r29 = particle->data;
        var_r28 = 0;
        var_r31 = particle->data;
        while (var_r28 < particle->maxCnt) {
            if (var_r28 % 2 == 0) {
                var_r31->zRot += (f32) var_r28 / 500.0f;
            } else {
                var_r31->zRot -= (f32) var_r28 / 500.0f;
            }
            var_r31->vel.x = (f32) ((f64) var_r31->vel.z * -sin((3.141592653589793 * (f64) (57.29578f * var_r31->zRot)) / 180.0));
            var_r31->vel.y = (f32) ((f64) var_r31->vel.z * cos((3.141592653589793 * (f64) (57.29578f * var_r31->zRot)) / 180.0));
            var_r31->pos.x = var_r31->vel.x;
            var_r31->pos.y = var_r31->vel.y;
            var_r31->pos.z = 0.0f;
            var_r31->color.a = fn_1_1F3AC(255.0f, 0.0f, var_r29->time, var_r29->parManId);
            var_r28 += 1;
            var_r31 += 1;
        }
        if ((var_r29->time += 1.0f) > var_r29->parManId) {
            particle->dataCnt = 0;
        }
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * 72);
}
