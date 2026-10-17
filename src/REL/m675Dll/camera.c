/* Camera paths, lighting, shadows and positioned audio for Banana Shake. */

#include "dolphin/math.h"
#include "game/object.h"
#include "game/memory.h"
#include "string.h"
#include "game/main.h"
#include "game/charman.h"
#include "game/hu3d.h"
#include "game/gamework.h"
#include "game/wipe.h"
#include "game/mg/timer.h"
#include <string.h>
typedef struct M675LightData {
    HU3D_LIGHTID lightId; /* Global light slot, or -1 before a light is selected. */
    OMOBJ_FUNC callback; /* Optional callback run by the light object's update. */
    Vec position, target, direction; /* World position, initial look-at point and light
                                      * direction. */
    GXColor color; /* Light RGBA components, each from 0 to 255. */
} M675LightData;
typedef struct M675ObjectState {
    void *userData; /* Controller-specific configuration. */
    void (*callback)(OMOBJ *); /* Optional callback run after the controller's own update. */
} M675ObjectState;
#include "dolphin/gx.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/mg/seqman.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#ifndef M675_SHARED_CANONICAL_H
#define M675_SHARED_CANONICAL_H
#define FIT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FIT_MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#define FIT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FIT_MAX(a, b) ((a) > (b) ? (a) : (b))
#undef lbl_1_bss_188
#undef fn_1_4540
#undef lbl_1_bss_48
#undef lbl_1_bss_188
#undef fn_1_45C4
typedef void (*CameraCallback)(OMOBJ *, u32, s32);
typedef struct M675CameraFitParams_s {
    Vec direction; /* Camera viewing direction in world space. */
    Vec up; /* Up vector for the fitting coordinate system. */
    float fov; /* Vertical field of view in radians. */
    s32 width; /* View width in pixels, used to calculate aspect ratio. */
    s32 height; /* View height in pixels. */
    s32 marginX; /* Inset on each horizontal edge, in pixels. */
    s32 marginY; /* Inset on each vertical edge, in pixels. */
} M675CameraFitParams;
typedef struct M675CameraFitSphere_s {
    Vec center; /* World-space center of an object to include in the camera view. */
    float radius; /* Bounding radius in world units. */
} M675CameraFitSphere;
typedef struct M675AudioConfigView_s {
    u32 cameras; /* Cameras sharing this mapping; zero terminates the configuration list. */
    s32 panMin, panMax; /* Pan endpoints for the horizontal 0 to 576 mapping. */
    s32 volumeMin, volumeMiddle, volumeMax; /* Volume at the far, middle and near depths. */
    float distanceFar, distanceMiddle, distanceNear; /* Camera-space depths in world units. */
} M675AudioConfigView;
typedef struct M675FogParam_s {
    s32 layer; /* Render layer using this fog; -1 terminates the list. */
    u32 cameras; /* Camera mask supplying the near and far clipping distances. */
    GXFogType type; /* GX fog projection and falloff mode. */
    float start, end; /* Fog distances in world units before scaling the end distance. */
    GXColor color; /* Fog RGBA color passed to GX. */
} M675FogParam;
typedef struct M675FogWork_s {
    M675FogParam *parameters; /* Fog records, terminated by layer -1. */
    s32 callbackWord; /* Optional object callback stored as a word; zero disables it. */
    float factor; /* Interpolation factor from each fog record's start to its end. */
} M675FogWork;
typedef struct M675CameraEntry_s {
    s32 active, motion, model; /* Motion-active flag, motion resource and camera model ID. */
    OM_CAMERA_VIEW view, previousView; /* Current view and view saved when camera motion stops. */
    Vec position, up, target; /* World-space camera pose read before this frame's view update. */
    float fov, nearPlane, farPlane, aspect; /* FOV in degrees, world clipping planes and aspect. */
    float viewport[4]; /* Pixel x, y, width and height. */
    float scissor[4]; /* Pixel x, y, width and height, converted to integers when applied. */
    s32 setupState; /* Zero requests the shadow callback's one-time camera-motion stop. */
    s32 setupTimer; /* Reset when the shadow callback advances the entry's setup state. */
    u8 unmappedCameraStorage[24]; /* Remaining storage cleared during camera setup. */
} M675CameraEntry;
typedef struct M675CameraWork_s {
    u32 cameras; /* Mask of cameras updated by this controller. */
    float shadowRadius, shadowFov; /* Shadow coverage in world units and FOV in degrees. */
    Vec shadowDirection; /* Light direction used to place the shadow camera. */
    GXColor shadowColor; /* Shadow tint; the update applies its RGB components. */
    void (*callback)(OMOBJ *, u32, s32); /* Per-camera update, passed its mask and index. */
    M675CameraEntry entries[16]; /* One view and optional motion model per camera bit. */
    s32 initializationCount; /* Shared guard for the shadow callback's one-time setup. */
    u8 unmappedManagerStorage[4]; /* Remaining storage cleared with the manager allocation. */
} M675CameraWork;
typedef struct M675AudioEntry_s {
    s32 handle, effect; /* Playing sound handle (-1 means unused) and sound effect ID. */
    u32 cameras; /* Camera mask used to calculate pan and volume. */
    s32 updatePan, updateVolume; /* Nonzero enables per-frame tracking of each setting. */
    Vec *position; /* Live world position; the sound slot keeps this pointer. */
} M675AudioEntry;
typedef struct M675AudioWork_s {
    M675AudioConfigView *config; /* Pan/volume mappings, terminated by a zero camera mask. */
    s32 callbackWord; /* Optional object callback stored as a word; zero disables it. */
    M675AudioEntry entries[64]; /* Tracked positional sounds; completed sounds free their slots. */
} M675AudioWork;
OMOBJ *lbl_1_bss_2D8; /* Shared controller object created at minigame startup. */
OMOBJ *lbl_1_bss_2D4; /* Lighting controller. */
OMOBJ *lbl_1_bss_2D0; /* Fog controller. */
OMOBJ *lbl_1_bss_2CC; /* Camera and shadow controller. */
OMOBJ *lbl_1_bss_2C8; /* Positional-audio controller. */
s32 lbl_1_bss_188[80]; /* Cached model/motion data numbers; -1 marks an empty slot. */
s32 lbl_1_bss_48[80]; /* Model or motion IDs paired with the cached data numbers. */
void fn_1_C80(OMOBJ *object);
void fn_1_44E8(void);
void fn_1_D8C(OMOBJ *obj);
void fn_1_235C(HuVecF *direction);
void fn_1_4470(unsigned int cameraMask, int effect, int handle, HuVecF *position, int panEnabled,
               int volumeEnabled);
void fn_1_11A0(OMOBJ *obj);
void fn_1_11F8(s16 layer);
void fn_1_16D8(OMOBJ *object);

Vec lbl_1_data_F0 = {0.0f, -1.0f, 0.0f};
GXColor lbl_1_data_FC = {0, 0, 0, 0};
void fn_1_1DD8(s32 camera, Vec *direction, Vec *target, float fov, float radius);
void fn_1_19F4(OMOBJ *object, u32 camera, s32 index);
void fn_1_222C(u32 cameras, float x, float y, float width, float height);
void fn_1_22AC(u32 cameras, float x, float y, float width, float height);
void fn_1_2FD4(OMOBJ *obj);
void fn_1_303C(OMOBJ *object);
void fn_1_3094(OMOBJ *object);
s32 fn_1_3318(u32 cameras, Vec *position);
s32 fn_1_3538(u32 cameras, Vec *position);
s32 fn_1_31D0(u32 cameras, float x);
s32 fn_1_37D0(u32 cameras, s32 effect, Vec *position);
s32 fn_1_3CA4(u32 cameras, s32 effect, Vec *position);
s32 fn_1_3F28(u32 cameras, s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled);

/* Audio pan and volume helpers use this to select the first camera in a mask. */
static inline s32 m675_local_273C(u32 cameras)
{
    s32 i;
    for (i = 0; i < 16; i++, cameras >>= 1) {
        if (cameras & 1) { return i; }
    }
    /* An empty mask uses camera zero. */
    return 0;
}

/* Audio pan helpers map a horizontal 0 to 576 coordinate through the camera's pan range. */
static inline s32 m675_local_34E8(u32 cameras, float x)
{
    M675AudioWork *work;
    M675AudioConfigView *config;
    s32 pan = 64;
    if (lbl_1_bss_2C8 != NULL) {
        work = lbl_1_bss_2C8->data;
        for (config = work->config; config->cameras; config++) {
            if (config->cameras & cameras) {
                x *= (config->panMax - config->panMin) / 576.0f;
                pan = config->panMin + x;
                if (pan < config->panMin) { pan = config->panMin; }
                if (pan > config->panMax) { pan = config->panMax; }
                break;
            }
        }
    }
    return pan;
}

/* Positional sound playback calls this to project a source and calculate its pan. */
static inline s32 m675_local_3630(u32 cameras, Vec *position)
{
    M675CameraWork *work = lbl_1_bss_2CC->data;
    M675CameraEntry *camera = &work->entries[m675_local_273C(cameras)];
    Vec screen;
    float x;
    Hu3D3Dto2D(position, cameras, &screen);
    x = screen.x / 576.0f;
    x = 0.9f * camera->viewport[0] + x * (0.9f * camera->viewport[2]);
    return m675_local_34E8(cameras, x);
}

/* Positional sound playback calls this to scale volume by camera-space distance. */
static inline s32 m675_local_3850(u32 cameras, Vec *position)
{
    M675AudioWork *audio;
    M675AudioConfigView *config;
    M675CameraWork *work;
    M675CameraEntry *camera;
    Mtx matrix;
    Vec cameraPosition;
    float distance, factor;
    s32 volume = 127;
    if (lbl_1_bss_2C8 != NULL) {
        audio = lbl_1_bss_2C8->data;
        work = lbl_1_bss_2CC->data;
        camera = &work->entries[m675_local_273C(cameras)];
        for (config = audio->config; config->cameras; config++) {
            if (config->cameras & cameras) {
                C_MTXLookAt(matrix, &camera->position, &camera->up, &camera->target);
                PSMTXMultVec(matrix, position, &cameraPosition);
                distance = -cameraPosition.z;
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
/* Camera setup takes the light controller's direction for its shadow camera. */
static inline Vec *m675_local_1810_018BC(void)
{
    M675LightData *work = lbl_1_bss_2D4->data;
    return &work->direction;
}

extern inline float tanf(float x) { return (float)tan(x); }

/* The motion cache uses this to find an existing handle or claim an empty slot. */
static inline s32 m675_local_4858_049E4(s32 handle)
{
    s32 index;
    s32 existing;
    if (handle != -1) {
        for (index = 0; index < 80; index++) {
            existing = lbl_1_bss_188[index];
            if (handle == existing) return index;
            if (existing == -1) {
                lbl_1_bss_188[index] = handle;
                return index;
            }
        }
    }
    return -1;
}
void fn_1_18FC(OMOBJ *, u32, s32);
void fn_1_19F4(OMOBJ *, u32, s32);
/* Minigame setup creates the shared controller object and clears its controller handles. */
void fn_1_BBC(OMOBJMAN *manager) {
    OMOBJ *object=omAddObjEx(manager,22,0,0,OM_GRP_NONE,fn_1_C80);
    void *state;
    lbl_1_bss_2D8=object;
    lbl_1_bss_2D4=lbl_1_bss_2D0=lbl_1_bss_2CC=lbl_1_bss_2C8=NULL;
    state=HuMemDirectMallocNum(HEAP_HEAP,4,HU_MEMNUM_OVL);
    object->data=state;
    memset(state,0,4);
    fn_1_44E8();
}
/* This shared-controller hook performs no work when called. */
void fn_1_C7C(void)
{
}
/* The shared controller's scheduled update intentionally performs no per-frame work. */
void fn_1_C80(OMOBJ *object)
{
}
/* Light setup creates a controller that keeps the light's position, target and color. */
void fn_1_C84(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color)
{
    M675LightData *work;
    OMOBJ *object;
    object = omAddObjEx(manager, 24, 0, 0, OM_GRP_NONE, fn_1_D8C);
    lbl_1_bss_2D4 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M675LightData), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M675LightData));
    work->position = *position;
    work->target = *target;
    work->color = *color;
    work->lightId = -1;
    PSVECSubtract(&work->target, &work->position, &work->direction);
    PSVECNormalize(&work->direction, &work->direction);
}
/* The light object's update callback invokes the callback registered for its controller. */
void fn_1_D8C(OMOBJ *obj)
{
    M675LightData *light = omObjGetDataAs(lbl_1_bss_2D4, M675LightData);
    if (light->callback != (OMOBJ_FUNC)(void *)0) {
        light->callback(obj);
    }
}
/* Light setup creates a directional light and copies its direction to the shadow controller. */
void fn_1_DE4(void) {
    M675LightData *light = omObjGetDataAs(lbl_1_bss_2D4, M675LightData);

    light->lightId = Hu3DGLightCreateV(&light->position, &light->direction, &light->color);
    Hu3DGLightInfinitytSet(light->lightId);
    Hu3DGLightStaticSet(light->lightId, 1);
    fn_1_235C(&light->direction);
}

/* Model-light setup imports the model's lights and selects the requested named light. */
void fn_1_E48(s16 modelId, char *lightName, s32 unusedMode) {
    HU3D_LIGHT *globalLightTable;
    M675LightData *light;

    light = lbl_1_bss_2D4->data;
    globalLightTable = Hu3DGlobalLight;
    Hu3DModelLightInfoSet((s16) modelId, 1);
    /* An omitted name selects global slot zero; the supplied mode is ignored. */
    if (lightName != NULL) {
        light->lightId = Hu3DModelLightIdGet((s16) modelId, lightName);
    } else {
        light->lightId = 0;
    }
    if (light->lightId >= 0) {
        Hu3DGLightParamGet(light->lightId, &light->position,
                           &light->direction,
                           &light->color);
        fn_1_235C(&light->direction);
    }
}
/* Light control registers the callback run by the light object's per-frame update. */
void fn_1_EFC(OMOBJ_FUNC callback) {
    M675LightData *light=omObjGetDataAs(lbl_1_bss_2D4,M675LightData);
    light->callback=callback;
}
/* Light control moves the active light while preserving its stored direction. */
void fn_1_F24(HuVecF *position) {
    M675LightData *light=omObjGetDataAs(lbl_1_bss_2D4,M675LightData);
    light->position=*position;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}
/* Light control aims the active light; its stored direction remains as supplied. */
void fn_1_F8C(HuVecF *direction) {
    M675LightData *light=omObjGetDataAs(lbl_1_bss_2D4,M675LightData);
    light->direction=*direction;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}
/* Light control exposes the stored direction for callers that need its current value. */
HuVecF *fn_1_FF4() {
    M675LightData *light=omObjGetDataAs(lbl_1_bss_2D4,M675LightData);
    return &light->direction;
}
/* Light-control code updates the stored RGBA color and applies it to the active light. */
void fn_1_101C(int red,int green,int blue,int alpha) {
    M675LightData *light=omObjGetDataAs(lbl_1_bss_2D4,M675LightData);
    light->color.r=red;
    light->color.g=green;
    light->color.b=blue;
    light->color.a=alpha;
    Hu3DGLightColorSet(light->lightId,red,green,blue,alpha);
}
/* Minigame setup stores the fog list and installs a render hook for each listed layer. */
void fn_1_10A0(OMOBJMAN *manager, void *parameters)
{
    M675FogWork *work;
    s32 i;
    OMOBJ *object;
    object = omAddObjEx(manager, 26, 0, 0, OM_GRP_NONE, fn_1_11A0);
    lbl_1_bss_2D0 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M675FogWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M675FogWork));
    work->parameters = ((M675FogParam *)parameters);
    work->factor = 1.0f;
    for (i = 0; ((M675FogParam *)parameters)->layer != -1; i++, ((M675FogParam *)parameters)++) {
        if (i == 0)
            Hu3DFogSet(((M675FogParam *) parameters)->start, ((M675FogParam *) parameters)->end,
                       ((M675FogParam *) parameters)->color.r,
                       ((M675FogParam *) parameters)->color.g,
                       ((M675FogParam *) parameters)->color.b);
        Hu3DLayerHookSet(((M675FogParam *)parameters)->layer, fn_1_11F8);
    }
}
/* The fog object's per-frame update calls its callback word when that word is nonzero. */
void fn_1_11A0(OMOBJ *obj)
{
    M675LightData *fogStateView = omObjGetDataAs(lbl_1_bss_2D0, M675LightData);
    if (fogStateView->callback != (OMOBJ_FUNC)(void *)0) {
        fogStateView->callback(obj);
    }
}
/* A render-layer hook applies that layer's fog range, scaled by the current fog factor. */
void fn_1_11F8(s16 layer)
{
    M675FogWork *work;
    M675FogParam *parameters;
    float fov, nearPlane, farPlane;
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

/* Fog control stores the callback word used by the fog object's update; zero disables it. */
void fn_1_12EC(s32 callbackWord) {
    s32 *fogState;

    if (lbl_1_bss_2D0 != NULL) {
        fogState = lbl_1_bss_2D0->data;
        fogState[1] = callbackWord;
    }
}
/* Fog control changes the factor used to interpolate each layer's fog end distance. */
void fn_1_132C(f32 fogFactor) {
    M675LightData *fogStateView;

    if (lbl_1_bss_2D0 != NULL) {
        fogStateView = omObjGetDataAs(lbl_1_bss_2D0, M675LightData);
        fogStateView->position.x = fogFactor;
    }
}
/* Minigame setup initializes camera views and schedules their controller before view output. */
void fn_1_136C(OMOBJMAN *manager, u32 cameras, float fov, float nearPlane, float farPlane,
    float shadowRadius, OM_CAMERA_VIEW *view)
{
    M675CameraEntry *entry;
    M675CameraWork *work;
    OMOBJ *object;
    s32 i;
    u32 bit;
    s32 count = 0;
    object = omAddObjEx(manager, 28, 0, 0, OM_GRP_NONE, fn_1_16D8);
    lbl_1_bss_2CC = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M675CameraWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M675CameraWork));
    work->cameras = cameras;
    work->shadowRadius = shadowRadius;
    work->shadowFov = 20.0f;
    work->shadowColor = lbl_1_data_FC;
    if (lbl_1_bss_2D4 == NULL) {
        work->shadowDirection = lbl_1_data_F0;
    } else {
        work->shadowDirection = *m675_local_1810_018BC();
    }
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            /* View output visits through the highest selected camera, including gaps. */
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
/* The camera controller's update refreshes each selected camera and its render settings. */
void fn_1_16D8(OMOBJ *object)
{
    M675CameraWork *work = object->data;
    M675CameraEntry *entry;
    s32 i, shadowDone = 0;
    u32 bit;

    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (work->cameras & bit) {
            entry = &work->entries[i];
            Hu3DCameraPosGet(bit, &entry->position, &entry->up, &entry->target);
            Hu3DCameraPerspectiveGet(bit, &entry->fov, &entry->nearPlane, &entry->farPlane);
            if (shadowDone == 0) {
                if (work->shadowRadius > 0.0) {
                    fn_1_1DD8(work->cameras, &work->shadowDirection, &entry->target,
                        work->shadowFov, work->shadowRadius);
                    Hu3DShadowColSet(work->shadowColor.r, work->shadowColor.g, work->shadowColor.b);
                }
                shadowDone = 1;
            }
            if (work->entries[i].active) {
                fn_1_18FC(object, bit, i);
            }
            if (work->callback != NULL) { work->callback(object, bit, i); }
            CenterM[i] = entry->view.center;
            CRotM[i] = entry->view.rot;
            CZoomM[i] = entry->view.zoom;
            Hu3DCameraPerspectiveSet(bit, entry->fov, entry->nearPlane, entry->farPlane,
                                     entry->aspect);
            Hu3DCameraViewportSet(bit, entry->viewport[0], entry->viewport[1], entry->viewport[2],
                entry->viewport[3], 0.0f, 1.0f);
            Hu3DCameraScissorSet(
                bit, __cvt_fp2unsigned(entry->scissor[0]), __cvt_fp2unsigned(entry->scissor[1]),
                __cvt_fp2unsigned(entry->scissor[2]), __cvt_fp2unsigned(entry->scissor[3]));
        }
    }
}
/* The camera controller updates a motion and saves its final view when it reaches an end. */
void fn_1_18FC(OMOBJ *object, u32 camera, s32 index)
{
    M675CameraWork *work;
    M675CameraEntry *entry;

    work = object->data;
    entry = &work->entries[index];
    fn_1_19F4(object, camera, index);
    if (Hu3DMotionEndCheck((s16)entry->model) != 0) {
        Hu3DModelKill((s16)entry->model);
        entry->previousView.center = CenterM[index];
        entry->previousView.rot = CRotM[index];
        entry->previousView.zoom = CZoomM[index];
        entry->active = 0;
    }
}
/* Camera-motion updates derive the view center, rotation and zoom from the camera position. */
void fn_1_19F4(OMOBJ *object, u32 camera, s32 index)
{
    M675CameraEntry *entry;
    M675CameraWork *work = object->data;
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
static const Vec lbl_1_rodata_70[2] = {{0.0f,1.0f,0.0f},{0.0f,0.0f,1.0f}};
/* The camera update fits the shadow view around its target using the requested radius and FOV. */
void fn_1_1DD8(s32 camera, Vec *direction, Vec *target, float fov, float radius)
{
    Vec position, unit;
    const Vec *up = lbl_1_rodata_70;
    float distance;
    float radians;
    radians = M_PI * (fov / 2.0f) / 180.0;
    distance = radius / tanf(radians);
    PSVECNormalize(direction, &unit);
    PSVECScale(&unit, &position, distance);
    PSVECSubtract(target, &position, &position);
    /* Use Z as up for steep light directions to avoid a nearly parallel up vector. */
    if (fabs(unit.y) > 0.5) {
        up++;
    }
    Hu3DShadowMultiCreate(fov, distance - radius, distance + radius, camera);
    Hu3DShadowMultiPosSet(&position, (Vec *)up, target, camera);
    Hu3DShadowMultiSizeSet(480, camera);
    Hu3DShadowMultiTPLvlSet(0.4f, camera);
}
/* Camera helpers use this to turn a camera bit mask into its first camera index. */
int fn_1_1F84(unsigned int mask)
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
/* Minigame setup registers the callback run for each camera before its view is output. */
extern void fn_1_1FC8(CameraCallback callback) {
    M675CameraWork *state=omObjGetDataAs(lbl_1_bss_2CC,M675CameraWork);
    state->callback=callback;
}
/* Camera setup changes the aspect ratio for every camera selected by the mask. */
void fn_1_1FF0(u32 cameras, float aspectRatio)
{
    OMOBJ *object = lbl_1_bss_2CC;
    M675CameraEntry *entry;
    s32 i;
    u32 bit;
    M675CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->aspect = aspectRatio;
        }
    }
}
/* The camera layout helper expands the viewport for margins and keeps the requested scissor. */
void fn_1_2064(u32 cameras, float x, float y, float width, float height)
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
    fn_1_222C(cameras, x + offsetX, y + offsetY, width + extraWidth, height + extraHeight);
    fn_1_22AC(cameras, x, y, width, height);
}
/* Camera layout code sets the viewport rectangle for each selected camera. */
void fn_1_222C(u32 cameras, float x, float y, float width, float height)
{
    M675CameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2CC;
    M675CameraWork *work = object->data;
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
/* Camera layout code sets the scissor rectangle for each selected camera. */
void fn_1_22AC(u32 cameras, float x, float y, float width, float height)
{
    M675CameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2CC;
    M675CameraWork *work = object->data;
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
/* Shadow control changes the field of view used by the camera update, in degrees. */
extern void fn_1_232C(f32 shadowFov) {
    OMOBJ *object = (OMOBJ *)lbl_1_bss_2CC;
    M675LightData *cameraStateView = (M675LightData *)object->data;
    cameraStateView->position.x = shadowFov;
}
/* Light control stores the direction used to aim the camera controller's shadow view. */
void fn_1_235C(HuVecF *direction) {
    OMOBJ *object=lbl_1_bss_2CC;
    if(object != NULL) {
        M675CameraWork *state=omObjGetDataAs(object,M675CameraWork);
        state->shadowDirection=*direction;
    }
}
/* Shadow control exposes the first component of the stored shadow direction for direct edits. */
extern f32 *fn_1_23AC() {
    OMOBJ *object = (OMOBJ *)lbl_1_bss_2CC;
    M675LightData *cameraStateView = (M675LightData *)object->data;
    return &cameraStateView->position.y;
}
/* Shadow setup changes the RGB tint used when the camera controller renders its shadow. */
void fn_1_23DC(u8 red, u8 green, u8 blue)
{
    OMOBJ *object = lbl_1_bss_2CC;
    M675CameraWork *work = object->data;
    work->shadowColor.r = red;
    work->shadowColor.g = green;
    work->shadowColor.b = blue;
}
/* Camera setup creates and starts a camera-motion model for each selected camera. */
void fn_1_2414(u32 cameras, void *data)
{
    M675CameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2CC;
    M675CameraWork *work = object->data;
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
/* Camera control stops active camera-motion models and saves each camera's resulting view. */
void fn_1_24CC(u32 cameraMask)
{
    u32 bit;
    OMOBJ *object;
    M675CameraWork *work;
    M675CameraEntry *entry;
    s32 index;

    object = lbl_1_bss_2CC;
    work = object->data;
    for (index = 0, bit = 1; index < 16; index++, bit <<= 1) {
        if ((cameraMask & bit) != 0) {
            entry = &work->entries[index];
            if (entry->active != 0) {
                fn_1_19F4(object, bit, index);
                Hu3DModelKill((s16)entry->model);
                entry->previousView.center = CenterM[index];
                entry->previousView.rot = CRotM[index];
                entry->previousView.zoom = CZoomM[index];
                entry->active = 0;
            }
        }
    }
}
/* Gameplay checks this to see whether any selected camera still has a motion model active. */
BOOL fn_1_296C(u32 cameras)
{
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2CC;
    M675CameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if ((cameras & bit) && work->entries[i].active) {
            return TRUE;
        }
    }
    return FALSE;
}
/* Camera fitting returns a viewing distance and world position for a nonempty list of spheres. */
double fn_1_29F0(void *fitParameters, void *spheres,
    s32 sphereCount, Vec *cameraPosition)
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
    M675CameraFitSphere *sphere;
    s32 i;

    MTXLookAt(view, &origin, &((M675CameraFitParams *) fitParameters)->up,
              &((M675CameraFitParams *) fitParameters)->direction);
    MTXInverse(view, inverse);
    {
        double aspect = (double) ((M675CameraFitParams *) fitParameters)->width /
                        ((M675CameraFitParams *) fitParameters)->height;
        tangent = tan(((M675CameraFitParams *)fitParameters)->fov / 2.0f);
        slopeX =
            aspect * (tangent * ((double) (((M675CameraFitParams *) fitParameters)->width -
                                           2 * ((M675CameraFitParams *) fitParameters)->marginX) /
                                 ((M675CameraFitParams *) fitParameters)->width));
        slopeY = tangent * ((double) (((M675CameraFitParams *) fitParameters)->height -
                                      2 * ((M675CameraFitParams *) fitParameters)->marginY) /
                            ((M675CameraFitParams *) fitParameters)->height);
    }
    sphere = ((M675CameraFitSphere *)spheres);
    MTXMultVecSR(view, &sphere->center, &lower);
    upper = lower;
    nearZ = lower.z - sphere->radius;
    sphere++;
    for (i = 1; i < sphereCount; i++, sphere++) {
        Vec position;
        MTXMultVecSR(view, &sphere->center, &position);
        nearZ = FIT_MIN(nearZ, position.z - sphere->radius);
    }
    /* Expand each sphere at the nearest depth before choosing the limiting view dimension. */
    for (sphere = ((M675CameraFitSphere *)spheres), i = 0; i < sphereCount; i++, sphere++) {
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
        VECNormalize(&((M675CameraFitParams *)fitParameters)->direction, &offset);
        VECScale(&offset, &offset, distance);
        VECSubtract(&center, &offset, cameraPosition);
    }
    return distance;
}

/* Minigame setup creates the positional-audio controller and initializes its sound slots. */
void fn_1_2F38(HUPROCESS *objectManager, u32 audioConfig) {
    OMOBJ *object;
    u32 *objectState;

    object = omAddObjEx(objectManager, 30, 0, 0, -1, fn_1_2FD4);
    lbl_1_bss_2C8 = object;
    objectState = (u32 *)HuMemDirectMallocNum(HEAP_HEAP, 1544, HU_MEMNUM_OVL);
    object->data = objectState;
    memset(objectState, 0, 1544);
    *objectState = audioConfig;
    fn_1_303C(object);
}
/* The audio controller updates positional sounds, then runs its registered object callback. */
void fn_1_2FD4(OMOBJ *obj) {
    M675ObjectState *state = omObjGetDataAs(lbl_1_bss_2C8, M675ObjectState);

    fn_1_3094(obj);
    if (state->callback != NULL) {
        state->callback(obj);
    }
}
/* Audio setup marks all 64 positional-sound slots as unused. */
void fn_1_303C(OMOBJ *object)
{
    M675AudioWork *state = omObjGetDataAs(lbl_1_bss_2C8, M675AudioWork);
    M675AudioEntry *record = state->entries;
    int index;

    for (index = 0; index < 64; index++, record++) {
        record->handle = -1;
    }
}
/* The audio controller checks active sounds each frame and refreshes enabled pan and volume. */
void fn_1_3094(OMOBJ *object)
{
    M675AudioWork *work;
    M675AudioEntry *entry;
    s32 i;
    work = lbl_1_bss_2C8->data;
    entry = work->entries;
    for (i = 0; i < 64; i++, entry++) {
        if (entry->handle != -1) {
            if (HuAudFXStatusGet(entry->handle) != 2) {
                entry->handle = -1;
                continue;
            }
            if (entry->updatePan)
                HuAudFXPanning(entry->handle, fn_1_3318(entry->cameras, entry->position));
            if (entry->updateVolume)
                HuAudFXVolSet(entry->handle, fn_1_3538(entry->cameras, entry->position));
        }
    }
}

/* Audio control stores the callback word run after positional updates; zero disables it. */
void fn_1_3164(s32 callbackWord) {
    s32 *audioState;

    if (lbl_1_bss_2C8 != NULL) {
        audioState = lbl_1_bss_2C8->data;
        audioState[1] = callbackWord;
    }
}
/* Audio control maps a horizontal coordinate to pan using camera zero's configuration. */
extern int fn_1_31A4(float horizontalPosition) {
    return fn_1_31D0(1,horizontalPosition);
}
/* Positional-audio updates map a projected horizontal position into the configured pan range. */
s32 fn_1_31D0(u32 cameras, float x)
{
    M675AudioWork *work;
    M675AudioConfigView *config;
    s32 pan = 64;
    if (lbl_1_bss_2C8 != NULL) {
        work = lbl_1_bss_2C8->data;
        for (config = work->config; config->cameras; config++) {
            if (config->cameras & cameras) {
                x *= (config->panMax - config->panMin) / 576.0f;
                pan = config->panMin + x;
                if (pan < config->panMin) { pan = config->panMin; }
                if (pan > config->panMax) { pan = config->panMax; }
                break;
            }
        }
    }
    return pan;
}
/* Audio control calculates camera zero's positional pan and discards the result. */
void fn_1_32EC(void *position)
{
    fn_1_3318(1, position);
}
/* Positional audio projects a world position and converts its screen x coordinate to pan. */
s32 fn_1_3318(u32 cameras, Vec *position)
{
    M675CameraWork *work = lbl_1_bss_2CC->data;
    M675CameraEntry *camera = &work->entries[m675_local_273C(cameras)];
    Vec screen;
    float x;
    Hu3D3Dto2D(position, cameras, &screen);
    x = screen.x / 576.0f;
    x = 0.9f * camera->viewport[0] + x * (0.9f * camera->viewport[2]);
    return m675_local_34E8(cameras, x);
}
/* Audio control calculates camera zero's positional volume and discards the result. */
void fn_1_350C(void *position)
{
    fn_1_3538(1, position);
}
/* Positional audio measures camera-space depth and maps it to the configured volume range. */
s32 fn_1_3538(u32 cameras, Vec *position)
{
    M675AudioWork *audio;
    M675AudioConfigView *config;
    M675CameraWork *work;
    M675CameraEntry *camera;
    Mtx matrix;
    Vec cameraPosition;
    float distance, factor;
    s32 volume = 127;
    if (lbl_1_bss_2C8 != NULL) {
        audio = lbl_1_bss_2C8->data;
        work = lbl_1_bss_2CC->data;
        camera = &work->entries[m675_local_273C(cameras)];
        for (config = audio->config; config->cameras; config++) {
            if (config->cameras & cameras) {
                C_MTXLookAt(matrix, &camera->position, &camera->up, &camera->target);
                PSMTXMultVec(matrix, position, &cameraPosition);
                distance = -cameraPosition.z;
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
/* Sound callers play a positioned effect with camera zero's pan and distance-based volume. */
void fn_1_379C(s32 effect, Vec *position) {
    fn_1_37D0(1, effect, position);
}
/* Sound playback plays the effect for each selected camera with calculated pan and volume. */
s32 fn_1_37D0(u32 cameras, s32 effect, Vec *position)
{
    s32 i;
    u32 bit;
    s32 handle = -1;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayVolPan(effect, m675_local_3850(bit, position),
                                       m675_local_3630(bit, position));
        }
    }
    return handle;
}
/* Gameplay plays a positioned effect with camera zero's pan and the default sound volume. */
void fn_1_3C70(s32 effect, Vec *position) {
    fn_1_3CA4(1, effect, position);
}
/* fn_1_3C70 calls this to play a positioned effect with pan for each selected camera. */
s32 fn_1_3CA4(u32 cameras, s32 effect, Vec *position)
{
    s32 i;
    u32 bit;
    s32 handle = -1;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayPan(effect, m675_local_3630(bit, position));
        }
    }
    return handle;
}
/* Gameplay starts a camera-zero sound and optionally tracks its live world position each frame. */
s32 fn_1_3EE4(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled)
{
    return fn_1_3F28(1, effect, position, panEnabled, volumeEnabled);
}
/* fn_1_3EE4 uses this to play per-camera audio and track enabled pan or volume updates. */
s32 fn_1_3F28(u32 cameras, s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled)
{
    s32 i;
    u32 bit;
    s32 pan, volume;
    s32 handle = -1;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            if (panEnabled) { pan = m675_local_3630(bit, position); }
            else { pan = 64; }
            if (volumeEnabled) { volume = m675_local_3850(bit, position); }
            else { volume = 127; }
            handle = HuAudFXPlayVolPan(effect, volume, pan);
            fn_1_4470(bit, effect, handle, position, panEnabled, volumeEnabled);
        }
    }
    return handle;
}
/* Audio control registers an existing sound for camera zero's enabled position updates. */
extern void fn_1_4424(int effect, int handle, Vec *position, int panEnabled, int volumeEnabled) {
    fn_1_4470(1, effect, handle, position, panEnabled, volumeEnabled);
}
/* Sound playback claims an empty tracking slot; a full table leaves the new sound untracked. */
void fn_1_4470(unsigned int cameraMask, int effect, int handle, HuVecF *position, int panEnabled,
               int volumeEnabled)
{
    M675AudioWork *state=omObjGetDataAs(lbl_1_bss_2C8,M675AudioWork);
    M675AudioEntry *record=state->entries;
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

/* Minigame setup calls this to mark all model and motion cache entries unused. */
void fn_1_44E8(void) {
    s32 index;

    index = 0;
    while (index < 80) {
        lbl_1_bss_188[index] = -1;
        lbl_1_bss_48[index] = -1;
        index += 1;
    }
}
/* Model loading calls this to find a cached data handle or claim an empty cache slot. */
int fn_1_4540(int handle) {
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
/* Gameplay resource setup loads each model once, then links later instances from this cache. */
int fn_1_45C4(int dataNum) {
    int slot=fn_1_4540(dataNum);
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
/* Gameplay object setup loads and caches joint motions for models as their resources are needed. */
s32 fn_1_46CC(s32 model, s32 dataNum)
{
    s32 slot = m675_local_4858_049E4(dataNum);
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
/* Player and sequence updates turn toward a target along the shortest arc, wrapping each frame. */
float fn_1_47DC(float start, float end, float factor)
{
    float angle;
    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    angle = fmodf(start + factor * angle, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    return angle;
}
/* Player and sequence updates use this signed shortest-angle difference to check turn
 * completion. */
float fn_1_48F8(float start, float end)
{
    float angle;
    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    return angle;
}

/* Prop movement updates calculate the heading in degrees from the current point to its target. */
float fn_1_49AC(HuVecF *currentPosition, HuVecF *nextPosition) {
    float deltaX = nextPosition->x - currentPosition->x;
    float deltaZ = nextPosition->z - currentPosition->z;
    return atan2(deltaX, deltaZ) / 3.141592653589793 * 180.0;
}
/* A gameplay prop update uses horizontal distance to choose its next movement phase. */
float fn_1_4A48(Vec *first, Vec *second)
{
    float x, z;
    x = second->x - first->x;
    z = second->z - first->z;
    return sqrtf(x * x + z * z);
}
/* Camera transitions and actor updates interpolate a point each frame by the supplied factor. */
void fn_1_4BB4(Point3d *start, Point3d *end, Point3d *position, f32 factor)
{
    position->x = start->x + (factor * (end->x - start->x));
    position->y = start->y + (factor * (end->y - start->y));
    position->z = start->z + (factor * (end->z - start->z));
}
/* Sequence movement calls this to calculate a jump velocity and its frame duration. */
s32 fn_1_4C0C(Vec *start, Vec *end, f32 gravity, f32 lengthSeedY, Vec *velocity)
{
    f32 distance;
    f32 verticalDelta;
    f32 verticalSpeed;
    f32 trajectoryDenominator;

    velocity->x = end->x - start->x;
    /* The caller's Y value seeds the length calculation, then the solved speed replaces it. */
    velocity->y = lengthSeedY;
    velocity->z = end->z - start->z;
    distance = PSVECMag(velocity);
    if (0.0 == distance) {
        return 0;
    }
    verticalDelta = end->y - start->y;
    trajectoryDenominator = 2.0f * (distance - verticalDelta);
    if (0.0 == trajectoryDenominator) {
        verticalSpeed = 0.0f;
    } else {
        verticalSpeed = (f32)sqrt((f64)((gravity * (distance * distance)) / trajectoryDenominator));
    }
    PSVECScale(velocity, velocity, verticalSpeed / distance);
    velocity->y = verticalSpeed;
    if (0.0 == verticalSpeed) {
        return 0;
    }
    return (s32)(distance / verticalSpeed);
}
