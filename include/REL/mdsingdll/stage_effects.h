/* Screen distortion and particle models used by Solo menu animations. */
#ifndef REL_MDSINGDLL_STAGE_EFFECTS_H
#define REL_MDSINGDLL_STAGE_EFFECTS_H
#include "REL/minigame_texture_transition.h"

#include "dolphin/gx.h"
#include "dolphin/types.h"
#include "game/hu3d.h"
#include "game/object.h"

extern MinigameTextureTransition lbl_1_bss_1428[2];
extern s32 lbl_1_data_CD8[9];
extern ANIMDATA *lbl_1_bss_1478[9];
extern HU3D_MODELID lbl_1_bss_1426;
extern HU3D_MODELID lbl_1_bss_141E[4];
extern HU3D_MODELID lbl_1_bss_141C;
extern HU3D_MODELID lbl_1_bss_1414[4];
extern HU3D_MODELID lbl_1_bss_1412;
extern HU3D_MODELID lbl_1_bss_140E[2];
extern HU3D_MODELID lbl_1_bss_140A[2];
extern HU3D_MODELID lbl_1_bss_13CE[6][5];
extern s16 lbl_1_bss_13CC;
extern void *lbl_1_bss_13C8;
extern ANIMDATA *lbl_1_bss_13C4;
extern ANIMDATA *lbl_1_bss_13C0;
extern OMOBJ *lbl_1_bss_13BC;
extern HUPROCESS *lbl_1_bss_13B8;

/* Solo Mode stage and particle effect callbacks. */
float fn_1_2CF38(float startValue, float endValue, float elapsedFrames, float durationFrames);
float fn_1_2D020(float startValue, float endValue, float elapsedFrames, float durationFrames);
float fn_1_2D0FC(float startValue, float endValue, float elapsedFrames, float durationFrames);
float fn_1_2D140(float currentValue, float targetValue, float weight);
void fn_1_2D170(s16 layerNo);
void fn_1_2D1DC(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
void fn_1_2D65C(OMOBJ *obj);
void fn_1_2D75C(s16 textureSlot, HuVecF *position);
void fn_1_2D850(OMOBJ *obj);
void fn_1_2DA34(OMOBJ *obj);
void fn_1_2DAE4(void);
void fn_1_2DB34(void);
void fn_1_2DC10(Point3d *position);
void fn_1_2DCD0(s16 layerNo);
void fn_1_2E68C(void);
void fn_1_2E6B8(void);
void fn_1_2E6DC(
    s16 particleNo, float lifetimeFrames, float targetScale, HuVecF *position, GXColor color);
void fn_1_2E7DC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_2EA9C(void);
void fn_1_2EBB0(void);
void fn_1_2EBDC(s16 effectNo, float lifetimeFrames, HuVecF *position, GXColor color);
void fn_1_2ECC4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_2F20C(void);
void fn_1_2F3CC(void);
void fn_1_2F424(void);
void fn_1_2F4C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_2FC50(void);
void fn_1_2FD50(void);
void fn_1_2FD7C(s16 effectNo, float durationFrames, HuVecF *position);
void fn_1_2FE48(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_30730(void);
void fn_1_308C4(void);
void fn_1_3091C(HuVecF *position, GXColor *color, float lifetimeFrames, float targetScale);
void fn_1_30A40(float lifetimeFrames, float travelDistance, HuVecF *position,
    GXColor *color);
void fn_1_30BE0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_31088(void);
void fn_1_3116C(void);
void fn_1_31198(s16 effectNo, s16 visible);
void fn_1_31214(s16 effectNo, HuVecF *position, GXColor *color);
void fn_1_31318(s16 effectNo);
void fn_1_313A0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_31818(void);
void fn_1_31984(void);
void fn_1_319DC(s16 effectNo, s16 visible);
void fn_1_31A58(s16 effectNo, HuVecF *position, GXColor *color);
void fn_1_31B90(s16 effectNo);
void fn_1_31BE4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_321E4(void);
void fn_1_32350(void);
void fn_1_323A8(s16 modelNo, HuVecF *position, s16 displayMode);
void fn_1_3276C(s16 groupNo, s16 show);
void fn_1_32820(s16 groupNo, HuVecF *spawnPosition, GXColor *color);
void fn_1_32988(s16 groupNo);
void fn_1_32A08(HU3D_MODEL *model, HU3D_PARTICLE *emitter, Mtx matrix);
void fn_1_32FB4(void);
void fn_1_331E0(void);
void fn_1_33264(HUPROCESS *process);
void fn_1_33D84(void);
void fn_1_33FC0(HuVecF *position);
void fn_1_341B8(s16 particleNo, Point3d *position, s16 colorNo);
void fn_1_344B4(s16 particleNo, HuVecF *position, s16 colorNo, s16 mode);
void fn_1_34958(void);
void fn_1_34A18(HuVecF *position);
void fn_1_34FA4(s16 groupNo, HuVecF *position, s16 mode, s16 colorNo);

#endif
