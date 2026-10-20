/* Scene lighting, camera control, positional sound and movement math. */
#include "dolphin/math.h"
#include "REL/m676Dll/camera_types.h"
#include "game/audio.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/hsfex.h"
#include <string.h>

#define FIT_MIN(a,b) ((a)<(b)?(a):(b))
#define FIT_MAX(a,b) ((a)>(b)?(a):(b))
typedef void (*CameraCallback)(OMOBJ *, u32, s32);
Vec lbl_1_data_120 = {0, -1, 0};
GXColor lbl_1_data_12C = {0, 0, 0, 0};

/* Float math entry points share the ordinary double implementations. */
extern inline float tanf(float x) { return (float)tan(x); }

void fn_1_16A0(OMOBJMAN *manager);
void fn_1_1760(void);
void fn_1_1764(OMOBJ *object);
void fn_1_1768(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color);
void fn_1_1870(OMOBJ *obj);
void fn_1_18C8(void);
void fn_1_192C(s32 modelId, char *lightName);
void fn_1_19E0(OMOBJ_FUNC callback);
void fn_1_1A08(HuVecF *position);
void fn_1_1A70(HuVecF *direction);
HuVecF *fn_1_1AD8();
void fn_1_1B00(int red,int green,int blue,int alpha);
void fn_1_1B84(OMOBJMAN *manager, M676FogParam *parameters);
void fn_1_1C84(OMOBJ *obj);
void fn_1_1CDC(s16 layer);
void fn_1_1DD0(OMOBJ_FUNC callback);
void fn_1_1E10(f32 fogFactor);
void fn_1_1E50(OMOBJMAN *manager, u32 cameras, float fov, float nearPlane, float farPlane,
    float shadowRadius, OM_CAMERA_VIEW *view);
void fn_1_21BC(OMOBJ *object);
void fn_1_23E0(OMOBJ *object, u32 camera, s32 index);
void fn_1_24D8(OMOBJ *object, u32 camera, s32 index);
void fn_1_28BC(s32 camera, Vec *direction, Vec *target, float fov, float radius);
int fn_1_2A68(unsigned int mask);
void fn_1_2AAC(CameraCallback callback);
void fn_1_2AD4(u32 cameras, float aspect);
void fn_1_2B48(u32 cameras, float x, float y, float width, float height);
void fn_1_2D10(u32 cameras, float x, float y, float width, float height);
void fn_1_2D90(u32 cameras, float x, float y, float width, float height);
void fn_1_2E10(f32 fov);
void fn_1_2E40(HuVecF *direction);
Vec *fn_1_2E90(void);
void fn_1_2EC0(u8 red, u8 green, u8 blue);
void fn_1_2EF8(u32 cameras, void *data);
void fn_1_2FB0(u32 cameraMask);
BOOL fn_1_3450(u32 cameras);
double fn_1_34D4(M676CameraFitParams *params, M676CameraFitSphere *spheres,
    s32 count, Vec *out);
void fn_1_3A1C(HUPROCESS *process, M676AudioConfig *config);
void fn_1_3AB8(OMOBJ *obj);
void fn_1_3B20(OMOBJ *object);
void fn_1_3B78(OMOBJ *object);
void fn_1_3C48(OMOBJ_FUNC callback);
int fn_1_3C88(float screenX);
s32 fn_1_3CB4(u32 cameras, float x);
s32 fn_1_3DD0(Vec *position);
s32 fn_1_3DFC(u32 cameras, Vec *position);
void fn_1_3FF0(Vec *position);
s32 fn_1_401C(u32 cameras, Vec *position);
void fn_1_4280(s32 effect, Vec *position);
s32 fn_1_42B4(u32 cameras, s32 effect, Vec *position);
void fn_1_4754(s32 effect, Vec *position);
s32 fn_1_4788(u32 cameras, s32 effect, Vec *position);
s32 fn_1_49C8(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled);
s32 fn_1_4A0C(u32 cameras, s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled);
void fn_1_4F08(int effect, int handle, Vec *position, int panEnabled, int volumeEnabled);
void fn_1_4F54(unsigned int cameraMask, int effect, int handle, HuVecF *position, int panEnabled,
               int volumeEnabled);
void fn_1_4FCC(void);
int fn_1_5024(int dataNum);
int fn_1_50A8(int dataNum);
s32 fn_1_51B0(s32 model, s32 dataNum);
float fn_1_52C0(float start, float end, float factor);
f32 fn_1_53DC(f32 start, f32 end);
float fn_1_5490(HuVecF *from, HuVecF *to);
float fn_1_552C(Vec *first, Vec *second);
void fn_1_5698(Point3d *start, Point3d *end, f32 factor, Point3d *out);
s32 fn_1_56F0(Vec *start, Vec *end, f32 gravity, f32 initialY, Vec *velocity);

OMOBJ *lbl_1_bss_430;
OMOBJ *lbl_1_bss_42C;
OMOBJ *lbl_1_bss_428;
OMOBJ *lbl_1_bss_424;
OMOBJ *lbl_1_bss_420;
s32 lbl_1_bss_2E0[80];
s32 lbl_1_bss_1A0[80];

/* Creates the shared scene object, resets the light, fog, camera, and audio pointers, and clears
 * the model/motion cache. */
void fn_1_16A0(OMOBJMAN *manager)
{
    OMOBJ *object = omAddObjEx(manager, 22, 0, 0, OM_GRP_NONE, fn_1_1764);
    void *state;

    lbl_1_bss_430 = object;
    lbl_1_bss_42C = lbl_1_bss_428 = lbl_1_bss_424 = lbl_1_bss_420 = NULL;
    state = HuMemDirectMallocNum(HEAP_HEAP, 4, HU_MEMNUM_OVL);
    object->data = state;
    memset(state, 0, 4);
    fn_1_4FCC();
}

void fn_1_1760(void)
{
}

void fn_1_1764(OMOBJ *object)
{
}

/* Creates the shared light controller and stores its position, target, and color;
 * its light ID starts at -1. */
void fn_1_1768(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color)
{
    M676LightWork *work;
    OMOBJ *object;
    object = omAddObjEx(manager, 24, 0, 0, OM_GRP_NONE, fn_1_1870);
    lbl_1_bss_42C = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M676LightWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M676LightWork));
    work->position = *position;
    work->target = *target;
    work->color = *color;
    work->lightId = -1;
    PSVECSubtract(&work->target, &work->position, &work->direction);
    PSVECNormalize(&work->direction, &work->direction);
}

/* Light object's update callback: forwards each frame to its registered callback. */
void fn_1_1870(OMOBJ *obj)
{
    M676LightWork *light = omObjGetDataAs(lbl_1_bss_42C, M676LightWork);
    if (light->callback != (OMOBJ_FUNC)(void *)0) {
        light->callback(obj);
    }
}

/* Light setup: creates the shared directional light from the stored work values. */
void fn_1_18C8(void) {
    M676LightWork *light = omObjGetDataAs(lbl_1_bss_42C, M676LightWork);

    light->lightId = Hu3DGLightCreateV(&light->position, &light->direction, &light->color);
    Hu3DGLightInfinitytSet(light->lightId);
    Hu3DGLightStaticSet(light->lightId, 1);
    fn_1_2E40(&light->direction);
}

/* Imports the model's lights and selects the named global light, or global slot zero
 * when unnamed. */
void fn_1_192C(s32 modelId, char *lightName) {
    HU3D_LIGHT *lightRecords;
    M676LightWork *light;

    light = lbl_1_bss_42C->data;
    lightRecords = Hu3DGlobalLight;
    Hu3DModelLightInfoSet((s16) modelId, 1);
    if (lightName != NULL) {
        light->lightId = Hu3DModelLightIdGet((s16) modelId, lightName);
    } else {
        light->lightId = 0;
    }
    if (light->lightId >= 0) {
        Hu3DGLightParamGet(light->lightId, &light->position, &light->direction, &light->color);
        fn_1_2E40(&light->direction);
    }
}

void fn_1_19E0(OMOBJ_FUNC callback) {
    M676LightWork *light=omObjGetDataAs(lbl_1_bss_42C,M676LightWork);
    light->callback=callback;
}

/* Updates the shared light's position and reapplies its stored direction. */
void fn_1_1A08(HuVecF *position) {
    M676LightWork *light=omObjGetDataAs(lbl_1_bss_42C,M676LightWork);
    light->position=*position;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}

/* Updates the shared light's direction and reapplies its stored position. */
void fn_1_1A70(HuVecF *direction) {
    M676LightWork *light=omObjGetDataAs(lbl_1_bss_42C,M676LightWork);
    light->direction=*direction;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}

HuVecF *fn_1_1AD8() {
    M676LightWork *light=omObjGetDataAs(lbl_1_bss_42C,M676LightWork);
    return &light->direction;
}

/* Sets the stored shared-light color and the global light's RGBA color. */
void fn_1_1B00(int red,int green,int blue,int alpha) {
    M676LightWork *light=omObjGetDataAs(lbl_1_bss_42C,M676LightWork);
    light->color.r=red;
    light->color.g=green;
    light->color.b=blue;
    light->color.a=alpha;
    Hu3DGLightColorSet(light->lightId,red,green,blue,alpha);
}

/* Scene setup: installs the per-layer fog callback and initial fog distance. */
void fn_1_1B84(OMOBJMAN *manager, M676FogParam *parameters)
{
    M676FogWork *work;
    s32 index;
    OMOBJ *object;

    object = omAddObjEx(manager, 26, 0, 0, OM_GRP_NONE, fn_1_1C84);
    lbl_1_bss_428 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M676FogWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M676FogWork));
    work->parameters = parameters;
    work->factor = 1.0f;
    for (index = 0; parameters->layer != -1; index++, parameters++) {
        if (index == 0) {
            Hu3DFogSet(parameters->start, parameters->end, parameters->color.r,
                parameters->color.g, parameters->color.b);
        }
        Hu3DLayerHookSet(parameters->layer, fn_1_1CDC);
    }
}

/* Fog object's update callback: forwards each frame to its registered callback. */
void fn_1_1C84(OMOBJ *obj)
{
    M676FogWork *fogWork = omObjGetDataAs(lbl_1_bss_428, M676FogWork);
    if (fogWork->callback != (OMOBJ_FUNC)(void *)0) {
        fogWork->callback(obj);
    }
}

/* Render-layer callback: applies this layer's fog range using the current blend factor. */
void fn_1_1CDC(s16 layer)
{
    M676FogWork *work;
    M676FogParam *parameters;
    float fov, nearPlane, farPlane;
    float end;

    work = lbl_1_bss_428->data;
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

/* Sets the fog object's per-frame callback when the object exists. */
void fn_1_1DD0(OMOBJ_FUNC callback)
{
    M676FogWork *record;
    if (lbl_1_bss_428 != NULL) {
        record = lbl_1_bss_428->data;
        record->callback = callback;
    }
}

/* Updates the fog interpolation factor used by subsequent layer callbacks. */
void fn_1_1E10(f32 fogFactor)
{
    M676FogWork *work;
    if (lbl_1_bss_428 != NULL) {
        work = omObjGetDataAs(lbl_1_bss_428, M676FogWork);
        work->factor = fogFactor;
    }
}

/* Scene setup: initializes selected cameras and the per-frame camera controller. */
void fn_1_1E50(OMOBJMAN *manager, u32 cameras, float fov, float nearPlane, float farPlane,
    float shadowRadius, OM_CAMERA_VIEW *view)
{
    M676CameraRecord *entry;
    M676CameraWork *work;
    OMOBJ *object;
    s32 i;
    u32 bit;
    s32 count = 0;
    object = omAddObjEx(manager, 28, 0, 0, OM_GRP_NONE, fn_1_21BC);
    lbl_1_bss_424 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M676CameraWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M676CameraWork));
    work->cameras = cameras;
    work->shadowRadius = shadowRadius;
    work->shadowFov = 20.0f;
    work->shadowColor = lbl_1_data_12C;
    if (lbl_1_bss_42C == NULL) {
        work->shadowDirection = lbl_1_data_120;
    } else {
        work->shadowDirection = *fn_1_1AD8();
    }
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            count = i + 1;
            entry = &work->entries[i];
            entry->currentPose = *view;
            entry->targetPose = *view;
            entry->position = view->center;
            entry->position.z -= 1000.0;
            entry->up.x = entry->up.z = 0.0f;
            entry->up.y = 1.0f;
            entry->target = view->center;
            entry->fov = fov;
            entry->nearPlane = nearPlane;
            entry->farPlane = farPlane;
            entry->aspect = 1.2f;
            entry->viewport[0] = entry->scissor[0] = 0.0f;
            entry->viewport[1] = entry->scissor[1] = 0.0f;
            entry->viewport[2] = entry->scissor[2] = 640.0f;
            entry->viewport[3] = entry->scissor[3] = 480.0f;
        }
    }
    Hu3DCameraCreate(cameras);
    Hu3DCameraPerspectiveSet(cameras, fov, nearPlane, farPlane, 1.2f);
    omCameraViewSetMulti(cameras, view);
    object = omAddObjEx(manager, 29, 0, 0, OM_GRP_NONE, omOutViewMulti);
    object->work[0] = count;
    omOutViewMulti(object);
}

/* Camera object's update callback: refreshes camera state and runs active transitions. */
void fn_1_21BC(OMOBJ *object)
{
    M676CameraWork *work = object->data;
    M676CameraRecord *entry;
    s32 i, shadowDone = 0;
    u32 bit;

    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (work->cameras & bit) {
            entry = &work->entries[i];
            Hu3DCameraPosGet(bit, &entry->position, &entry->up, &entry->target);
            Hu3DCameraPerspectiveGet(bit, &entry->fov, &entry->nearPlane, &entry->farPlane);
            if (shadowDone == 0) {
                if (work->shadowRadius > 0.0) {
                    fn_1_28BC(work->cameras, &work->shadowDirection, &entry->target,
                        work->shadowFov, work->shadowRadius);
                    Hu3DShadowColSet(work->shadowColor.r, work->shadowColor.g, work->shadowColor.b);
                }
                shadowDone = 1;
            }
            if (work->entries[i].active) {
                fn_1_23E0(object, bit, i);
            }
            if (work->callback != NULL) { work->callback(object, bit, i); }
            CenterM[i] = entry->currentPose.center;
            CRotM[i] = entry->currentPose.rot;
            CZoomM[i] = entry->currentPose.zoom;
            Hu3DCameraPerspectiveSet(bit, entry->fov, entry->nearPlane, entry->farPlane,
                                     entry->aspect);
            Hu3DCameraViewportSet(bit, entry->viewport[0], entry->viewport[1], entry->viewport[2],
                entry->viewport[3], 0.0f, 1.0f);
            Hu3DCameraScissorSet(bit, (u32)entry->scissor[0], (u32)entry->scissor[1],
                (u32)entry->scissor[2], (u32)entry->scissor[3]);
        }
    }
}

/* Derives the live camera pose; when motion ends, kills its model and saves that pose
 * as the target. */
void fn_1_23E0(OMOBJ *object, u32 camera, s32 index)
{
    M676CameraWork *work;
    M676CameraRecord *entry;

    work = object->data;
    entry = &work->entries[index];
    fn_1_24D8(object, camera, index);
    if (Hu3DMotionEndCheck((s16)entry->model) != 0) {
        Hu3DModelKill((s16)entry->model);
        entry->targetPose.center = CenterM[index];
        entry->targetPose.rot = CRotM[index];
        entry->targetPose.zoom = CZoomM[index];
        entry->active = 0;
    }
}

/* Calculates center, pitch, yaw, and zoom from the camera position and target; forces roll to zero
* and ignores the stored up vector. */
void fn_1_24D8(OMOBJ *object, u32 camera, s32 index)
{
    M676CameraRecord *entry;
    M676CameraWork *work = object->data;
    Vec delta;
    entry = &work->entries[index];
    CenterM[index] = entry->target;
    delta.x = entry->position.x - entry->target.x;
    delta.y = entry->position.y - entry->target.y;
    delta.z = entry->position.z - entry->target.z;
    CRotM[index].x = 180.0 * (atan2(-delta.y,
        sqrtf(delta.x * delta.x + delta.z * delta.z)) / M_PI);
    CRotM[index].y = 180.0 * (atan2(delta.x, delta.z) / M_PI);
    CRotM[index].z = 0.0f;
    CZoomM[index] = sqrtf(delta.z * delta.z + (delta.x * delta.x + delta.y * delta.y));
    entry->currentPose.center = CenterM[index];
    entry->currentPose.rot = CRotM[index];
    entry->currentPose.zoom = CZoomM[index];
}

const Vec lbl_1_rodata_138[2] = {{0, 1, 0}, {0, 0, 1}};

/* Sets a shadow camera behind the target along the light direction. */
void fn_1_28BC(s32 camera, Vec *direction, Vec *target, float fov, float radius)
{
    Vec position, unit;
    const Vec *up = lbl_1_rodata_138;
    float distance;
    float radians;
    radians = 3.141592653589793 * (fov / 2.0f) / 180.0;
    distance = radius / tanf(radians);
    PSVECNormalize(direction, &unit);
    PSVECScale(&unit, &position, distance);
    PSVECSubtract(target, &position, &position);
    if (fabs(unit.y) > 0.5) {
        up++;
    }
    Hu3DShadowMultiCreate(fov, distance - radius, distance + radius, camera);
    Hu3DShadowMultiPosSet(&position, (Vec *)up, target, camera);
    Hu3DShadowMultiSizeSet(480, camera);
    Hu3DShadowMultiTPLvlSet(0.4f, camera);
}

/* Returns the lowest set camera-bit index, or zero when the mask is empty. */
int fn_1_2A68(unsigned int mask)
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

void fn_1_2AAC(CameraCallback callback) {
    M676CameraWork *state=omObjGetDataAs(lbl_1_bss_424,M676CameraWork);
    state->callback=callback;
}

/* Sets the aspect ratio for each camera selected by the mask. */
void fn_1_2AD4(u32 cameras, float aspect)
{
    OMOBJ *object = lbl_1_bss_424;
    M676CameraRecord *entry;
    s32 i;
    u32 bit;
    M676CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->aspect = aspect;
        }
    }
}

/* Expands the viewport outward by size-dependent margins and retains the supplied rectangle
 * as the scissor. */
void fn_1_2B48(u32 cameras, float x, float y, float width, float height)
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
    fn_1_2D10(cameras, x + offsetX, y + offsetY, width + extraWidth, height + extraHeight);
    fn_1_2D90(cameras, x, y, width, height);
}

/* Stores the viewport rectangle for each selected camera. */
void fn_1_2D10(u32 cameras, float x, float y, float width, float height)
{
    M676CameraRecord *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_424;
    M676CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->viewport[0] = x;
            entry->viewport[1] = y;
            entry->viewport[2] = width;
            entry->viewport[3] = height;
        }
    }
}

/* Stores the scissor rectangle for each selected camera. */
void fn_1_2D90(u32 cameras, float x, float y, float width, float height)
{
    M676CameraRecord *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_424;
    M676CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->scissor[0] = x;
            entry->scissor[1] = y;
            entry->scissor[2] = width;
            entry->scissor[3] = height;
        }
    }
}

/* Sets the stored field of view used by subsequent shared-shadow updates. */
void fn_1_2E10(f32 fov)
{
    OMOBJ *object = lbl_1_bss_424;
    M676CameraWork *work = object->data;
    work->shadowFov = fov;
}

/* Light setup uses this to update the shadow direction when camera state exists. */
void fn_1_2E40(HuVecF *direction) {
    OMOBJ *object=lbl_1_bss_424;
    if(object != NULL) {
        M676CameraWork *state=omObjGetDataAs(object,M676CameraWork);
        state->shadowDirection=*direction;
    }
}

/* Returns the shared shadow direction vector to the per-frame camera callback. */
Vec *fn_1_2E90(void)
{
    OMOBJ *object = lbl_1_bss_424;
    M676CameraWork *work = object->data;
    return &work->shadowDirection;
}

/* Sets the RGB color used by the shared shadow map. */
void fn_1_2EC0(u8 red, u8 green, u8 blue)
{
    OMOBJ *object = lbl_1_bss_424;
    M676CameraWork *work = (M676CameraWork *)object->data;

    work->shadowColor.r = red;
    work->shadowColor.g = green;
    work->shadowColor.b = blue;
}

/* Starts the supplied camera motion on each selected camera. */
void fn_1_2EF8(u32 cameras, void *data)
{
    M676CameraRecord *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_424;
    M676CameraWork *work = object->data;
    s32 motion = Hu3DMotionCreate(data);
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->motion = motion;
            entry->model = Hu3DModelCameraCreate(motion, bit);
            Hu3DCameraMotionStart(entry->model, bit);
            entry->active = 1;
        }
    }
}

/* Stops active camera motions on selected cameras and saves each live view as the new target
 * pose. */
void fn_1_2FB0(u32 cameraMask)
{
    u32 bit;
    OMOBJ *object;
    M676CameraWork *work;
    M676CameraRecord *entry;
    s32 index;

    object = lbl_1_bss_424;
    work = object->data;
    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if ((cameraMask & bit) != 0) {
            entry = &work->entries[index];
            if (entry->active != 0) {
                fn_1_24D8(object, bit, index);
                Hu3DModelKill((s16)entry->model);
                entry->targetPose.center = CenterM[index];
                entry->targetPose.rot = CRotM[index];
                entry->targetPose.zoom = CZoomM[index];
                entry->active = 0;
            }
        }
    }
}

/* Returns whether any selected camera has an active camera-motion model. */
BOOL fn_1_3450(u32 cameras)
{
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_424;
    M676CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if ((cameras & bit) && work->entries[i].active) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Fits the camera, writes its position to out, and returns zoom; initializes from spheres[0]
 * even when count is zero. */
double fn_1_34D4(M676CameraFitParams *params, M676CameraFitSphere *spheres,
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
    M676CameraFitSphere *sphere;
    s32 i;

    MTXLookAt(view, &origin, &params->up, &params->direction);
    MTXInverse(view, inverse);
    {
        double aspect = (double)params->width / params->height;
        tangent = tan(params->fov / 2.0f);
        slopeX =
            aspect * (tangent * ((double) (params->width - 2 * params->marginX) / params->width));
        slopeY = tangent * ((double)(params->height - 2 * params->marginY) / params->height);
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

/* Scene setup: creates the shared sound object and stores its configuration. */
void fn_1_3A1C(HUPROCESS *process, M676AudioConfig *config) {
    OMOBJ *object;
    M676AudioWork *data;

    object = omAddObjEx(process, 30, 0, 0, -1, fn_1_3AB8);
    lbl_1_bss_420 = object;
    data = HuMemDirectMallocNum(HEAP_HEAP, 1544, HU_MEMNUM_OVL);
    object->data = data;
    memset(data, 0, 1544);
    data->config = config;
    fn_1_3B20(object);
}

/* Audio object's update callback: refreshes tracked sounds and then calls its client. */
void fn_1_3AB8(OMOBJ *obj) {
    M676AudioWork *state = omObjGetDataAs(lbl_1_bss_420, M676AudioWork);

    fn_1_3B78(obj);
    if (state->callback != NULL) {
        state->callback(obj);
    }
}

/* Audio setup: marks all 64 sound records as unused. */
void fn_1_3B20(OMOBJ *object)
{
    M676AudioWork *state = omObjGetDataAs(lbl_1_bss_420, M676AudioWork);
    M676AudioRecord *record = state->records;
    int index;

    for (index = 0; index < 64; index++, record++) {
        record->handle = -1;
    }
}

/* Audio object's update: releases ended sounds and refreshes active pan and volume. */
void fn_1_3B78(OMOBJ *object)
{
    M676AudioWork *state;
    M676AudioRecord *record;
    int index;

    state = lbl_1_bss_420->data;
    record = state->records;
    for (index = 0; index < 64; index++, record++) {
        if (record->handle != -1) {
            if (HuAudFXStatusGet(record->handle) != 2) {
                record->handle = -1;
            } else {
                if (record->panEnabled) {
                    HuAudFXPanning(record->handle,
                        (s16)fn_1_3DFC(record->cameraMask, record->position));
                }
                if (record->volumeEnabled) {
                    HuAudFXVolSet(record->handle,
                        (s16)fn_1_401C(record->cameraMask, record->position));
                }
            }
        }
    }
}

/* Sets the client callback invoked by the shared audio object's update. */
void fn_1_3C48(OMOBJ_FUNC callback)
{
    M676AudioWork *record;
    if (lbl_1_bss_420 != NULL) {
        record = lbl_1_bss_420->data;
        record->callback = callback;
    }
}

/* Returns the default-camera pan for screen X over a 576-pixel span. */
int fn_1_3C88(float screenX) {
    return fn_1_3CB4(1,screenX);
}

/* Maps screen X over a 576-pixel span into the first overlapping camera configuration's
 * clamped pan range; returns center pan when no configuration applies. */
s32 fn_1_3CB4(u32 cameras, float panCoordinate)
{
    M676AudioWork *work;
    M676AudioConfig *config;
    s32 pan = 64;
    if (lbl_1_bss_420 != NULL) {
        work = lbl_1_bss_420->data;
        for (config = work->config; config->cameras; config++) {
            if (config->cameras & cameras) {
                /* Convert the 576-pixel screen coordinate to an offset within the pan range. */
                panCoordinate *= (config->panMax - config->panMin) / 576.0f;
                pan = config->panMin + panCoordinate;
                if (pan < config->panMin) { pan = config->panMin; }
                if (pan > config->panMax) { pan = config->panMax; }
                break;
            }
        }
    }
    return pan;
}

s32 fn_1_3DD0(Vec *position)
{
    return fn_1_3DFC(1, position);
}

/* Projects a world position through one camera and maps it to the configured sound pan. */
s32 fn_1_3DFC(u32 cameras, Vec *position)
{
    M676CameraWork *work = lbl_1_bss_424->data;
    M676CameraRecord *camera = &work->entries[fn_1_2A68(cameras)];
    Vec screenPosition;
    float screenX;
    Hu3D3Dto2D(position, cameras, &screenPosition);
    screenX = screenPosition.x / 576.0f;
    screenX = 0.9f * camera->viewport[0] +
        screenX * (0.9f * camera->viewport[2]);
    return fn_1_3CB4(cameras, screenX);
}

/* Computes default-camera volume and discards the result; no module caller uses this wrapper. */
void fn_1_3FF0(Vec *position)
{
    fn_1_401C(1, position);
}

/* Computes sound volume from the position's depth in the selected camera view. */
s32 fn_1_401C(u32 cameras, Vec *position)
{
    M676AudioWork *audio;
    M676AudioConfig *config;
    M676CameraWork *work;
    M676CameraRecord *camera;
    Mtx matrix;
    Vec cameraSpacePosition;
    float distance, factor;
    s32 volume = 127;
    if (lbl_1_bss_420 != NULL) {
        audio = lbl_1_bss_420->data;
        work = lbl_1_bss_424->data;
        camera = &work->entries[fn_1_2A68(cameras)];
        for (config = audio->config; config->cameras; config++) {
            if (config->cameras & cameras) {
                C_MTXLookAt(matrix, &camera->position, &camera->up, &camera->target);
                PSMTXMultVec(matrix, position, &cameraSpacePosition);
                distance = -cameraSpacePosition.z;
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
                if (volume < config->volumeMin) { volume = config->volumeMin; }
                if (volume > config->volumeMax) { volume = config->volumeMax; }
                break;
            }
        }
    }
    return volume;
}

/* Plays a spatial sound for the default camera and discards its handle. */
void fn_1_4280(s32 effect, Vec *position) {
    fn_1_42B4(1, effect, position);
}

/* Plays the effect for each selected camera using projected pan and depth volume; returns the last
 * handle, or -1 if none are selected. */
s32 fn_1_42B4(u32 cameras, s32 effect, Vec *position)
{
    s32 i;
    u32 bit;
    s32 handle = -1;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayVolPan(effect, fn_1_401C(bit, position), fn_1_3DFC(bit, position));
        }
    }
    return handle;
}

/* Plays a panned sound for the default camera and discards its handle. */
void fn_1_4754(s32 effect, Vec *position) {
    fn_1_4788(1, effect, position);
}

/* Plays the effect for each selected camera using projected pan; returns the last handle, or -1 if
 * none are selected. */
s32 fn_1_4788(u32 cameras, s32 effect, Vec *position)
{
    s32 i;
    u32 bit;
    s32 handle = -1;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayPan(effect, fn_1_3DFC(bit, position));
        }
    }
    return handle;
}

s32 fn_1_49C8(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled)
{
    return fn_1_4A0C(1, effect, position, panEnabled, volumeEnabled);
}

/* Plays a sound for each selected camera with optional spatial updates, records it when a slot is
 * free, and returns the last handle or -1; a full table leaves it untracked. */
s32 fn_1_4A0C(u32 cameras, s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled)
{
    s32 i;
    u32 bit;
    s32 pan, volume;
    s32 handle = -1;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            if (panEnabled) { pan = fn_1_3DFC(bit, position); }
            else { pan = 64; }
            if (volumeEnabled) { volume = fn_1_401C(bit, position); }
            else { volume = 127; }
            handle = HuAudFXPlayVolPan(effect, volume, pan);
            fn_1_4F54(bit, effect, handle, position, panEnabled, volumeEnabled);
        }
    }
    return handle;
}

/* Registers a sound for optional default-camera pan and volume updates. */
void fn_1_4F08(int effect, int handle, Vec *position, int panEnabled, int volumeEnabled) {
    fn_1_4F54(1, effect, handle, position, panEnabled, volumeEnabled);
}

/* Stores one active sound so the audio object's update can adjust or release it. */
void fn_1_4F54(unsigned int cameraMask, int effect, int handle, HuVecF *position, int panEnabled,
               int volumeEnabled)
{
    M676AudioWork *state=omObjGetDataAs(lbl_1_bss_420,M676AudioWork);
    M676AudioRecord *record=state->records;
    int index;
    for(index=0;index<64;index++,record++) {
        if(record->handle==-1) {
            record->handle=handle;
            record->effect=effect;
            record->cameraMask=cameraMask;
            record->panEnabled=panEnabled;
            record->volumeEnabled=volumeEnabled;
            record->position=position;
            break;
        }
    }
}

/* Scene-object setup calls this to mark every model and motion cache slot unused. */
void fn_1_4FCC(void) {
    s32 entryIndex;

    entryIndex = 0;
    while (entryIndex < 80) {
        lbl_1_bss_2E0[entryIndex] = -1;
        lbl_1_bss_1A0[entryIndex] = -1;
        entryIndex += 1;
    }
}

/* Model and motion cache lookups find an existing data slot or reserve the first empty one. */
int fn_1_5024(int dataNum) {
    int index;
    int existing;
    if(dataNum!=-1) {
        for(index=0;index<80;index++) {
            existing=lbl_1_bss_2E0[index];
            if(dataNum==existing) return index;
            if(existing==-1) {
                lbl_1_bss_2E0[index]=dataNum;
                return index;
            }
        }
    }
    return -1;
}

/* Returns the first model created for a data entry; later requests create linked instances
 * from that cached model. */
int fn_1_50A8(int dataNum) {
    int slot=fn_1_5024(dataNum);
    int model;
    if(slot>=0) {
        model=lbl_1_bss_1A0[slot];
        if(model==-1) {
            model=Hu3DModelCreateData(dataNum);
            lbl_1_bss_1A0[slot]=model;
            return model;
        }
        return Hu3DModelLink(model);
    }
    return -1;
}

/* Reuses joint motion by data entry alone; the supplied model is used only when creating
 * the cached motion. */
s32 fn_1_51B0(s32 model, s32 dataNum)
{
    s32 slot = fn_1_5024(dataNum);
    s32 motion;
    if (slot >= 0) {
        motion = lbl_1_bss_1A0[slot];
        if (motion == -1) {
            motion = Hu3DJointMotionData(model, dataNum);
            lbl_1_bss_1A0[slot] = motion;
            return motion;
        }
        return motion;
    }
    return -1;
}

/* Camera and actor updates interpolate an angle along the shorter turn. */
float fn_1_52C0(float start, float end, float factor)
{
    float angle;
    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    angle = fmodf(start + factor * angle, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    return angle;
}

/* Actor steering and movement wrap an angle difference to the shortest signed turn. */
f32 fn_1_53DC(f32 start, f32 end)
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

/* Returns the heading in degrees from the first horizontal point to the second. */
float fn_1_5490(HuVecF *from, HuVecF *to) {
    float deltaX = to->x - from->x;
    float deltaZ = to->z - from->z;
    return atan2(deltaX, deltaZ) / 3.141592653589793 * 180.0;
}

/* Player movement and prop selection use this to measure horizontal x/z separation. */
float fn_1_552C(Vec *first, Vec *second)
{
    float deltaX, deltaZ;
    deltaX = second->x - first->x;
    deltaZ = second->z - first->z;
    return sqrtf(deltaX * deltaX + deltaZ * deltaZ);
}

/* Movement and collision code use this to interpolate a point between two positions. */
void fn_1_5698(Point3d *start, Point3d *end, f32 factor, Point3d *out)
{
    out->x = start->x + (factor * (end->x - start->x));
    out->y = start->y + (factor * (end->y - start->y));
    out->z = start->z + (factor * (end->z - start->z));
}

/* Computes launch velocity and travel time; initialY only contributes to the initial
 * magnitude, then velocity Y is overwritten. */
s32 fn_1_56F0(Vec *start, Vec *end, f32 gravity, f32 initialY, Vec *velocity)
{
    f32 distance;
    f32 verticalDelta;
    f32 verticalSpeed;
    f32 flightValue;

    velocity->x = end->x - start->x;
    velocity->y = initialY;
    velocity->z = end->z - start->z;
    distance = PSVECMag(velocity);
    if (0.0 == distance) {
        return 0;
    }
    verticalDelta = end->y - start->y;
    flightValue = 2.0f * (distance - verticalDelta);
    if (0.0 == flightValue) {
        verticalSpeed = 0.0f;
    } else {
        verticalSpeed = (f32)sqrt((f64)((gravity * (distance * distance)) / flightValue));
    }
    PSVECScale(velocity, velocity, verticalSpeed / distance);
    velocity->y = verticalSpeed;
    if (0.0 == verticalSpeed) {
        return 0;
    }
    return (s32)(distance / verticalSpeed);
}
