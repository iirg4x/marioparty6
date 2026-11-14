#include "math.h"
#include "REL/m603DLL/camera_owner.h"
#include "REL/framework/list_object.h"
#include "REL/m603DLL/player_position.h"
extern "C" {
#include "game/hu3d.h"
#include "game/hsfex.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/audio.h"
#include <stdio.h>
f32 lbl_1_bss_40[4];
s32 lbl_1_bss_50;
}
#include "REL/framework/debug.h"
#include "game/pad.h"
#include "REL/m603DLL/camera_group.h"
extern "C" {
const s32 viewportCameraBits[4]={1,2,4,8};
}
/* Initialize four player camera frames and their volume factors. */
CameraOwner::CameraOwner()
    : transitionRequested(0), callback(&CameraOwner::followActiveCamerasAtStart), selectedCamera(-1)
{
    s32 index;
    s32 resetIndex;
    Vec setupValues[4] = {
        {0, 0, 0}, {361.0f, 0, 0},
        {0, 240.0f, 0}, {361.0f, 240.0f, 0}
    };
    static const Vec initialPosition={0,2800.0f,3000.0f};
    static const Vec initialUp={0,1.0f,0};
    static const Vec initialTarget={0,0,200.0f};
    for (index = 0; index < 4; index++) {
        CameraViewport *group = &this->cameras[index];

        group->position.x = initialPosition.x;
        group->position.y = initialPosition.y;
        group->position.z = initialPosition.z;
        group->up.x = initialUp.x;
        group->up.y = initialUp.y;
        group->up.z = initialUp.z;
        group->target.x = initialTarget.x;
        group->target.y = initialTarget.y;
        group->target.z = initialTarget.z;
        group->active = 0;
        group->marked = 0;
        /* The frame counter is reset on each of the four passes. */
        this->elapsedFrames = 0;
    }
    for (resetIndex = 0; resetIndex < 4; resetIndex++) {
        lbl_1_bss_40[resetIndex] = 1.0f;
    }
}

/* Mark a player camera for the additional target offset. */
void CameraOwner::fn_1_F30(int playerIndex)
{
    CameraViewport *player = &cameras[playerIndex];
    player->marked = 1;
}

/* Clamp a viewport and create, update or destroy its camera. */
void CameraOwner::fn_1_F58(s32 cameraIndex, f32 left, f32 top,
               f32 width, f32 height)
{
    CameraViewport *viewport =
        &cameras[cameraIndex];

    if (left < 0.0f) left = 0.0f;
    if (top < 0.0f) top = 0.0f;
    if (left > 722.0f) left = 722.0f;
    if (top > 480.0f) top = 480.0f;
    if (width < 0.0f) width = 0.0f;
    if (height < 0.0f) height = 0.0f;
    if (width > 722.0f) width = 722.0f;
    if (height > 480.0f) height = 480.0f;
    viewport->left = left;
    viewport->top = top;
    viewport->width = width;
    viewport->height = height;
    if (0.0f != width && 0.0f != height) {
        if (viewport->active == 0) {
            Hu3DCameraCreate(viewportCameraBits[cameraIndex]);
            viewport->active = 1;
            printf("Create %d\n", cameraIndex);
        }
    } else if (viewport->active == 1) {
        Hu3DCameraKill(viewportCameraBits[cameraIndex]);
        viewport->active = 0;
    }
    if (viewport->active != 0) {
        Hu3DCameraPerspectiveSet(viewportCameraBits[cameraIndex],
                                45.0f, 100.0f, 5000.0f, 1.35375f);
        left = 17.0 + 0.84 * left;
        top = 39.0 + 0.84 * top;
        width *= 0.84;
        height *= 0.84;
        Hu3DCameraViewportSet(viewportCameraBits[cameraIndex],
                             left, top, width, height, 0.0f, 1.0f);
        Hu3DCameraScissorSet(viewportCameraBits[cameraIndex],
                            (u32)left, (u32)top,
                            (u32)width, (u32)height);
    }
}

/* Place four camera viewports around the supplied screen split. */
void CameraOwner::fn_1_12EC(f32 insetX, f32 insetY)
{
    f32 left;
    f32 top;
    f32 right;
    f32 bottom;

    this->insetX = insetX;
    this->insetY = insetY;

    left = top = 0.0f;
    right = insetX - 1.0f;
    bottom = insetY - 1.0f;
    fn_1_F58(0, left, top, right, bottom);

    left = 1.0f + insetX;
    top = 0.0f;
    right = 721.0f - insetX;
    bottom = insetY - 1.0f;
    fn_1_F58(1, left, top, right, bottom);

    left = 0.0f;
    top = 1.0f + insetY;
    right = insetX - 1.0f;
    bottom = 479.0f - insetY;
    fn_1_F58(2, left, top, right, bottom);

    left = 1.0f + insetX;
    top = 1.0f + insetY;
    right = 721.0f - insetX;
    bottom = 479.0f - insetY;
    fn_1_F58(3, left, top, right, bottom);
}

/* Ease the supplied target and position toward a player camera frame. */
void CameraOwner::fn_1_14DC(s32 playerIndex, Vec *position, Vec *direction)
{
    CameraViewport *player = &this->cameras[playerIndex];

    PSVECSubtract(position, &player->target, position);
    PSVECScale(position, position, 0.1f);
    PSVECAdd(position, &player->target, position);
    PSVECSubtract(direction, &player->position, direction);
    PSVECScale(direction, direction, 0.05f);
    PSVECAdd(direction, &player->position, direction);
}

/* Copy the queried player position and direction to the outputs. */
void CameraOwner::fn_1_15A4(int playerIndex, Vec *position, Vec *direction)
{
    Vec sourcePosition;
    Vec sourceDirection;

    fn_1_6918(playerIndex, &sourcePosition, &sourceDirection);
    position->x = sourcePosition.x;
    position->y = sourcePosition.y;
    position->z = sourcePosition.z;
    direction->x = sourceDirection.x;
    direction->y = sourceDirection.y;
    direction->z = sourceDirection.z;
}

/* Query two player vectors and discard both results. */
void CameraOwner::fn_1_161C(int playerIndex)
{
    Vec position;
    Vec direction;

    /* Both queried vectors are discarded. */
    fn_1_6A14(playerIndex, &position, &direction);
}

/* Follow players with each active camera and apply marked-camera offsets. */
void CameraOwner::followActiveCameras()
{
    s32 playerIndex;
    CameraViewport *player;
    Vec difference;
    Vec direction;
    Vec position;
    f32 distanceSquared;
    f32 distance;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        player = &this->cameras[playerIndex];
        if (player->active == 1) {
            fn_1_6918(playerIndex, &position, &direction);
            player->target.x = position.x;
            player->target.y = position.y;
            player->target.z = position.z;
            player->position.x = direction.x;
            player->position.y = direction.y;
            player->position.z = direction.z;
            if (player->marked != 0) {
                difference.x = player->position.x - player->target.x;
                difference.y = player->position.y - player->target.y;
                difference.z = player->position.z - player->target.z;
                distanceSquared = difference.x * difference.x +
                                  difference.y * difference.y +
                                  difference.z * difference.z;
                distance = (f32)sqrt(distanceSquared);
                player->position.x += 50.0f * difference.x / distance;
                player->position.y += 50.0f * difference.y / distance;
                player->position.z += 50.0f * difference.z / distance;
            }
            cameraApplyToHud(player, viewportCameraBits[playerIndex]);
        }
    }
}

/* Follow players with the initially selected camera callback. */
void CameraOwner::followActiveCamerasAtStart()
{
    s32 playerIndex;
    s32 queryPlayer;
    CameraViewport *player;
    Vec *positionVector;
    Vec *directionVector;
    Vec difference;
    Vec direction;
    Vec position;
    f32 distanceSquared;
    f32 distance;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        player = &this->cameras[playerIndex];
        if (player->active == 1) {
            directionVector = &player->position;
            positionVector = &player->target;
            queryPlayer = playerIndex;
            fn_1_6918(queryPlayer, &position, &direction);
            positionVector->x = position.x;
            positionVector->y = position.y;
            positionVector->z = position.z;
            directionVector->x = direction.x;
            directionVector->y = direction.y;
            directionVector->z = direction.z;
            if (player->marked != 0) {
                difference.x = player->position.x - player->target.x;
                difference.y = player->position.y - player->target.y;
                difference.z = player->position.z - player->target.z;
                distanceSquared = difference.x * difference.x +
                                  difference.y * difference.y +
                                  difference.z * difference.z;
                distance = (f32)sqrt(distanceSquared);
                player->position.x += 50.0f * difference.x / distance;
                player->position.y += 50.0f * difference.y / distance;
                player->position.z += 50.0f * difference.z / distance;
            }
            cameraApplyToHud(player, viewportCameraBits[playerIndex]);
        }
    }
}

/* Follow players after the camera callback request. */
void CameraOwner::followActiveCamerasOnRequest()
{
    s32 playerIndex;
    s32 queryPlayer;
    CameraViewport *player;
    Vec *positionVector;
    Vec *directionVector;
    Vec difference;
    Vec direction;
    Vec position;
    f32 distanceSquared;
    f32 distance;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        player = &this->cameras[playerIndex];
        if (player->active == 1) {
            directionVector = &player->position;
            positionVector = &player->target;
            queryPlayer = playerIndex;
            fn_1_6918(queryPlayer, &position, &direction);
            positionVector->x = position.x;
            positionVector->y = position.y;
            positionVector->z = position.z;
            directionVector->x = direction.x;
            directionVector->y = direction.y;
            directionVector->z = direction.z;
            if (player->marked != 0) {
                difference.x = player->position.x - player->target.x;
                difference.y = player->position.y - player->target.y;
                difference.z = player->position.z - player->target.z;
                distanceSquared = difference.x * difference.x +
                                  difference.y * difference.y +
                                  difference.z * difference.z;
                distance = (f32)sqrt(distanceSquared);
                player->position.x += 50.0f * difference.x / distance;
                player->position.y += 50.0f * difference.y / distance;
                player->position.z += 50.0f * difference.z / distance;
            }
            cameraApplyToHud(player, viewportCameraBits[playerIndex]);
        }
    }
}
/* Apply a fixed position and target to camera zero. */
void CameraOwner::fn_1_1F4C()
{
 CameraViewport *frame=&cameras[0];
 frame->position.x=0.0f;frame->position.y=550.0f;frame->position.z=250.0f;
 frame->target.x=0.0f;frame->target.y=120.0f;frame->target.z=-350.0f;
 cameraApplyToHud(frame,viewportCameraBits[0]);
}

/* Update the menu camera and select player following on button 0x800. */
void CameraOwner::fn_1_1FF0()
{
    debugEventHook(&this->menuState, 0);
    if ((lbl_802C0008[0] & 0x800) != 0) {
        CameraOwner::Callback values = &CameraOwner::followActiveCamerasOnRequest;
        this->callback = values;
    }
    cameraApplyToHud(&this->menuState, 1);
}

/* Release the transition model and follow players when the transition is requested. */
void CameraOwner::fn_1_2080()
{
    s32 index;
    CameraViewport *camera;

    if (transitionRequested != 0) {
        fn_800262D0(modelId);
        fn_1_12EC(
                   361.0f, 240.0f);

        index = 0;
    while (index < 4) {
        camera = &cameras[index];
        if (camera->active == 1) {
            Vec *targetSlot;
            CameraViewport *activeCamera;
            s32 playerIndex;
            Vec difference;
            Vec position;
            Vec target;
            f32 length;

            activeCamera = camera;
            targetSlot = &camera->target;
            playerIndex = index;
            fn_1_6918(playerIndex, &target, &position);
            *targetSlot = target;
            activeCamera->position = position;
            if (camera->marked != 0) {
                difference.x = camera->position.x - camera->target.x;
                difference.y = camera->position.y - camera->target.y;
                difference.z = camera->position.z - camera->target.z;
                length = sqrt((f64)(difference.x * difference.x +
                                    difference.y * difference.y +
                                    difference.z * difference.z));
                camera->position.x += 50.0f * difference.x / length;
                camera->position.y += 50.0f * difference.y / length;
                camera->position.z += 50.0f * difference.z / length;
            }
    cameraApplyToHud(camera, viewportCameraBits[index]);
        }
        index++;
        }
    }
}

/* Wait for the transition motion, then copy camera zero into all player frames. */
void CameraOwner::fn_1_251C()
{
    Vec cameraPosition;
    Vec cameraRotation;
    Vec cameraTarget;
    s32 index;
    CameraViewport *camera;

    if ((f32)this->elapsedFrames > 60.0f) {
        if ((f32)this->elapsedFrames > 120.0f) {
            this->flags &= ~1;
            CameraOwner::Callback callback = &CameraOwner::fn_1_2080;
            this->callback = callback;
        }
        if (fn_8002D75C(this->modelId) != 0) {
            fn_800262D0(this->modelId);
            this->elapsedFrames = 0;
            CameraOwner::Callback callback = &CameraOwner::fn_1_2080;
            this->callback = callback;
            for (index = 0; index < 4; index++) {
                camera = &cameras[index];
                /* All four viewports receive camera zero's frame. */
                fn_80027CD0(viewportCameraBits[0], &cameraPosition,
                                 &cameraRotation, &cameraTarget);
                camera->position.x = cameraPosition.x;
                camera->position.y = cameraPosition.y;
                camera->position.z = cameraPosition.z;
                camera->up.x = cameraRotation.x;
                camera->up.y = cameraRotation.y;
                camera->up.z = cameraRotation.z;
                camera->target.x = cameraTarget.x;
                camera->target.y = cameraTarget.y;
                camera->target.z = cameraTarget.z;
            }
        }
    }
    this->elapsedFrames++;
}

/* Expand camera zero to the screen and disable the other active cameras. */
void CameraOwner::fn_1_26F8()
{
    s32 index;
    CameraViewport *camera;

    Hu3DCameraPerspectiveSet(1, 45.0f, 100.0f,
                             5000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
                          640.0f, 480.0f,
                          0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    for (index = 1; index < 4; index++) {
        camera = &this->cameras[index];
        if (camera->active == 1) {
            Hu3DCameraKill(viewportCameraBits[index]);
            camera->active = 0;
        }
    }
}

/* Select camera callbacks and configure the requested camera transition. */
void CameraOwner::flagEnabled(int mode)
{
    lbl_1_bss_50 = mode;
    switch (mode) {
    case 0: {
        this->elapsedFrames = 0;
        CameraOwner::Callback callback = &CameraOwner::fn_1_251C;
        this->callback = callback;
        CameraViewport *camera = &this->cameras[0];
        Hu3DCameraCreate(1);
        s16 motion = fn_8002C4C0(fn_80008DD0(0x3E0007, 0x10000000,
                                                        2));
        this->modelId = fn_800283F4(motion, 1);
        fn_800284C8(this->modelId, 1);
        fn_1_26F8();
        break;
    }
    case 1:
        this->flags &= ~(1UL << mode);
        break;
    case 2: {
        CameraOwner::Callback callback = &CameraOwner::followActiveCamerasOnRequest;
        this->callback = callback;
        this->flags &= ~(1UL << mode);
        break;
    }
    case 3:
        this->flags &= ~(1UL << mode);
        break;
    case 4: {
        fn_1_26F8();
        CameraOwner::Callback callback = &CameraOwner::fn_1_1F4C;
        this->callback = callback;
        this->flags &= ~(1UL << mode);
        break;
    }
    default:
        break;
    }
}
/* Invoke the selected camera callback when one is present. */
void CameraOwner::update()
{
    if (callback) {
        (this->*callback)();
    }
}
/* Project a sound into its selected viewport and clamp the stereo panning value. */
s16 getSoundPanning(Vec *position, s32 playerIndex)
{
    Vec screenPosition;
    CameraOwner *configuration;
    s32 cameraIndex;
    f32 normalizedX;
    f32 panningValue;
    s32 panning;

    configuration = &CameraOwner::getChecked();
    cameraIndex = playerIndex;
    if (lbl_1_bss_50 == 1) {
        cameraIndex = 0;
    }
    if (lbl_1_bss_50 == 3 && configuration->selectedCamera >= 0) {
        cameraIndex = configuration->selectedCamera;
    }

    Hu3D3Dto2D(position, (s16)viewportCameraBits[cameraIndex], &screenPosition);
    normalizedX = screenPosition.x / 576.0f;
    panningValue = configuration->cameras[cameraIndex].left +
                   normalizedX * configuration->cameras[cameraIndex].width;
    panningValue *= 0.11111111f;
    if ((s32)(32.0f + panningValue) < 0) {
        panning = 0;
    } else if ((s32)(32.0f + panningValue) > 0x7F) {
        panning = 0x7F;
    } else {
        panning = (s32)(32.0f + panningValue);
    }
    return (s16)panning;
}

/* Project a sound into its selected viewport and apply stereo panning. */
static inline void setSoundPanning(Vec *position, s32 soundHandle, s32 playerIndex)
{
    Vec screenPosition;
    CameraOwner *configuration;
    s32 cameraIndex;
    f32 normalizedX;
    f32 panningValue;
    s32 roundedPanning;
    s32 panning;
    s16 panningOutput;

    configuration = &CameraOwner::getChecked();
    cameraIndex = playerIndex;
    if (lbl_1_bss_50 == 1) {
        cameraIndex = 0;
    }
    if (lbl_1_bss_50 == 3 && configuration->selectedCamera >= 0) {
        cameraIndex = configuration->selectedCamera;
    }

    Hu3D3Dto2D(position, (s16)viewportCameraBits[cameraIndex], &screenPosition);
    normalizedX = screenPosition.x / 576.0f;
    panningValue = normalizedX * configuration->cameras[cameraIndex].width +
                   configuration->cameras[cameraIndex].left;
    panningValue *= 0.11111111f;
    roundedPanning = (s32)(32.0f + panningValue);
    if (roundedPanning < 0) {
        panning = 0;
    } else if (roundedPanning > 0x7F) {
        panning = 0x7F;
    } else {
        panning = roundedPanning;
    }
    panningOutput = panning;
    HuAudFXPanning(soundHandle, panningOutput);
}

/* Play a sound effect and pan it in the selected camera viewport. */
s32 fn_1_2DB4(Vec *position, s32 soundEffect, s32 playerIndex)
{
    s32 soundHandle = HuAudFXPlay(soundEffect);
    setSoundPanning(position, soundHandle, playerIndex);
    return soundHandle;
}

/* Play and pan a sound effect only during request two. */
s32 fn_1_2F58(Vec *position, s32 soundEffect, s32 playerIndex)
{
    s32 soundHandle;

    soundHandle = -1;
    if (currentRequest() == 2) {
        soundHandle = HuAudFXPlay(soundEffect);
        setSoundPanning(position, soundHandle, playerIndex);
    }
    return soundHandle;
}

/* Set a sound effect's projected panning, bounded pitch and player volume. */
void setSoundPositionPitchAndVolume(Vec *position, s32 soundHandle,
                           s32 playerIndex, f32 pitchInput)
{
    Vec screenPosition;
    CameraOwner *configuration;
    s32 pitch;
    s32 cameraIndex;
    s32 clampedPanning;
    s32 panningValueInteger;
    s16 panning;
    f32 normalizedX;
    f32 panningValue;

    pitch = (s32)(8191.0 *
                  (2.0 * (pitchInput - 0.5)));
    if (pitch > 0x1FFF) {
        pitch = 0x1FFF;
    }
    if (pitch < -0x2000) {
        pitch = -0x2000;
    }

    configuration = &CameraOwner::getChecked();
    cameraIndex = playerIndex;
    if (lbl_1_bss_50 == 1) {
        cameraIndex = 0;
    }
    if (lbl_1_bss_50 == 3 && configuration->selectedCamera >= 0) {
        cameraIndex = configuration->selectedCamera;
    }

    Hu3D3Dto2D(position, (s16)viewportCameraBits[cameraIndex], &screenPosition);
    normalizedX = screenPosition.x / 576.0f;
    panningValue = configuration->cameras[cameraIndex].left +
                   normalizedX * configuration->cameras[cameraIndex].width;
    panningValue *= 0.11111111f;
    panningValueInteger = (s32)(32.0f + panningValue);
    if (panningValueInteger < 0) {
        clampedPanning = 0;
    } else if (panningValueInteger > 0x7F) {
        clampedPanning = 0x7F;
    } else {
        clampedPanning = panningValueInteger;
    }
    panning = (s16)clampedPanning;

    HuAudFXPanning(soundHandle, panning);
    HuAudFXPitchSet(soundHandle, (s16)pitch);
    HuAudFXVolSet(soundHandle, 112.0f * lbl_1_bss_40[playerIndex]);
}

/* Test the projected horizontal position against a camera visibility interval. */
s32 fn_1_3340(Vec *position, s32 cameraIndex, u8 widescreen)
{
    Vec screenPosition;
    f32 width;
    f32 height;
    f32 normalizedX;
    s32 visible;

    Hu3D3Dto2D(position, (s16)viewportCameraBits[cameraIndex], &screenPosition);
    width = 640.0f;
    if (widescreen != 0) {
        width *= 0.9602649f;
    }
    height = 288.0f;
    normalizedX = (screenPosition.x - height) / height;
    visible = 0;
    if (normalizedX > -(722.0f / width) &&
        normalizedX < 722.0f / width) {
        visible = 1;
    }
    return visible;
}

/* Prints the projected horizontal position selected for the camera. */
void fn_1_3448(Vec *position, s32 index) {
    Vec projected;

    Hu3D3Dto2D(position, (s16)viewportCameraBits[index], &projected);
    printf("center %f\n", projected.x);
}
