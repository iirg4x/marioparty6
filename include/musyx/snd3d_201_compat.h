#ifndef MP6_MUSYX_SND3D_201_COMPAT_H
#define MP6_MUSYX_SND3D_201_COMPAT_H

/*
 * MusyX 2.0.1's 3D runtime has a private ABI view that predates the public
 * MP6 headers.  Keep that view in this translation unit: other MusyX users
 * continue to include the repository headers unchanged.
 */
#include "musyx/platform.h"

#ifndef MUSY_VERSION_CHECK
#define MUSY_VERSION_CHECK(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#endif

#if defined(MUSY_VERSION) && MUSY_VERSION != MUSY_VERSION_CHECK(2, 0, 1)
#error "snd3d.c requires the MusyX 2.0.1 ABI"
#endif
#ifndef MUSY_VERSION
#define MUSY_VERSION MUSY_VERSION_CHECK(2, 0, 1)
#endif

#define _MATH_H
#include "dolphin/math.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if MUSY_TARGET == MUSY_TARGET_DOLPHIN
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long s32;
typedef unsigned long u32;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
#elif MUSY_TARGET == MUSY_TARGET_PC
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef bool8
typedef unsigned char bool8;
#endif
#ifndef __cplusplus
#if __STDC_VERSION__ <= 199901L
typedef unsigned long bool;
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#endif

typedef u32 SND_VOICEID;
typedef u16 SND_GROUPID;
typedef u16 SND_FXID;

extern u8 sndActive;

typedef struct SND_FVECTOR {
  f32 x;
  f32 y;
  f32 z;
} SND_FVECTOR;

typedef struct SND_FMATRIX {
  f32 m[3][3];
  f32 t[3];
} SND_FMATRIX;

typedef struct SND_PARAMETER {
  u8 ctrl;
  union _paraData {
    u8 value7;
    u16 value14;
  } paraData;
} SND_PARAMETER;

typedef struct SND_PARAMETER_INFO {
  u8 numPara;
  SND_PARAMETER* paraArray;
} SND_PARAMETER_INFO;

typedef struct SND_STUDIO_INPUT {
  u8 vol;
  u8 volA;
  u8 volB;
  u8 srcStudio;
} SND_STUDIO_INPUT;

typedef enum {
  SND_STUDIO_TYPE_STD = 0,
  SND_STUDIO_TYPE_DPL2,
  SND_STUDIO_TYPE_RESERVED1,
  SND_STUDIO_TYPE_RESERVED2
} SND_STUDIO_TYPE;

typedef struct SND_ROOM {
  struct SND_ROOM* next;
  struct SND_ROOM* prev;

  u32 flags;
  SND_FVECTOR pos;
  f32 distance;

  u8 studio;

  void (*activateReverb)(u8 studio, void* para);
  void (*deActivateReverb)(u8 studio);
  void* user;

  u32 curMVol;
} SND_ROOM;

typedef struct SND_DOOR {
  struct SND_DOOR* next;
  struct SND_DOOR* prev;

  SND_FVECTOR pos;

  f32 open;
  f32 dampen;
  u8 fxVol;

  u8 destStudio;

  SND_ROOM* a;
  SND_ROOM* b;

  u32 flags;

  s16 filterCoef[4];
  SND_STUDIO_INPUT input;
} SND_DOOR;

typedef struct SND_LISTENER {
  struct SND_LISTENER* next;
  struct SND_LISTENER* prev;
#if MUSY_VERSION <= MUSY_VERSION_CHECK(2, 0, 0)
  SND_ROOM* room;
#endif

  u32 flags;
  SND_FVECTOR pos;
  f32 volPosOff;
  SND_FVECTOR dir;
  SND_FVECTOR heading;
  SND_FVECTOR right;
  SND_FVECTOR up;
  SND_FMATRIX mat;
  f32 surroundDisFront;
  f32 surroundDisBack;
  f32 soundSpeed;
  f32 vol;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
  float oneMeter;
#endif
} SND_LISTENER;

typedef struct SND_EMITTER {
  struct SND_EMITTER* next;
  struct SND_EMITTER* prev;
#if MUSY_VERSION <= MUSY_VERSION_CHECK(2, 0, 0)
  SND_ROOM* room;
#endif

  SND_PARAMETER_INFO* paraInfo;

  u32 flags;
  SND_FVECTOR pos;
  SND_FVECTOR dir;
  f32 maxDis;
  f32 maxVol;
  f32 minVol;
  f32 volPush;
  SND_VOICEID vid;
  u32 group;
  SND_FXID fxid;

  u8 studio;
  u8 maxVoices;
  u16 VolLevelCnt;
  f32 fade;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
  unsigned long userData;
#endif
} SND_EMITTER;

typedef void* (*SND_S3D_OCCLUSION_CALLBACK)(SND_EMITTER* emitter,
                                            const SND_FVECTOR* listenerPos,
                                            const SND_FVECTOR* listenerHeading,
                                            const SND_FVECTOR* listenerUp,
                                            const SND_FVECTOR* emitterPos,
                                            const SND_FVECTOR* emitterHeading,
                                            f32* volOcclusionFactor,
                                            f32* frqOcclusionFactor);

typedef struct SND_LISTENER_EXPARAMETER {
  float oneMeter;
} SND_LISTENER_EXPARAMETER;

typedef struct FX_TAB {
  u16 id;
  u16 macro;
  u8 maxVoices;
  u8 priority;
  u8 volume;
  u8 panning;
  u8 key;
  u8 vGroup;
} FX_TAB;

typedef struct SND_3DINFO {
  u8 vol;
  u8 pan;
  u8 span;
  u16 doppler;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
  u16 lpfFactor;
#endif
} SND_3DINFO;

void hwDisableIrq(void);
void hwEnableIrq(void);
void hwChangeStudio(u32 v, u8 studio);

void salApplyMatrix(const SND_FMATRIX* a, const SND_FVECTOR* b, SND_FVECTOR* out);
f32 salNormalizeVector(SND_FVECTOR* vec);
void salCrossProduct(SND_FVECTOR* out, const SND_FVECTOR* a, const SND_FVECTOR* b);
void salInvertMatrix(SND_FMATRIX* out, const SND_FMATRIX* in);

bool synthFXSetCtrl(SND_VOICEID vid, u8 ctrl, u8 value);
bool synthFXSetCtrl14(SND_VOICEID vid, u8 ctrl, u16 value);
bool synthSendKeyOff(SND_VOICEID vid);
SND_VOICEID synthFXStart(u16 fid, u8 vol, u8 pan, u8 studio, u32 itd);
int synthCheckFXRealloc(unsigned short fid);
u8 synthFXGetMaxVoices(u16 fid);
void synthActivateStudio(u8 studio, u32 isMaster, SND_STUDIO_TYPE type);
void synthDeactivateStudio(u8 studio);
bool synthAddStudioInput(u8 studio, SND_STUDIO_INPUT* in_desc);
bool synthRemoveStudioInput(u8 studio, SND_STUDIO_INPUT* in_desc);
s32 voiceKillSound(u32 voiceid);
SND_VOICEID sndFXCheck(SND_VOICEID vid);
u32 vidGetInternalId(SND_VOICEID id);

#ifdef __cplusplus
}
#endif

/* Prevent the public MP6 headers from reintroducing incompatible declarations
 * in this translation unit. */
#define _MUSYX_MUSYX
#define _MUSYX_HARDWARE
#define SAL_H
#define _MUSYX_SYNTH

#endif /* MP6_MUSYX_SND3D_201_COMPAT_H */
