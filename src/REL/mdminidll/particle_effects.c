/* Particle groups, screen distortion and colored bursts for minigame-mode menus. */
#include "dolphin/math.h"
#include "datadir_enum.h"
/* Clears the display-off bit while preserving every other model attribute. */
#define MDMIN_VISIBLE_MODEL_MASK 0xFFFFFFFEU
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"
#include "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"

#include "game/hu3d.h"

#include "game/main.h"

#include "game/main.h"
#include "game/hsfex.h"
#include "game/sprite.h"
#include "REL/mdminidll/readonly_scalars.h"

typedef struct MDMinIndirectTextureConsumedView {
    f32 warpMatrix[2][3]; /* Indirect-texture transform applied to the captured scene. */
    GXColor tint; /* Color and blend amount supplied to the warp stages. */
    f32 scrollY; /* Vertical texture-coordinate translation. */
    f32 scrollStep; /* Translation added on each material-hook call. */
} MDMinIndirectTextureConsumedView;

extern ANIMDATA *lbl_1_bss_880;
extern ANIMDATA *lbl_1_bss_884;
extern void *lbl_1_bss_888;

#include "game/object.h"
#include "game/hu3d.h"
#include "REL/mdminidll/layer_effect_state.h"

#include "game/object.h"
#include "game/hu3d.h"

#include "REL/mdminidll/layer_effect_state.h"

extern OMOBJ *lbl_1_bss_87C;
#include "game/object.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "REL/mdminidll/layer_effect_state.h"

extern ANIMDATA *lbl_1_bss_880;
extern ANIMDATA *lbl_1_bss_884;
extern void *lbl_1_bss_888;
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"

extern void *lbl_1_bss_888;
extern s16 lbl_1_bss_88C;

#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "game/data.h"

typedef struct MDMinidllHalfwordTemplate {
    s16 particleCounts[5]; /* Particle capacity of each character-transition effect type. */
} MDMinidllHalfwordTemplate;

extern const MDMinidllHalfwordTemplate lbl_1_rodata_398;

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
#include "game/hu3d.h"
#include "game/animdata.h"
#include "REL/mdminidll/readonly_scalars.h"

extern s16 lbl_1_bss_8DC[4];
extern ANIMDATA *lbl_1_bss_938[9];
#include "game/main.h"
#include "game/hu3d.h"
#include "game/frand.h"
#include "REL/mdminidll/readonly_scalars.h"

#include "game/main.h"
#include "game/hu3d.h"
#include "game/frand.h"
#include "REL/mdminidll/readonly_scalars.h"

#include "game/object.h"
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"

extern OMOBJ *lbl_1_bss_87C;
extern s16 lbl_1_bss_8E4;
extern const GXColor lbl_1_rodata_3A2;

#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"

extern s16 lbl_1_bss_8E4;
#include "game/hu3d.h"

typedef struct MDMinidllParticleBytePair {
    u8 scaleAlpha[2]; /* Target size followed by target alpha, in the range 0-255. */
} MDMinidllParticleBytePair;

typedef struct MDMinidllParticleByteTemplate {
    MDMinidllParticleBytePair scaleAlphaPairs[5]; /* Repeating preview-glow size and alpha
                                                   * targets. */
} MDMinidllParticleByteTemplate;

#include "game/hu3d.h"

extern s16 lbl_1_bss_8CA[2];

typedef struct MDMinModelHookDataView {
    s16 emitting; /* 1 permits new trail particles; 0 lets existing particles finish. */
    f32 baseColor[3]; /* Base RGB intensities, copied from the supplied 0-255 channels. */
    Point3d spawnCenter; /* Center of the particle-spawn region in model coordinates. */
} MDMinModelHookDataView;
#include "game/hu3d.h"

extern s16 lbl_1_bss_88E[6][5];

typedef struct MDMinigHookDataView {
    s16 emitting; /* 1 permits new particles; 0 lets existing particles finish. */
    u8 unusedEmitterState[2]; /* Emitter state left unchanged by row controls. */
    f32 baseRed; /* Base red intensity, copied from an 8-bit channel. */
    f32 baseGreen; /* Base green intensity, copied from an 8-bit channel. */
    f32 baseBlue; /* Base blue intensity, copied from an 8-bit channel. */
    f32 spawnX; /* X center of the particle-spawn region in model coordinates. */
    f32 spawnY; /* Y center of the particle-spawn region in model coordinates. */
    f32 spawnZ; /* Z center of the particle-spawn region in model coordinates. */
} MDMinigHookDataView;

extern s16 lbl_1_bss_8CE[];

typedef struct MDMinidllColorTemplate {
    u8 rows[2][4]; /* RGB and an unused alpha byte for each preview effect. */
} MDMinidllColorTemplate;
#include "game/main.h"
#include "game/hu3d.h"

extern s16 lbl_1_bss_8DC[4];

#include "game/hu3d.h"

extern s16 lbl_1_bss_8D2[];

#include "game/hu3d.h"

extern s16 lbl_1_bss_88E[6][5];
#include "game/hu3d.h"

extern s16 lbl_1_bss_88E[6][5];

#include "dolphin/types.h"
#include "REL/mdminidll/camera.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/player_config.h"
#include "dolphin/mtx/GeoTypes.h"
#include "game/animdata.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "humath.h"

typedef struct MDMinTrajectory {
    s16 state; /* Menu path phase selected by its animation callback. */
    HuVecF current; /* First position passed to the menu path interpolator. */
    HuVecF from; /* Second position passed to the menu path interpolator. */
    HuVecF to; /* Third position passed to the menu path interpolator. */
    f32 elapsed; /* Number of animation updates completed in the current phase. */
    f32 limit; /* Duration of the current animation phase, in updates. */
} MDMinTrajectory;

ANIMDATA *lbl_1_bss_938[9]; /* Flash, ring, spiral, trail and five character-effect textures. */
MDMinidllLayerEffectStateView lbl_1_bss_8E8[2]; /* Texture warp and countdown state for each
                                                 * slot. */
HU3D_MODELID lbl_1_bss_8E4; /* Shared eight-slot growing-flash pool. */
HU3D_MODELID lbl_1_bss_8DC[4]; /* Four rotating radial-ring models. */
HU3D_MODELID lbl_1_bss_8DA; /* Spiral emitter used during the menu-exit transition. */
HU3D_MODELID lbl_1_bss_8D2[4]; /* Four spherical character-burst models. */
HU3D_MODELID lbl_1_bss_8CE[2]; /* Layered glows following the two preview positions. */
HU3D_MODELID lbl_1_bss_8CA[2]; /* Particle trails following the two preview positions. */
HU3D_MODELID lbl_1_bss_88E[6][5]; /* Five character-effect emitters per row. */
s16 lbl_1_bss_88C; /* Captured-scene blend progress, clamped to 0-90. */
void *lbl_1_bss_888; /* 320x240 RGB565 copy of the menu scene. */
ANIMDATA *lbl_1_bss_884; /* Second scrolling-warp texture instance. */
ANIMDATA *lbl_1_bss_880; /* First scrolling-warp texture instance. */
OMOBJ *lbl_1_bss_87C; /* Screen-distortion model and its countdown callback. */
HUPROCESS *lbl_1_bss_878; /* Object-manager process receiving the distortion object. */

/* Resource IDs for the nine particle-animation slots above. */
s32 lbl_1_data_998[9] = {
    DATANUM(DATA_mdmini, 90), DATANUM(DATA_mdmini, 91),
    DATANUM(DATA_mdmini, 89), DATANUM(DATA_mdmini, 92),
    DATANUM(DATA_mdmini, 98), DATANUM(DATA_mdmini, 98),
    DATANUM(DATA_mdmini, 98), DATANUM(DATA_mdmini, 98),
    DATANUM(DATA_mdmini, 98)
};

#include "dolphin/gx.h"

extern void *lbl_1_bss_888;
#include <string.h>

#include "datadir_enum.h"

#include "game/data.h"

#include "game/hu3d.h"

#include "game/memory.h"

#include "game/object.h"

#include "game/sprite.h"

#include "REL/mdminidll/layer_effect_state.h"

extern ANIMDATA *lbl_1_bss_880;

extern ANIMDATA *lbl_1_bss_884;

extern void *lbl_1_bss_888;

#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "REL/mdminidll/layer_effect_state.h"

extern HUPROCESS *lbl_1_bss_4;
extern void *lbl_1_bss_888;
#include "game/object.h"

extern HUPROCESS *lbl_1_bss_878;
extern OMOBJ *lbl_1_bss_87C;
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "REL/mdminidll/layer_effect_state.h"

extern OMOBJ *lbl_1_bss_87C;
extern void *lbl_1_bss_888;
#include "game/object.h"
#include "game/hu3d.h"
#include "REL/mdminidll/layer_effect_state.h"

extern OMOBJ *lbl_1_bss_87C;
#include "game/hu3d.h"

#include "game/hu3d.h"

extern ANIMDATA *lbl_1_bss_938[];
extern HU3D_MODELID lbl_1_bss_8E4;
#include "game/hu3d.h"

extern s16 lbl_1_bss_8E4;
#include "game/hu3d.h"

extern s16 lbl_1_bss_8DC[4];
#include "game/hu3d.h"

extern s16 lbl_1_bss_8DA;
#include "game/hu3d.h"

extern ANIMDATA *lbl_1_bss_938[9];
extern HU3D_MODELID lbl_1_bss_8DA;
#include "game/hu3d.h"

extern s16 lbl_1_bss_8DA;
#include "game/main.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/hu3d.h"
#include "game/sprite.h"
extern ANIMDATA *lbl_1_bss_938[9];
extern s16 lbl_1_bss_8D2[4];
#include "game/hu3d.h"

extern s16 lbl_1_bss_8D2[4];
#include "game/hu3d.h"
extern s16 lbl_1_bss_8CE[];
#include "game/hu3d.h"

extern s16 lbl_1_bss_8CE[];
extern ANIMDATA *lbl_1_bss_938[];
#include "game/hu3d.h"

extern s16 lbl_1_bss_8CE[2];
#include "game/hu3d.h"
extern s16 lbl_1_bss_8CA[];
#include "game/hu3d.h"

extern s16 lbl_1_bss_8CA[];
extern ANIMDATA *lbl_1_bss_938[];
#include "game/hu3d.h"

extern s16 lbl_1_bss_8CA[2];
#include "game/hu3d.h"

extern const MDMinidllHalfwordTemplate lbl_1_rodata_398;
extern ANIMDATA *lbl_1_bss_938[9];
extern HU3D_MODELID lbl_1_bss_88E[6][5];
#include "game/hu3d.h"

extern s16 lbl_1_bss_88E[6][5];
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "game/data.h"

extern const MDMinidllHalfwordTemplate lbl_1_rodata_398;
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
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "REL/mdminidll/layer_effect_state.h"

extern s16 lbl_1_bss_8E4;
extern s16 lbl_1_bss_8DC[4];
extern s16 lbl_1_bss_8DA;
extern s16 lbl_1_bss_8D2[4];
extern s16 lbl_1_bss_8CE[2];
extern s16 lbl_1_bss_8CA[2];
extern s16 lbl_1_bss_88E[6][5];
extern OMOBJ *lbl_1_bss_87C;
extern void *lbl_1_bss_888;
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "REL/mdminidll/layer_effect_state.h"
#include "REL/mdminidll/readonly_scalars.h"

extern OMOBJ *lbl_1_bss_87C;
extern s16 lbl_1_bss_8E4;
extern const GXColor lbl_1_rodata_3A2;
#include "game/main.h"
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"

extern s16 lbl_1_bss_8DC[];
extern s16 lbl_1_bss_8E4;
#include "game/main.h"
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"

extern s16 lbl_1_bss_8D2[];
extern s16 lbl_1_bss_8DC[];
extern s16 lbl_1_bss_8E4;
#include "game/hu3d.h"

extern s16 lbl_1_bss_8DA;

f32 fn_1_1F3AC(f32 startValue, f32 endValue, f32 elapsed, f32 duration);
f32 fn_1_1F494(f32 startValue, f32 endValue, f32 elapsed, f32 duration);
f32 fn_1_1F4D8(f32 currentValue, f32 targetValue, f32 smoothingDivisor);
void fn_1_1F508(s16 layerNo);
void fn_1_1F574(HU3D_DRAW_OBJ *drawObject, HSF_MATERIAL *material);
void fn_1_1F9F4(OMOBJ *effectObject);
void fn_1_1FAF4(s16 effectIndex, Point3d *position);
void fn_1_1FBE8(OMOBJ *obj);
void fn_1_1FDCC(OMOBJ *effectObject);
void fn_1_1FE7C(void);
void fn_1_1FECC(void);
void fn_1_1FFA8(Point3d *position);
void fn_1_20068(s16 layerNo);
void fn_1_20A24(void);
void fn_1_20A50(void);
void fn_1_20A74(s16 particleIndex, Point3d *position, GXColor *color, f32 lifetime,
                f32 targetScale);
void fn_1_20B74(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform);
void fn_1_20E34(void);
void fn_1_20F48(void);
void fn_1_20F74(s16 burstIndex, f32 lifetime, Point3d *position, GXColor *color);
void fn_1_2105C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform);
void fn_1_215A4(void);
void fn_1_21764(void);
void fn_1_217BC(void);
void fn_1_2185C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform);
void fn_1_21FE8(void);
void fn_1_220E8(void);
void fn_1_22114(s16 burstIndex, f32 lifetime, Point3d *position);
void fn_1_221E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform);
void fn_1_22AC8(void);
void fn_1_22C5C(void);
void fn_1_22CB4(s16 effectIndex, s16 visible);
void fn_1_22D30(s16 effectIndex, HuVecF *position, GXColor *color);
void fn_1_22E34(s16 effectIndex);
void fn_1_22EBC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform);
void fn_1_23334(void);
void fn_1_234A0(void);
void fn_1_234F8(s16 effectIndex, s16 visible);
void fn_1_23574(s16 effectIndex, Point3d *position, u8 *baseColor);
void fn_1_236AC(s16 effectIndex);
void fn_1_23700(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform);
void fn_1_23D00(void);
void fn_1_23E6C(void);
void fn_1_23EC4(s16 effectIndex, HuVecF *position, s16 mode);
void fn_1_24288(s16 row, s16 visible);
void fn_1_2433C(s16 row, Point3d *position, u8 *baseColor);
void fn_1_244A4(s16 row);
void fn_1_24524(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform);
void fn_1_24AD0(void);
void fn_1_24CFC(void);
void fn_1_24D80(HUPROCESS *process);
void fn_1_257D0(void);
void fn_1_259FC(HuVecF *position);
void fn_1_25BF4(s16 particleNo, Point3d *position, s16 colorNo);
void fn_1_25EF0(s16 particleNo, HuVecF *position, s16 colorNo, s16 mode);
void fn_1_26394(void);
void fn_1_26454(s16 row, Point3d *position, s16 mode, s16 colorIndex);

/* Used by particle updates to ease from start to end with a quarter-sine curve; clamps at both
 * endpoints. */
f32 fn_1_1F3AC(f32 startValue, f32 endValue, f32 elapsed, f32 duration)
{
    if (elapsed <= 0.0f) {
        return startValue;
    }
    if (elapsed >= duration) {
        return endValue;
    }
    return (f32) ((f64) startValue +
        ((f64) (endValue - startValue) *
         sin((3.141592653589793 *
              (f64) ((90.0f / duration) * elapsed)) /
             180.0)));
}

/* Used by effects and screen blending to interpolate linearly, clamping before and after the
 * duration. */
f32 fn_1_1F494(f32 startValue, f32 endValue, f32 elapsed, f32 duration)
{
    if (elapsed <= 0.0f) {
        return startValue;
    }
    if (elapsed >= duration) {
        return endValue;
    }
    return startValue + ((elapsed / duration) * (endValue - startValue));
}

/* Used by glow updates to move a value toward its target by one smoothing-divisor fraction per
 * call. */
f32 fn_1_1F4D8(f32 currentValue, f32 targetValue, f32 smoothingDivisor)
{
    if (currentValue == targetValue) {
        return targetValue;
    }
    return (targetValue + (currentValue * (smoothingDivisor - 1.0f))) / smoothingDivisor;
}

/* Layer-1 hook during screen distortion: captures the 640x480 scene into the half-size texture. */
void fn_1_1F508(s16 layerNo)
{
    if (lbl_1_bss_888) {
        GXSetTexCopySrc(0, 0, 640, 480);
        GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_TRUE);
        GXCopyTex(lbl_1_bss_888, GX_FALSE);
    }
}

/* Material hook for the distortion model: projects the captured scene and scrolls the warp
 * textures. The material argument is unused. */
void fn_1_1F574(HU3D_DRAW_OBJ *drawObject, HSF_MATERIAL *material)
{
    Mtx textureTransform;
    Mtx textureScale;
    Mtx textureMatrix;
    Mtx projectionMatrix;
    Mtx inverseCamera;
    GXTexObj captureTexture;
    MDMinIndirectTextureConsumedView *textureState;
    HU3D_CAMERA *camera;

    /* The model hook holds the warp matrix, tint and scrolling offset. */
    textureState = drawObject->model->hookData;
    camera = Hu3DCamera;
    GXInitTexObj(&captureTexture, lbl_1_bss_888, 320U, 240U, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 0U);
    GXInitTexObjLOD(&captureTexture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0U, 0U, GX_ANISO_1);
    GXLoadTexObj(&captureTexture, GX_TEXMAP1);
    HuSprTexLoad(lbl_1_bss_880, 0, 2, 1, 1, 1);
    HuSprTexLoad(lbl_1_bss_884, 0, 3, 1, 1, 1);
    GXSetNumTexGens(3U);
    GXSetNumTevStages(3U);
    C_MTXLightPerspective(projectionMatrix, camera->fov, 1.2f, 0.5f, (-0.5f), 0.5f, 0.5f);
    PSMTXInverse(Hu3DCameraMtx, inverseCamera);
    PSMTXConcat(inverseCamera, drawObject->matrix, textureTransform);
    PSMTXConcat(projectionMatrix, Hu3DCameraMtx, textureMatrix);
    PSMTXConcat(textureMatrix, textureTransform, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, 33U, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, 33U, 0U, 125U);
    PSMTXTrans(textureTransform, 0.0f,
               (textureState->scrollY += textureState->scrollStep),
               0.0f);
    PSMTXScale(textureScale, 0.8f, 0.8f, 1.0f);
    PSMTXConcat(textureScale, textureTransform, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, 30U, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, 30U, 0U, 125U);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, 60U, 0U, 125U);
    GXSetTevColor(GX_TEVREG0, textureState->tint);
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
    /* Distort the projected scene with the scrolling texture in map 2. */
    GXSetNumIndStages(1U);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP2);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 1U, 0U, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, textureState->warpMatrix, -1);
}

/* While screen distortion is active, the object callback reduces the first slot's warp and scroll
 * speed with its countdown, then hides the model once the countdown becomes negative. */
void fn_1_1F9F4(OMOBJ *effectObject)
{
    MDMinidllLayerEffectStateView *state =
        (MDMinidllLayerEffectStateView *) lbl_1_bss_8E8;

    /* Both warp axes use the same offset; direct S/T contributions stay zero. */
    state->warpSFromS = state->warpTFromS = 0.0f;
    state->warpSFromT = state->warpTFromT = 0.0f;
    state->warpSOffset = state->warpTOffset =
        (0.09f * state->remainingUpdates) / 90.0f;
    state->scrollStep =
        ((-0.02f) * state->remainingUpdates) / 90.0f;
    if ((state->remainingUpdates -= 1.0f) < 0.0f) {
        Hu3DModelAttrSet(*effectObject->mdlId, HU3D_ATTR_DISPOFF);
        Hu3DLayerHookReset(1);
        effectObject->objFunc = NULL;
    }
}

/* A menu event reveals the indexed distortion model and resets that slot's scrolling and countdown.
 * The installed update callback always advances the first state slot and model. */
void fn_1_1FAF4(s16 effectIndex, Point3d *position)
{
    OMOBJ *obj = lbl_1_bss_87C;

    if (obj) {
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)[effectIndex].scrollY =
            0.0f;
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)[effectIndex].remainingUpdates =
            90.0f;
        /* Use the requested x/y, but fix the distortion plane at z = -840. */
        Hu3DModelPosSet(obj->mdlId[effectIndex], position->x, position->y, (-840.0f));
        Hu3DModelAttrReset(obj->mdlId[effectIndex], HU3D_ATTR_DISPOFF);
        Hu3DLayerHookSet(1, fn_1_1F508);
        obj->objFunc = fn_1_1F9F4;
    }
}

/* Object initialization callback: loads the menu warp textures, capture buffer and hidden
 * distortion model. */
void fn_1_1FBE8(OMOBJ *obj)
{
    HU3D_MODEL *model = NULL;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    lbl_1_bss_880 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 88), HU_MEMNUM_OVL,
            HEAP_MODEL));
    lbl_1_bss_884 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 88), HU_MEMNUM_OVL,
            HEAP_MODEL));
    lbl_1_bss_888 = HuMemDirectMallocNum(
        HEAP_MODEL,
        GXGetTexBufferSize(320, 240, GX_TF_RGB565, GX_FALSE, 0),
        HU_MEMNUM_OVL);
    obj->mdlId[0] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 87), HU_MEMNUM_OVL,
            HEAP_MODEL));
    Hu3DModelPosSet(obj->mdlId[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(obj->mdlId[0], 90.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 6.0f, 6.0f, 6.0f);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelMatHookSet(obj->mdlId[0], fn_1_1F574);
    /* Only the first state slot is cleared and attached to this model. */
    memset(lbl_1_bss_8E8, 0, sizeof(MDMinidllLayerEffectStateView));
    model = &Hu3DData[obj->mdlId[0]];
    model->hookData = lbl_1_bss_8E8;
    obj->objFunc = NULL;
}

/* Effect cleanup helper: removes the capture hook and releases the supplied distortion model and
 * buffer. */
void fn_1_1FDCC(OMOBJ *effectObject)
{
    HU3D_MODEL *model;

    model = NULL;
    Hu3DLayerHookReset(1);
    if (effectObject) {
        model = &Hu3DData[effectObject->mdlId[0]];
        model->hookData = lbl_1_bss_8E8;
        Hu3DModelKill(effectObject->mdlId[0]);
        if (lbl_1_bss_888) {
            HuMemDirectFree(lbl_1_bss_888);
        }
        lbl_1_bss_888 = NULL;
    }
    /* Clearing this parameter changes only the local pointer. */
    effectObject = NULL;
}

/* Creates the screen-distortion object when the menu requests its effect resources. */
void fn_1_1FE7C(void)
{
    lbl_1_bss_87C = omAddObjEx(lbl_1_bss_878, 4096, 1, 0, -1, fn_1_1FBE8);
}

/* Menu cleanup releases the distortion model and capture buffer, then clears the shared object
 * pointer. */
void fn_1_1FECC(void)
{
    OMOBJ *obj;
    HU3D_MODEL *model;
    if (lbl_1_bss_87C) {
        obj = lbl_1_bss_87C;
        model = NULL;
        Hu3DLayerHookReset(1);
        if (obj) {
            model = &Hu3DData[*obj->mdlId];
            model->hookData = lbl_1_bss_8E8;
            Hu3DModelKill(*obj->mdlId);
            if (lbl_1_bss_888) {
                HuMemDirectFree(lbl_1_bss_888);
            }
            lbl_1_bss_888 = NULL;
        }

        obj = NULL;
    }
    lbl_1_bss_87C = NULL;
}

/* A menu event starts the first distortion model at the supplied x/y with z fixed at -840 and its
 * countdown set to 90; the update hides it after the countdown becomes negative. */
void fn_1_1FFA8(Point3d *position)
{
    OMOBJ *obj = lbl_1_bss_87C;

    if (obj) {
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)->scrollY =
            0.0f;
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)->remainingUpdates =
            90.0f;
        Hu3DModelPosSet(*obj->mdlId, position->x, position->y, -840.0f);
        Hu3DModelAttrReset(*obj->mdlId, HU3D_ATTR_DISPOFF);
        Hu3DLayerHookSet(1, fn_1_1F508);
        obj->objFunc = fn_1_1F9F4;
    }
}

/* Layer-14 hook blends four expanded copies of the captured scene, captures the result and clears
 * depth. With a zero initial counter, the blend reaches its maximum on call 91. */
void fn_1_20068(s16 layerNo)
{
    Mtx44 screenProjection;
    Mtx screenTransform;
    GXTexObj captureTexture;
    s16 quadIndex;
    f32 blendProgress;

    C_MTXOrtho(screenProjection, 0.0f, 480.0f,
        0.0f, 640.0f, 0.0f, 10.0f);
    GXSetProjection(screenProjection, GX_ORTHOGRAPHIC);
    GXInitTexObj(&captureTexture, lbl_1_bss_888, 320U, 240U, GX_TF_RGB565, GX_CLAMP, GX_CLAMP,
                 GX_FALSE);
    GXInitTexObjLOD(&captureTexture, GX_LINEAR, GX_LINEAR, 0.0f,
        0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&captureTexture, GX_TEXMAP0);
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
    PSMTXIdentity(screenTransform);
    GXLoadPosMtxImm(screenTransform, 0U);
    /* Draw the previous capture with growing alpha; each copy expands by one pixel. */
    blendProgress = fn_1_1F494(0.0f, 1.0f, (f32) lbl_1_bss_88C, 90.0f);
    lbl_1_bss_88C += 1;
    if (lbl_1_bss_88C > 90) {
        lbl_1_bss_88C = 90;
    }
    GXBegin(GX_QUADS, GX_VTXFMT0, 16U);
    quadIndex = 0;
    while (quadIndex < 4) {
        GXWGFifo.f32 = (f32) (quadIndex * -1);
        GXWGFifo.f32 = (f32) (quadIndex * -1);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (128.0f * blendProgress);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = (f32) (quadIndex + 640);
        GXWGFifo.f32 = (f32) (quadIndex * -1);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (128.0f * blendProgress);
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = (f32) (quadIndex + 640);
        GXWGFifo.f32 = (f32) (quadIndex + 480);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (128.0f * blendProgress);
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = (f32) (quadIndex * -1);
        GXWGFifo.f32 = (f32) (quadIndex + 480);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (255.0f - (8.0f * blendProgress));
        GXWGFifo.u8 = (s32) (128.0f * blendProgress);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = 1.0f;
        quadIndex += 1;
    }
    GXSetTexCopySrc(0U, 0U, 640U, 480U);
    GXSetTexCopyDst(320U, 240U, GX_TF_RGB565, GX_TRUE);
    GXCopyTex(lbl_1_bss_888, GX_FALSE);
    Hu3DZClear();
}

/* Enables the captured-scene blend hook for a menu transition. */
void fn_1_20A24(void)
{
    Hu3DLayerHookSet(14, fn_1_20068);
}

/* Disables the captured-scene blend hook when the transition ends. */
void fn_1_20A50(void)
{
    Hu3DLayerHookReset(14);
}

/* Effect trigger for menu events: starts a growing, fading particle with the supplied color and
* lifetime. Reseeding a slot increments the flash count even if its previous flash is active. */
void fn_1_20A74(s16 particleIndex, Point3d *position, GXColor *color, f32 lifetime, f32 targetScale)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_8E4];
    particle = model->hookData;
    data = &particle->data[particleIndex];
    data->time = 1;
    data->pos.x = position->x;
    data->pos.y = position->y;
    data->pos.z = position->z;
    data->scale = 0.0f;
    data->color.r = color->r;
    data->color.g = color->g;
    data->color.b = color->b;
    data->color.a = color->a;
    /* These slots hold elapsed time, lifetime, target scale and starting alpha for the flash. */
    data->vel.x = 0.0f;
    data->vel.y = lifetime;
    data->accel.x = targetScale;
    data->accel.y = (f32) color->a;
    particle->dataCnt++;
    model->attr &= MDMIN_VISIBLE_MODEL_MASK;
}

/* Particle draw callback: grows each active flash and fades its alpha until its lifetime
 * expires. */
void fn_1_20B74(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform)
{
    s16 particleIndex;
    HU3D_PARTICLE_DATA *data;

    if (particle->dataCnt <= 0) {
        model->attr |= HU3D_ATTR_DISPOFF;
        return;
    }
    particleIndex = 0;
    data = particle->data;
    while (particleIndex < particle->maxCnt) {
        if (data->time == 1) {
            data->scale = fn_1_1F3AC(0.0f, data->accel.x, data->vel.x, data->vel.y);
            data->color.a = fn_1_1F3AC(data->accel.y, 0.0f, data->vel.x, data->vel.y);
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->time = 0;
                data->scale = 0.0f;
                particle->dataCnt -= 1;
            }
        }
        particleIndex += 1;
        data += 1;
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Effect setup creates the eight-slot flash pool on layer 7, initially hidden. */
void fn_1_20E34(void)
{
    lbl_1_bss_8E4 = Hu3DParticleCreate(lbl_1_bss_938[0], 8);
    Hu3DModelPosSet(lbl_1_bss_8E4, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelScaleSet(lbl_1_bss_8E4, 1.0f, 1.0f,
                      1.0f);
    Hu3DModelLayerSet(lbl_1_bss_8E4, 7);
    Hu3DModelAttrSet(lbl_1_bss_8E4, HU3D_ATTR_DISPOFF);
    Hu3DParticleScaleSet(lbl_1_bss_8E4, 1.0f);
    Hu3DParticleHookSet(lbl_1_bss_8E4, fn_1_20B74);
    Hu3DParticleBlendModeSet(lbl_1_bss_8E4, HU3D_PARTICLE_BLEND_ADDCOL);
}

/* Effect teardown destroys the flash-particle model. */
void fn_1_20F48(void)
{
    Hu3DModelKill(lbl_1_bss_8E4);
}

/* Menu effect trigger: seeds a colored rotating ring at the supplied position for the requested
 * lifetime. */
void fn_1_20F74(s16 burstIndex, f32 lifetime, Point3d *position, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *seedParticle;

    model = &Hu3DData[lbl_1_bss_8DC[burstIndex]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_8DC[burstIndex], position);
    seedParticle = particle->data;
    seedParticle->time = 1;
    seedParticle->color.r = color->r;
    seedParticle->color.g = color->g;
    seedParticle->color.b = color->b;
    seedParticle->color.a = color->a;
    /* The seed slot becomes the ring clock; its earlier time value is immediately overwritten. */
    seedParticle->time = 0;
    seedParticle->parManId = (s16) lifetime;
    particle->dataCnt = 1;
    model->attr &= MDMIN_VISIBLE_MODEL_MASK;
}

/* Particle draw callback: initializes a radial ring, then counter-rotates alternating particles and
 * fades the ring using the first particle as its shared clock. */
void fn_1_2105C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform)
{
    s16 particleIndex;
    HU3D_PARTICLE_DATA *burstClock;
    HU3D_PARTICLE_DATA *particleData;

    if (particle->dataCnt <= 0) {
        model->attr |= HU3D_ATTR_DISPOFF;
        return;
    }
    /* dataCnt selects initialization (1), animation (2) or completion (0). */
    if (particle->dataCnt == 1) {
        burstClock = particle->data;
        particleIndex = 0;
        particleData = particle->data;
        while (particleIndex < particle->maxCnt) {
            particleData->attr |= HU3D_PARTICLE_ATTR_SCALEY;
            particleData->zRot = 0.017453292f * (f32) frandmod(360);
            particleData->vel.z = (f32) (frandmod(25) + 50);
            particleData->vel.x =
                (f32) ((f64) particleData->vel.z *
                       -sin((3.141592653589793 * (f64) (57.29578f * particleData->zRot)) / 180.0));
            particleData->vel.y =
                (f32) ((f64) particleData->vel.z *
                       cos((3.141592653589793 * (f64) (57.29578f * particleData->zRot)) / 180.0));
            particleData->scale = 0.1f * particleData->vel.z;
            particleData->scaleY = 3.0f * particleData->vel.z;
            particleData->color.r = burstClock->color.r;
            particleData->color.g = burstClock->color.g;
            particleData->color.b = burstClock->color.b;
            particleData->color.a = burstClock->color.a;
            particleData->pos.x = particleData->vel.x;
            particleData->pos.y = particleData->vel.y;
            particleData->pos.z = 0.0f;
            particleIndex += 1;
            particleData += 1;
        }
        particle->dataCnt = 2;
    } else if (particle->dataCnt == 2) {
        burstClock = particle->data;
        particleIndex = 0;
        particleData = particle->data;
        while (particleIndex < particle->maxCnt) {
            if (particleIndex % 2 == 0) {
                particleData->zRot += (f32) particleIndex / 500.0f;
            } else {
                particleData->zRot -= (f32) particleIndex / 500.0f;
            }
            particleData->vel.x =
                (f32) ((f64) particleData->vel.z *
                       -sin((3.141592653589793 * (f64) (57.29578f * particleData->zRot)) / 180.0));
            particleData->vel.y =
                (f32) ((f64) particleData->vel.z *
                       cos((3.141592653589793 * (f64) (57.29578f * particleData->zRot)) / 180.0));
            particleData->pos.x = particleData->vel.x;
            particleData->pos.y = particleData->vel.y;
            particleData->pos.z = 0.0f;
            /* Ring animation starts its fade from alpha 255, regardless of the seed color alpha. */
            particleData->color.a =
                fn_1_1F3AC(255.0f, 0.0f, burstClock->time, burstClock->parManId);
            particleIndex += 1;
            particleData += 1;
        }
        /* The first slot stores the shared age and the requested duration in parManId. */
        if ((burstClock->time += 1.0f) > burstClock->parManId) {
            particle->dataCnt = 0;
        }
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * 72);
}

/* Effect setup creates four hidden radial-ring pools with sixteen particles each. */
void fn_1_215A4(void)
{
    s16 burstIndex;

    burstIndex = 0;
    while (burstIndex < 4) {
        lbl_1_bss_8DC[burstIndex] = Hu3DParticleCreate(lbl_1_bss_938[1], 16);
        Hu3DModelPosSet(lbl_1_bss_8DC[burstIndex], 200.0f * (f32) burstIndex,
                        200.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_8DC[burstIndex], 1.0f,
                          1.0f, 1.0f);
        Hu3DModelLayerSet(lbl_1_bss_8DC[burstIndex], 7);
        Hu3DModelAttrSet(lbl_1_bss_8DC[burstIndex], HU3D_ATTR_DISPOFF);
        Hu3DParticleScaleSet(lbl_1_bss_8DC[burstIndex], 1.0f);
        Hu3DParticleHookSet(lbl_1_bss_8DC[burstIndex], fn_1_2105C);
        Hu3DParticleBlendModeSet(lbl_1_bss_8DC[burstIndex], HU3D_PARTICLE_BLEND_ADDCOL);
        burstIndex += 1;
    }
}

/* Effect teardown destroys all four radial-ring pools. */
void fn_1_21764(void)
{
    s16 burstIndex;

    burstIndex = 0;
    while (burstIndex < 4) {
        Hu3DModelKill(lbl_1_bss_8DC[burstIndex]);
        burstIndex += 1;
    }
}

/* Menu transition trigger: clears the spiral particles and enables their continuous emission. */
void fn_1_217BC(void)
{
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *particleData;
    s16 particleIndex;
    HU3D_MODEL *model;

    model = &Hu3DData[lbl_1_bss_8DA];
    particle = model->hookData;
    particleIndex = 0;
    particleData = particle->data;
    while (particleIndex < particle->maxCnt) {
        particleData->time = 0;
        particleData->scale = 0.0f;
        particleIndex += 1;
        particleData += 1;
    }
    particle->dataCnt = 1;
    model->attr &= MDMIN_VISIBLE_MODEL_MASK;
}

/* Particle draw callback during the menu transition: emits up to seven spiral particles per update
* and moves them backward, fading over their final ten updates. Animation replaces the seeded size
* and radius with curves starting at 150 and 500, so later particles can jump outward before
* approaching the shrinking inner radius. */
void fn_1_2185C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform)
{

    s16 randomChannel;
    s16 particleIndex;
    s16 emittedCount;
    f32 targetRadius;
    HU3D_PARTICLE_DATA *particleData;

    emittedCount = 0;
    targetRadius = 125.0f;
    if (particle->dataCnt == 1) {
        particle->emitCnt -= 2;
        if (particle->emitCnt < 0) {
            particle->emitCnt += 360.0f;
        }
        if (particle->count > 100U) {
            targetRadius -= (f32) (particle->count - 100);
            if (targetRadius < 1.0f) {
                targetRadius = 1.0f;
            }
        }
        particleIndex = 0;
        particleData = particle->data;
        while (particleIndex < particle->maxCnt) {
            if (particleData->time == 0) {
                if (emittedCount < 7) {
                    /* Consume random red and green values even though both channels become 255. */
                    randomChannel = rand8() + 128;
                    randomChannel &= 0xFF;
                    particleData->color.r = 255;
                    randomChannel = rand8() + 128;
                    randomChannel &= 0xFF;
                    particleData->color.g = 255;
                    randomChannel = rand8() % 204;
                    randomChannel &= 0xFF;
                    particleData->color.b = (u8) randomChannel;
                    if ((s32) (particleIndex % 4) == 0) {
                        particleData->color.b = 255;
                    }
                    particleData->color.a = 255;
                    particleData->scale = 80.0f;
                    particleData->vel.x =
                        (45.0f * (f32) (particleIndex % 8)) + (f32) particle->emitCnt;
                    particleData->vel.z = 750.0f - (f32) (particle->count * 2);
                    if (particleData->vel.z < 1.0f) {
                        particleData->vel.z = 1.0f;
                    }
                    /* accel.x/y store elapsed updates and lifetime; vel.x/z store angle and
                     * radius. */
                    particleData->accel.x = 0.0f;
                    particleData->accel.y = (f32) (frandmod(30) + 90);
                    particleData->pos.x =
                        (f32) ((f64) particleData->vel.z *
                               -sin((3.141592653589793 * (f64) particleData->vel.x) / 180.0));
                    particleData->pos.y =
                        (f32) ((f64) particleData->vel.z *
                               cos((3.141592653589793 * (f64) particleData->vel.x) / 180.0));
                    particleData->pos.z = 800.0f;
                    particleData->time = 1;
                    emittedCount += 1;
                }
            } else if (particleData->time == 1) {
                particleData->pos.x =
                    (f32) ((f64) particleData->vel.z *
                           -sin((3.141592653589793 * (f64) particleData->vel.x) / 180.0));
                particleData->pos.y =
                    (f32) ((f64) particleData->vel.z *
                           cos((3.141592653589793 * (f64) particleData->vel.x) / 180.0));
                particleData->pos.z =
                    fn_1_1F494(800.0f, -2000.0f, particleData->accel.x, particleData->accel.y);
                particleData->zRot += 0.10000000149011612f;
                particleData->scale =
                    fn_1_1F494(150.0f, 50.0f, particleData->accel.x, particleData->accel.y);
                particleData->vel.x += 2.0f;
                particleData->vel.z =
                    fn_1_1F494(500.0f, targetRadius, particleData->accel.x, particleData->accel.y);
                if (particleData->accel.x > (particleData->accel.y - 10.0f)) {
                    particleData->color.a =
                        fn_1_1F494(255.0f, 0.0f,
                                   particleData->accel.x - (particleData->accel.y - 10.0f), 10.0f);
                }
                if ((particleData->accel.x += 1.0f) > particleData->accel.y) {
                    particleData->time = 0;
                    particleData->scale = 0.0f;
                }
            }
            particleIndex += 1;
            particleData += 1;
        }
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * 72);
}

/* Effect setup creates the hidden thousand-slot spiral emitter on layer 7. */
void fn_1_21FE8(void)
{
    lbl_1_bss_8DA = Hu3DParticleCreate(lbl_1_bss_938[2], 1000);
    Hu3DModelPosSet(lbl_1_bss_8DA,
                    0.0f,
                    275.0f,
                    0.0f);
    Hu3DModelScaleSet(lbl_1_bss_8DA,
                      1.0f,
                      1.0f,
                      1.0f);
    Hu3DModelLayerSet(lbl_1_bss_8DA, 7);
    Hu3DModelAttrSet(lbl_1_bss_8DA, HU3D_ATTR_DISPOFF);
    Hu3DParticleScaleSet(lbl_1_bss_8DA, 1.0f);
    Hu3DParticleHookSet(lbl_1_bss_8DA, fn_1_2185C);
}

/* Effect teardown destroys the spiral-emitter model. */
void fn_1_220E8(void)
{
    Hu3DModelKill(lbl_1_bss_8DA);
}

/* Menu effect trigger: positions a spherical burst and starts its shared lifetime clock. */
void fn_1_22114(s16 burstIndex, f32 lifetime, Point3d *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *particleData;

    model = &Hu3DData[lbl_1_bss_8D2[burstIndex]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_8D2[burstIndex], position);
    particleData = particle->data;
    particleData->time = 0;
    particleData->parManId = (s16) lifetime;
    particle->dataCnt = 1;
    model->attr &= MDMIN_VISIBLE_MODEL_MASK;
}

/* Particle draw callback: seeds radial targets and grows to size 125 over five elapsed ticks.
* After sixteen swelling updates, spreading resets size to 50, moves particles outward and shrinks
* them over the supplied duration using a restarted clock. */
void fn_1_221E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform)
{
    Point3d randomDirection;
    Point3d unitDirection;
    s16 particleIndex;
    s16 randomChannel;
    f32 burstRadius;
    HU3D_PARTICLE_DATA *burstClock;
    HU3D_PARTICLE_DATA *particleData;

    /* dataCnt advances through initialization, swelling and outward expansion. */
    if (particle->dataCnt == 1) {
        particleIndex = 0;
        particleData = particle->data;
        while (particleIndex < particle->maxCnt) {
            randomDirection.x = (f32) (frandmod(100) - 50);
            randomDirection.y = (f32) (frandmod(100) - 50);
            randomDirection.z = (f32) (frandmod(100) - 50);
            PSVECNormalize(&randomDirection, &unitDirection);
            burstRadius = 200.0f;
            particleData->vel.x = unitDirection.x * burstRadius;
            particleData->vel.y = unitDirection.y * burstRadius;
            particleData->vel.z = unitDirection.z * burstRadius;
            randomChannel = rand8() + 128;
            randomChannel &= 0xFF;
            particleData->color.r = (u8) randomChannel;
            randomChannel = rand8() + 128;
            randomChannel &= 0xFF;
            particleData->color.g = (u8) randomChannel;
            randomChannel = (s16) ((s32) rand8() % 128);
            randomChannel &= 0xFF;
            particleData->color.b = (u8) randomChannel;
            particleData->color.a = 204;
            /* accel preserves the tiny initial offset; vel holds the outward target position. */
            particleData->pos.x = particleData->accel.x = (f32) (frandmod(4) - 2);
            particleData->pos.y = particleData->accel.y = (f32) (frandmod(4) - 2);
            particleData->pos.z = particleData->accel.z = (f32) (frandmod(4) - 2);
            particleData->scale = 0.0f;
            particleIndex += 1;
            particleData += 1;
        }
        particle->dataCnt = 2;
    } else if (particle->dataCnt == 2) {
        burstClock = particle->data;
        particleIndex = 0;
        particleData = particle->data;
        while (particleIndex < particle->maxCnt) {
            particleData->scale = fn_1_1F3AC(0.0f, 125.0f, burstClock->time, 5.0f);
            particleIndex += 1;
            particleData += 1;
        }
        if ((burstClock->time += 1.0f) > 15) {
            burstClock->time = 0;
            particle->dataCnt = 3;
        }
    } else if (particle->dataCnt == 3) {
        burstClock = particle->data;
        particleIndex = 0;
        particleData = particle->data;
        while (particleIndex < particle->maxCnt) {
            particleData->pos.x = fn_1_1F3AC(particleData->accel.x, particleData->vel.x,
                                             burstClock->time, burstClock->parManId);
            particleData->pos.y = fn_1_1F3AC(particleData->accel.y, particleData->vel.y,
                                             burstClock->time, burstClock->parManId);
            particleData->pos.z = fn_1_1F3AC(particleData->accel.z, particleData->vel.z,
                                             burstClock->time, burstClock->parManId);
            particleData->scale = fn_1_1F494(50.0f, 0.0f, burstClock->time, burstClock->parManId);
            particleIndex += 1;
            particleData += 1;
        }
        /* The first slot stores the shared age and the requested duration in parManId. */
        if ((burstClock->time += 1.0f) > burstClock->parManId) {
            particle->dataCnt = 0;
            model->attr |= HU3D_ATTR_DISPOFF;
        }
    }
}

/* Effect setup creates four hidden spherical-burst pools with sixty-four particles each. */
void fn_1_22AC8(void)
{
    s16 burstIndex;

    burstIndex = 0;
    while (burstIndex < 4) {
        lbl_1_bss_8D2[burstIndex] = Hu3DParticleCreate(lbl_1_bss_938[2], 64);
        Hu3DModelPosSet(lbl_1_bss_8D2[burstIndex], 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_8D2[burstIndex], 1.0f, 1.0f, 1.0f);
        Hu3DModelLayerSet(lbl_1_bss_8D2[burstIndex], 7);
        Hu3DModelAttrSet(lbl_1_bss_8D2[burstIndex], HU3D_ATTR_DISPOFF);
        Hu3DParticleScaleSet(lbl_1_bss_8D2[burstIndex], 1.0f);
        Hu3DParticleHookSet(lbl_1_bss_8D2[burstIndex], fn_1_221E0);
        Hu3DParticleBlendModeSet(lbl_1_bss_8D2[burstIndex], HU3D_PARTICLE_BLEND_ADDCOL);
        burstIndex += 1;
    }
}

/* Effect teardown destroys all four spherical-burst pools. */
void fn_1_22C5C(void)
{
    s16 burstIndex;

    burstIndex = 0;
    while (burstIndex < 4) {
        Hu3DModelKill(lbl_1_bss_8D2[burstIndex]);
        burstIndex += 1;
    }
}

/* Menu effect control toggles visibility of one of the two preview glows. */
void fn_1_22CB4(s16 effectIndex, s16 visible)
{
    if (visible != 0) {
        Hu3DModelAttrReset(lbl_1_bss_8CE[effectIndex], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_8CE[effectIndex], HU3D_ATTR_DISPOFF);
    }
}

/* Preview-transition trigger: enables its glow, optionally updating its position and RGB color. */
void fn_1_22D30(s16 effectIndex, HuVecF *position, GXColor *color)
{
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle;
    s16 particleIndex;
    HU3D_MODEL *model;

    model = &Hu3DData[lbl_1_bss_8CE[effectIndex]];
    particle = model->hookData;
    particleIndex = 0;
    data = particle->data;
    while (particleIndex < particle->maxCnt) {
        data->time = 1;
        if (color != NULL) {
            data->color.r = color->r;
            data->color.g = color->g;
            data->color.b = color->b;
        }
        particleIndex += 1;
        data++;
    }
    if (position != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_8CE[effectIndex], position);
    }
    Hu3DModelAttrReset(lbl_1_bss_8CE[effectIndex], HU3D_ATTR_DISPOFF);
}

/* Preview-transition control puts every glow particle into its fading state. */
void fn_1_22E34(s16 effectIndex)
{
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    s16 particleIndex;
    HU3D_MODEL *model;

    model = &Hu3DData[lbl_1_bss_8CE[effectIndex]];
    particle = model->hookData;
    particleIndex = 0;
    data = particle->data;
    while (particleIndex < particle->maxCnt) {
        data->time = 2;
        particleIndex += 1;
        data++;
    }
}

/* Particle draw callback: centers each preview-glow layer and smooths its size and alpha toward the
 * five repeating targets, or toward zero while fading. */
void fn_1_22EBC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform)
{
    MDMinidllParticleByteTemplate sizeAlphaTable = {
        { { { 16, 255 } }, { { 16, 255 } }, { { 48, 128 } }, { { 128, 64 } }, { { 255, 32 } } }
    };
    s16 particleIndex;
    HU3D_PARTICLE_DATA *particleData;

    /* The first draw clears the glow slot states. */
    if (particle->count == 0) {
        particleIndex = 0;
        particleData = particle->data;
        while (particleIndex < particle->maxCnt) {
            particleData->time = 0;
            particleIndex += 1;
            particleData += 1;
        }
    }
    particleIndex = 0;
    particleData = particle->data;
    while (particleIndex < particle->maxCnt) {
        if (particleData->time == 1) {
            particleData->pos.x = 0.0f;
            particleData->pos.y = 0.0f;
            particleData->pos.z = 0.0f;
            particleData->vel.x =
                (f32) sizeAlphaTable.scaleAlphaPairs[particleIndex % 5].scaleAlpha[1];
            particleData->vel.y =
                (f32) sizeAlphaTable.scaleAlphaPairs[particleIndex % 5].scaleAlpha[0];
            particleData->color.a =
                fn_1_1F4D8((f32) particleData->color.a, particleData->vel.x, 10.0f);
            particleData->scale = fn_1_1F4D8(particleData->scale, particleData->vel.y, 10.0f);
        } else if (particleData->time == 2) {
            particleData->pos.x = 0.0f;
            particleData->pos.y = 0.0f;
            particleData->pos.z = 0.0f;
            particleData->vel.x = 0.0f;
            particleData->vel.y = 0.0f;
            particleData->color.a =
                fn_1_1F4D8((f32) particleData->color.a, particleData->vel.x, 10.0f);
            particleData->scale = fn_1_1F4D8(particleData->scale, particleData->vel.y, 10.0f);
        } else {
            particleData->color.a = 0;
            particleData->scale = 0.0f;
        }
        particleIndex += 1;
        particleData += 1;
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * 72);
}

/* Effect setup creates two hidden ten-particle preview glows on layer 2. */
void fn_1_23334(void)
{
    s16 effectIndex;

    effectIndex = 0;
    while (effectIndex < 2) {
        lbl_1_bss_8CE[effectIndex] = Hu3DParticleCreate(*lbl_1_bss_938, 10);
        Hu3DModelPosSet(lbl_1_bss_8CE[effectIndex], 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_8CE[effectIndex], 1.0f, 1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_8CE[effectIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_8CE[effectIndex], 2);
        Hu3DParticleHookSet(lbl_1_bss_8CE[effectIndex], fn_1_22EBC);
        Hu3DParticleBlendModeSet(lbl_1_bss_8CE[effectIndex], HU3D_PARTICLE_BLEND_ADDCOL);
        effectIndex += 1;
    }
}

/* Effect teardown destroys both preview-glow models. */
void fn_1_234A0(void)
{
    s16 effectIndex;

    effectIndex = 0;
    while (effectIndex < 2) {
        Hu3DModelKill(lbl_1_bss_8CE[effectIndex]);
        effectIndex += 1;
    }
}

/* Menu effect control toggles visibility of one of the two preview particle trails. */
void fn_1_234F8(s16 effectIndex, s16 visible)
{
    if (visible != 0) {
        Hu3DModelAttrReset(lbl_1_bss_8CA[effectIndex], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_8CA[effectIndex], HU3D_ATTR_DISPOFF);
    }
}

/* Preview-transition trigger: enables its trail and optionally updates the base RGB and spawn
 * center. */
void fn_1_23574(s16 effectIndex, Point3d *position, u8 *baseColor)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_8CA[effectIndex]];
    MDMinModelHookDataView *hookData = model->hookData;

    hookData->emitting = 1;
    if (baseColor != NULL) {
        hookData->baseColor[0] = (f32) baseColor[0];
        hookData->baseColor[1] = (f32) baseColor[1];
        hookData->baseColor[2] = (f32) baseColor[2];
    }
    if (position != NULL) {
        hookData->spawnCenter.x = position->x;
        hookData->spawnCenter.y = position->y;
        hookData->spawnCenter.z = position->z;
    }
    Hu3DModelAttrReset(lbl_1_bss_8CA[effectIndex], HU3D_ATTR_DISPOFF);
}

/* Preview-transition control stops new trail particles; existing particles keep drifting and
* flickering until their lifetimes end. */
void fn_1_236AC(s16 effectIndex)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    model = &Hu3DData[lbl_1_bss_8CA[effectIndex]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* Particle draw callback for preview movement: emits up to four particles per update, drifts them
 * downward and varies their rotation, alpha and shrinking size. */
void fn_1_23700(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform)
{
    s16 particleIndex;
    s16 emittedCount;
    f32 channelIntensity;
    f32 brightnessOrScale;
    HU3D_PARTICLE_DATA *particleData;

    emittedCount = 0;
    /* The first draw clears slots and forces emission on with white base RGB, overriding earlier
    * emission and color settings. */
    if (particle->count == 0) {
        particleIndex = 0;
        particleData = particle->data;
        while (particleIndex < particle->maxCnt) {
            particleData->time = 0;
            particleIndex += 1;
            particleData += 1;
        }
        particle->dataCnt = 1;
        particle->pos.x = 255.0f;
        particle->pos.y = 255.0f;
        particle->pos.z = 255.0f;
    }
    particleIndex = 0;
    particleData = particle->data;
    while (particleIndex < particle->maxCnt) {
        if ((particleData->time == 0) && (particle->dataCnt == 1) && (emittedCount <= 3)) {
            emittedCount += 1;
            particleData->time = 1;
            /* vel.x/y hold elapsed updates and lifetime; accel supplies the drift per update. */
            particleData->vel.x = 0.0f;
            particleData->vel.y = (f32) (frandmod(30) + 60);
            particleData->accel.x = (f32) (frandmod(100) - 50);
            particleData->accel.y = (f32) (-frandmod(100) - 50);
            particleData->accel.z = (f32) (frandmod(100) - 50);
            PSVECNormalize(&particleData->accel, &particleData->accel);
            particleData->accel.x *= 2.0f;
            particleData->accel.y *= 5.0f;
            particleData->accel.z *= 2.0f;
            particleData->pos.x = (particle->spawnCenter.x + (f32) frandmod(100)) - 50.0f;
            particleData->pos.y = (particle->spawnCenter.y + (f32) frandmod(100)) - 50.0f;
            particleData->pos.z = (particle->spawnCenter.z + (f32) frandmod(100)) - 50.0f;
            brightnessOrScale = (f32) frandmod(128);
            channelIntensity = particle->pos.x + brightnessOrScale;
            if (channelIntensity > 255.0f) {
                channelIntensity = 255.0f;
            }
            particleData->color.r = channelIntensity;
            channelIntensity = particle->pos.y + brightnessOrScale;
            if (channelIntensity > 255.0f) {
                channelIntensity = 255.0f;
            }
            particleData->color.g = channelIntensity;
            channelIntensity = particle->pos.z + brightnessOrScale;
            if (channelIntensity > 255.0f) {
                channelIntensity = 255.0f;
            }
            particleData->color.b = channelIntensity;
            particleData->color.a = 0;
        } else if (particleData->time == 1) {
            particleData->pos.x += particleData->accel.x;
            particleData->pos.y += particleData->accel.y;
            particleData->pos.z += particleData->accel.z;
            if ((s32) (rand8() % 5) == 0) {
                particleData->zRot = 0.01745329238474369f * (f32) frandmod(360);
                particleData->color.a = frandmod(127) + 128;
            }
            brightnessOrScale = fn_1_1F494(1.0f, 0.0f, particleData->vel.x, particleData->vel.y);
            particleData->scale = brightnessOrScale * (f32) frandmod(90);
            if ((particleData->vel.x += 1.0f) > particleData->vel.y) {
                particleData->time = 0;
                particleData->scale = 0.0f;
                particleData->color.a = 0;
            }
        }
        particleIndex += 1;
        particleData += 1;
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * 72);
}

/* Effect setup creates two hidden 256-slot preview-trail emitters on layer 2. */
void fn_1_23D00(void)
{
    s16 effectIndex;

    effectIndex = 0;
    while (effectIndex < 2) {
        lbl_1_bss_8CA[effectIndex] = Hu3DParticleCreate(lbl_1_bss_938[3], 256);
        Hu3DModelPosSet(lbl_1_bss_8CA[effectIndex], 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_8CA[effectIndex], 1.0f, 1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_8CA[effectIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_8CA[effectIndex], 2);
        Hu3DParticleHookSet(lbl_1_bss_8CA[effectIndex], fn_1_23700);
        Hu3DParticleBlendModeSet(lbl_1_bss_8CA[effectIndex], HU3D_PARTICLE_BLEND_ADDCOL);
        effectIndex += 1;
    }
}

/* Effect teardown destroys both preview-trail emitters. */
void fn_1_23E6C(void)
{
    s16 effectIndex;

    effectIndex = 0;
    while (effectIndex < 2) {
        Hu3DModelKill(lbl_1_bss_8CA[effectIndex]);
        effectIndex += 1;
    }
}

/* Preview-transition control starts a color-coded glow and trail together. Mode 0 fades the glow
* and stops new trail particles; mode 2 hides both. */
void fn_1_23EC4(s16 effectIndex, HuVecF *position, s16 mode)
{
    MDMinidllColorTemplate localColors = {{{255, 114, 46, 0}, {109, 207, 246, 0}}};

    if (mode == 1) {
        fn_1_22D30(effectIndex, position, (GXColor *)localColors.rows[effectIndex]);
        fn_1_23574(effectIndex, position, localColors.rows[effectIndex]);
        return;
    } else if (mode == 0) {
        fn_1_22E34(effectIndex);
        fn_1_236AC(effectIndex);
        return;
    } else if (mode == 2) {
        Hu3DModelAttrSet(lbl_1_bss_8CE[effectIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_8CA[effectIndex], HU3D_ATTR_DISPOFF);
    }
}

/* Menu effect control toggles visibility of all five particle models in one effect row. */
void fn_1_24288(s16 row, s16 visible)
{
    s16 effectType;

    for (effectType = 0; effectType < 5; effectType++) {
        if (visible) {
            Hu3DModelAttrReset(lbl_1_bss_88E[row][effectType], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_88E[row][effectType], HU3D_ATTR_DISPOFF);
        }
    }
}

/* Character-transition trigger: enables all five emitters in a row and optionally updates their
 * shared base RGB and spawn center. */
void fn_1_2433C(s16 row, Point3d *position, u8 *baseColor)
{
    s16 effectType;
    HU3D_MODEL *model;
    MDMinigHookDataView *emitterState;

    effectType = 0;
    while (effectType < 5) {
        model = &Hu3DData[lbl_1_bss_88E[row][effectType]];
        emitterState = model->hookData;
        emitterState->emitting = 1;
        if (baseColor != NULL) {
            emitterState->baseRed = (f32) baseColor[0];
            emitterState->baseGreen = (f32) baseColor[1];
            emitterState->baseBlue = (f32) baseColor[2];
        }
        if (position != NULL) {
            emitterState->spawnX = position->x;
            emitterState->spawnY = position->y;
            emitterState->spawnZ = position->z;
        }
        Hu3DModelAttrReset(lbl_1_bss_88E[row][effectType], HU3D_ATTR_DISPOFF);
        effectType += 1;
    }
}

/* Character-transition control stops new particles in all five emitters of the selected row. */
void fn_1_244A4(s16 row)
{
    s16 effectType;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    effectType = 0;
    while (effectType < 5) {
        model = &Hu3DData[lbl_1_bss_88E[row][effectType]];
        particle = model->hookData;
        particle->dataCnt = 0;
        effectType += 1;
    }
}

/* Particle draw callback for character transitions: emits at most one particle per update while
 * enabled, launches it upward, applies downward acceleration and shrinks it over its lifetime. */
void fn_1_24524(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx transform)
{
    s16 particleIndex;
    s16 emittedCount;
    f32 channelIntensity;
    f32 brightnessOrScale;
    HU3D_PARTICLE_DATA *particleData;

    emittedCount = 0;
    /* The first draw clears slots and forces emission on with white base RGB, overriding earlier
    * emission and color settings. */
    if (particle->count == 0) {
        particleIndex = 0;
        particleData = particle->data;
        while (particleIndex < particle->maxCnt) {
            particleData->time = 0;
            particleIndex += 1;
            particleData += 1;
        }
        particle->dataCnt = 1;
        particle->pos.x = 255.0f;
        particle->pos.y = 255.0f;
        particle->pos.z = 255.0f;
    }
    particleIndex = 0;
    particleData = particle->data;
    while (particleIndex < particle->maxCnt) {
        if ((particleData->time == 0) && (particle->dataCnt == 1) && (emittedCount < 1)) {
            emittedCount += 1;
            particleData->time = 1;
            /* vel.x/y hold elapsed updates and lifetime; colorIdx stores vertical speed. */
            particleData->vel.x = 0.0f;
            particleData->vel.y = (f32) (frandmod(30) + 30);
            particleData->accel.x = (f32) (frandmod(100) - 50);
            particleData->accel.y = (f32) (-frandmod(100) - 50);
            particleData->accel.z = (f32) (frandmod(100) - 50);
            PSVECNormalize(&particleData->accel, &particleData->accel);
            particleData->accel.x *= 2.0f;
            particleData->accel.z *= 2.0f;
            particleData->colorIdx = (f32) (frandmod(10) + 5);
            particleData->pos.x = (particle->spawnCenter.x + (f32) frandmod(100)) - 50.0f;
            particleData->pos.y = particle->spawnCenter.y;
            particleData->pos.z = (particle->spawnCenter.z + (f32) frandmod(100)) - 50.0f;
            brightnessOrScale = (f32) frandmod(32);
            channelIntensity = particle->pos.x + brightnessOrScale;
            if (channelIntensity > 255.0f) {
                channelIntensity = 255.0f;
            }
            particleData->color.r = (u8) channelIntensity;
            channelIntensity = particle->pos.y + brightnessOrScale;
            if (channelIntensity > 255.0f) {
                channelIntensity = 255.0f;
            }
            particleData->color.g = (u8) channelIntensity;
            channelIntensity = particle->pos.z + brightnessOrScale;
            if (channelIntensity > 255.0f) {
                channelIntensity = 255.0f;
            }
            particleData->color.b = (u8) channelIntensity;
            particleData->color.a = 0;
        } else if (particleData->time == 1) {
            /* Horizontal drift is initialized above but only the vertical component is applied. */
            particleData->pos.y += particleData->colorIdx;
            particleData->colorIdx += particleData->accel.y;
            if ((s32) (rand8() % 5) == 0) {
                particleData->zRot = 0.01745329238474369f * (f32) frandmod(360);
                particleData->color.a = frandmod(127) + 128;
            }
            brightnessOrScale = fn_1_1F494(1.0f, 0.0f, particleData->vel.x, particleData->vel.y);
            particleData->scale = 40.0f * brightnessOrScale;
            if ((particleData->vel.x += 1.0f) > particleData->vel.y) {
                particleData->time = 0;
                particleData->scale = 0.0f;
                particleData->color.a = 0;
            }
        }
        particleIndex += 1;
        particleData += 1;
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * 72);
}

const MDMinidllHalfwordTemplate lbl_1_rodata_398 = {{10, 10, 10, 10, 256}};

/* Effect setup creates six rows of five hidden character-transition emitters using their capacity
 * table. */
void fn_1_24AD0(void)
{
    MDMinidllHalfwordTemplate capacities;
    s16 effectType;
    s16 row;

    capacities = lbl_1_rodata_398;
    for (row = 0; row < 6; row += 1) {
        for (effectType = 0; effectType < 5; effectType += 1) {
            lbl_1_bss_88E[row][effectType] = Hu3DParticleCreate(
                lbl_1_bss_938[effectType + 4],
                capacities.particleCounts[effectType]);
            Hu3DModelPosSet(lbl_1_bss_88E[row][effectType],
                0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(lbl_1_bss_88E[row][effectType],
                1.0f, 1.0f, 1.0f);
            Hu3DModelAttrSet(lbl_1_bss_88E[row][effectType], HU3D_ATTR_DISPOFF);
            Hu3DModelLayerSet(lbl_1_bss_88E[row][effectType], 2);
            Hu3DParticleHookSet(lbl_1_bss_88E[row][effectType], fn_1_24524);
            Hu3DParticleBlendModeSet(lbl_1_bss_88E[row][effectType], HU3D_PARTICLE_BLEND_ADDCOL);
        }
    }
}

/* Effect teardown destroys every particle model in the six character-transition rows. */
void fn_1_24CFC(void)
{
    s16 row;
    s16 column;

    for (row = 0; row < 6; row++) {
        column = 0;
        while (column < 5) {
            Hu3DModelKill(lbl_1_bss_88E[row][column]);
            column += 1;
        }
    }
}

/* Called during minigame-mode setup to load all effect animations and create the hidden
 * screen-warp, flash, ring, spiral, preview and character-transition models. */
void fn_1_24D80(HUPROCESS *process)
{
    MDMinidllHalfwordTemplate capacities;
    s16 ringIndex;
    s16 effectType;
    s16 burstIndex;
    s16 glowIndex;
    s16 trailIndex;
    s16 row;
    s16 animationIndex;

    lbl_1_bss_878 = process;
    lbl_1_bss_87C = omAddObjEx(lbl_1_bss_878, 4096, 1U, 0U, -1, fn_1_1FBE8);
    animationIndex = 0;
    while (animationIndex < 9) {
        lbl_1_bss_938[animationIndex] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_998[animationIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        animationIndex += 1;
    }
    lbl_1_bss_8E4 = Hu3DParticleCreate(lbl_1_bss_938[0], 8);
    Hu3DModelPosSet(lbl_1_bss_8E4, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(lbl_1_bss_8E4, 1.0f, 1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_8E4, 7);
    Hu3DModelAttrSet(lbl_1_bss_8E4, HU3D_ATTR_DISPOFF);
    Hu3DParticleScaleSet(lbl_1_bss_8E4, 1.0f);
    Hu3DParticleHookSet(lbl_1_bss_8E4, fn_1_20B74);
    Hu3DParticleBlendModeSet(lbl_1_bss_8E4, HU3D_PARTICLE_BLEND_ADDCOL);
    ringIndex = 0;
    while (ringIndex < 4) {
        lbl_1_bss_8DC[ringIndex] = Hu3DParticleCreate(lbl_1_bss_938[1], 16);
        Hu3DModelPosSet(lbl_1_bss_8DC[ringIndex], 200.0f * (f32) ringIndex,
            200.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_8DC[ringIndex], 1.0f,
            1.0f, 1.0f);
        Hu3DModelLayerSet(lbl_1_bss_8DC[ringIndex], 7);
        Hu3DModelAttrSet(lbl_1_bss_8DC[ringIndex], HU3D_ATTR_DISPOFF);
        Hu3DParticleScaleSet(lbl_1_bss_8DC[ringIndex], 1.0f);
        Hu3DParticleHookSet(lbl_1_bss_8DC[ringIndex], fn_1_2105C);
        Hu3DParticleBlendModeSet(lbl_1_bss_8DC[ringIndex], HU3D_PARTICLE_BLEND_ADDCOL);
        ringIndex += 1;
    }
    lbl_1_bss_8DA = Hu3DParticleCreate(lbl_1_bss_938[2], 1000);
    Hu3DModelPosSet(lbl_1_bss_8DA, 0.0f, 275.0f, 0.0f);
    Hu3DModelScaleSet(lbl_1_bss_8DA, 1.0f,
        1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_8DA, 7);
    Hu3DModelAttrSet(lbl_1_bss_8DA, HU3D_ATTR_DISPOFF);
    Hu3DParticleScaleSet(lbl_1_bss_8DA, 1.0f);
    Hu3DParticleHookSet(lbl_1_bss_8DA, fn_1_2185C);
    burstIndex = 0;
    while (burstIndex < 4) {
        lbl_1_bss_8D2[burstIndex] = Hu3DParticleCreate(lbl_1_bss_938[2], 64);
        Hu3DModelPosSet(lbl_1_bss_8D2[burstIndex], 0.0f,
            0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_8D2[burstIndex], 1.0f,
            1.0f, 1.0f);
        Hu3DModelLayerSet(lbl_1_bss_8D2[burstIndex], 7);
        Hu3DModelAttrSet(lbl_1_bss_8D2[burstIndex], HU3D_ATTR_DISPOFF);
        Hu3DParticleScaleSet(lbl_1_bss_8D2[burstIndex], 1.0f);
        Hu3DParticleHookSet(lbl_1_bss_8D2[burstIndex], fn_1_221E0);
        Hu3DParticleBlendModeSet(lbl_1_bss_8D2[burstIndex], HU3D_PARTICLE_BLEND_ADDCOL);
        burstIndex += 1;
    }
    glowIndex = 0;
    while (glowIndex < 2) {
        lbl_1_bss_8CE[glowIndex] = Hu3DParticleCreate(lbl_1_bss_938[0], 10);
        Hu3DModelPosSet(lbl_1_bss_8CE[glowIndex], 0.0f,
            0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_8CE[glowIndex], 1.0f,
            1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_8CE[glowIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_8CE[glowIndex], 2);
        Hu3DParticleHookSet(lbl_1_bss_8CE[glowIndex], fn_1_22EBC);
        Hu3DParticleBlendModeSet(lbl_1_bss_8CE[glowIndex], HU3D_PARTICLE_BLEND_ADDCOL);
        glowIndex += 1;
    }
    trailIndex = 0;
    while (trailIndex < 2) {
        lbl_1_bss_8CA[trailIndex] = Hu3DParticleCreate(lbl_1_bss_938[3], 256);
        Hu3DModelPosSet(lbl_1_bss_8CA[trailIndex], 0.0f,
            0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_8CA[trailIndex], 1.0f,
            1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_8CA[trailIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_8CA[trailIndex], 2);
        Hu3DParticleHookSet(lbl_1_bss_8CA[trailIndex], fn_1_23700);
        Hu3DParticleBlendModeSet(lbl_1_bss_8CA[trailIndex], HU3D_PARTICLE_BLEND_ADDCOL);
        trailIndex += 1;
    }
    capacities = lbl_1_rodata_398;
    for (row = 0; row < 6; row += 1) {
        for (effectType = 0; effectType < 5; effectType += 1) {
            lbl_1_bss_88E[row][effectType] = Hu3DParticleCreate(
                lbl_1_bss_938[effectType + 4], capacities.particleCounts[effectType]);
            Hu3DModelPosSet(lbl_1_bss_88E[row][effectType],
                0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(lbl_1_bss_88E[row][effectType],
                1.0f, 1.0f, 1.0f);
            Hu3DModelAttrSet(lbl_1_bss_88E[row][effectType], HU3D_ATTR_DISPOFF);
            Hu3DModelLayerSet(lbl_1_bss_88E[row][effectType], 2);
            Hu3DParticleHookSet(lbl_1_bss_88E[row][effectType], fn_1_24524);
            Hu3DParticleBlendModeSet(lbl_1_bss_88E[row][effectType], HU3D_PARTICLE_BLEND_ADDCOL);
        }
    }
}

/* Called when minigame mode leaves its menu: destroys all particle models, disables the scene hooks
 * and releases the screen-distortion model and capture buffer. */
void fn_1_257D0(void)
{
    OMOBJ *effectObject;
    s16 ringIndex;
    s16 burstIndex;
    s16 glowIndex;
    s16 trailIndex;
    s16 effectType;
    s16 row;
    HU3D_MODEL *model;

    Hu3DModelKill(lbl_1_bss_8E4);
    ringIndex = 0;
    while (ringIndex < 4) {
        Hu3DModelKill(lbl_1_bss_8DC[ringIndex]);
        ringIndex += 1;
    }
    Hu3DModelKill(lbl_1_bss_8DA);
    burstIndex = 0;
    while (burstIndex < 4) {
        Hu3DModelKill(lbl_1_bss_8D2[burstIndex]);
        burstIndex += 1;
    }
    glowIndex = 0;
    while (glowIndex < 2) {
        Hu3DModelKill(lbl_1_bss_8CE[glowIndex]);
        glowIndex += 1;
    }
    trailIndex = 0;
    while (trailIndex < 2) {
        Hu3DModelKill(lbl_1_bss_8CA[trailIndex]);
        trailIndex += 1;
    }
    row = 0;
    while (row < 6) {
        effectType = 0;
        while (effectType < 5) {
            Hu3DModelKill(lbl_1_bss_88E[row][effectType]);
            effectType += 1;
        }
        row += 1;
    }
    Hu3DLayerHookReset(14);
    if ((OMOBJ *) lbl_1_bss_87C) {
        effectObject = lbl_1_bss_87C;
        model = NULL;
        Hu3DLayerHookReset(1);
        if (effectObject) {
            model = &Hu3DData[*effectObject->mdlId];
            model->hookData = lbl_1_bss_8E8;
            Hu3DModelKill(*effectObject->mdlId);
            if ((void *) lbl_1_bss_888) {
                HuMemDirectFree(lbl_1_bss_888);
            }
            lbl_1_bss_888 = NULL;
        }
        effectObject = NULL;
    }
    lbl_1_bss_87C = NULL;
}

const GXColor lbl_1_rodata_3A2 = {128, 128, 255, 128};

/* Menu-selection transition trigger: starts a large blue flash in slot 5 at the supplied position
* and screen distortion at its x/y with z fixed at -840. Reseeding the flash increments its count
* even if the previous flash is active. */
void fn_1_259FC(HuVecF *position)
{
    GXColor savedColor;
    GXColor sourceColor;
    GXColor *colorBuffer;
    HU3D_PARTICLE_DATA *particleData;
    OMOBJ *object;
    HU3D_PARTICLE *particle;
    HU3D_MODEL *model;

    object = lbl_1_bss_87C;
    if (object) {
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)->scrollY = 0.0f;
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)->remainingUpdates = 90.0f;
        Hu3DModelPosSet(*object->mdlId, position->x, position->y, -840.0f);
        Hu3DModelAttrReset(*object->mdlId, HU3D_ATTR_DISPOFF);
        Hu3DLayerHookSet(1, fn_1_1F508);
        object->objFunc = fn_1_1F9F4;
    }
    sourceColor = lbl_1_rodata_3A2;
    colorBuffer = &sourceColor;
    savedColor = *colorBuffer;
    model = &Hu3DData[lbl_1_bss_8E4];
    particle = model->hookData;
    particleData = &particle->data[5];
    particleData->time = 1;
    particleData->pos.x = position->x;
    particleData->pos.y = position->y;
    particleData->pos.z = position->z;
    particleData->scale = 0.0f;
    particleData->color = savedColor;
    particleData->vel.x = 0.0f;
    particleData->vel.y = 120.0f;
    particleData->accel.x = 3000.0f;
    particleData->accel.y = (f32) savedColor.a;
    particle->dataCnt += 1;
    model->attr &= MDMIN_VISIBLE_MODEL_MASK;
}

/* Character-selection confirmation trigger: starts a short colored flash and rotating ring slightly
* in front of the selected character. Reseeding a flash increments its count even if its previous
* flash is active. */
void fn_1_25BF4(s16 particleNo, Point3d *position, s16 colorNo)
{
    GXColor palette[5] = {
        { 254, 77, 75, 255 },
        { 50, 127, 200, 255 },
        { 199, 175, 0, 255 },
        { 52, 192, 63, 255 },
        { 159, 93, 200, 255 },
    };
    Point3d positionOffset;
    GXColor firstColor;
    GXColor secondColor;
    HU3D_PARTICLE *firstParticle;
    HU3D_MODEL *firstModel;
    HU3D_PARTICLE *secondParticle;
    HU3D_MODEL *secondModel;
    HU3D_PARTICLE_DATA *firstData;
    HU3D_PARTICLE_DATA *secondData;

    positionOffset.x = position->x;
    positionOffset.y = position->y;
    positionOffset.z = 10.0f + position->z;
    firstColor = palette[colorNo];
    firstModel = &Hu3DData[lbl_1_bss_8E4];
    firstParticle = firstModel->hookData;
    firstData = &firstParticle->data[particleNo];
    firstData->time = 1;
    firstData->pos.x = positionOffset.x;
    firstData->pos.y = positionOffset.y;
    firstData->pos.z = positionOffset.z;
    firstData->scale = 0.0f;
    firstData->color = firstColor;
    firstData->vel.x = 0.0f;
    firstData->vel.y = 10.0f;
    firstData->accel.x = 500.0f;
    firstData->accel.y = (f32) firstColor.a;
    firstParticle->dataCnt += 1;
    firstModel->attr &= MDMIN_VISIBLE_MODEL_MASK;
    secondColor = palette[colorNo];
    secondModel = &Hu3DData[lbl_1_bss_8DC[particleNo]];
    secondParticle = secondModel->hookData;
    Hu3DModelPosSetV(lbl_1_bss_8DC[particleNo], &positionOffset);
    secondData = secondParticle->data;
    secondData->time = 1;
    secondData->color = secondColor;
    /* The ring uses its first particle as a clock, replacing the time value set above. */
    secondData->time = 0;
    secondData->parManId = 10;
    secondParticle->dataCnt = 1;
    secondModel->attr &= MDMIN_VISIBLE_MODEL_MASK;
}

/* Character-model entrance after start confirmation: mode 0 combines a short flash with a spherical
* burst; other modes combine a longer flash with a rotating ring at the character position.
* Reseeding a flash slot increments its count even when the previous flash is still active. */
void fn_1_25EF0(s16 particleNo, HuVecF *position, s16 colorNo, s16 mode)
{
    GXColor palette[5] = {
        { 254, 77, 75, 255 },
        { 50, 127, 200, 255 },
        { 199, 175, 0, 255 },
        { 52, 192, 63, 255 },
        { 159, 93, 200, 255 },
    };
    HuVecF particlePosition;
    GXColor color;
    GXColor secondColor;
    GXColor thirdColor;
    HU3D_PARTICLE_DATA *firstData;
    HU3D_PARTICLE_DATA *alternateFirstData;
    HU3D_PARTICLE_DATA *alternateSecondData;
    HU3D_PARTICLE *firstParticle;
    HU3D_MODEL *firstModel;
    HU3D_PARTICLE_DATA *zeroModeData;
    HU3D_PARTICLE *zeroModeParticle;
    HU3D_MODEL *zeroModeModel;
    HU3D_PARTICLE *alternateFirstParticle;
    HU3D_MODEL *alternateFirstModel;
    HU3D_PARTICLE *alternateSecondParticle;
    HU3D_MODEL *alternateSecondModel;

    particlePosition.x = position->x;
    particlePosition.y = position->y;
    particlePosition.z = position->z;
    Hu3DZClearLayerSet(7);
    if (mode == 0) {
        color = palette[colorNo];
        firstModel = &Hu3DData[lbl_1_bss_8E4];
        firstParticle = firstModel->hookData;
        firstData = &firstParticle->data[particleNo];
        firstData->time = 1;
        firstData->pos.x = particlePosition.x;
        firstData->pos.y = particlePosition.y;
        firstData->pos.z = particlePosition.z;
        firstData->scale = 0.0f;
        firstData->color = color;
        firstData->vel.x = 0.0f;
        firstData->vel.y = 20.0f;
        firstData->accel.x = 300.0f;
        firstData->accel.y = (f32) color.a;
        firstParticle->dataCnt += 1;
        firstModel->attr &= MDMIN_VISIBLE_MODEL_MASK;
        zeroModeModel = &Hu3DData[lbl_1_bss_8D2[particleNo]];
        zeroModeParticle = zeroModeModel->hookData;
        Hu3DModelPosSetV(lbl_1_bss_8D2[particleNo], &particlePosition);
        zeroModeData = zeroModeParticle->data;
        zeroModeData->time = 0;
        zeroModeData->parManId = 30;
        zeroModeParticle->dataCnt = 1;
        zeroModeModel->attr &= MDMIN_VISIBLE_MODEL_MASK;
        return;
    }
    secondColor = palette[colorNo];
    alternateFirstModel = &Hu3DData[lbl_1_bss_8E4];
    alternateFirstParticle = alternateFirstModel->hookData;
    alternateFirstData = &alternateFirstParticle->data[particleNo];
    alternateFirstData->time = 1;
    alternateFirstData->pos.x = particlePosition.x;
    alternateFirstData->pos.y = particlePosition.y;
    alternateFirstData->pos.z = particlePosition.z;
    alternateFirstData->scale = 0.0f;
    alternateFirstData->color = secondColor;
    alternateFirstData->vel.x = 0.0f;
    alternateFirstData->vel.y = 60.0f;
    alternateFirstData->accel.x = 300.0f;
    alternateFirstData->accel.y = (f32) secondColor.a;
    alternateFirstParticle->dataCnt += 1;
    alternateFirstModel->attr &= MDMIN_VISIBLE_MODEL_MASK;
    thirdColor = palette[colorNo];
    alternateSecondModel = &Hu3DData[lbl_1_bss_8DC[particleNo]];
    alternateSecondParticle = alternateSecondModel->hookData;
    Hu3DModelPosSetV(lbl_1_bss_8DC[particleNo], &particlePosition);
    alternateSecondData = alternateSecondParticle->data;
    alternateSecondData->time = 1;
    alternateSecondData->color = thirdColor;
    /* The ring uses its first particle as a clock, replacing the time value set above. */
    alternateSecondData->time = 0;
    alternateSecondData->parManId = 30;
    alternateSecondParticle->dataCnt = 1;
    alternateSecondModel->attr &= MDMIN_VISIBLE_MODEL_MASK;
}

/* Menu-exit transition trigger: enables captured-scene blending and restarts the spiral
 * particles. */
void fn_1_26394(void)
{
    HU3D_PARTICLE_DATA *particleData;
    HU3D_PARTICLE *particle;
    s16 particleIndex;
    HU3D_MODEL *model;

    /* Restarting clears particle slots but preserves the emitter angle and draw count. */
    Hu3DLayerHookSet(14, fn_1_20068);
    model = &Hu3DData[lbl_1_bss_8DA];
    particle = model->hookData;
    particleIndex = 0;
    particleData = particle->data;
    while (particleIndex < particle->maxCnt) {
        particleData->time = 0;
        particleData->scale = 0.0f;
        particleIndex += 1;
        particleData += 1;
    }
    particle->dataCnt = 1;
    model->attr &= MDMIN_VISIBLE_MODEL_MASK;
}

/* Character-transition effect control: mode 1 enables a row with a palette color, mode 0 stops
 * emission and mode 2 hides it. */
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
