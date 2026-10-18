/* Solo menu camera framing and animated transitions between menu views. */
#ifndef REL_MDSINGDLL_CAMERA_H
#define REL_MDSINGDLL_CAMERA_H

#include "game/object.h"

typedef struct MdsingCamera MDSING_CAMERA;
typedef void (*MDSING_CAMERA_CALLBACK)(OMOBJ *obj, MDSING_CAMERA *camera);

struct MdsingCamera {
    /* Object that advances this camera once per scene update. */
    OMOBJ *obj;
    /* Current and target look-at centers, in world coordinates. */
    HuVecF center;
    HuVecF targetCenter;
    /* Current and target orbit angles, in degrees. */
    HuVecF rot;
    HuVecF targetRot;
    /* Current and target distance from the look-at center, in world units. */
    f32 zoom;
    f32 targetZoom;
    /* Active framing transition, invoked by the camera object's update. */
    MDSING_CAMERA_CALLBACK callback;
    /* Elapsed updates for transitions that interpolate from saved targets. */
    f32 transitionFrame;
    /* No field accesses by the Solo camera callbacks. */
    u8 trailingState[12];
};

typedef MDSING_CAMERA MDSING_CAMERA_WORK;
typedef MDSING_CAMERA MDCAMERA_WORK;
typedef MDSING_CAMERA MDMinidllCameraState;
typedef MDSING_CAMERA_CALLBACK MDCAMERA_CALLBACK;
typedef MDSING_CAMERA_CALLBACK MDMinidllCameraCallback;

extern MDSING_CAMERA lbl_1_bss_1348;

#endif
