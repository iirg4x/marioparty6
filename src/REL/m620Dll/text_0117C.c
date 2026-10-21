/* Camera, lighting, fog, audio, and player helpers used by Trap Ease Artist. */
#include "dolphin/math.h"

#include "dolphin/mtx.h"

#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "REL/m620Dll/module_types.h"

#include "game/audio.h"

#include "game/memory.h"

#include "datadir_enum.h"

#include <string.h>

extern s32 lbl_1_data_F18;

extern float lbl_1_bss_1BC;

extern s8 lbl_1_bss_218;

extern s8 lbl_1_bss_219;

extern s8 lbl_1_data_2758;

s32 fn_1_367CC(s32 *player);

extern OMOBJ *lbl_1_bss_2FC;

extern void fn_1_134C(OMOBJ *object);

extern OMOBJ *lbl_1_bss_300;

extern OMOBJ *lbl_1_bss_2F0;

extern OMOBJ *lbl_1_bss_2F4;

extern OMOBJ *lbl_1_bss_2F8;

void fn_1_1240(OMOBJ *object);

extern void fn_1_432C(void);

/* Creates the shared update object and clears the module's object slots during setup. */
void fn_1_117C(OMOBJMAN *manager)
{
    OMOBJ *object = omAddObjEx(manager, 22, 0, 0, OM_GRP_NONE, fn_1_1240);
    void *state;

    lbl_1_bss_300 = object;
    lbl_1_bss_2FC = lbl_1_bss_2F8 = lbl_1_bss_2F4 = lbl_1_bss_2F0 = NULL;
    state = HuMemDirectMallocNum(HEAP_HEAP, 4, HU_MEMNUM_OVL);
    object->data = state;
    memset(state, 0, 4);
    fn_1_432C();
}

void fn_1_123C(void)
{
}

void fn_1_1240(OMOBJ *object)
{
}

/* Creates a light object and stores its position, target, color, and direction. */
void fn_1_1244(OMOBJMAN *manager, HuVecF *position,
    HuVecF *target, GXColor *color)
{
    M620LightData *work;
    OMOBJ *object;

    object = omAddObjEx(manager, 24, 0, 0, OM_GRP_NONE, fn_1_134C);
    lbl_1_bss_2FC = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M620LightData), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M620LightData));
    work->position = *position;
    work->target = *target;
    work->color = *color;
    work->lightId = -1;
    PSVECSubtract(&work->target, &work->position, &work->direction);
    PSVECNormalize(&work->direction, &work->direction);
}

/* Runs the light object's optional callback on each object update. */
void fn_1_134C(OMOBJ *object)
{
    M620LightData *light = omObjGetDataAs(lbl_1_bss_2FC, M620LightData);
    if (light->callback != (OMOBJ_FUNC)(void *)0) {
        light->callback(object);
    }
}

extern void fn_1_2640(HuVecF *direction);

/* Creates a directional global light from the stored light data during setup. */
void fn_1_13A4(void)
{
    M620LightData *light = omObjGetDataAs(lbl_1_bss_2FC, M620LightData);

    light->lightId = Hu3DGLightCreateV(&light->position, &light->direction,
        &light->color);
    Hu3DGLightInfinitytSet(light->lightId);
    Hu3DGLightStaticSet(light->lightId, 1);
    fn_1_2640(&light->direction);
}

/* Imports a model's lights and selects the named light, or global slot 0 if unnamed. */
void fn_1_1408(s32 model, char *lightName)
{
    HU3D_LIGHT *globalLight;
    M620LightData *light = omObjGetDataAs(lbl_1_bss_2FC, M620LightData);

    globalLight = Hu3DGlobalLight;
    Hu3DModelLightInfoSet((s16)model, 1);
    if (lightName != NULL) {
        light->lightId = Hu3DModelLightIdGet((s16)model, lightName);
    } else {
        light->lightId = 0;
    }
    if (light->lightId >= 0) {
        Hu3DGLightParamGet(light->lightId, &light->position, &light->direction,
            &light->color);
        fn_1_2640(&light->direction);
    }
}

/* Sets the callback run by the light object's update function. */
void fn_1_14BC(OMOBJ_FUNC callback)
{
    M620LightData *light = omObjGetDataAs(lbl_1_bss_2FC, M620LightData);
    light->callback = callback;
}

/* Updates the stored light position and applies it to the selected global light. */
void fn_1_14E4(HuVecF *position) {
    M620LightData *light=omObjGetDataAs(lbl_1_bss_2FC,M620LightData);
    light->position=*position;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}

/* Updates the stored light direction and applies it to the selected global light. */
void fn_1_154C(HuVecF *direction) {
    M620LightData *light=omObjGetDataAs(lbl_1_bss_2FC,M620LightData);
    light->direction=*direction;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}

/* Returns a pointer to the stored global light direction. */
HuVecF *fn_1_15B4(void)
{
    M620LightData *light = omObjGetDataAs(lbl_1_bss_2FC, M620LightData);
    return &light->direction;
}

/* Stores and applies the selected global light's RGBA color. */
void fn_1_15DC(s32 red, s32 green, s32 blue, s32 alpha)
{
    M620LightData *light = omObjGetDataAs(lbl_1_bss_2FC, M620LightData);
    light->color.r = red;
    light->color.g = green;
    light->color.b = blue;
    light->color.a = alpha;
    Hu3DGLightColorSet(light->lightId, red, green, blue, alpha);
}

typedef struct M620FogParam_s {
    s32 layer;
    u32 cameras;
    GXFogType type;
    f32 start, end;
    GXColor color;
} M620FogParam;

typedef struct M620FogWork_s {
    M620FogParam *parameters;
    OMOBJ_FUNC callback;
    f32 factor;
} M620FogWork;

void fn_1_1760(OMOBJ *object);

void fn_1_17B8(s16 layer);

/* Creates the fog update object, applies the first fog entry, and hooks its layers. */
void fn_1_1660(OMOBJMAN *manager, M620FogParam *parameters)
{
    M620FogWork *work;
    s32 i;
    OMOBJ *object;

    object = omAddObjEx(manager, 26, 0, 0, OM_GRP_NONE, fn_1_1760);
    lbl_1_bss_2F8 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M620FogWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M620FogWork));
    work->parameters = parameters;
    work->factor = 1.0f;
    for (i = 0; parameters->layer != -1; i++, parameters++) {
        if (i == 0) {
            Hu3DFogSet(parameters->start, parameters->end, parameters->color.r,
                parameters->color.g, parameters->color.b);
        }
        Hu3DLayerHookSet(parameters->layer, fn_1_17B8);
    }
}

/* Runs the fog object's optional callback during its object update. */
void fn_1_1760(OMOBJ *object)
{
    M620FogWork *work = omObjGetDataAs(lbl_1_bss_2F8, M620FogWork);
    if (work->callback != (OMOBJ_FUNC)(void *)0) {
        work->callback(object);
    }
}

/* Each registered layer hook calls this during camera rendering to set its interpolated fog
 * range. */
void fn_1_17B8(s16 layer)
{
    M620FogWork *work;
    M620FogParam *parameters;
    f32 fov, nearPlane, farPlane;
    f32 end;

    work = lbl_1_bss_2F8->data;
    for (parameters = work->parameters; parameters->layer != -1; parameters++) {
        if (parameters->layer == layer) {
            Hu3DCameraPerspectiveGet(parameters->cameras, &fov, &nearPlane, &farPlane);
            end = parameters->start + work->factor * (parameters->end - parameters->start);
            GXSetFog(parameters->type, parameters->start, end, nearPlane, farPlane,
                parameters->color);
            break;
        }
    }
}

/* Sets the callback run by the fog object's update function. */
void fn_1_18AC(OMOBJ_FUNC callback)
{
    M620FogWork *work;

    if (lbl_1_bss_2F8 != NULL) {
        work = lbl_1_bss_2F8->data;
        work->callback = callback;
    }
}

/* Sets the scale used to interpolate each fog end distance during rendering. */
void fn_1_18EC(f32 factor)
{
    M620FogWork *work;

    if (lbl_1_bss_2F8 != NULL) {
        work = lbl_1_bss_2F8->data;
        work->factor = factor;
    }
}

#define M620_CAMERA_COUNT 16

typedef struct M620CameraEntry {
    s32 active, motion, model;
    OM_CAMERA_VIEW view;
    OM_CAMERA_VIEW previousView;
    Vec position, up, target;
    f32 fov, nearPlane, farPlane, aspect;
    f32 viewportX, viewportY, viewportWidth, viewportHeight;
    /* fn_1_BF0 uses the first eight bytes for its phase and frame count.
     * The remaining 24 bytes are not accessed by the camera callbacks. */
    u8 callbackStateBytes[32];
} M620CameraEntry;

typedef struct M620CameraWork {
    u32 cameras;
    f32 shadowRadius, shadowFov;
    Vec shadowDirection;
    GXColor shadowColor;
    void (*callback)(OMOBJ *, u32, s32);
    M620CameraEntry entries[M620_CAMERA_COUNT];
} M620CameraWork;

extern f32 CZoomM[16];

void fn_1_1C88(OMOBJ *object);

void fn_1_1EAC(OMOBJ *object, u32 camera, s32 index);

void fn_1_2320(Vec *direction, Vec *target, f32 fov, f32 radius);

void omOutViewMulti(OMOBJ *object);

/* Returns the stored light direction used to initialize camera shadow state. */
static inline Vec *m620_light_direction_192C(void)
{
    M620LightData *light = lbl_1_bss_2FC->data;
    return &light->direction;
}

/* Creates cameras for the selected views and initializes their perspective and update object. */
void fn_1_192C(OMOBJMAN *manager, u32 cameras, f32 fov, f32 nearPlane,
               f32 farPlane, f32 shadowRadius, OM_CAMERA_VIEW *view)
{
    M620CameraEntry *entry;
    M620CameraWork *work;
    OMOBJ *object;
    s32 i;
    u32 bit;
    s32 count = 0;

    object = omAddObjEx(manager, 28, 0, 0, OM_GRP_NONE, fn_1_1C88);
    lbl_1_bss_2F4 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M620CameraWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M620CameraWork));
    work->cameras = cameras;
    work->shadowRadius = shadowRadius;
    work->shadowFov = 20.0f;
    work->shadowColor = lbl_1_data_CC;
    if (lbl_1_bss_2FC == NULL) {
        work->shadowDirection = lbl_1_data_C0;
    } else {
        work->shadowDirection = *m620_light_direction_192C();
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
            entry->viewportX = 0.0f;
            entry->viewportY = 0.0f;
            entry->viewportWidth = 640.0f;
            entry->viewportHeight = 480.0f;
        }
    }
    Hu3DCameraCreate(cameras);
    Hu3DCameraPerspectiveSet(cameras, fov, nearPlane, farPlane, 1.2f);
    omCameraViewSetMulti((s16)cameras, view);
    object = omAddObjEx(manager, 29, 0, 0, OM_GRP_NONE, omOutViewMulti);
    object->work[0] = count;
    omOutViewMulti(object);
}

/* Refreshes selected camera state and applies its perspective, viewport, and scissor each frame. */
void fn_1_1C88(OMOBJ *object)
{
    M620CameraWork *work = object->data;
    M620CameraEntry *entry;
    s32 i, shadowDone = 0;
    u32 bit;

    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (work->cameras & bit) {
            entry = &work->entries[i];
            Hu3DCameraPosGet(bit, &entry->position, &entry->up, &entry->target);
            Hu3DCameraPerspectiveGet(bit, &entry->fov, &entry->nearPlane, &entry->farPlane);
            if (shadowDone == 0) {
                if (work->shadowRadius > 0.0) {
                    fn_1_2320(&work->shadowDirection, &entry->target,
                              work->shadowFov, work->shadowRadius);
                    Hu3DShadowColSet(work->shadowColor.r, work->shadowColor.g,
                                     work->shadowColor.b);
                }
                shadowDone = 1;
            }
            if (work->entries[i].active) {
                fn_1_1EAC(object, bit, i);
            } else {
                if (work->callback != NULL) {
                    work->callback(object, bit, i);
                }
                CenterM[i] = entry->view.center;
                CRotM[i] = entry->view.rot;
                CZoomM[i] = entry->view.zoom;
            }
            Hu3DCameraPerspectiveSet(bit, entry->fov, entry->nearPlane,
                                     entry->farPlane, entry->aspect);
            Hu3DCameraViewportSet(bit, entry->viewportX, entry->viewportY,
                                  entry->viewportWidth, entry->viewportHeight,
                                  0.0f, 1.0f);
            Hu3DCameraScissorSet(bit, (u32)entry->viewportX,
                                 (u32)entry->viewportY,
                                 (u32)entry->viewportWidth,
                                 (u32)entry->viewportHeight);
        }
    }
}

/* Derives a camera view from its position and target while a camera motion is active. */
void fn_1_1EAC(OMOBJ *object, u32 camera, s32 index)
{
    M620CameraWork *work = object->data;
    M620CameraEntry *entry = &work->entries[index];
    Vec delta;

    CenterM[index] = entry->target;
    delta.x = entry->position.x - entry->target.x;
    delta.y = entry->position.y - entry->target.y;
    delta.z = entry->position.z - entry->target.z;
    CRotM[index].x = 180.0 * (atan2(-delta.y,
        sqrtf(delta.x * delta.x + delta.z * delta.z)) / M_PI);
    CRotM[index].y = 180.0 * (atan2(delta.x, delta.z) / M_PI);
    CRotM[index].z = 0.0f;
    CZoomM[index] = sqrtf(delta.z * delta.z + (delta.x * delta.x + delta.y * delta.y));
    entry->view.center = CenterM[index];
    entry->view.rot = CRotM[index];
    entry->view.zoom = CZoomM[index];
    if (Hu3DMotionEndCheck(entry->model)) {
        Hu3DModelKill(entry->model);
        entry->previousView.center = CenterM[index];
        entry->previousView.rot = CRotM[index];
        entry->previousView.zoom = CZoomM[index];
        entry->active = 0;
    }
}

/* Uses the double-precision tangent operation needed by the shadow-camera calculation. */
static inline float tanf_inlined(float value)
{
    return (float)tan(value);
}

static const Vec lbl_1_rodata_B0[2] = {
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f }
};

/* Positions the shadow camera opposite the supplied direction at
* radius / tan(fov / 2) from the target. */
void fn_1_2320(Vec *direction, Vec *target, f32 fov, f32 radius)
{
    Vec position, unit;
    Vec *up = (Vec *)lbl_1_rodata_B0;
    f32 distance;
    f32 radians;

    radians = 3.141592653589793 * (fov / 2.0f) / 180.0;
    distance = radius / tanf_inlined(radians);
    PSVECNormalize(direction, &unit);
    PSVECScale(&unit, &position, distance);
    PSVECSubtract(target, &position, &position);
    if (fabs(unit.y) > 0.5) {
        up++;
    }
    Hu3DShadowCreate(fov, distance - radius, distance + radius);
    Hu3DShadowPosSet(&position, (Vec *)up, target);
    Hu3DShadowSizeSet(480);
    Hu3DShadowTPLvlSet(0.4f);
}

typedef void (*M620CameraCallback)(OMOBJ *, u32, s32);

typedef struct M620CameraStatePrefix {
    u32 cameraMask;
    f32 shadowRadius, shadowFov;
    HuVecF lightDirection;
    GXColor shadowColor;
    M620CameraCallback callback;
} M620CameraStatePrefix;

/* Returns the lowest selected camera index, or 0 when the mask is empty. */
int fn_1_24B0(u32 cameraMask)
{
    int bit = 0;

    while (bit < 16) {
        if (cameraMask & 1) {
            return bit;
        }
        bit++;
        cameraMask >>= 1;
    }
    return 0;
}

/* Sets the callback invoked when a selected camera is not using a camera motion. */
void fn_1_24F4(M620CameraCallback callback)
{
    M620CameraStatePrefix *state = omObjGetDataAs(lbl_1_bss_2F4, M620CameraStatePrefix);
    state->callback = callback;
}

/* Sets the aspect ratio for every camera selected by the mask. */
void fn_1_251C(u32 cameras, float aspect)
{
    OMOBJ *object = lbl_1_bss_2F4;
    M620CameraEntry *entry;
    s32 i;
    u32 bit;
    M620CameraWork *work = object->data;

    for (i = 0, bit = 1; i < M620_CAMERA_COUNT; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->aspect = aspect;
        }
    }
}

/* Sets the viewport rectangle for every camera selected by the mask. */
void fn_1_2590(u32 cameras, float x, float y, float width, float height)
{
    OMOBJ *object = lbl_1_bss_2F4;
    M620CameraWork *work = object->data;
    s32 index;
    u32 bit;

    for (index = 0, bit = 1; index < M620_CAMERA_COUNT; index++, bit <<= 1) {
        if (cameras & bit) {
            M620CameraEntry *entry = &work->entries[index];
            entry->viewportX = x;
            entry->viewportY = y;
            entry->viewportWidth = width;
            entry->viewportHeight = height;
        }
    }
}

/* Set the camera shadow field of view through the light-data view of camera work. */
void fn_1_2610(f32 x)
{
    OMOBJ *object = lbl_1_bss_2F4;
    M620LightData *light = object->data;
    light->position.x = x;
}

/* Copies a light direction into camera state when the camera object exists. */
void fn_1_2640(HuVecF *direction)
{
    OMOBJ *object = lbl_1_bss_2F4;
    if (object != NULL) {
        M620CameraStatePrefix *state = omObjGetDataAs(object, M620CameraStatePrefix);
        state->lightDirection = *direction;
    }
}

/* Return a pointer to the camera shadow direction's X coordinate through the light-data view. */
f32 *fn_1_2690(void)
{
    OMOBJ *object = lbl_1_bss_2F4;
    M620LightData *light = object->data;
    return &light->position.y;
}

typedef struct M620ShadowColorData {
    u32 cameras;
    f32 shadowRadius;
    f32 shadowFov;
    Vec shadowDirection;
    GXColor shadowColor;
} M620ShadowColorData;

/* Updates the stored RGB shadow color. */
void fn_1_26C0(u8 red, u8 green, u8 blue)
{
    OMOBJ *object = lbl_1_bss_2F4;
    M620ShadowColorData *work = object->data;
    work->shadowColor.r = red;
    work->shadowColor.g = green;
    work->shadowColor.b = blue;
}

/* Creates and starts a camera motion model for each selected camera. */
void fn_1_26F8(u32 cameras, void *motionData)
{
    OMOBJ *object = lbl_1_bss_2F4;
    M620CameraWork *work = object->data;
    s32 motion = Hu3DMotionCreate(motionData);
    s32 index;
    u32 bit;

    for (index = 0, bit = 1; index < M620_CAMERA_COUNT; index++, bit <<= 1) {
        if (cameras & bit) {
            M620CameraEntry *entry = &work->entries[index];
            entry->motion = motion;
            entry->model = Hu3DModelCameraCreate(motion, bit);
            Hu3DCameraMotionStart(entry->model, bit);
            entry->active = 1;
        }
    }
}

/* Reports whether any selected camera has an active camera motion. */
s32 fn_1_27B0(u32 cameras)
{
    OMOBJ *object = lbl_1_bss_2F4;
    M620CameraWork *work = object->data;
    s32 index;
    u32 bit;

    for (index = 0, bit = 1; index < M620_CAMERA_COUNT; index++, bit <<= 1) {
        if (cameras & bit) {
            if (work->entries[index].active != 0) {
                return 1;
            }
        }
    }
    return 0;
}

typedef struct M620CameraFitParams {
    Vec direction;
    Vec up;
    f32 fov;
    s32 width;
    s32 height;
    s32 marginX;
    s32 marginY;
} M620CameraFitParams;

typedef struct M620CameraFitSphere {
    Vec center;
    f32 radius;
} M620CameraFitSphere;

#define M620_FIT_MIN(a, b) ((a) < (b) ? (a) : (b))

#define M620_FIT_MAX(a, b) ((a) > (b) ? (a) : (b))

/* Fits a view around the supplied spheres and returns its camera distance. */
double fn_1_2834(M620CameraFitParams *params, M620CameraFitSphere *spheres,
                s32 count, Vec *out)
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
    M620CameraFitSphere *sphere;
    s32 index;

    MTXLookAt(view, &origin, &params->up, &params->direction);
    MTXInverse(view, inverse);
    {
        double aspect = (double)params->width / params->height;
        tangent = tan(params->fov / 2.0f);
        slopeX = aspect * (tangent *
            ((double)(params->width - 2 * params->marginX) / params->width));
        slopeY = tangent *
            ((double)(params->height - 2 * params->marginY) / params->height);
    }
    sphere = spheres;
    MTXMultVecSR(view, &sphere->center, &lower);
    upper = lower;
    nearZ = lower.z - sphere->radius;
    sphere++;
    for (index = 1; index < count; index++, sphere++) {
        Vec position;
        MTXMultVecSR(view, &sphere->center, &position);
        nearZ = M620_FIT_MIN(nearZ, position.z - sphere->radius);
    }
    for (sphere = spheres, index = 0; index < count; index++, sphere++) {
        Vec position;
        MTXMultVecSR(view, &sphere->center, &position);
        extentX = sphere->radius + slopeX * (position.z - nearZ);
        extentY = sphere->radius + slopeY * (position.z - nearZ);
        lower.x = M620_FIT_MIN(lower.x, position.x - extentX);
        lower.y = M620_FIT_MIN(lower.y, position.y - extentY);
        upper.x = M620_FIT_MAX(upper.x, position.x + extentX);
        upper.y = M620_FIT_MAX(upper.y, position.y + extentY);
    }
    distanceX = ((upper.x - lower.x) / 2.0f) / slopeX;
    distanceY = ((upper.y - lower.y) / 2.0f) / slopeY;
    distance = M620_FIT_MAX(distanceX, distanceY);
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

#undef M620_FIT_MIN

#undef M620_FIT_MAX

typedef struct M620AudioRecord {
    s32 handle;
    s32 effect;
    u32 cameraMask;
    s32 panEnabled;
    s32 volumeEnabled;
    Vec *position;
} M620AudioRecord;

typedef struct M620AudioState {
    void *userData;
    OMOBJ_FUNC callback;
    M620AudioRecord records[64];
} M620AudioState;

typedef struct M620AudioConfig {
    u32 cameras;
    s32 panMin;
    s32 panMax;
    s32 volumeMin;
    s32 volumeMiddle;
    s32 volumeMax;
    f32 distanceFar;
    f32 distanceMiddle;
    f32 distanceNear;
} M620AudioConfig;

typedef struct M620AudioWork {
    M620AudioConfig *config;
} M620AudioWork;

void fn_1_2E18(OMOBJ *object);

void fn_1_2E80(OMOBJ *object);

void fn_1_2ED8(OMOBJ *object);

s32 fn_1_315C(u32 cameras, Vec *position);

s32 fn_1_337C(u32 cameras, Vec *position);

void fn_1_42B4(u32 cameras, s32 effect, s32 handle, Vec *position, s32 panEnabled,
               s32 volumeEnabled);

/* Creates the audio object, clears its sound records, and stores its config pointer. */
void fn_1_2D7C(HUPROCESS *process, u32 userData)
{
    OMOBJ *object;
    u32 *data;

    object = omAddObjEx(process, 30, 0, 0, OM_GRP_NONE, fn_1_2E18);
    lbl_1_bss_2F0 = object;
    data = HuMemDirectMallocNum(HEAP_HEAP, 1544, HU_MEMNUM_OVL);
    object->data = data;
    memset(data, 0, 1544);
    *data = userData;
    fn_1_2E80(object);
}

/* Updates active sound positioning, then runs the audio object's optional callback. */
void fn_1_2E18(OMOBJ *object)
{
    M620AudioState *state = omObjGetDataAs(lbl_1_bss_2F0, M620AudioState);

    fn_1_2ED8(object);
    if (state->callback != NULL) {
        state->callback(object);
    }
}

/* Marks all sound-record slots as free when the audio object is initialized. */
void fn_1_2E80(OMOBJ *object)
{
    M620AudioState *state = omObjGetDataAs(lbl_1_bss_2F0, M620AudioState);
    M620AudioRecord *record = state->records;
    s32 index;

    for (index = 0; index < 64; index++, record++) {
        record->handle = -1;
    }
}

/* Reclaims finished sounds and refreshes the pan or volume of active tracked sounds. */
void fn_1_2ED8(OMOBJ *object)
{
    M620AudioState *state;
    M620AudioRecord *record;
    s32 index;

    state = lbl_1_bss_2F0->data;
    record = state->records;
    for (index = 0; index < 64; index++, record++) {
        if (record->handle != -1) {
            if (HuAudFXStatusGet(record->handle) != 2) {
                record->handle = -1;
            } else {
                if (record->panEnabled) {
                    HuAudFXPanning(record->handle,
                        (s16)fn_1_315C(record->cameraMask, record->position));
                }
                if (record->volumeEnabled) {
                    HuAudFXVolSet(record->handle,
                        (s16)fn_1_337C(record->cameraMask, record->position));
                }
            }
        }
    }
}

/* Sets the callback run by the audio object's update function. */
void fn_1_2FA8(OMOBJ_FUNC callback)
{
    M620AudioState *state;

    if (lbl_1_bss_2F0 != NULL) {
        state = lbl_1_bss_2F0->data;
        state->callback = callback;
    }
}

s32 fn_1_3014(u32 cameras, f32 x);

/* Converts a screen X coordinate to the default camera's configured pan range. */
s32 fn_1_2FE8(f32 value)
{
    return fn_1_3014(1, value);
}

/* fn_1_2FE8 calls this for camera 0 pan; it maps screen X to the first range and clamps it. */
s32 fn_1_3014(u32 cameras, f32 x)
{
    M620AudioState *work;
    M620AudioConfig *config;
    s32 pan = 64;

    if (lbl_1_bss_2F0 != NULL) {
        work = lbl_1_bss_2F0->data;
        for (config = work->userData; config->cameras; config++) {
            if (config->cameras & cameras) {
                x *= (config->panMax - config->panMin) / 576.0f;
                pan = config->panMin + x;
                if (pan < config->panMin) {
                    pan = config->panMin;
                }
                if (pan > config->panMax) {
                    pan = config->panMax;
                }
                break;
            }
        }
    }
    return pan;
}

/* Finds the first camera selected by the mask, defaulting to camera 0. */
static inline s32 m620_camera_index(u32 cameras)
{
    s32 index;

    for (index = 0; index < 16; index++, cameras >>= 1) {
        if (cameras & 1) return index;
    }
    return 0;
}

/* Maps a screen X coordinate into a selected camera's pan range. */
static inline s32 m620_pan_from_screen_x(u32 cameras, f32 x)
{
    M620AudioWork *audio;
    M620AudioConfig *config;
    s32 pan = 64;

    if (lbl_1_bss_2F0 != NULL) {
        audio = lbl_1_bss_2F0->data;
        for (config = audio->config; config->cameras; config++) {
            if (config->cameras & cameras) {
                x *= (config->panMax - config->panMin) / 576.0f;
                pan = config->panMin + x;
                if (pan < config->panMin) pan = config->panMin;
                if (pan > config->panMax) pan = config->panMax;
                break;
            }
        }
    }
    return pan;
}

/* Projects a world position and maps its horizontal screen coordinate to pan. */
static inline s32 m620_camera_pan(u32 cameras, Vec *position)
{
    M620CameraWork *work = lbl_1_bss_2F4->data;
    M620CameraEntry *camera = &work->entries[m620_camera_index(cameras)];
    Vec screen;
    f32 x;

    Hu3D3Dto2D(position, (s16)cameras, &screen);
    x = screen.x / 576.0f;
    x = 0.9f * camera->viewportX + x * (0.9f * camera->viewportWidth);
    return m620_pan_from_screen_x(cameras, x);
}

/* Scales sound volume by camera-space depth using the selected camera's config. */
static inline s32 m620_camera_volume(u32 cameras, Vec *position)
{
    M620AudioWork *audio;
    M620AudioConfig *config;
    M620CameraWork *work;
    M620CameraEntry *camera;
    Mtx matrix;
    Vec local;
    f32 distance, factor;
    s32 volume = 127;

    if (lbl_1_bss_2F0 != NULL) {
        audio = lbl_1_bss_2F0->data;
        work = lbl_1_bss_2F4->data;
        camera = &work->entries[m620_camera_index(cameras)];
        for (config = audio->config; config->cameras; config++) {
            if (config->cameras & cameras) {
                C_MTXLookAt(matrix, &camera->position, &camera->up, &camera->target);
                PSMTXMultVec(matrix, position, &local);
                distance = -local.z;
                if (distance < config->distanceMiddle) {
                    factor = (config->distanceMiddle - distance) /
                             (config->distanceMiddle - config->distanceNear);
                    factor *= config->volumeMax - config->volumeMiddle;
                    volume = config->volumeMiddle + factor;
                } else {
                    factor = (distance - config->distanceMiddle) /
                             (config->distanceFar - config->distanceMiddle);
                    factor *= config->volumeMiddle - config->volumeMin;
                    volume = config->volumeMiddle - factor;
                }
                if (volume < config->volumeMin) volume = config->volumeMin;
                if (volume > config->volumeMax) volume = config->volumeMax;
                break;
            }
        }
    }
    return volume;
}

/* Compute camera-0 pan; the character-sound caller uses the call's result
 * despite this void declaration. */
void fn_1_3130(void *object)
{
    fn_1_315C(1, object);
}

/* Returns the projected pan for a position and camera mask. */
s32 fn_1_315C(u32 cameras, Vec *position)
{
    return m620_camera_pan(cameras, position);
}

/* Compute camera-0 depth volume; the character-sound caller uses the result
 * despite this void declaration. */
void fn_1_3350(void *object)
{
    fn_1_337C(1, object);
}

/* Returns the depth-scaled volume for a position and camera mask. */
s32 fn_1_337C(u32 cameras, Vec *position)
{
    return m620_camera_volume(cameras, position);
}

s32 fn_1_3614(u32 cameras, s32 effect, Vec *position);

s32 fn_1_3AE8(s32 cameras, s32 effect, Vec *position);

/* Plays a sound with camera-0 pan and volume. */
s32 fn_1_35E0(s32 effect, Vec *position)
{
    return fn_1_3614(1, effect, position);
}

/* Plays the sound once for each selected camera using its pan and depth volume. */
s32 fn_1_3614(u32 cameras, s32 effect, Vec *position)
{
    s32 index;
    u32 bit;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayVolPan(effect, m620_camera_volume(bit, position),
                                       m620_camera_pan(bit, position));
        }
    }
    return handle;
}

/* Plays a sound with camera-0 pan and the default volume. */
void fn_1_3AB4(s32 effect, Vec *position)
{
    fn_1_3AE8(1, effect, position);
}

/* Plays the sound once for each selected camera using its projected pan. */
s32 fn_1_3AE8(s32 cameras, s32 effect, Vec *position)
{
    s32 index;
    u32 bit;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayPan(effect, m620_camera_pan(bit, position));
        }
    }
    return handle;
}

s32 fn_1_3D6C(s32 cameras, s32 effect, Vec *position, s32 panEnabled,
              s32 volumeEnabled);

/* Starts a camera-0 sound and tracks it for requested pan and volume updates. */
void fn_1_3D28(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled)
{
    fn_1_3D6C(1, effect, position, panEnabled, volumeEnabled);
}

/* Starts and tracks one positioned sound for each selected camera. */
s32 fn_1_3D6C(s32 cameras, s32 effect, Vec *position, s32 panEnabled,
               s32 volumeEnabled)
{
    s32 index;
    u32 bit;
    s32 pan;
    s32 volume;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            if (panEnabled) pan = m620_camera_pan(bit, position);
            else pan = 64;
            if (volumeEnabled) volume = m620_camera_volume(bit, position);
            else volume = 127;
            handle = HuAudFXPlayVolPan(effect, volume, pan);
            fn_1_42B4(bit, effect, handle, position, panEnabled, volumeEnabled);
        }
    }
    return handle;
}

typedef struct M620AudioRecordView {
    s32 handle;
    s32 effect;
    u32 cameraMask;
    s32 panEnabled;
    s32 volumeEnabled;
    Vec *position;
} M620AudioRecordView;

typedef struct M620AudioStateView {
    void *userData;
    OMOBJ_FUNC callback;
    M620AudioRecordView records[64];
} M620AudioStateView;

extern int lbl_1_bss_70[80];

extern int lbl_1_bss_1B0[80];

void fn_1_502C(OMOBJ *object);

extern OMOBJ *lbl_1_bss_330;

extern OMOBJ *lbl_1_bss_320[4];

void fn_1_4BD0(OMOBJ *object);

void fn_1_4B70(OMOBJ *object);

void fn_1_4D68(OMOBJ *object);

void fn_1_10D4(s32 player, s32 character, s32 charNo);

extern s32 lbl_1_bss_30C;

extern void *lbl_1_bss_310[4];

extern s32 lbl_1_bss_308;

extern void fn_1_4C98(void *state);

void fn_1_6314(void *state);

void fn_1_5CC4(void *state);

s32 fn_1_516C(void *state);

s32 fn_1_52CC(void *state);

s32 fn_1_5724(void *state);

s32 fn_1_5A50(void *state);

void fn_1_5CEC(OMOBJ *object);

void fn_1_5B60(OMOBJ *object);

void fn_1_5C68(OMOBJ *object);

/* Tracks an existing sound using the single-camera record. */
void fn_1_4268(s32 effect, s32 handle, Vec *position, s32 panEnabled,
               s32 volumeEnabled)
{
    fn_1_42B4(1, effect, handle, position, panEnabled, volumeEnabled);
}

/* Stores an active positioned sound in the first free tracking record. */
void fn_1_42B4(u32 cameras, s32 effect, s32 handle, Vec *position,
               s32 panEnabled, s32 volumeEnabled)
{
    M620AudioStateView *state = lbl_1_bss_2F0->data;
    M620AudioRecordView *record = state->records;
    s32 index;

    for (index = 0; index < 64; index++, record++) {
        if (record->handle == -1) {
            record->handle = handle;
            record->effect = effect;
            record->cameraMask = cameras;
            record->panEnabled = panEnabled;
            record->volumeEnabled = volumeEnabled;
            record->position = position;
            break;
        }
    }
}

/* Clears the model and motion cache slots when the shared manager is created. */
void fn_1_432C(void)
{
    s32 index;

    index = 0;
    while (index < 80) {
        lbl_1_bss_1B0[index] = -1;
        lbl_1_bss_70[index] = -1;
        index += 1;
    }
}

/* Finds a data entry in the cache or stores it in the first free slot. */
int fn_1_4384(int dataNum)
{
    int index;
    int existing;

    if (dataNum != -1) {
        for (index = 0; index < 80; index++) {
            existing = lbl_1_bss_1B0[index];
            if (dataNum == existing) return index;
            if (existing == -1) {
                lbl_1_bss_1B0[index] = dataNum;
                return index;
            }
        }
    }
    return -1;
}

/* Creates a model for a cached data entry, or links the existing model. */
int fn_1_4408(int dataNum)
{
    int slot = fn_1_4384(dataNum);
    int model;

    if (slot >= 0) {
        model = lbl_1_bss_70[slot];
        if (model == -1) {
            model = Hu3DModelCreateData(dataNum);
            lbl_1_bss_70[slot] = model;
            return model;
        }
        return Hu3DModelLink(model);
    }
    return -1;
}

/* Finds a data entry in the shared model and motion cache or claims a free slot. */
static inline int m620_local_4510(int dataNum)
{
    int index;
    int existing;
    if (dataNum != -1) {
        for (index = 0; index < 80; index++) {
            existing = lbl_1_bss_1B0[index];
            if (dataNum == existing) return index;
            if (existing == -1) {
                lbl_1_bss_1B0[index] = dataNum;
                return index;
            }
        }
    }
    return -1;
}

/* Return a cached motion, or load the motion data and resolve its tracks against the supplied
 * model. */
int fn_1_4510(int model, int dataNum)
{
    int slot = m620_local_4510(dataNum);
    int motion;
    if (slot >= 0) {
        motion = lbl_1_bss_70[slot];
        if (motion == -1) {
            motion = Hu3DJointMotion((s16)model,
                HuDataSelHeapReadNum(dataNum, HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_70[slot] = motion;
            return motion;
        }
        return motion;
    }
    return -1;
}

/* Interpolates an angle along the shorter arc and wraps the result into 0..360. */
float fn_1_4620(float start, float end, float factor)
{
    float angle;

    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    angle = fmodf(start + factor * angle, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    return angle;
}

/* Returns the signed shortest angular difference from start to end. */
f32 fn_1_473C(f32 start, f32 end)
{
    f32 remainder = (f32)fmod((f64)(end - start), 360.0);
    f32 angle = remainder;

    if (angle < 0.0f) {
        angle += 360.0f;
    }
    if (angle > 180.0f) {
        angle -= 360.0f;
    }
    return angle;
}

/* Returns the angle from start to end in the XZ plane, in degrees. */
float fn_1_47F0(HuVecF *start, HuVecF *end)
{
    float deltaX = end->x - start->x;
    float deltaZ = end->z - start->z;

    return atan2(deltaX, deltaZ) / 3.141592653589793 * 180.0;
}

/* Returns the horizontal distance between two positions in the XZ plane. */
float fn_1_488C(Vec *first, Vec *second)
{
    float x, z;
    x = second->x - first->x;
    z = second->z - first->z;
    return sqrtf(x * x + z * z);
}

extern void fn_1_2E8(s16, s16);
extern void fn_1_324(s16, s16);
extern void fn_1_5C8(s16, s16);
extern void fn_1_638(s16, s16);
extern void fn_1_684(s16, s16);
extern void fn_1_708(s16, s16);
extern void fn_1_9E0(s16, s16);
extern void fn_1_AB8(s16, s16);
extern void fn_1_AF0(s16, s16);

Vec lbl_1_data_C0 = {0.0f, -1.0f, 0.0f};
GXColor lbl_1_data_CC = {0, 0, 0, 0};
s32 lbl_1_data_D0[6] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mario, 157), DATANUM(DATA_mario, 158),
    DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 40)
};

typedef struct M620MotionParam {
    u16 motion;
    s16 overlay;
    f32 blend;
    u32 attr;
} M620MotionParam;

#define M620_PLAYER_CONFIG_50_0_BIT_PATTERN 0x42480000
#define M620_PLAYER_CONFIG_200_0_BIT_PATTERN 0x43480000
#define M620_PLAYER_CONFIG_11_6667_BIT_PATTERN 0x413AAAAB
#define M620_PLAYER_CONFIG_2_0_BIT_PATTERN 0x40000000
#define M620_PLAYER_CONFIG_1_5_BIT_PATTERN 0x3FC00000

M620MotionParam lbl_1_data_E8[6] = {
    {0, -1, 0.2f, HU3D_MOTATTR_LOOP},
    {1, -1, 0.2f, HU3D_MOTATTR_LOOP},
    {2, -1, 0.2f, HU3D_MOTATTR_LOOP},
    {3, -1, 0.2f, HU3D_MOTATTR_NONE},
    {4, -1, 0.2f, HU3D_MOTATTR_NONE},
    {5, -1, 0.2f, HU3D_MOTATTR_NONE}
};
Vec lbl_1_data_130[4] = {
    {-510.0f, 63.0f, -510.0f},
    {510.0f, 63.0f, -510.0f},
    {-510.0f, 63.0f, 510.0f},
    {510.0f, 63.0f, 510.0f}
};
f32 lbl_1_data_160[7] = {50.0f, 310.0f, 130.0f, 230.0f, 0.0f, 0.0f, 0.0f};
M620PlayerConfig lbl_1_data_17C = {
    {M620_PLAYER_CONFIG_50_0_BIT_PATTERN, M620_PLAYER_CONFIG_200_0_BIT_PATTERN,
     M620_PLAYER_CONFIG_11_6667_BIT_PATTERN},
    0,
    1,
    {M620_PLAYER_CONFIG_2_0_BIT_PATTERN, 1, M620_PLAYER_CONFIG_1_5_BIT_PATTERN},
    2,
    3,
    4,
    5
};

typedef struct M620EventProfile {
    s32 initialTimer;
    s32 timerRange;
    f32 advanceChance;
    s32 nearbyObjectLimit;
} M620EventProfile;

M620EventProfile lbl_1_data_1AC[4] = {
    {300, 180, 0.2f, 1},
    {240, 210, 0.15f, 3},
    {210, 240, 0.1f, 5},
    {180, 270, 0.05f, 7}
};
