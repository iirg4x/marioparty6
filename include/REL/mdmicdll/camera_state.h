/* Camera state shared by microphone mode's object callbacks and setup screens. */
#ifndef REL_MDMICDLL_CAMERA_STATE_H
#define REL_MDMICDLL_CAMERA_STATE_H

#include "game/object.h"
#include "game/window.h"

typedef struct MDMicCameraWork MDMIC_CAMERA_WORK;
typedef void (*MDMIC_CAMERA_CALLBACK)(OMOBJ *object, MDMIC_CAMERA_WORK *camera);

struct MDMicCameraWork {
    OMOBJ *obj; /* Object that updates this camera. */
    HuVecF center; /* Current camera center. */
    HuVecF targetCenter; /* Center approached during the transition. */
    HuVecF rot; /* Current camera rotation in degrees. */
    HuVecF targetRot; /* Rotation approached during the transition. */
    f32 zoom; /* Current camera zoom. */
    f32 targetZoom; /* Zoom approached during the transition. */
    MDMIC_CAMERA_CALLBACK callback; /* Camera update routine for this screen. */
    f32 transitionFrame; /* Progress through the current camera transition. */
    u8 reservedState[12]; /* Additional camera state with no established use. */
};

extern MDMIC_CAMERA_WORK lbl_1_bss_9F4;
extern HUWINID lbl_1_bss_A44[4];
extern HU3D_LIGHTID lbl_1_bss_A4C[2];

#endif
