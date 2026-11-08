/* Dunk Bros. camera, lighting, fog, audio and model services. */
#include <math.h>
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/object.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/hu3d.h"
#include "game/audio.h"
#include "game/data.h"
#include <string.h>
#include "REL/m699Dll.h"
#include "humath.h"

/* Fog object (lbl_1_bss_418): the layer table, a callback and the fog distance factor. */
typedef struct M699FogParam {
    s32 layer;
    u32 cameras;
    GXFogType type;
    float start, end;
    GXColor color;
} M699FogParam;

typedef struct M699FogWork {
    M699FogParam *parameters;
    OMOBJ_FUNC callback;
    float factor;
} M699FogWork;

/* Light object (lbl_1_bss_41C): the global light's id, a callback, its position, target,
 * direction and color. */
typedef struct M699LightWork {
    HU3D_LIGHTID lightId;
    OMOBJ_FUNC callback;
    Vec position, target, direction;
    GXColor color;
} M699LightWork;

/* Camera object (lbl_1_bss_414): shadow settings, a callback and one entry per camera. */
typedef struct M699CameraEntry {
    s32 active, motion, model;
    OM_CAMERA_VIEW view, previousView;
    Vec position, up, target;
    float fov, nearPlane, farPlane, aspect;
    float viewportX, viewportY, viewportWidth, viewportHeight;
    float scissorX, scissorY, scissorWidth, scissorHeight;
    u8 unk_98[0x20]; /* No code names this field. */
} M699CameraEntry;

typedef void (*M699CameraCallback)(OMOBJ *object, u32 camera, s32 index);

typedef struct M699CameraWork {
    u32 cameras;
    float shadowRadius, shadowFov;
    Vec shadowDirection;
    GXColor shadowColor;
    M699CameraCallback callback;
    M699CameraEntry entries[16];
    u8 unk_BA0[8]; /* No code names this field. */
} M699CameraWork;

/* Sound object (lbl_1_bss_410): per-camera pan/volume ranges and the playing effects. */
typedef struct AudioRange {
    u32 cameraMask;
    s32 panMin;
    s32 panMax;
    s32 volumeMin;
    s32 volumeMiddle;
    s32 volumeMax;
    float distanceFar;
    float distanceMiddle;
    float distanceNear;
} AudioRange;

typedef struct AudioRecord {
    s32 handle;
    s32 effect;
    u32 cameraMask;
    s32 panEnabled;
    s32 volumeEnabled;
    HuVecF *position;
} AudioRecord;

typedef struct AudioState {
    AudioRange *ranges;
    OMOBJ_FUNC callback;
    AudioRecord records[64];
} AudioState;

typedef struct CameraFitParams {
    Vec direction;
    Vec up;
    float fov;
    s32 width;
    s32 height;
    s32 marginX;
    s32 marginY;
} CameraFitParams;

typedef struct CameraFitSphere {
    Vec center;
    float radius;
} CameraFitSphere;

void fn_1_137C(OMOBJ *object);
void fn_1_1488(OMOBJ *object);
void fn_1_2A48(HuVecF *direction);
void fn_1_4BD4(void);
void fn_1_189C(OMOBJ *object);
void fn_1_18F4(s16 layer);
void fn_1_1DD4(OMOBJ *object);
void fn_1_1FE8(OMOBJ *object, u32 camera, s32 index);
void fn_1_20E0(OMOBJ *object, u32 camera, s32 index);
void fn_1_24C4(s32 camera, Vec *direction, Vec *target, float fov, float radius);
void fn_1_2918(u32 cameras, float x, float y, float width, float height);
void fn_1_2998(u32 cameras, float x, float y, float width, float height);
void fn_1_36C0(OMOBJ *object);
void fn_1_3728(OMOBJ *object);
void fn_1_3780(OMOBJ *object);
s32 fn_1_38BC(s32 cameras, float x);
s32 fn_1_3A04(u32 cameras, HuVecF *position);
s32 fn_1_3C24(u32 cameras, HuVecF *position);
s32 fn_1_3EBC(u32 cameras, s32 effect, HuVecF *position);
s32 fn_1_4390(u32 cameras, s32 effect, HuVecF *position);
s32 fn_1_4614(u32 cameras, s32 effect, HuVecF *position, s32 panEnabled, s32 volumeEnabled);
void fn_1_4B5C(u32 cameraMask, s32 effect, s32 handle, HuVecF *position, s32 panEnabled,
    s32 volumeEnabled);

void fn_1_12B8(OMOBJMAN *manager)
{
    OMOBJ *object = fn_8003AB38(manager, 22, 0, 0, -1, fn_1_137C);
    void *state;
    lbl_1_bss_420 = object;
    lbl_1_bss_41C = lbl_1_bss_418 = lbl_1_bss_414 = lbl_1_bss_410 = NULL;
    state = HuMemDirectMallocNum(HEAP_HEAP, 4, 0x10000000);
    object->data = state;
    memset(state, 0, 4);
    fn_1_4BD4();
}

void fn_1_1378(void)
{
}

void fn_1_137C(OMOBJ *object)
{
}

void fn_1_1380(OMOBJMAN *manager, HuVecF *position, HuVecF *target, GXColor *color)
{
    M699LightWork *work;
    OMOBJ *object;

    object = fn_8003AB38(manager, 24, 0, 0, -1, fn_1_1488);
    lbl_1_bss_41C = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M699LightWork), 0x10000000);
    object->data = work;
    memset(work, 0, sizeof(M699LightWork));
    work->position = *position;
    work->target = *target;
    work->color = *color;
    work->lightId = -1;
    PSVECSubtract(&work->target, &work->position, &work->direction);
    PSVECNormalize(&work->direction, &work->direction);
}

void fn_1_1488(OMOBJ *object)
{
    M699LightWork *work = lbl_1_bss_41C->data;
    if (work->callback != NULL) {
        work->callback(object);
    }
}

void fn_1_14E0(void)
{
    M699LightWork *work = lbl_1_bss_41C->data;

    work->lightId = fn_800286D0(&work->position, &work->direction, &work->color);
    Hu3DGLightInfinitytSet(work->lightId);
    Hu3DGLightStaticSet(work->lightId, 1);
    fn_1_2A48(&work->direction);
}

void fn_1_1544(s32 model, char *lightName)
{
    HU3D_LIGHT *globalLight;
    M699LightWork *work;

    work = lbl_1_bss_41C->data;
    globalLight = Hu3DGlobalLight;
    fn_80029850((s16)model, 1);
    if (lightName != NULL) {
        work->lightId = fn_80029D24((s16)model, lightName);
    } else {
        work->lightId = 0;
    }
    if (work->lightId >= 0) {
        fn_800297D4(work->lightId, &work->position, &work->direction, &work->color);
        fn_1_2A48(&work->direction);
    }
}

void fn_1_15F8(OMOBJ_FUNC callback)
{
    M699LightWork *work = lbl_1_bss_41C->data;
    work->callback = callback;
}

void fn_1_1620(HuVecF *position)
{
    M699LightWork *work = lbl_1_bss_41C->data;
    work->position = *position;
    Hu3DGLightPosSetV(work->lightId, &work->position, &work->direction);
}

void fn_1_1688(HuVecF *direction)
{
    M699LightWork *work = lbl_1_bss_41C->data;
    work->direction = *direction;
    Hu3DGLightPosSetV(work->lightId, &work->position, &work->direction);
}

Vec *fn_1_16F0(void)
{
    M699LightWork *work = lbl_1_bss_41C->data;
    return &work->direction;
}

void fn_1_1718(int red, int green, int blue, int alpha)
{
    M699LightWork *work = lbl_1_bss_41C->data;
    work->color.r = red;
    work->color.g = green;
    work->color.b = blue;
    work->color.a = alpha;
    Hu3DGLightColorSet(work->lightId, red, green, blue, alpha);
}

Vec lbl_1_data_E8 = { 0.0f, -1.0f, 0.0f };
/* fn_1_1A68 loads all four color bytes at .data F4..F7 (lbz at +0..+3).
 * The split's one-byte symbol and following three-byte gap form this GXColor. */
static GXColor lbl_1_data_F4 = { 0, 0, 0, 0 };

void fn_1_179C(OMOBJMAN *manager, M699FogParam *parameters)
{
    M699FogWork *work;
    s32 i;
    OMOBJ *object;
    object = fn_8003AB38(manager, 26, 0, 0, -1, fn_1_189C);
    lbl_1_bss_418 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M699FogWork), 0x10000000);
    object->data = work;
    memset(work, 0, sizeof(M699FogWork));
    work->parameters = parameters;
    work->factor = 1.0f;
    for (i = 0; parameters->layer != -1; i++, parameters++) {
        if (i == 0) Hu3DFogSet(parameters->start, parameters->end,
            parameters->color.r, parameters->color.g, parameters->color.b);
        fn_80025918(parameters->layer, fn_1_18F4);
    }
}

void fn_1_189C(OMOBJ *object)
{
    M699FogWork *work = lbl_1_bss_418->data;
    if (work->callback != NULL) {
        work->callback(object);
    }
}

void fn_1_18F4(s16 layer)
{
    M699FogWork *work;
    M699FogParam *parameters;
    float fov, nearPlane, farPlane;
    float end;
    work = lbl_1_bss_418->data;
    for (parameters = work->parameters; parameters->layer != -1; parameters++) {
        if (parameters->layer == layer) {
            fn_80027D8C(parameters->cameras, &fov, &nearPlane, &farPlane);
            end = parameters->start + work->factor * (parameters->end - parameters->start);
            GXSetFog(parameters->type, parameters->start, end, nearPlane, farPlane, parameters->color);
            break;
        }
    }
}

void fn_1_19E8(OMOBJ_FUNC callback)
{
    M699FogWork *work;

    if (lbl_1_bss_418 != NULL) {
        work = lbl_1_bss_418->data;
        work->callback = callback;
    }
}

void fn_1_1A28(f32 factor)
{
    M699FogWork *work;

    if (lbl_1_bss_418 != NULL) {
        work = lbl_1_bss_418->data;
        work->factor = factor;
    }
}

void fn_1_1A68(OMOBJMAN *manager, u32 cameras, float fov, float nearPlane, float farPlane,
    float shadowRadius, OM_CAMERA_VIEW *view)
{
    M699CameraEntry *entry;
    M699CameraWork *work;
    OMOBJ *object;
    s32 i;
    u32 bit;
    s32 count = 0;
    object = fn_8003AB38(manager, 28, 0, 0, -1, fn_1_1DD4);
    lbl_1_bss_414 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M699CameraWork), 0x10000000);
    object->data = work;
    memset(work, 0, sizeof(M699CameraWork));
    work->cameras = cameras;
    work->shadowRadius = shadowRadius;
    work->shadowFov = 20.0f;
    work->shadowColor = lbl_1_data_F4;
    if (lbl_1_bss_41C == NULL) {
        work->shadowDirection = lbl_1_data_E8;
    } else {
        work->shadowDirection = *fn_1_16F0();
    }
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            count = i + 1;
            entry = &work->entries[i];
            entry->view = *view;
            entry->previousView = *view;
            entry->position = view->center;
            entry->position.z -= 1000.0;
            entry->up.x = entry->up.z = 0.0f;
            entry->up.y = 1.0f;
            entry->target = view->center;
            entry->fov = fov;
            entry->nearPlane = nearPlane;
            entry->farPlane = farPlane;
            entry->aspect = 1.2f;
            entry->viewportX = entry->scissorX = 0.0f;
            entry->viewportY = entry->scissorY = 0.0f;
            entry->viewportWidth = entry->scissorWidth = 640.0f;
            entry->viewportHeight = entry->scissorHeight = 480.0f;
        }
    }
    Hu3DCameraCreate(cameras);
    Hu3DCameraPerspectiveSet(cameras, fov, nearPlane, farPlane, 1.2f);
    fn_8003E488(cameras, view);
    object = fn_8003AB38(manager, 29, 0, 0, -1, fn_8003DAA4);
    object->work[0] = count;
    fn_8003DAA4(object);
}

void fn_1_1DD4(OMOBJ *object)
{
    M699CameraWork *work = object->data;
    M699CameraEntry *entry;
    s32 index;
    u32 bit;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (work->cameras & bit) {
            entry = &work->entries[index];
            fn_80027CD0(bit, &entry->position, &entry->up, &entry->target);
            fn_80027D8C(bit, &entry->fov, &entry->nearPlane, &entry->farPlane);
            if (work->shadowRadius > 0.0) {
                fn_1_24C4(work->cameras, &work->shadowDirection, &entry->target,
                          work->shadowFov, work->shadowRadius);
                fn_8002AAB4(work->shadowColor.r, work->shadowColor.g,
                                 work->shadowColor.b);
            }
            if (work->entries[index].active) {
                fn_1_1FE8(object, bit, index);
            }
            if (work->callback != NULL) {
                work->callback(object, bit, index);
            }
            lbl_80266B18[index] = entry->view.center;
            lbl_80266A58[index] = entry->view.rot;
            lbl_80266BD8[index] = entry->view.zoom;
            Hu3DCameraPerspectiveSet(bit, entry->fov, entry->nearPlane, entry->farPlane,
                                     entry->aspect);
            Hu3DCameraViewportSet(bit, entry->viewportX, entry->viewportY, entry->viewportWidth,
                                  entry->viewportHeight, 0.0f, 1.0f);
            Hu3DCameraScissorSet(bit, (u32) entry->scissorX, (u32) entry->scissorY,
                                 (u32) entry->scissorWidth, (u32) entry->scissorHeight);
        }
    }
}

void fn_1_1FE8(OMOBJ *object, u32 camera, s32 index)
{
    M699CameraWork *work;
    M699CameraEntry *entry;

    work = object->data;
    entry = &work->entries[index];
    fn_1_20E0(object, camera, index);
    if (fn_8002D75C((s16)entry->model) != 0) {
        fn_800262D0((s16)entry->model);
        entry->previousView.center = lbl_80266B18[index];
        entry->previousView.rot = lbl_80266A58[index];
        entry->previousView.zoom = lbl_80266BD8[index];
        entry->active = 0;
    }
}

void fn_1_20E0(OMOBJ *object, u32 camera, s32 index)
{
    M699CameraEntry *entry;
    M699CameraWork *work = object->data;
    Vec delta;
    entry = &work->entries[index];
    lbl_80266B18[index] = entry->target;
    delta.x = entry->position.x - entry->target.x;
    delta.y = entry->position.y - entry->target.y;
    delta.z = entry->position.z - entry->target.z;
    lbl_80266A58[index].x = 180.0 * (atan2(-delta.y,
        sqrtf(delta.x * delta.x + delta.z * delta.z)) / M_PI);
    lbl_80266A58[index].y = 180.0 * (atan2(delta.x, delta.z) / M_PI);
    lbl_80266A58[index].z = 0.0f;
    lbl_80266BD8[index] = sqrtf(delta.z * delta.z + (delta.x * delta.x + delta.y * delta.y));
    entry->view.center = lbl_80266B18[index];
    entry->view.rot = lbl_80266A58[index];
    entry->view.zoom = lbl_80266BD8[index];
}



const Vec lbl_1_rodata_110[2] = { { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void fn_1_24C4(s32 camera, Vec *direction, Vec *target, float fov, float radius)
{
    Vec position, unit;
    const Vec *up = lbl_1_rodata_110;
    float distance;
    float radians;
    radians = M_PI * (fov / 2.0f) / 180.0;
    distance = radius / tanf(radians);
    PSVECNormalize(direction, &unit);
    PSVECScale(&unit, &position, distance);
    PSVECSubtract(target, &position, &position);
    if (fabs(unit.y) > 0.5) {
        up++;
    }
    fn_8002A3B0(fov, distance - radius, distance + radius, camera);
    fn_8002A674(&position, (Vec *)up, target, camera);
    fn_8002A8CC(480, camera);
    fn_8002A7D8(0.4f, camera);
}

int fn_1_2670(unsigned int mask)
{
    int bit = 0;

    while (bit < 16) {
        if (mask & 1) {
            return bit;
        }
        bit++;
        mask >>= 1;
    }
    return 0;
}

void fn_1_26B4(M699CameraCallback callback)
{
    M699CameraWork *work = lbl_1_bss_414->data;
    work->callback = callback;
}

void fn_1_26DC(u32 cameras, float aspect)
{
    OMOBJ *object = lbl_1_bss_414;
    M699CameraEntry *entry;
    s32 i;
    u32 bit;
    M699CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->aspect = aspect;
        }
    }
}

void fn_1_2750(u32 cameras, float x, float y, float width, float height)
{
    float extraWidth, extraHeight;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    extraWidth = 32.0 * (1.0 - width / 640.0f);
    extraHeight = 80.0 * (1.0 - height / 480.0f);
    if (x + 0.5 * width >= 320.0) {
        offsetX = -extraWidth;
    }
    if (y + 0.5 * height >= 240.0) {
        offsetY = -extraHeight;
    }
    fn_1_2918(cameras, x + offsetX, y + offsetY, width + extraWidth, height + extraHeight);
    fn_1_2998(cameras, x, y, width, height);
}

void fn_1_2918(u32 cameras, float x, float y, float width, float height)
{
    M699CameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_414;
    M699CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->viewportX = x;
            entry->viewportY = y;
            entry->viewportWidth = width;
            entry->viewportHeight = height;
        }
    }
}

void fn_1_2998(u32 cameras, float x, float y, float width, float height)
{
    M699CameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_414;
    M699CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->scissorX = x;
            entry->scissorY = y;
            entry->scissorWidth = width;
            entry->scissorHeight = height;
        }
    }
}

void fn_1_2A18(f32 fov)
{
    OMOBJ *object = lbl_1_bss_414;
    M699CameraWork *work = object->data;
    work->shadowFov = fov;
}

void fn_1_2A48(HuVecF *direction)
{
    OMOBJ *object = lbl_1_bss_414;
    if (object != NULL) {
        M699CameraWork *work = object->data;
        work->shadowDirection = *direction;
    }
}

Vec *fn_1_2A98(void)
{
    OMOBJ *object = lbl_1_bss_414;
    M699CameraWork *work = object->data;
    return &work->shadowDirection;
}

void fn_1_2AC8(u8 red, u8 green, u8 blue)
{
    OMOBJ *object = lbl_1_bss_414;
    M699CameraWork *work = object->data;
    work->shadowColor.r = red;
    work->shadowColor.g = green;
    work->shadowColor.b = blue;
}

void fn_1_2B00(u32 cameras, void *data)
{
    M699CameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_414;
    M699CameraWork *work = object->data;
    s32 motion = fn_8002C4C0(data);
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->motion = motion;
            entry->model = fn_800283F4(motion, bit);
            fn_800284C8(entry->model, bit);
            entry->active = 1;
        }
    }
}

void fn_1_2BB8(u32 cameraMask)
{
    u32 bit;
    OMOBJ *object;
    M699CameraWork *work;
    M699CameraEntry *entry;
    s32 index;

    object = lbl_1_bss_414;
    work = object->data;
    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if ((cameraMask & bit) != 0) {
            entry = &work->entries[index];
            if (entry->active != 0) {
                fn_1_20E0(object, bit, index);
                fn_800262D0((s16)entry->model);
                entry->previousView.center = lbl_80266B18[index];
                entry->previousView.rot = lbl_80266A58[index];
                entry->previousView.zoom = lbl_80266BD8[index];
                entry->active = 0;
            }
        }
    }
}

BOOL fn_1_3058(u32 cameras)
{
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_414;
    M699CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if ((cameras & bit) && work->entries[i].active) {
            return TRUE;
        }
    }
    return FALSE;
}

#define FIT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FIT_MAX(a, b) ((a) > (b) ? (a) : (b))

double fn_1_30DC(CameraFitParams *params, CameraFitSphere *spheres, s32 count, Vec *out)
{
    Mtx view;
    Mtx inverse;
    Vec lower;
    Vec upper;
    Vec origin = { 0.0f, 0.0f, 0.0f };
    double nearZ;
    double extentX;
    double extentY;
    double slopeX;
    double slopeY;
    double distanceX;
    double distanceY;
    double tangent;
    double distance;
    CameraFitSphere *sphere;
    s32 i;

    MTXLookAt(view, &origin, &params->up, &params->direction);
    MTXInverse(view, inverse);
    {
        double aspect = (double) params->width / params->height;
        tangent = tan(params->fov / 2.0f);
        slopeX = aspect * (tangent * ((double) (params->width - 2 * params->marginX) /
                                      params->width));
        slopeY = tangent * ((double) (params->height - 2 * params->marginY) /
                            params->height);
    }
    sphere = spheres;
    MTXMultVecSR(view, &sphere->center, &lower);
    upper = lower;
    nearZ = lower.z - sphere->radius;
    sphere++;
    for (i = 1; i < count; i++, sphere++) {
        Vec position;
        MTXMultVecSR(view, &sphere->center, &position);
        nearZ = FIT_MIN(nearZ, position.z - sphere->radius);
    }
    for (sphere = spheres, i = 0; i < count; i++, sphere++) {
        Vec position;
        MTXMultVecSR(view, &sphere->center, &position);
        extentX = sphere->radius + slopeX * (position.z - nearZ);
        extentY = sphere->radius + slopeY * (position.z - nearZ);
        lower.x = FIT_MIN(lower.x, position.x - extentX);
        lower.y = FIT_MIN(lower.y, position.y - extentY);
        upper.x = FIT_MAX(upper.x, position.x + extentX);
        upper.y = FIT_MAX(upper.y, position.y + extentY);
    }
    distanceX = ((upper.x - lower.x) / 2.0f) / slopeX;
    distanceY = ((upper.y - lower.y) / 2.0f) / slopeY;
    distance = FIT_MAX(distanceX, distanceY);
    if (distanceX > distanceY) {
        double height = (upper.x - lower.x) * (slopeY / slopeX);
        upper.y = lower.y + height;
    }
    {
        Vec center;
        Vec offset;
        center.x = (lower.x + upper.x) / 2.0f;
        center.y = (lower.y + upper.y) / 2.0f;
        center.z = nearZ;
        MTXMultVecSR(inverse, &center, &center);
        VECNormalize(&params->direction, &offset);
        VECScale(&offset, &offset, distance);
        VECSubtract(&center, &offset, out);
    }
    return distance;
}

void fn_1_3624(OMOBJMAN *manager, AudioRange *ranges)
{
    OMOBJ *object;
    AudioState *state;

    object = fn_8003AB38(manager, 30, 0, 0, -1, fn_1_36C0);
    lbl_1_bss_410 = object;
    state = HuMemDirectMallocNum(HEAP_HEAP, sizeof(AudioState), 0x10000000);
    object->data = state;
    memset(state, 0, sizeof(AudioState));
    state->ranges = ranges;
    fn_1_3728(object);
}

void fn_1_36C0(OMOBJ *object)
{
    AudioState *state = lbl_1_bss_410->data;

    fn_1_3780(object);
    if (state->callback != NULL) {
        state->callback(object);
    }
}

void fn_1_3728(OMOBJ *object)
{
    AudioState *state = lbl_1_bss_410->data;
    AudioRecord *record = state->records;
    s32 index;

    for (index = 0; index < 64; index++, record++) {
        record->handle = -1;
    }
}

void fn_1_3780(OMOBJ *object)
{
    AudioState *state;
    AudioRecord *record;
    s32 index;

    state = lbl_1_bss_410->data;
    record = state->records;
    for (index = 0; index < 64; index++, record++) {
        if (record->handle != -1) {
            if (HuAudFXStatusGet(record->handle) != 2) {
                record->handle = -1;
            } else {
                if (record->panEnabled) {
                    HuAudFXPanning(record->handle,
                        (s16)fn_1_3A04(record->cameraMask, record->position));
                }
                if (record->volumeEnabled) {
                    HuAudFXVolSet(record->handle,
                        (s16)fn_1_3C24(record->cameraMask, record->position));
                }
            }
        }
    }
}

void fn_1_3850(OMOBJ_FUNC callback)
{
    AudioState *state;

    if (lbl_1_bss_410 != NULL) {
        state = lbl_1_bss_410->data;
        state->callback = callback;
    }
}

s32 fn_1_3890(float x)
{
    return fn_1_38BC(1, x);
}

s32 fn_1_38BC(s32 cameras, float x)
{
    AudioState *state;
    AudioRange *range;
    s32 pan = 64;

    if (lbl_1_bss_410 != NULL) {
        state = lbl_1_bss_410->data;
        for (range = state->ranges; range->cameraMask; range++) {
            if (range->cameraMask & cameras) {
                x *= (range->panMax - range->panMin) / 576.0f;
                pan = range->panMin + x;
                if (pan < range->panMin) pan = range->panMin;
                if (pan > range->panMax) pan = range->panMax;
                break;
            }
        }
    }
    return pan;
}

s32 fn_1_39D8(HuVecF *position)
{
    return fn_1_3A04(1, position);
}

s32 fn_1_3A04(u32 cameras, HuVecF *position)
{
    M699CameraWork *work = lbl_1_bss_414->data;
    M699CameraEntry *camera = &work->entries[fn_1_2670(cameras)];
    HuVecF screen;
    float x;

    Hu3D3Dto2D(position, (s16)cameras, &screen);
    x = screen.x / 576.0f;
    x = 0.9f * camera->viewportX + x * (0.9f * camera->viewportWidth);
    return fn_1_38BC(cameras, x);
}

s32 fn_1_3BF8(HuVecF *position)
{
    return fn_1_3C24(1, position);
}

s32 fn_1_3C24(u32 cameras, HuVecF *position)
{
    AudioState *audio;
    AudioRange *range;
    M699CameraWork *work;
    M699CameraEntry *camera;
    Mtx matrix;
    HuVecF local;
    float distance;
    float factor;
    s32 volume = 127;

    if (lbl_1_bss_410 != NULL) {
        audio = lbl_1_bss_410->data;
        work = lbl_1_bss_414->data;
        camera = &work->entries[fn_1_2670(cameras)];
        for (range = audio->ranges; range->cameraMask; range++) {
            if (range->cameraMask & cameras) {
                C_MTXLookAt(matrix, &camera->position, &camera->up, &camera->target);
                PSMTXMultVec(matrix, position, &local);
                distance = -local.z;
                if (distance < range->distanceMiddle) {
                    factor = (range->distanceMiddle - distance) /
                             (range->distanceMiddle - range->distanceNear);
                    factor *= range->volumeMax - range->volumeMiddle;
                    volume = range->volumeMiddle + factor;
                } else {
                    factor = (distance - range->distanceMiddle) /
                             (range->distanceFar - range->distanceMiddle);
                    factor *= range->volumeMiddle - range->volumeMin;
                    volume = range->volumeMiddle - factor;
                }
                if (volume < range->volumeMin) volume = range->volumeMin;
                if (volume > range->volumeMax) volume = range->volumeMax;
                break;
            }
        }
    }
    return volume;
}

s32 fn_1_3E88(s32 effect, HuVecF *position)
{
    return fn_1_3EBC(1, effect, position);
}

s32 fn_1_3EBC(u32 cameras, s32 effect, HuVecF *position)
{
    s32 index;
    u32 bit;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayVolPan(effect, fn_1_3C24(bit, position),
                                       fn_1_3A04(bit, position));
        }
    }
    return handle;
}

s32 fn_1_435C(s32 effect, HuVecF *position)
{
    return fn_1_4390(1, effect, position);
}

s32 fn_1_4390(u32 cameras, s32 effect, HuVecF *position)
{
    s32 index;
    u32 bit;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayPan(effect, fn_1_3A04(bit, position));
        }
    }
    return handle;
}

s32 fn_1_45D0(s32 effect, HuVecF *position, s32 panEnabled, s32 volumeEnabled)
{
    return fn_1_4614(1, effect, position, panEnabled, volumeEnabled);
}

s32 fn_1_4614(u32 cameras, s32 effect, HuVecF *position, s32 panEnabled,
              s32 volumeEnabled)
{
    s32 index;
    u32 bit;
    s32 pan;
    s32 volume;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            if (panEnabled) pan = fn_1_3A04(bit, position);
            else pan = 64;
            if (volumeEnabled) volume = fn_1_3C24(bit, position);
            else volume = 127;
            handle = HuAudFXPlayVolPan(effect, volume, pan);
            fn_1_4B5C(bit, effect, handle, position, panEnabled, volumeEnabled);
        }
    }
    return handle;
}

void fn_1_4B10(s32 effect, s32 handle, HuVecF *position, s32 panEnabled, s32 volumeEnabled)
{
    fn_1_4B5C(1, effect, handle, position, panEnabled, volumeEnabled);
}

void fn_1_4B5C(u32 cameraMask, s32 effect, s32 handle, HuVecF *position, s32 panEnabled,
    s32 volumeEnabled)
{
    AudioState *state = lbl_1_bss_410->data;
    AudioRecord *record = state->records;
    s32 index;
    for (index = 0; index < 64; index++, record++) {
        if (record->handle == -1) {
            record->handle = handle;
            record->effect = effect;
            record->cameraMask = cameraMask;
            record->panEnabled = panEnabled;
            record->volumeEnabled = volumeEnabled;
            record->position = position;
            break;
        }
    }
}

void fn_1_4BD4(void)
{
    s32 index;

    index = 0;
    while (index < 80) {
        lbl_1_bss_2D0[index] = -1;
        lbl_1_bss_190[index] = -1;
        index += 1;
    }
}

s32 fn_1_4C2C(s32 dataNum)
{
    s32 index;
    s32 existing;
    if (dataNum != -1) {
        for (index = 0; index < 80; index++) {
            existing = lbl_1_bss_2D0[index];
            if (dataNum == existing) return index;
            if (existing == -1) {
                lbl_1_bss_2D0[index] = dataNum;
                return index;
            }
        }
    }
    return -1;
}

s32 fn_1_4CB0(s32 dataNum)
{
    s32 slot = fn_1_4C2C(dataNum);
    s32 model;
    if (slot >= 0) {
        model = lbl_1_bss_190[slot];
        if (model == -1) {
            model = fn_80025A04(fn_80008DD0(dataNum, 0x10000000, 2));
            lbl_1_bss_190[slot] = model;
            return model;
        }
        return fn_80025D4C(model);
    }
    return -1;
}

s32 fn_1_4DB8(s32 model, s32 dataNum)
{
    s32 slot = fn_1_4C2C(dataNum);
    s32 motion;

    if (slot >= 0) {
        motion = lbl_1_bss_190[slot];
        if (motion == -1) {
            motion = fn_80030458(model, fn_80008DD0(dataNum, 0x10000000, 2));
            lbl_1_bss_190[slot] = motion;
            return motion;
        }
        return motion;
    }
    return -1;
}

float fn_1_4EC8(float start, float end, float factor)
{
    float angle;

    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    angle = fmodf(start + factor * angle, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    return angle;
}

float fn_1_4FE4(float start, float end)
{
    float angle;
    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    return angle;
}

float fn_1_5098(HuVecF *from, HuVecF *to)
{
    float deltaX = to->x - from->x;
    float deltaZ = to->z - from->z;
    return atan2(deltaX, deltaZ) / M_PI * 180.0;
}

float fn_1_5134(Vec *first, Vec *second)
{
    float deltaZ;
    float deltaX;
    float degrees;
    float angle;
    float result;

    deltaX = second->x - first->x;
    deltaZ = second->z - first->z;
    degrees = (float)(180.0 *
                      (atan2((double)deltaX, (double)deltaZ) / M_PI));
    angle = degrees;
    result = angle;
    if (result < 0.0f) {
        result += 360.0f;
    }
    return result;
}

float fn_1_5230(Vec *first, Vec *second)
{
    float x = second->x - first->x;
    float z = second->z - first->z;
    return sqrtf(x * x + z * z);
}

void fn_1_539C(Vec *start, Vec *end, Vec *out, f32 t)
{
    out->x = start->x + (t * (end->x - start->x));
    out->y = start->y + (t * (end->y - start->y));
    out->z = start->z + (t * (end->z - start->z));
}

s32 fn_1_53F4(Vec *start, Vec *end, f32 gravity, f32 initialVerticalSpeed,
               Vec *velocity)
{
    f32 verticalDelta;
    f32 distance;
    f32 flightValue;
    f32 verticalSpeed;

    verticalDelta = end->y - start->y;
    if (verticalDelta > 0.0) {
        flightValue = (f32)((2.0 * (f64)gravity) *
                            (f64)verticalDelta);
        if (initialVerticalSpeed < flightValue) {
            initialVerticalSpeed = flightValue;
        }
    }

    velocity->x = end->x - start->x;
    velocity->y = initialVerticalSpeed;
    velocity->z = end->z - start->z;
    distance = PSVECMag(velocity);
    if (0.0 == distance) {
        if (0.0 == verticalDelta) {
            velocity->y = 0.0f;
        }
        return 1;
    }

    flightValue = 2.0 * (distance - verticalDelta);
    if (flightValue <= 0.0) {
        return 1;
    }

    verticalSpeed = (f32)sqrt((f64)((gravity * (distance * distance)) /
                                    flightValue));
    PSVECScale(velocity, velocity, verticalSpeed / distance);
    velocity->y = verticalSpeed;
    return (s32)(distance / verticalSpeed);
}

OMOBJ *lbl_1_bss_420;
OMOBJ *lbl_1_bss_41C;
OMOBJ *lbl_1_bss_418;
OMOBJ *lbl_1_bss_414;
OMOBJ *lbl_1_bss_410;
s32 lbl_1_bss_2D0[80];
s32 lbl_1_bss_190[80];
