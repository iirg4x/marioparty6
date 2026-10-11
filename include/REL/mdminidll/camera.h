/* View settings and transition callback for the minigame-mode menu camera. */
#ifndef REL_MDMINIDLL_CAMERA_H
#define REL_MDMINIDLL_CAMERA_H

#include "game/object.h"

typedef struct MDMinidllCameraState MDMinidllCameraState;
typedef void (*MDMinidllCameraCallback)(OMOBJ *cameraObject, MDMinidllCameraState *cameraState);

/* The view object copies the current values to camera zero on every update. */
struct MDMinidllCameraState {
    OMOBJ *outViewObj; /* Object that updates camera zero from these settings. */
    HuVecF center; /* Current camera target position, in world units. */
    HuVecF centerTarget; /* Destination target position for smoothing and transitions. */
    HuVecF rotation; /* Current camera rotation, in degrees. */
    HuVecF rotationTarget; /* Destination rotation, in degrees. */
    f32 zoom; /* Current distance from the camera target, in world units. */
    f32 zoomTarget; /* Destination distance used by camera smoothing. */
    MDMinidllCameraCallback callback; /* Optional camera behavior run before applying the view. */
    f32 transitionFrame; /* Elapsed updates in the current camera transition. */
    u8 unusedState[12]; /* Cleared on initialization; not accessed by the camera callbacks. */
};

extern MDMinidllCameraState lbl_1_bss_808;

#endif
