/* Camera lookup helpers. */
#include <math.h>
#include "game/hu3d.h"

HU3D_CAMERA *fn_1_DADC(short mask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    int i;
    for (i = 0; i < 16; i++, camera++) {
        if (mask & (1U << i)) return camera;
    }
    return Hu3DCamera;
}

typedef struct CameraParam {
    int first, second;
    float value;
} CameraParam;

extern CameraParam lbl_1_data_5C8[];

CameraParam *fn_1_DB40(short mask)
{
    CameraParam *param = lbl_1_data_5C8;
    int i;
    for (i = 0; i < 16; i++, param++) {
        if (mask & (1U << i)) return param;
    }
    return lbl_1_data_5C8;
}
