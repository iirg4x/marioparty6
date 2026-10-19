/* Scene setup, camera and light control, positional audio, and motion utilities. */
#include "dolphin/math.h"
#include "dolphin/mtx.h"
#include "game/main.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "REL/m674Dll/scene.h"
#include "string.h"
OMOBJ *lbl_1_bss_2D8;
OMOBJ *lbl_1_bss_2D4;
OMOBJ *lbl_1_bss_2D0;
OMOBJ *lbl_1_bss_2CC;
OMOBJ *lbl_1_bss_2C8;
int lbl_1_bss_188[80];
int lbl_1_bss_48[80];
Vec lbl_1_data_C8 = {0.0f, -1.0f, 0.0f};
GXColor lbl_1_data_D4 = {0, 0, 0, 0};

void fn_1_F8C(OMOBJMAN *manager);
void fn_1_104C(void);
void fn_1_1050(OMOBJ *object);
void fn_1_1054(OMOBJMAN *manager, HuVecF *position, HuVecF *target, GXColor *color);
void fn_1_115C(OMOBJ *obj);
void fn_1_11B4(void);
void fn_1_1218(s32 modelId, char *lightName);
void fn_1_12CC(OMOBJ_FUNC callback);
void fn_1_12F4(HuVecF *position);
void fn_1_135C(HuVecF *direction);
HuVecF *fn_1_13C4();
void fn_1_13EC(int red,int green,int blue,int alpha);
void fn_1_1470(HUPROCESS *manager, M674FogParam *parameters);
void fn_1_1570(OMOBJ *obj);
void fn_1_15C8(s16 layer);
void fn_1_16BC(OMOBJ_FUNC callback);
void fn_1_16FC(f32 value);
void fn_1_173C(OMOBJMAN *manager, u32 cameras, float fov, float nearPlane, float farPlane,
    float shadowRadius, OM_CAMERA_VIEW *view);
void fn_1_1AA8(OMOBJ *object);
void fn_1_1CCC(OMOBJ *object, u32 camera, s32 index);
void fn_1_1DC4(OMOBJ *object, u32 camera, s32 index);
void fn_1_21A8(s32 camera, Vec *direction, Vec *target, float fov, float radius);
int fn_1_2354(unsigned int mask);
void fn_1_2398(CameraCallback callback);
void fn_1_23C0(u32 cameras, float value);
void fn_1_2434(s32 camera, f32 x, f32 y, f32 width, f32 height);
void fn_1_25FC(u32 cameras, float x, float y, float width, float height);
void fn_1_267C(u32 cameras, float x, float y, float width, float height);
void fn_1_26FC(f32 value);
void fn_1_272C(HuVecF *direction);
f32 *fn_1_277C();
void fn_1_27AC(u8 red, u8 green, u8 blue);
void fn_1_27E4(u32 cameras, void *data);
void fn_1_289C(u32 cameraMask);
BOOL fn_1_2D3C(u32 cameras);
double fn_1_2DC0(void *fitParams, void *sphereList, s32 sphereCount, Vec *cameraPosition);
void fn_1_3308(HUPROCESS *process, M674AudioRange *ranges);
void fn_1_33A4(OMOBJ *obj);
void fn_1_340C(OMOBJ *object);
void fn_1_3464(OMOBJ *object);
void fn_1_3534(OMOBJ_FUNC callback);
int fn_1_3574(float value);
s32 fn_1_35A0(s32 cameras, float x);
void fn_1_36BC(void *object);
s32 fn_1_36E8(u32 cameras, HuVecF *position);
void fn_1_38DC(void *object);
s32 fn_1_3908(u32 cameras, HuVecF *position);
s32 fn_1_3B6C(s32 effect, Vec *position);
s32 fn_1_3BA0(u32 cameras, s32 effect, HuVecF *position);
s32 fn_1_4040(s32 effect, Vec *position);
s32 fn_1_4074(u32 cameras, s32 effect, HuVecF *position);
s32 fn_1_42B4(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled);
s32 fn_1_42F8(u32 cameras, s32 effect, HuVecF *position, s32 panEnabled,
              s32 volumeEnabled);
void fn_1_47F4(int effect, int handle, Vec *position, int flag, int value);
void fn_1_4840(unsigned int cameraMask, int effect, int handle, HuVecF *position, int panEnabled,
               int volumeEnabled);
void fn_1_48B8(void);
int fn_1_4910(int handle);
int fn_1_4994(int dataNum);
s32 fn_1_4A9C(s32 model, s32 dataNum);
float fn_1_4BAC(float start, float end, float factor);
f32 fn_1_4CC8(f32 start, f32 end);
float fn_1_4D7C(HuVecF *firstPosition, HuVecF *secondPosition);
float fn_1_4E18(Vec *first, Vec *second);
void fn_1_4F84(Point3d *start, Point3d *end, f32 t, Point3d *out);
s32 fn_1_4FDC(Vec *start, Vec *end, f32 gravity, f32 initialY, Vec *velocity);

/* Selects the first set camera bit, returning zero when the mask is empty. */
static inline s32 cameraIndex(u32 cameras) {
    s32 index;
    for (index = 0; index < 16; index++, cameras >>= 1) {
        if (cameras & 1) return index;
    }
    return 0;
}

/* Creates the scene utility object, clears scene-object pointers, and resets resource caches. */
void fn_1_F8C(OMOBJMAN *manager) {
    OMOBJ *object=omAddObjEx(manager,22,0,0,OM_GRP_NONE,fn_1_1050);
    void *state;
    lbl_1_bss_2D8=object;
    lbl_1_bss_2D4=lbl_1_bss_2D0=lbl_1_bss_2CC=lbl_1_bss_2C8=NULL;
    state=HuMemDirectMallocNum(HEAP_HEAP,4,HU_MEMNUM_OVL);
    object->data=state;
    memset(state,0,4);
    fn_1_48B8();
}

void fn_1_104C(void)
{
}

void fn_1_1050(OMOBJ *object)
{
}

/* Creates the scene light object and stores its initial position, target, and color. */
void fn_1_1054(OMOBJMAN *manager, HuVecF *position, HuVecF *target, GXColor *color)
{
    M674LightWork *work;
    OMOBJ *object;

    object = omAddObjEx(manager, 24, 0, 0, OM_GRP_NONE, fn_1_115C);
    lbl_1_bss_2D4 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M674LightWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M674LightWork));
    work->position = *position;
    work->target = *target;
    work->color = *color;
    work->lightId = -1;
    PSVECSubtract(&work->target, &work->position, &work->direction);
    PSVECNormalize(&work->direction, &work->direction);
}

/* The object manager calls this each frame to run the registered light callback. */
void fn_1_115C(OMOBJ *obj)
{
    M674LightWork *light = omObjGetDataAs(lbl_1_bss_2D4, M674LightWork);
    if (light->callback != (OMOBJ_FUNC)(void *)0) {
        light->callback(obj);
    }
}

/* Creates and configures the scene's global light during light setup. */
void fn_1_11B4(void) {
    M674LightWork *light = omObjGetDataAs(lbl_1_bss_2D4, M674LightWork);

    light->lightId = Hu3DGLightCreateV(&light->position, &light->direction, &light->color);
    Hu3DGLightInfinitytSet(light->lightId);
    Hu3DGLightStaticSet(light->lightId, 1);
    fn_1_272C(&light->direction);
}

/* Selects a named model light, or light zero when no name is supplied. */
void fn_1_1218(s32 modelId, char *lightName) {
    HU3D_LIGHT *globalLights;
    M674LightWork *light;

    light = lbl_1_bss_2D4->data;
    globalLights = Hu3DGlobalLight;
    Hu3DModelLightInfoSet((s16)modelId, 1);
    if (lightName != NULL) {
        light->lightId = Hu3DModelLightIdGet((s16)modelId, lightName);
    } else {
        light->lightId = 0;
    }
    if (light->lightId >= 0) {
        Hu3DGLightParamGet(light->lightId, &light->position, &light->direction, &light->color);
        fn_1_272C(&light->direction);
    }
}

void fn_1_12CC(OMOBJ_FUNC callback) {
    M674LightWork *light=omObjGetDataAs(lbl_1_bss_2D4,M674LightWork);
    light->callback=callback;
}

/* Updates the scene light position and sends it to the active global light. */
void fn_1_12F4(HuVecF *position) {
    M674LightWork *light=omObjGetDataAs(lbl_1_bss_2D4,M674LightWork);
    light->position=*position;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}

/* Updates the scene light direction and sends it to the active global light. */
void fn_1_135C(HuVecF *direction) {
    M674LightWork *light=omObjGetDataAs(lbl_1_bss_2D4,M674LightWork);
    light->direction=*direction;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}

HuVecF *fn_1_13C4() {
    M674LightWork *light=omObjGetDataAs(lbl_1_bss_2D4,M674LightWork);
    return &light->direction;
}

/* Stores and applies the RGBA color of the active scene light. */
void fn_1_13EC(int red,int green,int blue,int alpha) {
    M674LightWork *light=omObjGetDataAs(lbl_1_bss_2D4,M674LightWork);
    light->color.r=red;
    light->color.g=green;
    light->color.b=blue;
    light->color.a=alpha;
    Hu3DGLightColorSet(light->lightId,red,green,blue,alpha);
}

/* Installs per-layer fog settings and creates an object for fog transitions. */
void fn_1_1470(HUPROCESS *manager, M674FogParam *parameters)
{
    M674FogWork *work;
    s32 index;
    OMOBJ *object;

    object = omAddObjEx(manager, 26, 0, 0, OM_GRP_NONE, fn_1_1570);
    lbl_1_bss_2D0 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M674FogWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M674FogWork));
    work->parameters = parameters;
    work->factor = 1.0f;
    for (index = 0; parameters->layer != -1; index++, parameters++) {
        if (index == 0) {
            Hu3DFogSet(parameters->start, parameters->end, parameters->color.r,
                parameters->color.g, parameters->color.b);
        }
        Hu3DLayerHookSet(parameters->layer, fn_1_15C8);
    }
}

/* The object manager calls this each frame to run the registered fog callback. */
void fn_1_1570(OMOBJ *obj)
{
    M674FogWork *work = omObjGetDataAs(lbl_1_bss_2D0, M674FogWork);
    if (work->callback != (OMOBJ_FUNC)(void *)0) {
        work->callback(obj);
    }
}

/* The layer hook applies that layer's fog settings using the current blend factor. */
void fn_1_15C8(s16 layer)
{
    M674FogWork *work;
    M674FogParam *parameters;
    float fov;
    float nearPlane;
    float farPlane;
    float end;

    work = lbl_1_bss_2D0->data;
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

/* Replaces the callback used to update the scene fog object. */
void fn_1_16BC(OMOBJ_FUNC callback) {
    M674FogWork *work;
    if (lbl_1_bss_2D0 != NULL) {
        work = lbl_1_bss_2D0->data;
        work->callback = callback;
    }
}

/* Sets the factor used to blend each configured fog range. */
void fn_1_16FC(f32 value) {
    M674FogWork *work;
    if (lbl_1_bss_2D0 != NULL) {
        work = omObjGetDataAs(lbl_1_bss_2D0, M674FogWork);
        work->factor = value;
    }
}

/* Creates camera state, initializes selected cameras, and installs their update object. */
void fn_1_173C(OMOBJMAN *manager, u32 cameras, float fov, float nearPlane, float farPlane,
    float shadowRadius, OM_CAMERA_VIEW *view)
{
    M674CameraEntry *entry;
    M674CameraWork *work;
    OMOBJ *object;
    s32 i;
    u32 bit;
    s32 count = 0;
    object = omAddObjEx(manager, 28, 0, 0, OM_GRP_NONE, fn_1_1AA8);
    lbl_1_bss_2CC = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M674CameraWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M674CameraWork));
    work->cameras = cameras;
    work->shadowRadius = shadowRadius;
    work->shadowFov = 20.0f;
    work->shadowColor = lbl_1_data_D4;
    if (lbl_1_bss_2D4 == NULL) {
        work->shadowDirection = lbl_1_data_C8;
    } else {
        work->shadowDirection = *fn_1_13C4();
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

/* The camera object refreshes selected camera state, runs callbacks, and applies views each
 * frame. */
void fn_1_1AA8(OMOBJ *object)
{
    M674CameraWork *work = object->data;
    M674CameraEntry *entry;
    s32 i, shadowDone = 0;
    u32 bit;

    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (work->cameras & bit) {
            entry = &work->entries[i];
            Hu3DCameraPosGet(bit, &entry->position, &entry->up, &entry->target);
            Hu3DCameraPerspectiveGet(bit, &entry->fov, &entry->nearPlane, &entry->farPlane);
            if (shadowDone == 0) {
                if (work->shadowRadius > 0.0) {
                    fn_1_21A8(work->cameras, &work->shadowDirection, &entry->target,
                        work->shadowFov, work->shadowRadius);
                    Hu3DShadowColSet(work->shadowColor.r, work->shadowColor.g, work->shadowColor.b);
                }
                shadowDone = 1;
            }
            if (work->entries[i].active) {
                fn_1_1CCC(object, bit, i);
            }
            if (work->callback != NULL) { work->callback(object, bit, i); }
            CenterM[i] = entry->view.center;
            CRotM[i] = entry->view.rot;
            CZoomM[i] = entry->view.zoom;
            Hu3DCameraPerspectiveSet(bit, entry->fov, entry->nearPlane, entry->farPlane,
                                     entry->aspect);
            Hu3DCameraViewportSet(bit, entry->viewport[0], entry->viewport[1], entry->viewport[2],
                entry->viewport[3], 0.0f, 1.0f);
            Hu3DCameraScissorSet(bit, (u32)entry->scissor[0], (u32)entry->scissor[1],
                (u32)entry->scissor[2], (u32)entry->scissor[3]);
        }
    }
}

/* Advances one camera transition and saves its final view when the motion ends. */
void fn_1_1CCC(OMOBJ *object, u32 camera, s32 index)
{
    M674CameraWork *work;
    M674CameraEntry *entry;

    work = object->data;
    entry = &work->entries[index];
    fn_1_1DC4(object, camera, index);
    if (Hu3DMotionEndCheck((s16)entry->model) != 0) {
        Hu3DModelKill((s16)entry->model);
        entry->previousView.center = CenterM[index];
        entry->previousView.rot = CRotM[index];
        entry->previousView.zoom = CZoomM[index];
        entry->active = 0;
    }
}

/* Rebuild center, pitch, yaw, and zoom from camera position and target, forcing roll to zero. */
inline void fn_1_1DC4(OMOBJ *object, u32 camera, s32 index)
{
    M674CameraEntry *entry;
    M674CameraWork *work = object->data;
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
    entry->view.center = CenterM[index];
    entry->view.rot = CRotM[index];
    entry->view.zoom = CZoomM[index];
}

static const Vec lbl_1_rodata_98[2] = {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

/* Float tangent for the shadow camera projection. */
extern inline float tanf(float x) { return (float)tan(x); }

/* Camera setup and updates call this to place a shadow camera along direction and fit its
 * clipping range around target with the supplied radius. */
void fn_1_21A8(s32 camera, Vec *direction, Vec *target, float fov, float radius)
{
    Vec position, unit;
    const Vec *up = lbl_1_rodata_98;
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

/* Returns the first set camera bit index, or zero when the mask is empty. */
int fn_1_2354(unsigned int mask)
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

void fn_1_2398(CameraCallback callback) {
    M674CameraWork *state=omObjGetDataAs(lbl_1_bss_2CC,M674CameraWork);
    state->callback=callback;
}

/* Sets the projection aspect ratio for each selected camera. */
void fn_1_23C0(u32 cameras, float value)
{
    OMOBJ *object = lbl_1_bss_2CC;
    M674CameraEntry *entry;
    s32 i;
    u32 bit;
    M674CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->aspect = value;
        }
    }
}

/* Expand the viewport by size-dependent margins and keep the requested pixel scissor rectangle. */
void fn_1_2434(s32 camera, f32 x, f32 y, f32 width, f32 height)
{
    f32 extraWidth;
    f32 extraHeight;
    f32 offsetX, offsetY;

    offsetX = 0.0f;
    offsetY = 0.0f;
    extraWidth = (f32) (32.0 * (1.0 - (f64) (width / 640.0f)));
    extraHeight = (f32) (80.0 * (1.0 - (f64) (height / 480.0f)));

    if (x + (0.5 * (f64) width) >= 320.0) {
        offsetX = -extraWidth;
    }
    if (y + (0.5 * (f64) height) >= 240.0) {
        offsetY = -extraHeight;
    }
    fn_1_25FC(camera, x + offsetX, y + offsetY, width + extraWidth, height + extraHeight);
    fn_1_267C(camera, x, y, width, height);
}

/* Store a pixel viewport rectangle for each selected camera. */
void fn_1_25FC(u32 cameras, float x, float y, float width, float height)
{
    M674CameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2CC;
    M674CameraWork *work = object->data;
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

/* Store a pixel scissor rectangle for each selected camera. */
void fn_1_267C(u32 cameras, float x, float y, float width, float height)
{
    M674CameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2CC;
    M674CameraWork *work = object->data;
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

/* Sets the field of view used when building the scene shadow projection. */
void fn_1_26FC(f32 value) {
    OMOBJ *object = (OMOBJ *)lbl_1_bss_2CC;
    M674CameraWork *work = (M674CameraWork *)object->data;
    work->shadowFov = value;
}

/* Copies the scene shadow direction into the camera object's shadow state. */
void fn_1_272C(HuVecF *direction) {
    OMOBJ *object=lbl_1_bss_2CC;
    if(object != NULL) {
        M674CameraWork *state=omObjGetDataAs(object,M674CameraWork);
        state->shadowDirection=*direction;
    }
}

/* Returns a pointer to the X component of the stored shadow direction. */
f32 *fn_1_277C() {
    OMOBJ *object = (OMOBJ *)lbl_1_bss_2CC;
    M674CameraWork *work = (M674CameraWork *)object->data;
    return &work->shadowDirection.x;
}

/* Sets the RGB color used for the scene's shadow projection. */
void fn_1_27AC(u8 red, u8 green, u8 blue)
{
    OMOBJ *object = (OMOBJ *)lbl_1_bss_2CC;
    M674CameraWork *work = (M674CameraWork *)object->data;
    work->shadowColor.r = red;
    work->shadowColor.g = green;
    work->shadowColor.b = blue;
}

/* Attaches a camera motion model to each selected camera entry. */
void fn_1_27E4(u32 cameras, void *data)
{
    M674CameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2CC;
    M674CameraWork *work = object->data;
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

/* Stops active camera motions for the selected cameras and saves their final views. */
void fn_1_289C(u32 cameraMask)
{
    u32 bit;
    OMOBJ *object;
    M674CameraWork *work;
    M674CameraEntry *entry;
    s32 index;

    object = lbl_1_bss_2CC;
    work = object->data;
    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if ((cameraMask & bit) != 0) {
            entry = &work->entries[index];
            if (entry->active != 0) {
                fn_1_1DC4(object, bit, index);
                Hu3DModelKill((s16)entry->model);
                entry->previousView.center = CenterM[index];
                entry->previousView.rot = CRotM[index];
                entry->previousView.zoom = CZoomM[index];
                entry->active = 0;
            }
        }
    }
}

/* Reports whether any selected camera currently has an active motion model. */
BOOL fn_1_2D3C(u32 cameras)
{
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2CC;
    M674CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if ((cameras & bit) && work->entries[i].active) {
            return TRUE;
        }
    }
    return FALSE;
}

#define FIT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FIT_MAX(a, b) ((a) > (b) ? (a) : (b))

/* Fits a camera position so the supplied world-space spheres fit inside its view. */
double fn_1_2DC0(void *fitParams, void *sphereList, s32 sphereCount, Vec *cameraPosition)
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
    CameraFitSphere *boundingSphere;
    s32 i;

    MTXLookAt(view, &origin, &((CameraFitParams *) fitParams)->up,
              &((CameraFitParams *) fitParams)->direction);
    MTXInverse(view, inverse);
    {
        double aspect = (double) ((CameraFitParams *) fitParams)->width /
                        ((CameraFitParams *) fitParams)->height;
        tangent = tan(((CameraFitParams *) fitParams)->fov / 2.0f);
        slopeX = aspect * (tangent * ((double) (((CameraFitParams *) fitParams)->width -
                                                2 * ((CameraFitParams *) fitParams)->marginX) /
                                      ((CameraFitParams *) fitParams)->width));
        slopeY = tangent * ((double) (((CameraFitParams *) fitParams)->height -
                                      2 * ((CameraFitParams *) fitParams)->marginY) /
                            ((CameraFitParams *) fitParams)->height);
    }
    boundingSphere = (CameraFitSphere *) sphereList;
    MTXMultVecSR(view, &boundingSphere->center, &lower);
    upper = lower;
    nearZ = lower.z - boundingSphere->radius;
    boundingSphere++;
    for (i = 1; i < sphereCount; i++, boundingSphere++) {
        Vec position;
        MTXMultVecSR(view, &boundingSphere->center, &position);
        nearZ = FIT_MIN(nearZ, position.z - boundingSphere->radius);
    }
    for (boundingSphere = (CameraFitSphere *) sphereList, i = 0; i < sphereCount;
         i++, boundingSphere++) {
        Vec position;
        MTXMultVecSR(view, &boundingSphere->center, &position);
        extentX = boundingSphere->radius + slopeX * (position.z - nearZ);
        extentY = boundingSphere->radius + slopeY * (position.z - nearZ);
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
        VECNormalize(&((CameraFitParams *) fitParams)->direction, &offset);
        VECScale(&offset, &offset, distance);
        VECSubtract(&center, &offset, cameraPosition);
    }
    return distance;
}

/* Creates the positional-audio object and initializes its active-sound records. */
void fn_1_3308(HUPROCESS *process, M674AudioRange *ranges) {
    OMOBJ *object;
    M674AudioState *data;
    object = omAddObjEx(process, 30, 0, 0, -1, fn_1_33A4);
    lbl_1_bss_2C8 = object;
    data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M674AudioState), HU_MEMNUM_OVL);
    object->data = data;
    memset(data, 0, sizeof(M674AudioState));
    data->ranges = ranges;
    fn_1_340C(object);
}

/* The audio object updates active sounds, then calls the registered scene callback. */
void fn_1_33A4(OMOBJ *obj) {
    M674AudioState *state = omObjGetDataAs(lbl_1_bss_2C8, M674AudioState);

    fn_1_3464(obj);
    if (state->callback != NULL) {
        state->callback(obj);
    }
}

/* Marks every positional-audio sound record as unused. */
void fn_1_340C(OMOBJ *object)
{
    M674AudioState *state = omObjGetDataAs(lbl_1_bss_2C8, M674AudioState);
    M674AudioRecord *record = state->records;
    int index;

    for (index = 0; index < 64; index++, record++) {
        record->handle = -1;
    }
}

/* Runs each frame to update pan and volume for active positional sounds. */
void fn_1_3464(OMOBJ *object)
{
    M674AudioState *state = lbl_1_bss_2C8->data;
    M674AudioRecord *record = state->records;
    int index;

    for (index = 0; index < 64; index++, record++) {
        if (record->handle != -1) {
            if (HuAudFXStatusGet(record->handle) != 2) {
                record->handle = -1;
            } else {
                if (record->panEnabled) {
                    HuAudFXPanning(record->handle,
                        (s16)fn_1_36E8(record->cameraMask, record->position));
                }
                if (record->volumeEnabled) {
                    HuAudFXVolSet(record->handle,
                        (s16)fn_1_3908(record->cameraMask, record->position));
                }
            }
        }
    }
}

/* Replaces the callback run by the positional-audio object each frame. */
void fn_1_3534(OMOBJ_FUNC callback) {
    M674AudioState *work;
    if (lbl_1_bss_2C8 != NULL) {
        work = lbl_1_bss_2C8->data;
        work->callback = callback;
    }
}

int fn_1_3574(float value) {
    return fn_1_35A0(1,value);
}

/* Map screen X on the 0..576 horizontal range to the configured audio pan limits. */
inline s32 fn_1_35A0(s32 cameras, float x)
{
    M674AudioState *state;
    M674AudioRange *range;
    s32 pan = 64;

    if (lbl_1_bss_2C8 != NULL) {
        state = omObjGetDataAs(lbl_1_bss_2C8, M674AudioState);
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

/* Call the camera-one pan calculation without a C return statement; module callers nevertheless
 * declare this wrapper as returning s32 and use its result. */
void fn_1_36BC(void *object)
{
    fn_1_36E8(1, object);
}

/* Projects a world position through a camera and maps its X coordinate to audio pan. */
inline s32 fn_1_36E8(u32 cameras, HuVecF *position)
{
    M674CameraWork *work = omObjGetDataAs(lbl_1_bss_2CC, M674CameraWork);
    M674CameraEntry *camera = &work->entries[cameraIndex(cameras)];
    HuVecF screen;
    float x;

    Hu3D3Dto2D(position, (s16)cameras, &screen);
    x = screen.x / 576.0f;
    x = 0.9f * camera->viewport[0] + x * (0.9f * camera->viewport[2]);
    return fn_1_35A0(cameras, x);
}

void fn_1_38DC(void *object)
{
    fn_1_3908(1, object);
}

/* Calculates sound volume from camera-space depth and the configured distance ranges. */
inline s32 fn_1_3908(u32 cameras, HuVecF *position)
{
    M674AudioState *audio;
    M674AudioRange *range;
    M674CameraWork *work;
    M674CameraEntry *camera;
    Mtx matrix;
    HuVecF local;
    float distance;
    float factor;
    s32 volume = 127;

    if (lbl_1_bss_2C8 != NULL) {
        audio = omObjGetDataAs(lbl_1_bss_2C8, M674AudioState);
        work = omObjGetDataAs(lbl_1_bss_2CC, M674CameraWork);
        camera = &work->entries[cameraIndex(cameras)];
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

s32 fn_1_3B6C(s32 effect, Vec *position) {
    return fn_1_3BA0(1, effect, position);
}

/* Plays a sound for each selected camera using projected volume and pan. */
s32 fn_1_3BA0(u32 cameras, s32 effect, HuVecF *position)
{
    s32 index;
    u32 bit;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayVolPan(effect, fn_1_3908(bit, position),
                                       fn_1_36E8(bit, position));
        }
    }
    return handle;
}

s32 fn_1_4040(s32 effect, Vec *position) {
    return fn_1_4074(1, effect, position);
}

/* Plays a sound for each selected camera using projected pan. */
s32 fn_1_4074(u32 cameras, s32 effect, HuVecF *position)
{
    s32 index;
    u32 bit;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayPan(effect, fn_1_36E8(bit, position));
        }
    }
    return handle;
}

s32 fn_1_42B4(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled)
{
    return fn_1_42F8(1, effect, position, panEnabled, volumeEnabled);
}

/* Play an effect for each selected camera with optional positional pan and volume, register its
 * position pointer for later updates, and return the last handle. */
s32 fn_1_42F8(u32 cameras, s32 effect, HuVecF *position, s32 panEnabled,
              s32 volumeEnabled)
{
    s32 index;
    u32 bit;
    s32 pan;
    s32 volume;
    s32 handle = -1;

    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if (cameras & bit) {
            if (panEnabled) pan = fn_1_36E8(bit, position);
            else pan = 64;
            if (volumeEnabled) volume = fn_1_3908(bit, position);
            else volume = 127;
            handle = HuAudFXPlayVolPan(effect, volume, pan);
            fn_1_4840(bit, effect, handle, position, panEnabled, volumeEnabled);
        }
    }
    return handle;
}

void fn_1_47F4(int effect, int handle, Vec *position, int flag, int value) {
    fn_1_4840(1, effect, handle, position, flag, value);
}

/* Store a sound and its followed position in the first free audio record; do nothing if full. */
void fn_1_4840(unsigned int cameraMask, int effect, int handle, HuVecF *position, int panEnabled,
               int volumeEnabled)
{
    M674AudioState *state=omObjGetDataAs(lbl_1_bss_2C8,M674AudioState);
    M674AudioRecord *record=state->records;
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

/* Clears the cached data and model slots before scene resource reuse. */
void fn_1_48B8(void) {
    s32 index;
    index = 0;
    while (index < 80) {
        lbl_1_bss_188[index] = -1;
        lbl_1_bss_48[index] = -1;
        index += 1;
    }
}

/* Finds a resource handle in the cache or claims the first unused cache slot. */
int fn_1_4910(int handle) {
    int index;
    int existing;
    if(handle!=-1) {
        for(index=0;index<80;index++) {
            existing=lbl_1_bss_188[index];
            if(handle==existing) return index;
            if(existing==-1) {
                lbl_1_bss_188[index]=handle;
                return index;
            }
        }
    }
    return -1;
}

/* Reuses a cached model or creates it from the requested data number. */
int fn_1_4994(int dataNum) {
    int slot=fn_1_4910(dataNum);
    int model;
    if(slot>=0) {
        model=lbl_1_bss_48[slot];
        if(model==-1) {
            model=Hu3DModelCreateData(dataNum);
            lbl_1_bss_48[slot]=model;
            return model;
        }
        return Hu3DModelLink(model);
    }
    return -1;
}

/* Reuses a cached joint motion or loads it for the requested model. */
s32 fn_1_4A9C(s32 model, s32 dataNum)
{
    s32 slot = fn_1_4910(dataNum);
    s32 motion;

    if (slot >= 0) {
        motion = lbl_1_bss_48[slot];
        if (motion == -1) {
            motion = Hu3DJointMotionData(model, dataNum);
            lbl_1_bss_48[slot] = motion;
            return motion;
        }
        return motion;
    }
    return -1;
}

/* Interpolate the shortest turn from start toward end, then wrap the result to 0..360 degrees. */

float fn_1_4BAC(float start, float end, float factor)
{
    float angle;

    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    angle = fmodf(start + factor * angle, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    return angle;
}

/* Returns the signed shortest turn from start to end in degrees. */
f32 fn_1_4CC8(f32 start, f32 end)
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

/* Returns the horizontal heading from the first position to the second in degrees. */
float fn_1_4D7C(HuVecF *firstPosition, HuVecF *secondPosition) {
    float deltaX = secondPosition->x - firstPosition->x;
    float deltaZ = secondPosition->z - firstPosition->z;
    return atan2(deltaX, deltaZ) / 3.141592653589793 * 180.0;
}

/* Measures the horizontal distance between two world positions. */
float fn_1_4E18(Vec *first, Vec *second)
{
    float x = second->x - first->x;
    float z = second->z - first->z;
    return sqrtf(x * x + z * z);
}

/* Interpolates each coordinate between two points by the supplied factor. */
void fn_1_4F84(Point3d *start, Point3d *end, f32 t, Point3d *out)
{
    out->x = start->x + (t * (end->x - start->x));
    out->y = start->y + (t * (end->y - start->y));
    out->z = start->z + (t * (end->z - start->z));
}

/* Calculate a launch vector and frame estimate; initialY shapes the initial vector length and
 * is replaced by the calculated vertical speed. */
s32 fn_1_4FDC(Vec *start, Vec *end, f32 gravity, f32 initialY, Vec *velocity)
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
