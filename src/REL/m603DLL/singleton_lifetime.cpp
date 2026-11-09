#include "math.h"
#include "REL/m603DLL/callback_owner.h"
#include "REL/m603DLL/singleton_32930.h"
#include "REL/m603DLL/player_position.h"
extern "C" {
#include "game/hu3d.h"
#include "game/hsfex.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/audio.h"
#include <stdio.h>
extern s16 lbl_802C0278;
f32 lbl_1_bss_40[4]; s32 lbl_1_bss_50; extern u16 lbl_802C0008[];
void cameraApplyToHud__FP11CameraFramel(void*,s32);
}
void debugEventHook(void*, int);
void operator delete(void*);
#include "REL/m603DLL/camera_group.h"
struct SetupValuesBBC { Vec points[4]; };
extern "C" {
const s32 lbl_1_rodata_60[4]={1,2,4,8};
const SetupValuesBBC lbl_1_rodata_70={{{0,0,0},{361.0f,0,0},{0,240.0f,0},{361.0f,240.0f,0}}};
const Vec lbl_1_rodata_A0={0,2800.0f,3000.0f}; const Vec lbl_1_rodata_AC={0,1.0f,0}; const Vec lbl_1_rodata_B8={0,0,200.0f};
}
CameraOwnerBBC::CameraOwnerBBC() : transitionRequested(0), callback(&CameraOwnerBBC::fn_1_1944), selectedCamera(-1)
{
 s32 index; s32 resetIndex;
    struct SetupValuesBBC setupValues = lbl_1_rodata_70;
    for (index = 0; index < 4; index++) {
        CameraGroupBBC *group = &this->cameras[index];

        group->position.x = lbl_1_rodata_A0.x;
        group->position.y = lbl_1_rodata_A0.y;
        group->position.z = lbl_1_rodata_A0.z;
        group->rotation.x = lbl_1_rodata_AC.x;
        group->rotation.y = lbl_1_rodata_AC.y;
        group->rotation.z = lbl_1_rodata_AC.z;
        group->target.x = lbl_1_rodata_B8.x;
        group->target.y = lbl_1_rodata_B8.y;
        group->target.z = lbl_1_rodata_B8.z;
        group->active = 0;
        group->marked = 0;
        this->elapsedFrames = 0;
    }
    for (resetIndex = 0; resetIndex < 4; resetIndex++) {
        lbl_1_bss_40[resetIndex] = 1.0f;
    }
}
void CameraOwnerBBC::fn_1_F30(int playerIndex)
{
    CameraGroupBBC *player = &cameras[playerIndex];
    player->marked = 1;
}

void CameraOwnerBBC::fn_1_F58(s32 cameraIndex, f32 left, f32 top,
               f32 width, f32 height)
{
    CameraGroupBBC *viewport =
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
            Hu3DCameraCreate(lbl_1_rodata_60[cameraIndex]);
            viewport->active = 1;
            printf("Create %d\n", cameraIndex);
        }
    } else if (viewport->active == 1) {
        Hu3DCameraKill(lbl_1_rodata_60[cameraIndex]);
        viewport->active = 0;
    }
    if (viewport->active != 0) {
        Hu3DCameraPerspectiveSet(lbl_1_rodata_60[cameraIndex],
                                45.0f, 100.0f, 5000.0f, 1.35375f);
        left = 17.0 + 0.84 * left;
        top = 39.0 + 0.84 * top;
        width *= 0.84;
        height *= 0.84;
        Hu3DCameraViewportSet(lbl_1_rodata_60[cameraIndex],
                             left, top, width, height, 0.0f, 1.0f);
        Hu3DCameraScissorSet(lbl_1_rodata_60[cameraIndex],
                            (u32)left, (u32)top,
                            (u32)width, (u32)height);
    }
}

void CameraOwnerBBC::fn_1_12EC(f32 insetX, f32 insetY)
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

void fn_1_14DC(
    CameraOwnerBBC *owner,
    s32 playerIndex,
    Vec *position,
    Vec *direction)
{
    CameraGroupBBC *player = &owner->cameras[playerIndex];

    PSVECSubtract(position, &player->target, position);
    PSVECScale(position, position, 0.1f);
    PSVECAdd(position, &player->target, position);
    PSVECSubtract(direction, &player->position, direction);
    PSVECScale(direction, direction, 0.05f);
    PSVECAdd(direction, &player->position, direction);
}

void fn_1_15A4(void *owner, int playerIndex, Vec *position, Vec *direction)
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

void fn_1_161C(void *owner, int playerIndex)
{
    Vec position;
    Vec direction;

    fn_1_6A14(playerIndex, &position, &direction);
}

void fn_1_164C(CameraOwnerBBC *records)
{
    s32 playerIndex;
    CameraGroupBBC *player;
    Vec difference;
    Vec direction;
    Vec position;
    f32 distanceSquared;
    f32 distance;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        player = &records->cameras[playerIndex];
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
            cameraApplyToHud__FP11CameraFramel(
                player, lbl_1_rodata_60[playerIndex]);
        }
    }
}

void CameraOwnerBBC::fn_1_1944()
{
    s32 playerIndex;
    s32 queryPlayer;
    CameraGroupBBC *player;
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
            cameraApplyToHud__FP11CameraFramel(
                player, lbl_1_rodata_60[playerIndex]);
        }
    }
}

void CameraOwnerBBC::fn_1_1C48()
{
    s32 playerIndex;
    s32 queryPlayer;
    CameraGroupBBC *player;
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
            cameraApplyToHud__FP11CameraFramel(
                player, lbl_1_rodata_60[playerIndex]);
        }
    }
}
void CameraOwnerBBC::fn_1_1F4C()
{
 CameraGroupBBC *frame=&cameras[0];
 frame->position.x=0.0f;frame->position.y=550.0f;frame->position.z=250.0f;frame->target.x=0.0f;frame->target.y=120.0f;frame->target.z=-350.0f;
 cameraApplyToHud__FP11CameraFramel(frame,lbl_1_rodata_60[0]);
}

void fn_1_1FF0(CameraOwnerBBC *object)
{
    debugEventHook(&object->menuState[0], 0);
    if ((lbl_802C0008[0] & 0x800) != 0) {
        CameraOwnerBBC::Callback values = &CameraOwnerBBC::fn_1_1C48;
        object->callback = values;
    }
    cameraApplyToHud__FP11CameraFramel(&object->menuState[0], 1);
}

void CameraOwnerBBC::fn_1_2080()
{
    s32 index;
    CameraGroupBBC *camera;

    if (transitionRequested != 0) {
        fn_800262D0(modelId);
        fn_1_12EC(
                   361.0f, 240.0f);

        index = 0;
    while (index < 4) {
        camera = &cameras[index];
        if (camera->active == 1) {
            Vec *targetSlot;
            CameraGroupBBC *activeCamera;
            s32 playerIndex;
            Vec difference;
            Vec position;
            Vec target;
            f32 length;

            activeCamera = camera, targetSlot = &camera->target;
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
    cameraApplyToHud__FP11CameraFramel(camera, lbl_1_rodata_60[index]);
        }
        index++;
        }
    }
}

void CameraOwnerBBC::fn_1_251C()
{
    Point3d cameraPosition;
    Point3d cameraRotation;
    Point3d cameraTarget;
    s32 index;
    CameraGroupBBC *camera;

    if ((f32)this->elapsedFrames > 60.0f) {
        if ((f32)this->elapsedFrames > 120.0f) {
            this->state &= ~1;
            {
                CameraOwnerBBC::Callback callback = &CameraOwnerBBC::fn_1_2080;
                this->callback = callback;
            }
        }
        if (fn_8002D75C(this->modelId) != 0) {
            fn_800262D0(this->modelId);
            this->elapsedFrames = 0;
            {
                CameraOwnerBBC::Callback callback = &CameraOwnerBBC::fn_1_2080;
                this->callback = callback;
            }
            index = 0;
            while (index < 4) {
                camera = &cameras[index];
                fn_80027CD0(lbl_1_rodata_60[0], &cameraPosition,
                                 &cameraRotation, &cameraTarget);
                camera->position.x = cameraPosition.x;
                camera->position.y = cameraPosition.y;
                camera->position.z = cameraPosition.z;
                camera->rotation.x = cameraRotation.x;
                camera->rotation.y = cameraRotation.y;
                camera->rotation.z = cameraRotation.z;
                camera->target.x = cameraTarget.x;
                camera->target.y = cameraTarget.y;
                camera->target.z = cameraTarget.z;
                index++;
            }
        }
    }
    this->elapsedFrames++;
}

void fn_1_26F8(CameraOwnerBBC *object)
{
    s32 index;
    CameraGroupBBC *camera;

    Hu3DCameraPerspectiveSet(1, 45.0f, 100.0f,
                             5000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
                          640.0f, 480.0f,
                          0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    for (index = 1; index < 4; index++) {
        camera = &object->cameras[index];
        if (camera->active == 1) {
            Hu3DCameraKill(lbl_1_rodata_60[index]);
            camera->active = 0;
        }
    }
}

void CameraOwnerBBC::update(unsigned long mode)
{
    lbl_1_bss_50 = mode;
    switch (mode) {
    case 0: {
        this->elapsedFrames = 0;
        CameraOwnerBBC::Callback callback = &CameraOwnerBBC::fn_1_251C;
        this->callback = callback;
        CameraGroupBBC *camera = &this->cameras[0];
        Hu3DCameraCreate(1);
        s16 motion = fn_8002C4C0(fn_80008DD0(0x3E0007, 0x10000000,
                                                        2));
        this->modelId = fn_800283F4(motion, 1);
        fn_800284C8(this->modelId, 1);
        fn_1_26F8(this);
        break;
    }
    case 1:
        this->state &= ~(1UL << mode);
        break;
    case 2: {
        CameraOwnerBBC::Callback callback = &CameraOwnerBBC::fn_1_1C48;
        this->callback = callback;
        this->state &= ~(1UL << mode);
        break;
    }
    case 3:
        this->state &= ~(1UL << mode);
        break;
    case 4: {
        fn_1_26F8(this);
        CameraOwnerBBC::Callback callback = &CameraOwnerBBC::fn_1_1F4C;
        this->callback = callback;
        this->state &= ~(1UL << mode);
        break;
    }
    default:
        break;
    }
}
void CameraOwnerBBC::extra(){if(callback){(this->*callback)();}}
s16 fn_1_2BFC(HuVecF *position, s32 playerIndex)
{
    HuVecF screenPosition;
    CameraOwnerBBC *configuration;
    s32 cameraIndex;
    f32 normalizedX;
    f32 panningValue;
    s32 panningValueInteger;
    s32 clampedPanning;
    s32 panning;

    configuration = (&CameraOwnerBBC::getChecked());
    cameraIndex = playerIndex;
    if (lbl_1_bss_50 == 1) {
        cameraIndex = 0;
    }
    if (lbl_1_bss_50 == 3 && configuration->selectedCamera >= 0) {
        cameraIndex = configuration->selectedCamera;
    }

    Hu3D3Dto2D(position, (s16)lbl_1_rodata_60[cameraIndex], &screenPosition);
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

static inline void setSoundPanning(HuVecF *position, s32 soundHandle, s32 playerIndex)
{
    HuVecF screenPosition;
    CameraOwnerBBC *configuration;
    s32 cameraIndex;
    f32 normalizedX;
    f32 panningValue;
    s32 roundedPanning;
    s32 panning;
    s16 panningOutput;

    configuration = (&CameraOwnerBBC::getChecked());
    cameraIndex = playerIndex;
    if (lbl_1_bss_50 == 1) {
        cameraIndex = 0;
    }
    if (lbl_1_bss_50 == 3 && configuration->selectedCamera >= 0) {
        cameraIndex = configuration->selectedCamera;
    }

    Hu3D3Dto2D(position, (s16)lbl_1_rodata_60[cameraIndex], &screenPosition);
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

s32 fn_1_2DB4(HuVecF *position, s32 soundEffect, s32 playerIndex)
{
    s32 soundHandle = HuAudFXPlay(soundEffect);
    setSoundPanning(position, soundHandle, playerIndex);
    return soundHandle;
}

s32 fn_1_2F58(HuVecF *position, s32 soundEffect, s32 playerIndex)
{
    s32 soundHandle;

    soundHandle = -1;
    if (fn_1_1AB30() == 2) {
        soundHandle = HuAudFXPlay(soundEffect);
        setSoundPanning(position, soundHandle, playerIndex);
    }
    return soundHandle;
}


void fn_1_310C(HuVecF *position, s32 soundHandle,
                           s32 playerIndex, f32 pitchInput)
{
    HuVecF screenPosition;
    CameraOwnerBBC *configuration;
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

    configuration = (&CameraOwnerBBC::getChecked());
    cameraIndex = playerIndex;
    if (lbl_1_bss_50 == 1) {
        cameraIndex = 0;
    }
    if (lbl_1_bss_50 == 3 && configuration->selectedCamera >= 0) {
        cameraIndex = configuration->selectedCamera;
    }

    Hu3D3Dto2D(position, (s16)lbl_1_rodata_60[cameraIndex], &screenPosition);
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

s32 fn_1_3340(HuVecF *position, s32 cameraIndex, u8 widescreen)
{
    HuVecF screenPosition;
    f32 width;
    f32 height;
    f32 normalizedX;
    s32 visible;

    Hu3D3Dto2D(position, (s16)lbl_1_rodata_60[cameraIndex], &screenPosition);
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
void fn_1_3448(HuVecF *position, s32 index) {
    HuVecF projected;

    Hu3D3Dto2D(position, (s16)lbl_1_rodata_60[index], &projected);
    printf("center %f\n", projected.x);
}

CameraOwnerBBC::~CameraOwnerBBC(){}

void CallbackOwner::extra()
{
}
