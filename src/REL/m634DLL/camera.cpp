extern "C" {
#include "dolphin/types.h"
}

extern "C" {
#include "dolphin/mtx.h"
}

extern "C" {
#include "dolphin/gx.h"
}

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/gamework.h"
}

#include "REL/framework/model.h"
#include "REL/framework/minigame_core.h"
#include "REL/m634DLL/scene_owner_vtable.h"

#include "REL/m634DLL/camera.h"
#include "REL/framework/event_callback_inline.h"

extern "C" {
#include "game/data.h"
}

extern "C" {
#include "game/memory.h"
}

#include "humath.h"

extern "C" {
#include "game/main.h"
}

extern "C" {
#include "game/object.h"
}

extern "C" {
#include "game/audio.h"
}

extern "C" {
#include "game/charman.h"
}

extern "C" {
#include "game/gamemes.h"
}

extern "C" {
#include "game/hsfex.h"
}

extern "C" {
#include "game/mg/seqman.h"
}

extern "C" {
#include "game/mg/timer.h"
}

extern "C" {
#include "game/mg/score.h"
}

extern "C" {
#include "game/pad.h"
}

extern "C" {
#include "game/frand.h"
}

extern "C" {
#include "game/sprite.h"
}

extern "C" {
#include "game/wipe.h"
}

extern "C" {
#include "game/mg/actman.h"
}

extern "C" {
#include "game/board/object.h"
}

extern "C" {
#include "game/board/tutorial.h"
}

extern "C" {
#include "datadir_enum.h"
}

#include "REL/m634DLL/sequence_owner.h"
#include <math.h>

const SceneCameraDefaults lbl_1_rodata_7C0 = {
    { -229.0f, 50.0f, -900.0f },
    { 0.0f, 1.0f, 0.0f },
    { -229.0f, 50.0f, 0.0f },
};

// Creates the cameras and registers their motion callbacks.
CameraSceneWork::CameraSceneWork()
    : GameObject(0x10)
    , registration(1)
{
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(2, 45.0f, 1.0f, 1000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 0, 2, 0x280, 0x1E0);
    Hu3DCameraPerspectiveSet(1, 45.0f, 300.0f, 1000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 2, 0x280, 0x1E0);
    SceneCameraDefaults cameraSetup = lbl_1_rodata_7C0;
    SceneCameraMotion *camera = &this->camera;
    camera->setPosition(camera->initialPosition = cameraSetup.initialPosition);
    camera->up = cameraSetup.up;
    camera->setTarget(camera->initialTarget = cameraSetup.initialTarget);
    camera->unk4C = 0;
    cameraApplyToHud(camera, 1);
    HU3D_MOTIONID motion = fn_8002C4C0(fn_80008DD0(0x5D0001, 0x10000000, 2));
    this->motionId = fn_800283F4((s16)motion, 2);
    fn_800284C8(this->motionId, 2);
    registerEventCallback(
        &this->registration, std::bind1st(std::mem_fun(&CameraSceneWork::fn_1_11064), this), 4);
    setCallback(&CameraSceneWork::fn_1_10E0C);
}

// Interpolates the camera and applies it to camera 2.
void CameraSceneWork::fn_1_10DB8()
{
    SceneCameraMotion *cameraView = &camera0;
    fn_1_10F8C(0.2f);
    cameraApplyToHud(cameraView, 2);
}

// Reads the camera and starts its next motion when the current motion finishes.
void CameraSceneWork::fn_1_10E0C()
{
    cameraReadFromHud(&camera0, 2);
    if (fn_8002D75C(motionId)) {
        SceneCameraMotion *motion;
        HU3D_CAMERA *camera;
        Vec *targetPosition;
        Vec *positionView;
        Vec *positionSource;
        Vec *endPosition;
        Vec *directionView;
        Vec *directionSource;

        fn_800262D0(motionId);
        camera = &Hu3DCamera[1];
        motion = &camera0;
        motion->initialPosition.x = camera->pos.x;
        motion->initialPosition.y = camera->pos.y;
        motion->initialPosition.z = camera->pos.z;
        targetPosition = &motion->initialPosition;
        positionView = targetPosition;
        positionSource = positionView;
        motion->position.x = positionSource->x;
        motion->position.y = positionSource->y;
        motion->position.z = positionSource->z;
        motion->up.x = camera->up.x;
        motion->up.y = camera->up.y;
        motion->up.z = camera->up.z;
        motion->initialTarget.x = camera->target.x;
        motion->initialTarget.y = camera->target.y;
        motion->initialTarget.z = camera->target.z;
        endPosition = &motion->initialTarget;
        directionView = endPosition;
        directionSource = directionView;
        motion->target.x = directionSource->x;
        motion->target.y = directionSource->y;
        motion->target.z = directionSource->z;
        motion->unk4C = 0;
        {
            MotionCallback initialValues = &CameraSceneWork::fn_1_10DB8;

            initialCallback = initialValues;
        }
    }
}

// Does nothing.
void fn_1_10F40() { }

// Runs the current camera motion callback.
void CameraSceneWork::update()
{
    if (initialCallback) {
        (this->*initialCallback)();
    }
}

// Interpolates camera position and target toward their final values.
void CameraSceneWork::fn_1_10F8C(f32 amount)
{
    Vec *first = &camera0.position;
    Vec *firstValue = first;
    Vec *second = first + 2;

    firstValue->x += amount * (first[3].x - firstValue->x);
    firstValue->y += amount * (first[3].y - firstValue->y);
    firstValue->z += amount * (first[3].z - firstValue->z);
    second->x += amount * (first[4].x - second->x);
    second->y += amount * (first[4].y - second->y);
    second->z += amount * (first[4].z - second->z);
}

struct CameraVectorValue {
    Vec value;
};

extern "C" {
extern const CameraVectorValue lbl_1_rodata_808[];
}

extern "C" {
extern const Vec lbl_1_rodata_820;
}

extern "C" {
extern const Vec lbl_1_rodata_82C[];
}

extern "C" {
const CameraVectorValue lbl_1_rodata_808[2]
    = { { { -360.0f, 0.0f, -300.0f } }, { { 164.0f, 0.0f, 85.0f } } };
}

extern "C" {
const Vec lbl_1_rodata_820 = { 0.0f, 1.0f, 0.0f };
}

extern "C" {
const Vec lbl_1_rodata_82C[2] = { { 600.0f, 1000.0f, 800.0f }, { 350.0f, 1400.0f, 1400.0f } };
}

// Sets the camera motion from the event selection.
void CameraSceneWork::fn_1_11064(const EventPacket &event)
{
    CameraVectorValue point;
    s32 index = *(s32 *)event.context;

    if (index != -1) {
        point = lbl_1_rodata_808[index];
        PSVECAdd(&point.value, &lbl_1_rodata_82C[index], &point.value);
        SceneCameraMotion *camera = &camera0;
        camera->setPosition(camera->initialPosition = point.value);
        camera->up.x = lbl_1_rodata_820.x;
        camera->up.y = lbl_1_rodata_820.y;
        camera->up.z = lbl_1_rodata_820.z;
        camera->setTarget(camera->initialTarget = lbl_1_rodata_808[index].value);
        camera->unk4C = 0;
        cameraReadFromHud(camera, 2);
    }
}

// Returns the yaw from the camera target to its position.
f32 CameraSceneWork::fn_1_111E0()
{
    Point3d displacement;

    PSVECSubtract(&camera0.position, &camera0.target, &displacement);
    return (f32)(180.0 * (atan2((f64)displacement.x, (f64)displacement.z) / M_PI));
}

// Releases the object and its member-owned resources.
CameraSceneWork::~CameraSceneWork() { }
