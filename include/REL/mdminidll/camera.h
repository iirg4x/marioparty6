#ifndef REL_MDMINIDLL_CAMERA_H
#define REL_MDMINIDLL_CAMERA_H

#include "game/object.h"

typedef struct MDMinidllCameraState MDMinidllCameraState;
typedef void (*MDMinidllCameraCallback)(OMOBJ *, MDMinidllCameraState *);

/* Layout observed in the camera callbacks and the 80-byte initialization. */
struct MDMinidllCameraState {
    OMOBJ *outViewObj;
    HuVecF center;
    HuVecF centerTarget;
    HuVecF rotation;
    HuVecF rotationTarget;
    f32 zoom;
    f32 zoomTarget;
    MDMinidllCameraCallback callback;
    f32 transitionFrame;
    /* Retail storage +0x44..+0x4f: original fields/purpose are unknown. */
    u8 unknown_44[12];
};

extern MDMinidllCameraState lbl_1_bss_808;

#endif
