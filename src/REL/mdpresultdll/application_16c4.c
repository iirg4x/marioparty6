#include <string.h>

#include <dolphin/gx/GXStruct.h>
#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;

typedef s16 HU3D_LIGHTID;
typedef struct omObj_s OMOBJ;
typedef struct MdResultCameraWork_s MDRESULT_CAMERA_WORK;
typedef void (*OMOBJ_FUNC)(OMOBJ *obj);
typedef void (*MDRESULT_CAMERA_CALLBACK)(OMOBJ *obj,
    MDRESULT_CAMERA_WORK *camera);

typedef struct MdResultVectorPair_s {
    HuVecF values[2];
} MDRESULT_VECTOR_PAIR;

struct MdResultCameraWork_s {
    OMOBJ *obj;
    HuVecF center;
    HuVecF targetCenter;
    HuVecF rot;
    HuVecF targetRot;
    float zoom;
    float targetZoom;
    MDRESULT_CAMERA_CALLBACK callback;
    s16 unk_40;
    s16 mode;
    float unk_44;
};

extern void *lbl_1_bss_0;
extern MDRESULT_CAMERA_WORK lbl_1_bss_12BC;
extern HU3D_LIGHTID lbl_1_bss_130E[5];
extern HuVecF Center;
extern HuVecF CRot;
extern float CZoom;

extern const float lbl_1_rodata_F4;
extern const float lbl_1_rodata_F8;
extern const float lbl_1_rodata_FC;
extern const float lbl_1_rodata_100;
extern const float lbl_1_rodata_104;
extern const float lbl_1_rodata_108;
extern const float lbl_1_rodata_10C;
extern const float lbl_1_rodata_110;
extern const float lbl_1_rodata_114;
extern const float lbl_1_rodata_118;
extern const float lbl_1_rodata_11C;
extern const float lbl_1_rodata_120;
extern const MDRESULT_VECTOR_PAIR lbl_1_rodata_124;
extern const MDRESULT_VECTOR_PAIR lbl_1_rodata_13C;
extern const GXColor lbl_1_rodata_154;

void Hu3DCameraCreate(int cameraBit);
void Hu3DCameraPerspectiveSet(int cameraBit, float fov, float near,
    float far, float aspect);
void Hu3DCameraViewportSet(int cameraBit, float x, float y, float width,
    float height, float near, float far);
void Hu3DCameraKill(int cameraBit);
HU3D_LIGHTID Hu3DGLightCreateV(HuVecF *position, HuVecF *direction,
    GXColor *color);
void Hu3DGLightInfinitytSet(HU3D_LIGHTID lightId);
void Hu3DGLightStaticSet(HU3D_LIGHTID lightId, BOOL isStatic);
void Hu3DGLightKill(HU3D_LIGHTID lightId);
OMOBJ *omAddObjEx(void *manager, s16 priority, u16 modelCount,
    u16 motionCount, s16 group, OMOBJ_FUNC callback);
void omDelObjEx(void *manager, OMOBJ *obj);
void omOutView(OMOBJ *obj);

float fn_1_1F8BC(float current, float target, float weight);
void fn_1_1FB50(HuVecF *current, const HuVecF *target, float weight);

void fn_1_16C4(MDRESULT_CAMERA_WORK *camera)
{
    memcpy(&camera->center, &camera->targetCenter, sizeof(HuVecF));
    memcpy(&camera->rot, &camera->targetRot, sizeof(HuVecF));
    camera->zoom = camera->targetZoom;
}

inline void fn_1_16C4(MDRESULT_CAMERA_WORK *camera);

void fn_1_1714(MDRESULT_CAMERA_WORK *camera)
{
    memcpy(&camera->targetCenter, &camera->center, sizeof(HuVecF));
    memcpy(&camera->targetRot, &camera->rot, sizeof(HuVecF));
    camera->targetZoom = camera->zoom;
}

inline void fn_1_1714(MDRESULT_CAMERA_WORK *camera);

void fn_1_1764(MDRESULT_CAMERA_WORK *camera, float weight)
{
    fn_1_1FB50(&camera->center, &camera->targetCenter, weight);
    fn_1_1FB50(&camera->rot, &camera->targetRot, weight);
    camera->zoom = fn_1_1F8BC(camera->zoom, camera->targetZoom, weight);
}

inline void fn_1_1764(MDRESULT_CAMERA_WORK *camera, float weight);

void fn_1_17D4(MDRESULT_CAMERA_CALLBACK callback)
{
    MDRESULT_CAMERA_WORK *camera = &lbl_1_bss_12BC;

    camera->callback = callback;
}

inline void fn_1_17D4(MDRESULT_CAMERA_CALLBACK callback);

void fn_1_17F4(OMOBJ *obj, MDRESULT_CAMERA_WORK *camera)
{
    if (camera->callback) {
        camera->callback(obj, camera);
    }
}

inline void fn_1_17F4(OMOBJ *obj, MDRESULT_CAMERA_WORK *camera);

void fn_1_1840(s16 mode)
{
    MDRESULT_CAMERA_WORK *camera = &lbl_1_bss_12BC;

    camera->mode = mode;
}

void fn_1_1860(OMOBJ *obj)
{
    MDRESULT_CAMERA_WORK *camera = &lbl_1_bss_12BC;

    fn_1_17F4(obj, camera);
    Center.x = camera->center.x;
    Center.y = camera->center.y;
    Center.z = camera->center.z;
    CRot.x = camera->rot.x;
    CRot.y = camera->rot.y;
    CRot.z = camera->rot.z;
    CZoom = camera->zoom;
    omOutView(obj);
}

void fn_1_1930(MDRESULT_CAMERA_CALLBACK callback)
{
    MDRESULT_CAMERA_WORK *camera = &lbl_1_bss_12BC;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_F4, lbl_1_rodata_F8,
        lbl_1_rodata_FC, lbl_1_rodata_100);
    Hu3DCameraViewportSet(1, lbl_1_rodata_104, lbl_1_rodata_104,
        lbl_1_rodata_108, lbl_1_rodata_10C, lbl_1_rodata_104,
        lbl_1_rodata_110);
    memset(camera, 0, sizeof(MDRESULT_CAMERA_WORK));
    camera->callback = callback;
    camera->center.x = lbl_1_rodata_104;
    camera->center.y = lbl_1_rodata_114;
    camera->center.z = lbl_1_rodata_118;
    camera->rot.x = lbl_1_rodata_11C;
    camera->rot.y = lbl_1_rodata_104;
    camera->rot.z = lbl_1_rodata_104;
    camera->zoom = lbl_1_rodata_120;
    camera->obj = omAddObjEx(lbl_1_bss_0, 0x100, 0, 0, -1, fn_1_1860);
}

inline void fn_1_1930(MDRESULT_CAMERA_CALLBACK callback);

void fn_1_1AA4(void)
{
    MDRESULT_CAMERA_WORK *camera = &lbl_1_bss_12BC;

    Hu3DCameraKill(1);
    if (camera->obj) {
        omDelObjEx(lbl_1_bss_0, camera->obj);
    }
    camera->obj = NULL;
}

inline void fn_1_1AA4(void);

void fn_1_1B00(void)
{
    MDRESULT_VECTOR_PAIR pos = lbl_1_rodata_124;
    MDRESULT_VECTOR_PAIR dir = lbl_1_rodata_13C;
    GXColor color = lbl_1_rodata_154;

    lbl_1_bss_130E[0] =
        Hu3DGLightCreateV(&pos.values[0], &dir.values[0], &color);
    Hu3DGLightInfinitytSet(lbl_1_bss_130E[0]);
    Hu3DGLightStaticSet(lbl_1_bss_130E[0], TRUE);
    lbl_1_bss_130E[1] =
        Hu3DGLightCreateV(&pos.values[1], &dir.values[1], &color);
    Hu3DGLightInfinitytSet(lbl_1_bss_130E[1]);
    Hu3DGLightStaticSet(lbl_1_bss_130E[1], TRUE);
}

inline void fn_1_1B00(void);

void fn_1_1C34(void)
{
    Hu3DGLightKill(lbl_1_bss_130E[0]);
    Hu3DGLightKill(lbl_1_bss_130E[1]);
}

inline void fn_1_1C34(void);
