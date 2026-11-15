#ifndef REL_M634DLL_MODEL_OWNER_H
#define REL_M634DLL_MODEL_OWNER_H

#include <stddef.h>

#include <math.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "game/hu3d.h"
#ifdef __cplusplus
}
#endif

typedef struct M634ModelOwner {
    unsigned char objectData[0xA4];
    unsigned char mode;
    unsigned char modeData;
    HU3D_MODELID modelId;
    s16 characterModel;
    unsigned char modelData[0x2A];
    s8 pending;
    unsigned char pendingData[3];
    s32 state;
    unsigned char motionData[0x20]; /* Unreconstructed object contents. */
    u32 currentMotion;
    u8 motionSequence[0x10];

#ifdef __cplusplus
    void fn_1_21C4();
    void fn_1_21F8();
#endif
} M634ModelOwner;

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#endif
