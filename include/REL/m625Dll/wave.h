#ifndef REL_M625DLL_WAVE_H
#define REL_M625DLL_WAVE_H

#include "dolphin/math.h"
#include "dolphin/gx.h"
#include "dolphin/gx/GXVert.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/frand.h"
#include "game/gamemes.h"
#include "game/gamework.h"
#include "game/hsfex.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/mic.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/window.h"
#include "humath.h"
#include "string.h"

typedef struct M625WaveState {
    s16 model;
    void *textureBuffer;
    ANIMDATA *anim0;
    ANIMDATA *anim1;
    f32 value;
    f32 rippleX[32];
    f32 amplitude[32];
    f32 phase[32];
} M625WaveState;

typedef struct M625LightAsset_s {
    s32 resourceNumber;
    f32 positionY;
    f32 motionValue;
} M625LightAsset;

typedef struct M625LightModelState_s {
    HU3D_MODELID baseModel;
    HU3D_MODELID effects[5];
    f32 positionsY[5];
    f32 motionValues[5];
} M625LightModelState;

typedef struct M625LightPreset {
    HU3D_LIGHTID light;
    Vec position;
    Vec direction;
    GXColor color;
} M625LightPreset;

extern M625WaveState lbl_1_bss_0;
extern M625LightModelState lbl_1_bss_194;
extern M625LightAsset lbl_1_data_108[10];
extern M625LightPreset lbl_1_data_180[2];
extern GXColor lbl_1_data_1C0;
extern f32 lbl_1_data_1C4[2][3];

#endif
