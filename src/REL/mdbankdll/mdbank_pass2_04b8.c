#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
typedef struct omObj_s OMOBJ;
typedef struct MdbankCameraWork MDBANK_CAMERA_WORK;
typedef void (*MDBANK_CAMERA_CALLBACK)(OMOBJ *object,
    MDBANK_CAMERA_WORK *camera);

struct MdbankCameraWork {
    OMOBJ *object;
    HuVecF center;
    HuVecF centerTarget;
    HuVecF rot;
    HuVecF rotTarget;
    float zoom;
    float zoomTarget;
    MDBANK_CAMERA_CALLBACK callback;
    s32 state[4];
};

extern MDBANK_CAMERA_WORK lbl_1_bss_1998;
extern HuVecF Center;
extern HuVecF CRot;
extern float CZoom;

void omOutView(OMOBJ *object);

void fn_1_4B8(OMOBJ *obj)
{
    MDBANK_CAMERA_WORK *camera = &lbl_1_bss_1998;

    if (camera->callback) {
        camera->callback(obj, camera);
    }
    Center.x = camera->center.x;
    Center.y = camera->center.y;
    Center.z = camera->center.z;
    CRot.x = camera->rot.x;
    CRot.y = camera->rot.y;
    CRot.z = camera->rot.z;
    CZoom = camera->zoom;
    omOutView(obj);
}
