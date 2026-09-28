#define _MATH_H
#include "REL/mdminidll/camera.h"
#include "string.h"

void fn_1_2514(MDMinidllCameraState *work)
{
    memcpy(&work->center, &work->centerTarget, sizeof(HuVecF));
    memcpy(&work->rotation, &work->rotationTarget, sizeof(HuVecF));
    work->zoom = work->zoomTarget;
}

void fn_1_2564(MDMinidllCameraState *work)
{
    memcpy(&work->centerTarget, &work->center, sizeof(HuVecF));
    memcpy(&work->rotationTarget, &work->rotation, sizeof(HuVecF));
    work->zoomTarget = work->zoom;
}
