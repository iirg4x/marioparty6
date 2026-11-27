/* Camera, lighting, audio and resource helpers used by Stamp By Me's minigame objects. */
#include "REL/minigame_positional_audio.h"
#include "REL/minigame_camera_fit.h"
#include "REL/minigame_fog.h"
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

#include "REL/m649Dll/module_types.h"

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

#ifndef M649_SHARED_CANONICAL_H

#define M649_SHARED_CANONICAL_H

#define FIT_MIN(a, b) ((a) < (b) ? (a) : (b))

#define FIT_MAX(a, b) ((a) > (b) ? (a) : (b))

#endif /* M649_SHARED_CANONICAL_H */

#define FIT_MIN(a, b) ((a) < (b) ? (a) : (b))

#define FIT_MAX(a, b) ((a) > (b) ? (a) : (b))

#undef lbl_1_bss_1B8

#undef fn_1_4858

#undef lbl_1_bss_78

#undef lbl_1_bss_1B8

#undef fn_1_48DC

typedef struct M649LightWork_s {
    s16 light; /* Renderer light handle, or -1 before creation. */
    s32 callbackValue; /* Optional object-update callback address, read by fn_1_15A8. */
    Vec position, target, direction; /* World-space light origin, target, and stored light
                                      * direction. */
    GXColor color; /* RGBA color sent to the renderer. */
} M649LightWork;

typedef struct M649CameraFitSphere_s {
    Vec center; /* Sphere center in world coordinates. */
    float radius; /* Sphere radius in world units. */
} M649CameraFitSphere;

typedef struct M649AudioWorkView_s {
    MinigameAudioRange *config; /* Zero-camera-terminated sound settings. */
} M649AudioWorkView;

typedef struct M649FogWork_s {
    MinigameFogParams *parameters; /* Fog settings terminated by a layer value of -1. */
    s32 callbackValue; /* Optional object-update callback address; unused by the layer hook. */
    float factor; /* Scales the configured range from start toward end. */
} M649FogWork;

typedef struct M649AudioWork_s {
    MinigameAudioRange *config; /* Audio settings table selected at object creation. */
    s32 callbackValue; /* Optional callback address invoked after the sound updates. */
    MinigameAudioRecord entries[64]; /* Active sound records; handle -1 marks a free slot. */
} M649AudioWork;

extern OMOBJ *lbl_1_bss_308;

extern OMOBJ *lbl_1_bss_2F8;

extern OMOBJ *lbl_1_bss_2FC;

extern OMOBJ *lbl_1_bss_300;

extern OMOBJ *lbl_1_bss_304;

s32 fn_1_3630(u32 cameras, Vec *position);

s32 fn_1_3850(u32 cameras, Vec *position);

void fn_1_4788(unsigned int cameraMask, int effect, int handle, HuVecF *position, int panEnabled,
               int volumeEnabled);

void fn_1_149C(OMOBJ *object);

void fn_1_4800(void);

void fn_1_15A8(OMOBJ *obj);

void fn_1_2B14(HuVecF *direction);

extern OMOBJ *lbl_1_bss_2FC, *lbl_1_bss_2F8;


void fn_1_19BC(OMOBJ *obj);

void fn_1_1A14(s16 layer);

extern OMOBJ *lbl_1_bss_2FC, *lbl_1_bss_304;

void fn_1_1EF4(OMOBJ *object);

extern GXColor lbl_1_data_C4;

extern Vec lbl_1_data_B8;


void fn_1_211C(OMOBJ *object, u32 camera, s32 index);

void fn_1_29E4(u32 cameras, float x, float y, float width, float height);

void fn_1_2A64(u32 cameras, float x, float y, float width, float height);

void fn_1_32EC(OMOBJ *obj);

void fn_1_3354(OMOBJ *object);

void fn_1_33AC(OMOBJ *object);

s32 fn_1_34E8(u32 cameras, float x);

s32 fn_1_3AE8(u32 cameras, s32 effect, Vec *position);

s32 fn_1_3FBC(u32 cameras, s32 effect, Vec *position);

s32 fn_1_4240(u32 cameras, s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled);

extern s32 lbl_1_bss_1B8[80];

extern s32 lbl_1_bss_78[80];

/* Selects the first set camera bit's index, returning 0 when the mask is empty.
* Positioned pan still passes the original mask to projection, which requires a selected camera. */
static inline s32 m649_local_273C(u32 cameras)
{
    s32 cameraIndex;
    for (cameraIndex = 0; cameraIndex < 16; cameraIndex++, cameras >>= 1) {
        if (cameras & 1) {
            return cameraIndex;
        }
    }
    return 0;
}

/* Convert screen position to sound pan when m649_local_3630 handles a sound. */
static inline s32 m649_local_34E8(u32 cameras, float x)
{
    M649AudioWorkView *work;
    MinigameAudioRange *config;
    s32 pan = MSM_PAN_CENTER;
    if (lbl_1_bss_2F8 != NULL) {
        work = lbl_1_bss_2F8->data;
        for (config = work->config; config->cameraMask; config++) {
            if (config->cameraMask & cameras) {
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

/* Map a world position to sound pan for fn_1_3AE8, fn_1_3FBC, and fn_1_4240. */
static inline s32 m649_local_3630(u32 cameras, Vec *position)
{
    MinigameCameraWork *work = lbl_1_bss_2FC->data;
    MinigameCameraEntry *camera = &work->entries[m649_local_273C(cameras)];
    Vec screenPosition;
    float panCoordinate;
    Hu3D3Dto2D(position, cameras, &screenPosition);
    /* Apply the camera's split-screen rectangle with the audio helper's 0.9 scale. */
    panCoordinate = screenPosition.x / 576.0f;
    panCoordinate = 0.9f * camera->viewport[0] + panCoordinate * (0.9f * camera->viewport[2]);
    return m649_local_34E8(cameras, panCoordinate);
}

/* Scale volume by camera depth when fn_1_3AE8 or fn_1_4240 plays a positioned sound. */
static inline s32 m649_local_3850(u32 cameras, Vec *position)
{
    M649AudioWorkView *audio;
    MinigameAudioRange *config;
    MinigameCameraWork *work;
    MinigameCameraEntry *camera;
    Mtx cameraViewMatrix;
    Vec cameraSpacePosition;
    float cameraDepth, volumeScale;
    s32 volume = MSM_VOL_MAX;
    if (lbl_1_bss_2F8 != NULL) {
        audio = lbl_1_bss_2F8->data;
        work = lbl_1_bss_2FC->data;
        camera = &work->entries[m649_local_273C(cameras)];
        for (config = audio->config; config->cameraMask; config++) {
            if (config->cameraMask & cameras) {
                C_MTXLookAt(cameraViewMatrix, &camera->position, &camera->up, &camera->target);
                PSMTXMultVec(cameraViewMatrix, position, &cameraSpacePosition);
                cameraDepth = -cameraSpacePosition.z;
                if (cameraDepth < config->distanceMiddle) {
                    volumeScale = (config->distanceMiddle - cameraDepth) /
                             (config->distanceMiddle - config->distanceNear);
                    volumeScale *= config->volumeMax - config->volumeMiddle;
                    volume = config->volumeMiddle + volumeScale;
                } else {
                    volumeScale = (cameraDepth - config->distanceMiddle) /
                             (config->distanceFar - config->distanceMiddle);
                    volumeScale *= config->volumeMiddle - config->volumeMin;
                    volume = config->volumeMiddle - volumeScale;
                }
                if (volume < config->volumeMin) {
                    volume = config->volumeMin;
                }
                if (volume > config->volumeMax) {
                    volume = config->volumeMax;
                }
                break;
            }
        }
    }
    return volume;
}

/* Reads the shared light direction while creating the camera object. */
static inline Vec *m649_local_1810_018BC(void)
{
    M649LightWork *work = lbl_1_bss_304->data;
    return &work->direction;
}

/* Returns the tangent using the double-precision math routine. */
extern inline float tanf(float x) { return (float)tan(x); }

/* fn_1_49E4 finds or reserves the resource's cache slot before loading a joint motion. */
static inline s32 m649_local_4858_049E4(s32 handle)
{
    s32 index;
    s32 cachedResource;
    if (handle != -1) {
        for (index = 0; index < 80; index++) {
            cachedResource = lbl_1_bss_1B8[index];
            if (handle == cachedResource) return index;
            if (cachedResource == -1) {
                lbl_1_bss_1B8[index] = handle;
                return index;
            }
        }
    }
    return -1;
}

/* Creates the utility owner at startup, clears shared object pointers, and resets resource
 * caches. */
void fn_1_13D8(OMOBJMAN *manager) {
    OMOBJ *object=fn_8003AB38(manager,22,0,0,-1,fn_1_149C);
    void *state;
    lbl_1_bss_308=object;
    lbl_1_bss_304=lbl_1_bss_300=lbl_1_bss_2FC=lbl_1_bss_2F8=NULL;
    state=HuMemDirectMallocNum(HEAP_HEAP,4,0x10000000);
    object->data=state;
    memset(state,0,4);
    fn_1_4800();
}

/* Empty utility cleanup hook; it changes no minigame state. */
void fn_1_1498(void)
{
}

/* The utility owner has no work to perform during object-manager updates. */
void fn_1_149C(OMOBJ *object)
{
}

/* Creates the light owner and derives its initial direction; renderer light setup is separate. */
void fn_1_14A0(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color)
{
    M649LightWork *work;
    OMOBJ *object;
    object = fn_8003AB38(manager, 24, 0, 0, -1, fn_1_15A8);
    lbl_1_bss_304 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M649LightWork), 0x10000000);
    object->data = work;
    memset(work, 0, sizeof(M649LightWork));
    work->position = *position;
    work->target = *target;
    work->color = *color;
    work->light = -1;
    PSVECSubtract(&work->target, &work->position, &work->direction);
    PSVECNormalize(&work->direction, &work->direction);
}

/* Runs from the light object's update callback and forwards the frame to its client callback. */
void fn_1_15A8(OMOBJ *obj)
{
    MinigameLightWork *light = lbl_1_bss_304->data;
    if (light->callback != NULL) {
        light->callback(obj);
    }
}

/* Creates a directional renderer light and shares its saved direction.
* Successful creation replaces a zero saved direction with +Z. */
void fn_1_1600(void) {
    MinigameLightWork *light = lbl_1_bss_304->data;

    light->lightId = fn_800286D0(&light->position, &light->direction, &light->color);
    Hu3DGLightInfinitytSet(light->lightId);
    Hu3DGLightStaticSet(light->lightId, 1);
    fn_1_2B14(&light->direction);
}

/* During prop setup, imports the model's lights, selects the named light, and shares its
 * direction. */
void fn_1_1664(s32 modelId, char *lightName) {
    HU3D_LIGHT *globalLightTable;
    M649LightWork *light;

    light = lbl_1_bss_304->data;
    globalLightTable = Hu3DGlobalLight;
    fn_80029850((s16) modelId, 1);
    if (lightName != NULL) {
        light->light = fn_80029D24((s16) modelId, lightName);
    } else {
        /* With no name, use global light slot 0 rather than a model-local light index. */
        light->light = 0;
    }
    if (light->light >= 0) {
        fn_800297D4(light->light, &light->position,
                           &light->direction,
                           &light->color);
        fn_1_2B14(&light->direction);
    }
}

/* Stores the optional light-object callback invoked by fn_1_15A8 on each update. */
void fn_1_1718(OMOBJ_FUNC callback) {
    MinigameLightWork *light=lbl_1_bss_304->data;
    light->callback=callback;
}

/* Moves the active directional light when its owner changes the world position. */
void fn_1_1740(HuVecF *position) {
    MinigameLightWork *light=lbl_1_bss_304->data;
    light->position=*position;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}

/* Changes the active directional light direction when its owner updates the lighting. */
void fn_1_17A8(HuVecF *direction) {
    MinigameLightWork *light=lbl_1_bss_304->data;
    light->direction=*direction;
    Hu3DGLightPosSetV(light->lightId,&light->position,&light->direction);
}

/* Returns a pointer to the stored light direction. */
HuVecF *fn_1_1810() {
    MinigameLightWork *light=lbl_1_bss_304->data;
    return &light->direction;
}

/* Updates the active minigame light color when a caller changes its lighting. */
void fn_1_1838(int red,int green,int blue,int alpha) {
    MinigameLightWork *light=lbl_1_bss_304->data;
    light->color.r=red;
    light->color.g=green;
    light->color.b=blue;
    light->color.a=alpha;
    Hu3DGLightColorSet(light->lightId,red,green,blue,alpha);
}

/* Sets global exponential fog from the first entry's distances and RGB, with alpha 255;
 * layer hooks apply each entry's own type and color. */
void fn_1_18BC(OMOBJMAN *manager, MinigameFogParams *parameters)
{
    M649FogWork *work;
    s32 layerIndex;
    OMOBJ *object;
    object = fn_8003AB38(manager, 26, 0, 0, -1, fn_1_19BC);
    lbl_1_bss_300 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M649FogWork), 0x10000000);
    object->data = work;
    memset(work, 0, sizeof(M649FogWork));
    work->parameters = parameters;
    work->factor = 1.0f;
    for (layerIndex = 0; parameters->layer != -1;
         layerIndex++, parameters++) {
        if (layerIndex == 0)
            Hu3DFogSet(parameters->start, parameters->end,
                       parameters->color.r,
                       parameters->color.g,
                       parameters->color.b);
        fn_80025918(parameters->layer, fn_1_1A14);
    }
}

/* Runs from the fog object's update callback and forwards the frame to its client callback. */
void fn_1_19BC(OMOBJ *obj)
{
    MinigameLightWork *fogState = lbl_1_bss_300->data;
    if (fogState->callback != NULL) {
        fogState->callback(obj);
    }
}

/* Applies the configured fog when its layer hook runs.
* Slots 0 through 7 run before their layer; slots 8 through 15 run after it. */
void fn_1_1A14(s16 layer)
{
    M649FogWork *work;
    MinigameFogParams *parameters;
    float fov, nearPlane, farPlane;
    float end;
    work = lbl_1_bss_300->data;
    for (parameters = work->parameters; parameters->layer != -1; parameters++) {
        if (parameters->layer == layer) {
            fn_80027D8C(parameters->cameras, &fov, &nearPlane, &farPlane);
            end = parameters->start + work->factor * (parameters->end - parameters->start);
            GXSetFog(parameters->type, parameters->start, end, nearPlane, farPlane,
                     parameters->color);
            break;
        }
    }
}

/* Stores the optional fog-object callback address used by fn_1_19BC. */
void fn_1_1B08(s32 callbackValue) {
    s32 *fogState;

    if (lbl_1_bss_300 != NULL) {
        fogState = lbl_1_bss_300->data;
        fogState[1] = callbackValue;
    }
}

/* Sets the fog-range factor: layer hooks use start + factor * (end - start). */
void fn_1_1B48(f32 factor) {
    M649FogWork *work;

    if (lbl_1_bss_300 != NULL) {
        work = lbl_1_bss_300->data;
        work->factor = factor;
    }
}

/* Creates the selected cameras during startup and initializes each camera's view and viewport. */
void fn_1_1B88(OMOBJMAN *manager, u32 cameras, float fov, float nearPlane, float farPlane,
    float shadowRadius, OM_CAMERA_VIEW *view)
{
    MinigameCameraEntry *entry;
    MinigameCameraWork *work;
    OMOBJ *object;
    s32 i;
    u32 bit;
    s32 count = 0;
    object = fn_8003AB38(manager, 28, 0, 0, -1, fn_1_1EF4);
    lbl_1_bss_2FC = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MinigameCameraWork), 0x10000000);
    object->data = work;
    memset(work, 0, sizeof(MinigameCameraWork));
    work->cameras = cameras;
    work->shadowRadius = shadowRadius;
    work->shadowFov = 20.0f;
    work->shadowColor = lbl_1_data_C4;
    if (lbl_1_bss_304 == NULL) {
        work->shadowDirection = lbl_1_data_B8;
    } else {
        work->shadowDirection = *m649_local_1810_018BC();
    }
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            count = i + 1;
            entry = &work->entries[i];
            entry->view = *view;
            entry->previousView = *view;
            entry->position = view->center;
            /* Seed the position 1000 units behind the center regardless of the saved view's
             * zoom. */
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
    fn_8003E488(cameras, view);
    object = fn_8003AB38(manager, 29, 0, 0, -1, fn_8003DAA4);
    object->work[0] = count;
    fn_8003DAA4(object);
}

/* Updates active cameras once per object-manager frame and refreshes their render viewports. */
void fn_1_1EF4(OMOBJ *object)
{
    MinigameCameraWork *work = object->data;
    MinigameCameraEntry *entry;
    s32 i, shadowDone = 0;
    u32 bit;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (work->cameras & bit) {
            entry = &work->entries[i];
            fn_80027CD0(bit, &entry->position, &entry->up, &entry->target);
            fn_80027D8C(bit, &entry->fov, &entry->nearPlane, &entry->farPlane);
            if (shadowDone == 0) {
                if (work->shadowRadius > 0.0) {
                    fn_1_2590(work->cameras, &work->shadowDirection, &entry->target,
                              work->shadowFov, work->shadowRadius);
                    /* This color setter changes only camera 0, regardless of the selected camera
                     * mask. */
                    fn_8002AAB4(work->shadowColor.r, work->shadowColor.g, work->shadowColor.b);
                }
                shadowDone = 1;
            }
            if (work->entries[i].active) {
                fn_1_211C(object, bit, i);
            } else {
                if (work->callback != NULL) {
                    work->callback(object, bit, i);
                }
                lbl_80266B18[i] = entry->view.center;
                lbl_80266A58[i] = entry->view.rot;
                lbl_80266BD8[i] = entry->view.zoom;
            }
            Hu3DCameraPerspectiveSet(bit, entry->fov, entry->nearPlane, entry->farPlane,
                                     entry->aspect);
            Hu3DCameraViewportSet(bit, entry->viewport[0], entry->viewport[1], entry->viewport[2],
                                  entry->viewport[3], 0.0f, 1.0f);
            Hu3DCameraScissorSet(bit, (u32) entry->scissor[0], (u32) entry->scissor[1],
                                 (u32) entry->scissor[2], (u32) entry->scissor[3]);
        }
    }
}

/* Copies the animated camera pose into its view during fn_1_1EF4 and saves it when motion ends. */
void fn_1_211C(OMOBJ *object, u32 camera, s32 index)
{
    MinigameCameraEntry *entry;
    MinigameCameraWork *work = object->data;
    Vec cameraOffset;
    entry = &work->entries[index];
    lbl_80266B18[index] = entry->target;
    cameraOffset.x = entry->position.x - entry->target.x;
    cameraOffset.y = entry->position.y - entry->target.y;
    cameraOffset.z = entry->position.z - entry->target.z;
    lbl_80266A58[index].x = 180.0 * (atan2(-cameraOffset.y,
        sqrtf(cameraOffset.x * cameraOffset.x
            + cameraOffset.z * cameraOffset.z)) / 3.141592653589793);
    lbl_80266A58[index].y = 180.0 * (atan2(cameraOffset.x, cameraOffset.z) / 3.141592653589793);
    lbl_80266A58[index].z = 0.0f;
    lbl_80266BD8[index] = sqrtf(cameraOffset.z * cameraOffset.z +
                          (cameraOffset.x * cameraOffset.x + cameraOffset.y * cameraOffset.y));
    entry->view.center = lbl_80266B18[index];
    entry->view.rot = lbl_80266A58[index];
    entry->view.zoom = lbl_80266BD8[index];
    if (fn_8002D75C(entry->model)) {
        fn_800262D0(entry->model);
        entry->previousView.center = lbl_80266B18[index];
        entry->previousView.rot = lbl_80266A58[index];
        entry->previousView.zoom = lbl_80266BD8[index];
        entry->active = 0;
    }
}

static const Vec lbl_1_rodata_E0[2] = {{0.0f,1.0f,0.0f},{0.0f,0.0f,1.0f}};

/* fn_1_1EF4 calls this during shadow updates to cover the requested sphere from the light. */
void fn_1_2590(s32 camera, Vec *direction, Vec *target, float fov, float radius)
{
    Vec shadowCameraPosition, normalizedDirection;
    const Vec *up = lbl_1_rodata_E0;
    float distance;
    float radians;
    radians = 3.141592653589793 * (fov / 2.0f) / 180.0;
    distance = radius / tanf(radians);
    PSVECNormalize(direction, &normalizedDirection);
    PSVECScale(&normalizedDirection, &shadowCameraPosition, distance);
    PSVECSubtract(target, &shadowCameraPosition, &shadowCameraPosition);
    if (fabs(normalizedDirection.y) > 0.5) {
        up++;
    }
    fn_8002A3B0(fov, distance - radius, distance + radius, camera);
    fn_8002A674(&shadowCameraPosition, (Vec *)up, target, camera);
    fn_8002A8CC(480, camera);
    fn_8002A7D8(0.4f, camera);
}

/* Returns the camera index represented by the first set bit, or zero for an empty mask. */
int fn_1_273C(unsigned int mask)
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

/* Stores the callback that updates camera view data during the camera object's frame update. */
extern void fn_1_2780(M649CameraCallback callback) {
    MinigameCameraWork *state=lbl_1_bss_2FC->data;
    state->callback=callback;
}

/* Stores the aspect ratio for each selected camera. */
void fn_1_27A8(u32 cameras, float aspectRatio)
{
    OMOBJ *object = lbl_1_bss_2FC;
    MinigameCameraEntry *entry;
    s32 i;
    u32 bit;
    MinigameCameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            entry = &work->entries[i];
            entry->aspect = aspectRatio;
        }
    }
}

/* Configures split-screen viewports and scissors, extending the viewport to avoid edge gaps. */
void fn_1_281C(u32 cameras, float x, float y, float width, float height)
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
    fn_1_29E4(cameras, x + offsetX, y + offsetY, width + extraWidth, height + extraHeight);
    fn_1_2A64(cameras, x, y, width, height);
}

/* Stores the viewport rectangle selected by fn_1_281C during camera frame updates. */
void fn_1_29E4(u32 cameras, float x, float y, float width, float height)
{
    MinigameCameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2FC;
    MinigameCameraWork *work = object->data;
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

/* Stores the scissor rectangle selected by fn_1_281C during camera frame updates. */
void fn_1_2A64(u32 cameras, float x, float y, float width, float height)
{
    MinigameCameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2FC;
    MinigameCameraWork *work = object->data;
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

/* Sets the camera shadow field of view in degrees. */
extern void fn_1_2AE4(f32 shadowFov) {
    OMOBJ *object = lbl_1_bss_2FC;
    MinigameCameraWork *work = object->data;
    work->shadowFov = shadowFov;
}

/* Saves the light direction in the shared camera state when light setup changes it. */
void fn_1_2B14(HuVecF *direction) {
    OMOBJ *object=lbl_1_bss_2FC;
    if(object != NULL) {
        MinigameCameraWork *state=object->data;
        state->shadowDirection=*direction;
    }
}

/* Returns the camera shadow direction's X component for in-place updates. */
extern f32 *fn_1_2B64() {
    OMOBJ *object = lbl_1_bss_2FC;
    MinigameCameraWork *work = object->data;
    return &work->shadowDirection.x;
}

/* Stores the RGB color used by camera shadow rendering. */
void fn_1_2B94(u8 red, u8 green, u8 blue)
{
    OMOBJ *object = lbl_1_bss_2FC;
    MinigameCameraWork *work = object->data;
    work->shadowColor.r = red;
    work->shadowColor.g = green;
    work->shadowColor.b = blue;
}

/* Starts the supplied camera motion on every selected camera during scene setup. */
void fn_1_2BCC(u32 cameras, void *data)
{
    MinigameCameraEntry *entry;
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2FC;
    MinigameCameraWork *work = object->data;
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

/* Called while waiting for camera transitions; reports whether a selected camera is moving. */
BOOL fn_1_2C84(u32 cameras)
{
    s32 i;
    u32 bit;
    OMOBJ *object = lbl_1_bss_2FC;
    MinigameCameraWork *work = object->data;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if ((cameras & bit) && work->entries[i].active) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Fits a camera around the supplied spheres and returns the required viewing distance. */
double fn_1_2D08(CameraFitParams *fitParams, M649CameraFitSphere *fitSpheres, s32 sphereCount, Vec *cameraPosition)
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
    M649CameraFitSphere *sphere;
    s32 i;

    MTXLookAt(view, &origin, &fitParams->up,
              &fitParams->direction);
    MTXInverse(view, inverse);
    {
        double aspect = (double) fitParams->width /
                        fitParams->height;
        tangent = tan(fitParams->fov / 2.0f);
        slopeX = aspect * (tangent * ((double) (fitParams->width -
                                                2 * fitParams->marginX) /
                                      fitParams->width));
        slopeY = tangent * ((double) (fitParams->height -
                                      2 * fitParams->marginY) /
                            fitParams->height);
    }
    sphere = fitSpheres;
    MTXMultVecSR(view, &sphere->center, &lower);
    upper = lower;
    nearZ = lower.z - sphere->radius;
    sphere++;
    for (i = 1; i < sphereCount; i++, sphere++) {
        Vec position;
        MTXMultVecSR(view, &sphere->center, &position);
        nearZ = FIT_MIN(nearZ, position.z - sphere->radius);
    }
    for (sphere = fitSpheres, i = 0; i < sphereCount; i++, sphere++) {
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
    /* When horizontal bounds set the distance, extend the vertical box upward from lower.y.
    * This also shifts the fitted center upward. */
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
        VECNormalize(&fitParams->direction, &offset);
        VECScale(&offset, &offset, distance);
        VECSubtract(&center, &offset, cameraPosition);
    }
    return distance;
}

/* ObjectSetup calls this at startup to create the audio object and its sound-tracking records. */
void fn_1_3250(HUPROCESS *objectManager, MinigameAudioRange *audioConfig) {
    OMOBJ *object;
    M649AudioWork *objectState;

    object = fn_8003AB38(objectManager, 30, 0, 0, -1, fn_1_32EC);
    lbl_1_bss_2F8 = object;
    objectState = HuMemDirectMallocNum(HEAP_HEAP, 1544, 0x10000000);
    object->data = objectState;
    memset(objectState, 0, 1544);
    objectState->config = audioConfig;
    fn_1_3354(object);
}

/* Runs each frame for the shared audio object, updating sounds before its optional callback. */
void fn_1_32EC(OMOBJ *obj) {
    M620ObjectState *state = lbl_1_bss_2F8->data;

    fn_1_33AC(obj);
    if (state->callback != NULL) {
        state->callback(obj);
    }
}

/* Initializes the sound tracking table when the audio object is created. */
void fn_1_3354(OMOBJ *object)
{
    M649AudioWork *state = lbl_1_bss_2F8->data;
    MinigameAudioRecord *record = state->entries;
    int index;

    for (index = 0; index < 64; index++, record++) {
        record->handle = -1;
    }
}

/* Each audio-object update refreshes PAUSEIN sounds and drops tracking for all other statuses. */
void fn_1_33AC(OMOBJ *object)
{
    M649AudioWork *work;
    MinigameAudioRecord *entry;
    s32 soundIndex;
    work = lbl_1_bss_2F8->data;
    entry = work->entries;
    for (soundIndex = 0; soundIndex < 64; soundIndex++, entry++) {
        if (entry->handle != -1) {
            /* Only PAUSEIN handles remain tracked; this branch does not stop the sound. */
            if (HuAudFXStatusGet(entry->handle) != MSM_SE_PAUSEIN) {
                entry->handle = -1;
                continue;
            }
            if (entry->panEnabled)
                HuAudFXPanning(entry->handle, fn_1_3630(entry->cameraMask, entry->position));
            if (entry->volumeEnabled)
                HuAudFXVolSet(entry->handle, fn_1_3850(entry->cameraMask, entry->position));
        }
    }
}

/* Stores the optional audio-object callback address invoked after sound updates. */
void fn_1_347C(s32 callbackValue) {
    s32 *audioState;

    if (lbl_1_bss_2F8 != NULL) {
        audioState = lbl_1_bss_2F8->data;
        audioState[1] = callbackValue;
    }
}

/* Maps a single-camera screen X coordinate to sound pan for legacy callers. */
extern int fn_1_34BC(float screenX) {
    return fn_1_34E8(1, screenX);
}

/* Maps screen X in the 0-to-576 range to the camera's configured pan range, then clamps it. */
s32 fn_1_34E8(u32 cameras, float x)
{
    M649AudioWorkView *work;
    MinigameAudioRange *config;
    s32 pan = MSM_PAN_CENTER;
    if (lbl_1_bss_2F8 != NULL) {
        work = lbl_1_bss_2F8->data;
        for (config = work->config; config->cameraMask; config++) {
            if (config->cameraMask & cameras) {
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

/* Calculates pan through camera 0, then discards the result. */
void fn_1_3604(void *position)
{
    fn_1_3630(1, position);
}

/* Player voice updates and tracked sounds compute pan through the first selected camera. */
s32 fn_1_3630(u32 cameras, Vec *position)
{
    return m649_local_3630(cameras, position);
}

/* Calculates volume through camera 0, then discards the result. */
void fn_1_3824(void *position)
{
    fn_1_3850(1, position);
}

/* Player voice updates and tracked sounds scale volume by depth in the first selected camera. */
s32 fn_1_3850(u32 cameras, Vec *position)
{
    return m649_local_3850(cameras, position);
}

/* Plays a positioned sound with volume and pan for camera 0, then discards its handle. */
void fn_1_3AB4(s32 effect, Vec *position) {
    fn_1_3AE8(1, effect, position);
}

/* Plays one effect per selected camera with position-based volume and pan; returns only the
 * last handle, or -1 if no camera is selected. */
s32 fn_1_3AE8(u32 cameras, s32 effect, Vec *position)
{
    s32 i;
    u32 bit;
    s32 handle = -1;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayVolPan(effect, m649_local_3850(bit, position),
                                       m649_local_3630(bit, position));
        }
    }
    return handle;
}

/* Plays a positioned sound with pan for camera 0, then discards its handle. */
void fn_1_3F88(s32 effect, Vec *position) {
    fn_1_3FBC(1, effect, position);
}

/* Plays one effect per selected camera with position-based pan; returns only the last handle,
 * or -1 if no camera is selected. */
s32 fn_1_3FBC(u32 cameras, s32 effect, Vec *position)
{
    s32 i;
    u32 bit;
    s32 handle = -1;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            handle = HuAudFXPlayPan(effect, m649_local_3630(bit, position));
        }
    }
    return handle;
}

/* Plays a positioned sound for the first camera with optional pan and volume updates. */
s32 fn_1_41FC(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled)
{
    return fn_1_4240(1, effect, position, panEnabled, volumeEnabled);
}

/* Plays one effect per selected camera and tracks it when a sound-record slot is free.
* Optional pan/volume follow the live position; returns the last handle, or -1 for no camera. */
s32 fn_1_4240(u32 cameras, s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled)
{
    s32 i;
    u32 bit;
    s32 pan, volume;
    s32 handle = -1;
    for (i = 0, bit = 1; i < 16; i++, bit <<= 1) {
        if (cameras & bit) {
            if (panEnabled) {
                pan = m649_local_3630(bit, position);
            }
            else { pan = MSM_PAN_CENTER; }
            if (volumeEnabled) {
                volume = m649_local_3850(bit, position);
            }
            else { volume = MSM_VOL_MAX; }
            handle = HuAudFXPlayVolPan(effect, volume, pan);
            fn_1_4788(bit, effect, handle, position, panEnabled, volumeEnabled);
        }
    }
    return handle;
}

/* Registers a sound for ongoing pan and volume updates on the first camera. */
extern void fn_1_473C(int effect, int handle, Vec *position, int panEnabled, int volumeEnabled) {
    fn_1_4788(1, effect, handle, position, panEnabled, volumeEnabled);
}

/* Registers the sound in the first free slot for ongoing position updates; if all 64 slots
* are occupied, it returns without tracking the sound. */
void fn_1_4788(unsigned int cameraMask, int effect, int handle, HuVecF *position, int panEnabled,
               int volumeEnabled)
{
    M649AudioWork *state=lbl_1_bss_2F8->data;
    MinigameAudioRecord *record=state->entries;
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

/* fn_1_13D8 calls this at startup to clear the 80 model/motion resource-cache slots. */
void fn_1_4800(void) {
    s32 index;

    index = 0;
    while (index < 80) {
        lbl_1_bss_1B8[index] = -1;
        lbl_1_bss_78[index] = -1;
        index += 1;
    }
}

/* Finds or reserves a cache slot for a model or motion resource during asset setup. */
int fn_1_4858(int handle) {
    int index;
    int existing;
    if(handle!=-1) {
        for(index=0;index<80;index++) {
            existing=lbl_1_bss_1B8[index];
            if(handle==existing) return index;
            if(existing==-1) {
                lbl_1_bss_1B8[index]=handle;
                return index;
            }
        }
    }
    return -1;
}

/* Loads a model resource once and links another model instance on later requests. */
s32 fn_1_48DC(s32 dataNum) {
    int slot=fn_1_4858(dataNum);
    int model;
    if(slot>=0) {
        model=lbl_1_bss_78[slot];
        if(model==-1) {
            model=fn_80025A04(fn_80008DD0(dataNum, 0x10000000, 2));
            lbl_1_bss_78[slot]=model;
            return model;
        }
        return fn_80025D4C(model);
    }
    return -1;
}

/* Caches a joint motion by data resource; later requests reuse it without binding it to the
 * supplied model. */
s32 fn_1_49E4(s32 model, s32 dataNum)
{
    s32 slot = m649_local_4858_049E4(dataNum);
    s32 motion;
    if (slot >= 0) {
        motion = lbl_1_bss_78[slot];
        if (motion == -1) {
            motion = fn_80030458(model, fn_80008DD0(dataNum, 0x10000000, 2));
            lbl_1_bss_78[slot] = motion;
            return motion;
        }
        return motion;
    }
    return -1;
}

/* Interpolates between angles along the shorter turn; player and prop movement call this per
 * update. */
float fn_1_4AF4(float start, float end, float factor)
{
    float angle;
    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    angle = fmodf(start + factor * angle, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    return angle;
}

/* fn_1_5B04 uses this signed shortest angle difference to check the player's facing after play. */
float fn_1_4C10(float start, float end)
{
    float angle;
    angle = fmodf(end - start, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;
    return angle;
}

/* fn_1_83BC uses this during route movement to face the next Stamp By Me point. */
float fn_1_4CC4(HuVecF *currentPosition, HuVecF *nextPosition) {
    float deltaX = nextPosition->x - currentPosition->x;
    float deltaZ = nextPosition->z - currentPosition->z;
    return atan2(deltaX, deltaZ) / 3.141592653589793 * 180.0;
}

/* Returns the horizontal XZ distance between route points during prop movement. */
float fn_1_4D60(Vec *first, Vec *second)
{
    float x, z;
    x = second->x - first->x;
    z = second->z - first->z;
    return sqrtf(x * x + z * z);
}
