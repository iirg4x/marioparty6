/* Maintains lights, cameras, fog, positional audio, and shared model resources. */
#include <math.h>
#include "REL/m626Dll/support.h"
#include "game/memory.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/hsfex.h"
#include <string.h>

#define FIT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FIT_MAX(a, b) ((a) > (b) ? (a) : (b))

void fn_1_A08(OMOBJMAN *manager);
void fn_1_AC8(void);
void fn_1_ACC(OMOBJ *object);
void fn_1_AD0(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color);
void fn_1_BD8(OMOBJ *obj);
void fn_1_C30(void);
void fn_1_C94(s32 modelId, char *lightName, s32 mode);
void fn_1_D48(OMOBJ_FUNC callback);
void fn_1_D70(HuVecF *position);
void fn_1_DD8(HuVecF *direction);
HuVecF *fn_1_E40();
void fn_1_E68(int red,int green,int blue,int alpha);
void fn_1_EEC(OMOBJMAN *manager, M626FogParameters *parameters);
void fn_1_FEC(OMOBJ *obj);
void fn_1_1044(s16 layer);
void fn_1_1138(OMOBJ_FUNC value);
void fn_1_1178(f32 value);
void fn_1_11B8(OMOBJMAN *manager, u32 cameras, f32 fov, f32 nearPlane, f32 farPlane,
               f32 shadowRadius, OM_CAMERA_VIEW *view);
void fn_1_1514(OMOBJ *object);
void fn_1_1738(OMOBJ *object, u32 camera, s32 index);
void fn_1_1BAC(Vec *direction, Vec *target, f32 fov, f32 radius);
int fn_1_1D3C(unsigned int mask);
void fn_1_1D80(M626CameraCallback callback);
void fn_1_1DA8(u32 cameras, float fov);
void fn_1_1E1C(u32 cameras, float x, float y, float width, float height);
void fn_1_1E9C(f32 value);
void fn_1_1ECC(HuVecF *direction);
f32 *fn_1_1F1C();
void fn_1_1F4C(u8 r, u8 g, u8 b);
void fn_1_1F84(u32 cameras, void *data);
s32 fn_1_203C(u32 cameras);
double fn_1_20C0(M626CameraFitParams *params, M626CameraFitSphere *spheres, s32 count, Vec *out);
void fn_1_2608(HUPROCESS *process, M626AudioParameters *audioConfig);
void fn_1_26A4(OMOBJ *obj);
void fn_1_270C(OMOBJ *object);
void fn_1_2764(OMOBJ *object);
void fn_1_2834(OMOBJ_FUNC value);
int fn_1_2874(float value);
s32 fn_1_28A0(u32 cameras, float x);
s32 fn_1_29BC(Vec *object);
s32 fn_1_29E8(u32 cameras, Vec *position);
s32 fn_1_2BDC(Vec *object);
s32 fn_1_2C08(u32 cameras, Vec *position);
void fn_1_2E6C(s32 left, Vec *right);
s32 fn_1_2EA0(u32 cameras, s32 effect, Vec *position);
s32 fn_1_3340(s32 left, Vec *right);
s32 fn_1_3374(u32 cameras, s32 effect, Vec *position);
s32 fn_1_35B4(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled);
s32 fn_1_35F8(u32 cameras, s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled);
void fn_1_3AF4(int effect, int handle, Vec *position, int flag, int value);
void fn_1_3B40(unsigned int cameraMask, int effect, int handle, HuVecF *position, int panEnabled,
               int volumeEnabled);
void fn_1_3BB8(void);
int fn_1_3C10(int handle);
int fn_1_3C94(int dataNum);
s32 fn_1_3D9C(s32 model, s32 dataNum);
float fn_1_3EAC(float start, float end, float factor);
float fn_1_3FC8(float start, float end);
float fn_1_407C(HuVecF *origin, HuVecF *target);
float fn_1_4118(Vec *first, Vec *second);

/* Initializes shared overlay state when Snow Brawl starts. */
void fn_1_A08(OMOBJMAN *manager) {
    OMOBJ *object=omAddObjEx(manager,22,0,0,OM_GRP_NONE,fn_1_ACC);
    void *state;
    lbl_1_bss_2C8=object;
    lbl_1_bss_2C4=lbl_1_bss_2C0=lbl_1_bss_2BC=lbl_1_bss_2B8=NULL;
    state=HuMemDirectMallocNum(HEAP_HEAP,4,HU_MEMNUM_OVL);
    object->data=state;
    memset(state,0,4);
    fn_1_3BB8();
}

void fn_1_AC8(void)
{
}

void fn_1_ACC(OMOBJ *object)
{
}

/* Creates the shared light work record during minigame setup. */
void fn_1_AD0(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color)
{
    M626LightWork *work;
    OMOBJ *object;
    object = omAddObjEx(manager, 24, 0, 0, OM_GRP_NONE, fn_1_BD8);
    lbl_1_bss_2C4 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M626LightWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M626LightWork));
    work->position = *position;
    work->target = *target;
    work->color = *color;
    work->lightId = -1;
    PSVECSubtract(&work->target, &work->position, &work->direction);
    PSVECNormalize(&work->direction, &work->direction);
}

/* Runs the registered light callback from the light object's update. */
void fn_1_BD8(OMOBJ *obj)
{
    M626LightWork *light = omObjGetDataAs(lbl_1_bss_2C4, M626LightWork);
    if (light->callback != (OMOBJ_FUNC)(void *)0) {
        light->callback(obj);
    }
}

/* Creates the directional global light from the configured light work record. */
void fn_1_C30(void) {
    M626LightWork *light = omObjGetDataAs(lbl_1_bss_2C4, M626LightWork);

    light->lightId = Hu3DGLightCreateV(&light->position, &light->direction, &light->color);
    Hu3DGLightInfinitytSet(light->lightId);
    Hu3DGLightStaticSet(light->lightId, 1);
    fn_1_1ECC(&light->direction);
}

/* Imports model lights and selects lightName (or slot 0 when NULL); mode is ignored. */
void fn_1_C94(s32 modelId, char *lightName, s32 mode) {
    HU3D_LIGHT *globalLightTable;
    M626LightWork *lightWork;

    lightWork = lbl_1_bss_2C4->data;
    globalLightTable = Hu3DGlobalLight;
    Hu3DModelLightInfoSet((s16) modelId, 1);
    if (lightName != NULL) {
        lightWork->lightId = Hu3DModelLightIdGet((s16) modelId, lightName);
    } else {
        lightWork->lightId = 0;
    }
    if (lightWork->lightId >= 0) {
        Hu3DGLightParamGet(lightWork->lightId, &lightWork->position, &lightWork->direction,
                           &lightWork->color);
        fn_1_1ECC(&lightWork->direction);
    }
}

void fn_1_D48(OMOBJ_FUNC callback) {
    M626LightWork *light=omObjGetDataAs(lbl_1_bss_2C4,M626LightWork);
    light->callback=callback;
}

/* Updates the active global light position from the scenery callback. */
void fn_1_D70(HuVecF *position) {
    M626LightWork *light=omObjGetDataAs(lbl_1_bss_2C4,M626LightWork);
    light->position=*position;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}

/* Updates the active global light direction from the scenery callback. */
void fn_1_DD8(HuVecF *direction) {
    M626LightWork *light=omObjGetDataAs(lbl_1_bss_2C4,M626LightWork);
    light->direction=*direction;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}

HuVecF *fn_1_E40() {
    M626LightWork *light=omObjGetDataAs(lbl_1_bss_2C4,M626LightWork);
    return &light->direction;
}

/* Changes the shared light color when scenery adjusts its lighting. */
void fn_1_E68(int red,int green,int blue,int alpha) {
    M626LightWork *light=omObjGetDataAs(lbl_1_bss_2C4,M626LightWork);
    light->color.r=red;
    light->color.g=green;
    light->color.b=blue;
    light->color.a=alpha;
    Hu3DGLightColorSet(light->lightId,red,green,blue,alpha);
}

/* Creates fog state and registers a layer hook while the scene is configured. */
void fn_1_EEC(OMOBJMAN *manager, M626FogParameters *parameters)
{
    M626FogWork *work;
    s32 i;
    OMOBJ *object;
    object = omAddObjEx(manager, 26, 0, 0, OM_GRP_NONE, fn_1_FEC);
    lbl_1_bss_2C0 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M626FogWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M626FogWork));
    work->parameters = parameters;
    work->factor = 1.0f;
    for (i = 0; parameters->layer != -1; i++, parameters++) {
        if (i == 0)
            Hu3DFogSet(parameters->start, parameters->end, parameters->color.r, parameters->color.g,
                       parameters->color.b);
        Hu3DLayerHookSet(parameters->layer, fn_1_1044);
    }
}

/* Runs the registered fog callback from the fog object's update. */
void fn_1_FEC(OMOBJ *obj)
{
    M626FogWork *light = omObjGetDataAs(lbl_1_bss_2C0, M626FogWork);
    if (light->callback != (OMOBJ_FUNC)(void *)0) {
        light->callback(obj);
    }
}

/* Applies the selected fog entry when its render layer is drawn. */
void fn_1_1044(s16 layer)
{
    M626FogWork *work;
    M626FogParameters *parameters;
    float fov, nearPlane, farPlane;
    float end;
    work = lbl_1_bss_2C0->data;
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

/* Stores the callback run by the fog object's per-frame update. */
void fn_1_1138(OMOBJ_FUNC callback) {
    M626FogWork *fogWork;

    if (lbl_1_bss_2C0 != NULL) {
        fogWork = lbl_1_bss_2C0->data;
        fogWork->callback = callback;
    }
}

/* Sets the fog distance interpolation factor when the scene changes fog. */
void fn_1_1178(f32 fogFactor) {
    M626FogWork *fogWork;

    if (lbl_1_bss_2C0 != NULL) {
        fogWork = omObjGetDataAs(lbl_1_bss_2C0, M626FogWork);
        fogWork->factor = fogFactor;
    }
}

/* Creates minigame cameras and their shadow settings during scene setup. */
void fn_1_11B8(OMOBJMAN *manager, u32 cameras, f32 fov, f32 nearPlane, f32 farPlane,
               f32 shadowRadius, OM_CAMERA_VIEW *view)
{
    M626CameraEntry *entry;
    M626CameraWork *work;
    OMOBJ *object;
    s32 i;
    u32 bit;
    s32 count = 0;

    object = omAddObjEx(manager, 28, 0, 0, OM_GRP_NONE, fn_1_1514);
    lbl_1_bss_2BC = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M626CameraWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M626CameraWork));
    work->cameras = cameras;
    work->shadowRadius = shadowRadius;
    work->shadowFov = 20.0f;
    work->shadowColor = lbl_1_data_144;
    if (lbl_1_bss_2C4 == NULL) {
        work->shadowDirection = lbl_1_data_138;
    } else {
        work->shadowDirection = *fn_1_E40();
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

/* Updates selected cameras and their viewports once per camera object frame. */
void fn_1_1514(OMOBJ *object)
{
    M626CameraWork *work = object->data;
    M626CameraEntry *entry;
    s32 i, shadowDone = 0;
    u32 bit;

    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (work->cameras & bit) {
            entry = &work->entries[i];
            Hu3DCameraPosGet(bit, &entry->position, &entry->up, &entry->target);
            Hu3DCameraPerspectiveGet(bit, &entry->fov, &entry->nearPlane, &entry->farPlane);
            if (shadowDone == 0) {
                if (work->shadowRadius > 0.0) {
                    fn_1_1BAC(&work->shadowDirection, &entry->target,
                              work->shadowFov, work->shadowRadius);
                    Hu3DShadowColSet(work->shadowColor.r, work->shadowColor.g, work->shadowColor.b);
                }
                shadowDone = 1;
            }
            if (work->entries[i].active) {
                fn_1_1738(object, bit, i);
            } else {
                if (work->callback != NULL) { work->callback(object, bit, i); }
                CenterM[i] = entry->view.center;
                CRotM[i] = entry->view.rot;
                CZoomM[i] = entry->view.zoom;
            }
            Hu3DCameraPerspectiveSet(bit, entry->fov, entry->nearPlane, entry->farPlane,
                                     entry->aspect);
            Hu3DCameraViewportSet(bit, entry->viewportX, entry->viewportY, entry->viewportWidth,
                                  entry->viewportHeight, 0.0f, 1.0f);
            Hu3DCameraScissorSet(bit, (u32)entry->viewportX, (u32)entry->viewportY,
                                 (u32)entry->viewportWidth, (u32)entry->viewportHeight);
        }
    }
}

/* Copies a camera-motion view into the shared view arrays during camera update. */
void fn_1_1738(OMOBJ *object, u32 camera, s32 index)
{
    M626CameraWork *work = object->data;
    M626CameraEntry *entry = &work->entries[index];
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

static const Vec lbl_1_rodata_78[2] = {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

extern inline float tanf(float x) { return (float)tan(x); }

/* Positions the scene shadow camera around a target during camera updates. */
void fn_1_1BAC(Vec *direction, Vec *target, f32 fov, f32 radius)
{
    Vec position, unit;
    const Vec *up = lbl_1_rodata_78;
    f32 distance;
    f32 radians;

    radians = 3.141592653589793 * (fov / 2.0f) / 180.0;
    distance = radius / tanf(radians);
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

/* Returns the first set camera-bit index for camera-array lookups. */
int fn_1_1D3C(unsigned int mask)
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

void fn_1_1D80(M626CameraCallback callback) {
    M626CameraWork *state=omObjGetDataAs(lbl_1_bss_2BC,M626CameraWork);
    state->callback=callback;
}

/* Sets the aspect ratio for each selected camera during view configuration. */
void fn_1_1DA8(u32 cameras, float fov)
{
    OMOBJ *object = lbl_1_bss_2BC;
    M626CameraWork *work = object->data;
    s32 index;
    u32 bit;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            M626CameraEntry *entry = &work->entries[index];
            entry->aspect = fov;
        }
    }
}

/* Sets viewport bounds for selected cameras during split-screen setup. */
void fn_1_1E1C(u32 cameras, float x, float y, float width, float height)
{
    OMOBJ *object = lbl_1_bss_2BC;
    M626CameraWork *work = object->data;
    s32 index;
    u32 bit;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            M626CameraEntry *entry = &work->entries[index];
            entry->viewportX = x;
            entry->viewportY = y;
            entry->viewportWidth = width;
            entry->viewportHeight = height;
        }
    }
}

/* Sets the field of view used to construct the camera shadow. */
void fn_1_1E9C(f32 shadowFov) {
    OMOBJ *object = lbl_1_bss_2BC;
    M626CameraWork *cameraWork = object->data;
    cameraWork->shadowFov = shadowFov;
}

/* Copies the current spotlight direction into the camera shadow settings. */
void fn_1_1ECC(HuVecF *direction) {
    OMOBJ *object=lbl_1_bss_2BC;
    if(object != NULL) {
        M626CameraWork *state=omObjGetDataAs(object,M626CameraWork);
        state->shadowDirection=*direction;
    }
}

f32 *fn_1_1F1C() {
    OMOBJ *object = lbl_1_bss_2BC;
    M626CameraWork *light = object->data;
    return &light->shadowDirection.x;
}

/* Changes the shared shadow color when scene lighting is configured. */
void fn_1_1F4C(u8 r, u8 g, u8 b)
{
    OMOBJ *object = lbl_1_bss_2BC;
    M626CameraWork *work = object->data;
    work->shadowColor.r = r;
    work->shadowColor.g = g;
    work->shadowColor.b = b;
}

/* Creates camera-motion models for selected cameras during view setup. */
void fn_1_1F84(u32 cameras, void *data)
{
    OMOBJ *object = lbl_1_bss_2BC;
    M626CameraWork *work = object->data;
    s32 motion = Hu3DMotionCreate(data);
    s32 index;
    u32 bit;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            M626CameraEntry *entry = &work->entries[index];
            entry->motion = motion;
            entry->model = Hu3DModelCameraCreate(motion, bit);
            Hu3DCameraMotionStart(entry->model, bit);
            entry->active = 1;
        }
    }
}

/* Reports whether any selected camera is still using a camera-motion model. */
s32 fn_1_203C(u32 cameras)
{
    OMOBJ *object = lbl_1_bss_2BC;
    M626CameraWork *work = object->data;
    s32 index;
    u32 bit;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            if (work->entries[index].active != 0) {
                return 1;
            }
        }
    }
    return 0;
}

/* Computes a view position and distance that fit the supplied spheres in frame. */
double fn_1_20C0(M626CameraFitParams *params, M626CameraFitSphere *spheres, s32 count, Vec *out)
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
    M626CameraFitSphere *sphere;
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

/* Creates positional-audio tracking when the minigame object manager starts. */
void fn_1_2608(HUPROCESS *process, M626AudioParameters *audioConfig) {
    OMOBJ *object;
    M626AudioWork *audioWork;

    object = omAddObjEx(process, 30, 0, 0, -1, fn_1_26A4);
    lbl_1_bss_2B8 = object;
    audioWork = HuMemDirectMallocNum(HEAP_HEAP, 1544, HU_MEMNUM_OVL);
    object->data = audioWork;
    memset(audioWork, 0, 1544);
    audioWork->config = audioConfig;
    fn_1_270C(object);
}

/* Updates tracked sound handles each frame, then calls the registered callback. */
void fn_1_26A4(OMOBJ *obj) {
    M626AudioWork *state = omObjGetDataAs(lbl_1_bss_2B8, M626AudioWork);

    fn_1_2764(obj);
    if (state->callback != NULL) {
        state->callback(obj);
    }
}

/* Marks every positional-audio handle slot free when audio tracking starts. */
void fn_1_270C(OMOBJ *object)
{
    M626AudioWork *state = omObjGetDataAs(lbl_1_bss_2B8, M626AudioWork);
    M626AudioEntry *record = state->entries;
    int index;

    for (index = 0; index < 64; index++, record++) {
        record->handle = -1;
    }
}

/* Refreshes tracked sound pan and volume from the audio object's frame update. */
void fn_1_2764(OMOBJ *object)
{
    M626AudioWork *work;
    M626AudioEntry *entry;
    s32 i;
    work = lbl_1_bss_2B8->data;
    entry = work->entries;
    for (i = 0; i < 64; i++, entry++) {
        if (entry->handle != -1) {
            if (HuAudFXStatusGet(entry->handle) != 2) {
                entry->handle = -1;
                continue;
            }
            if (entry->updatePan)
                HuAudFXPanning(entry->handle, fn_1_29E8(entry->cameras, entry->position));
            if (entry->updateVolume)
                HuAudFXVolSet(entry->handle, fn_1_2C08(entry->cameras, entry->position));
        }
    }
}

/* Stores the callback run by the positional-audio object each frame. */
void fn_1_2834(OMOBJ_FUNC callback) {
    M626AudioWork *audioWork;

    if (lbl_1_bss_2B8 != NULL) {
        audioWork = lbl_1_bss_2B8->data;
        audioWork->callback = callback;
    }
}

int fn_1_2874(float value) {
    return fn_1_28A0(1,value);
}

/* Maps a screen-space offset to pan using the selected camera audio range. */
s32 fn_1_28A0(u32 cameras, float x)
{
    M626AudioWork *work;
    M626AudioParameters *config;
    s32 pan = 64;

    if (lbl_1_bss_2B8 != NULL) {
        work = lbl_1_bss_2B8->data;
        for (config = work->config; config->cameras; config++) {
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

s32 fn_1_29BC(Vec *object)
{
    return fn_1_29E8(1, object);
}

/* Calculates positional-audio pan for a world point and selected cameras. */
s32 fn_1_29E8(u32 cameras, Vec *position)
{
    M626CameraWork *work = lbl_1_bss_2BC->data;
    M626CameraEntry *camera = &work->entries[fn_1_1D3C(cameras)];
    Vec screen;
    float x;

    Hu3D3Dto2D(position, (s16)cameras, &screen);
    x = screen.x / 576.0f;
    x = 0.9f * camera->viewportX + x * (0.9f * camera->viewportWidth);
    return fn_1_28A0(cameras, x);
}

s32 fn_1_2BDC(Vec *object)
{
    return fn_1_2C08(1, object);
}

/* Calculates positional-audio volume for a world point and selected cameras. */
s32 fn_1_2C08(u32 cameras, Vec *position)
{
    M626AudioWork *audio;
    M626AudioParameters *config;
    M626CameraWork *work;
    M626CameraEntry *camera;
    Mtx matrix;
    Vec local;
    float distance, factor;
    s32 volume = 127;

    if (lbl_1_bss_2B8 != NULL) {
        audio = lbl_1_bss_2B8->data;
        work = lbl_1_bss_2BC->data;
        camera = &work->entries[fn_1_1D3C(cameras)];
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

void fn_1_2E6C(s32 left, Vec *right) {
    fn_1_2EA0(1, left, right);
}

/* Plays a positional sound on each selected camera and tracks its handle. */
s32 fn_1_2EA0(u32 cameras, s32 effect, Vec *position)
{
    s32 index;
    u32 bit;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayVolPan(effect, fn_1_2C08(bit, position),
                                       fn_1_29E8(bit, position));
        }
    }
    return handle;
}

s32 fn_1_3340(s32 left, Vec *right) {
    return fn_1_3374(1, left, right);
}

/* Plays a panned sound for each selected camera and returns the last handle. */
s32 fn_1_3374(u32 cameras, s32 effect, Vec *position)
{
    s32 index;
    u32 bit;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayPan(effect, fn_1_29E8(bit, position));
        }
    }
    return handle;
}

s32 fn_1_35B4(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled)
{
    return fn_1_35F8(1, effect, position, panEnabled, volumeEnabled);
}

/* Plays and tracks a sound with optional positional pan and volume updates. */
s32 fn_1_35F8(u32 cameras, s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled)
{
    s32 index;
    u32 bit;
    s32 pan;
    s32 volume;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            if (panEnabled) pan = fn_1_29E8(bit, position);
            else pan = 64;
            if (volumeEnabled) volume = fn_1_2C08(bit, position);
            else volume = 127;
            handle = HuAudFXPlayVolPan(effect, volume, pan);
            fn_1_3B40(bit, effect, handle, position, panEnabled, volumeEnabled);
        }
    }
    return handle;
}

void fn_1_3AF4(int effect, int handle, Vec *position, int flag, int value) {
    fn_1_3B40(1, effect, handle, position, flag, value);
}

/* Stores a playing sound so its enabled pan and volume can follow its position. */
void fn_1_3B40(unsigned int cameraMask, int effect, int handle, HuVecF *position, int panEnabled,
               int volumeEnabled)
{
    M626AudioWork *state=omObjGetDataAs(lbl_1_bss_2B8,M626AudioWork);
    M626AudioEntry *record=state->entries;
    int index;
    for(index=0;index<64;index++,record++) {
        if(record->handle==-1) {
            record->handle=handle;
            record->effect=effect;
            record->cameras=cameraMask;
            record->updatePan=panEnabled;
            record->updateVolume=volumeEnabled;
            record->position=position;
            break;
        }
    }
}

/* Clears the model and data cache before Snow Brawl creates shared resources. */
void fn_1_3BB8(void) {
    s32 slot;

    slot = 0;
    while (slot < 80) {
        lbl_1_bss_178[slot] = -1;
        lbl_1_bss_38[slot] = -1;
        slot += 1;
    }
}

/* Finds a resource handle in the shared cache or claims its next free slot. */
int fn_1_3C10(int resourceHandle) {
    int index;
    int cachedHandle;
    if(resourceHandle!=-1) {
        for(index=0;index<80;index++) {
            cachedHandle=lbl_1_bss_178[index];
            if(resourceHandle==cachedHandle) return index;
            if(cachedHandle==-1) {
                lbl_1_bss_178[index]=resourceHandle;
                return index;
            }
        }
    }
    return -1;
}

/* Shares a cached model for a data resource requested by a caller. */
int fn_1_3C94(int dataNumber) {
    int cacheIndex=fn_1_3C10(dataNumber);
    int modelId;
    if(cacheIndex>=0) {
        modelId=lbl_1_bss_38[cacheIndex];
        if(modelId==-1) {
            modelId=Hu3DModelCreateData(dataNumber);
            lbl_1_bss_38[cacheIndex]=modelId;
            return modelId;
        }
        return Hu3DModelLink(modelId);
    }
    return -1;
}

/* Shares cached joint-motion data for a model resource requested by a caller. */
s32 fn_1_3D9C(s32 model, s32 dataNum)
{
    s32 slot = fn_1_3C10(dataNum);
    s32 motion;
    if (slot >= 0) {
        motion = lbl_1_bss_38[slot];
        if (motion == -1) {
            motion = Hu3DJointMotionData(model, dataNum);
            lbl_1_bss_38[slot] = motion;
            return motion;
        }
        return motion;
    }
    return -1;
}

/* Interpolates between angles along the shorter arc for movement updates. */
float fn_1_3EAC(float start, float end, float factor)
{
    float angle;

    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    angle = fmodf(start + factor * angle, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    return angle;
}

/* Returns the signed shortest angular difference for movement calculations. */
float fn_1_3FC8(float start, float end)
{
    float angle;

    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    return angle;
}

/* Returns the horizontal heading in degrees from one position to another. */
float fn_1_407C(HuVecF *origin, HuVecF *target) {
    float deltaX = target->x - origin->x;
    float deltaZ = target->z - origin->z;
    return atan2(deltaX, deltaZ) / 3.141592653589793 * 180.0;
}

/* Returns horizontal world-space distance between two positions. */
float fn_1_4118(Vec *first, Vec *second)
{
    float x, z;

    x = second->x - first->x;
    z = second->z - first->z;
    return sqrtf(x * x + z * z);
}

Vec lbl_1_data_138 = {0.0f, -1.0f, 0.0f};
GXColor lbl_1_data_144 = {0, 0, 0, 0};

OMOBJ *lbl_1_bss_2C8;
OMOBJ *lbl_1_bss_2C4;
OMOBJ *lbl_1_bss_2C0;
OMOBJ *lbl_1_bss_2BC;
OMOBJ *lbl_1_bss_2B8;
s32 lbl_1_bss_178[80];
s32 lbl_1_bss_38[80];
