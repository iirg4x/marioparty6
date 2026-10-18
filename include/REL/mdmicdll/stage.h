/* Shared state and callbacks for the microphone mode's scene effects. */
#ifndef REL_MDMICDLL_STAGE_H
#define REL_MDMICDLL_STAGE_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/sprite.h"

typedef struct MDMICTextureWork {
    f32 indirectMatrix[2][3]; /* Two indirect-texture transform rows. */
    GXColor color; /* Tint used by the overlay material. */
    float textureScrollY; /* Current vertical texture offset. */
    float textureScrollStep; /* Vertical offset added whenever the overlay material hook runs. */
    float remainingFrames; /* Frames left before the overlay is hidden. */
} MDMIC_TEXTURE_WORK;

typedef struct MDMICParticleTarget {
    u8 scale; /* Target particle size on the effect's 0-255 scale. */
    u8 alpha; /* Target opacity from 0 (transparent) to 255 (opaque). */
} MDMIC_PARTICLE_TARGET;

typedef struct MDMICParticleCounts {
    s16 values[5]; /* Particle capacities for the five group effect assets. */
} MDMIC_PARTICLE_COUNTS;

extern HUPROCESS *lbl_1_bss_A60; /* Process owning the effect objects. */
extern OMOBJ *lbl_1_bss_A64; /* Screen-capture overlay object. */
extern ANIMDATA *lbl_1_bss_A68; /* Overlay distortion texture. */
extern ANIMDATA *lbl_1_bss_A6C; /* Overlay scrolling texture. */
extern void *lbl_1_bss_A70; /* Half-resolution RGB565 screen capture. */
extern s16 lbl_1_bss_A74; /* Screen-trail fade progress, capped at 90 frames. */
extern HU3D_MODELID lbl_1_bss_A76[6][5]; /* Five colored emitters per effect group. */
extern HU3D_MODELID lbl_1_bss_AB2[2]; /* Color-spark particle models. */
extern HU3D_MODELID lbl_1_bss_AB6[2]; /* Color-transition particle models. */
extern HU3D_MODELID lbl_1_bss_ABA[4]; /* Expanding and scattering particle bursts. */
extern HU3D_MODELID lbl_1_bss_AC2; /* Rising-particle emitter. */
extern HU3D_MODELID lbl_1_bss_AC4[4]; /* Rotating radial particle bursts. */
extern HU3D_MODELID lbl_1_bss_ACC; /* Timed expanding particles. */
extern MDMIC_TEXTURE_WORK lbl_1_bss_AD0[2]; /* Overlay texture and lifetime settings. */
extern ANIMDATA *lbl_1_bss_B20[9]; /* Animation assets shared by the particle models. */
extern s32 lbl_1_data_FA0[9]; /* Data entries for the shared particle animations. */

float fn_1_35A10(float startValue, float endValue, float time, float duration);
float fn_1_35AF8(float startValue, float endValue, float time, float duration);
float fn_1_35B3C(float current, float target, float weight);
void fn_1_35B6C(s16 layerNo);
void fn_1_35BD8(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
void fn_1_36058(OMOBJ *obj);
void fn_1_36158(s16 textureSlot, HuVecF *position);
void fn_1_3624C(OMOBJ *obj);
void fn_1_36430(OMOBJ *obj);
void fn_1_364E0(void);
void fn_1_36530(void);
void fn_1_3660C(Point3d *position);
void fn_1_366CC(s16 layerNo);
void fn_1_37088(void);
void fn_1_370B4(void);
void fn_1_370D8(
    s16 particleNo, float lifetime, float targetScale, HuVecF *position, GXColor color);
void fn_1_371D8(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_37498(void);
void fn_1_375AC(void);
void fn_1_375D8(s16 effectNo, float duration, HuVecF *position, GXColor color);
void fn_1_376C0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_37C08(void);
void fn_1_37DC8(void);
void fn_1_37E20(void);
void fn_1_37EC0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_3864C(void);
void fn_1_3874C(void);
void fn_1_38778(s16 effectNo, float duration, HuVecF *position);
void fn_1_38844(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_3912C(void);
void fn_1_392C0(void);
void fn_1_39318(s16 index, s16 visible);
void fn_1_39394(s16 effectNo, HuVecF *position, GXColor *color);
void fn_1_39498(s16 effectNo);
void fn_1_39520(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_39998(void);
void fn_1_39B04(void);
void fn_1_39B5C(s16 index, s16 visible);
void fn_1_39BD8(s16 effectNo, HuVecF *position, GXColor *color);
void fn_1_39D10(s16 effectNo);
void fn_1_39D64(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_3A364(void);
void fn_1_3A4D0(void);
void fn_1_3A528(s16 modelNo, HuVecF *pos, s16 display);
void fn_1_3A8EC(s16 groupNo, s16 show);
void fn_1_3A9A0(s16 groupNo, HuVecF *spawnPosition, GXColor *color);
void fn_1_3AB08(s16 groupNo);
void fn_1_3AB88(HU3D_MODEL *model, HU3D_PARTICLE *emitter, Mtx matrix);
void fn_1_3B134(void);
void fn_1_3B360(void);
void fn_1_3B3E4(HUPROCESS *process);
void fn_1_3BE34(void);
void fn_1_3C060(HuVecF *position);
void fn_1_3C258(s16 particleNo, Point3d *position, s16 colorNo);
void fn_1_3C554(s16 particleNo, HuVecF *position, s16 colorNo, s16 mode);
void fn_1_3C9F8(void);
void fn_1_3CAB8(s16 groupNo, HuVecF *pos, s16 mode, s16 colorNo);

#endif
