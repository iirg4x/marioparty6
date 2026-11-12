#include <new>
#include "REL/m655DLL/game_constants.h"
#include "REL/framework/debug.h"
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/mtx.h"

extern "C" {
#include "math.h"
}

#include "REL/framework/camera_frame.h"
#include "REL/framework/category_sphere.h"
#include <vector>

void set_camera_frame_from_pose(CameraFrame *, const CameraPose *);
void cameraApplyToHud(CameraFrame *, s32);
void cameraTiltToAngle(CameraFrame *, float);
void get_camera_pose_from_frame(const CameraFrame *, CameraPose *);
void interpolate_camera_poses(CameraFrame *, const CameraPose *, const CameraPose *, float);
void cameraRotateAroundRightAndWorldYAxis(CameraFrame *, float, float);

extern "C" {
#include "game/hu3d.h"
}

#include "REL/m655DLL/camera_work.h"

#include "REL/framework/linknode.h"

#include "REL/m655DLL/goal.h"

#include "humath.h"

#include "REL/m655DLL/module.h"

#include <math.h>
#include "REL/framework/world_units.h"

/* Three camera poses at .rodata 0x340..0x394. */
extern "C" {
extern const CameraPose lbl_1_rodata_340[3] = {
    { {-30.0f, 1563.6318f, 560.0f}, {-0.31585145f, 0.0f, -0.0f, 0.9488087f} },
    { {-30.0f, 1010.82f, 580.0f}, {-0.21452136f, 0.0f, -0.0f, 0.9767193f} },
    { {793.18365f, 997.13837f, 541.37195f}, {-0.100574255f, 0.0f, -0.0f, 0.9949296f} }
};
}

void copyDebugCameraPose(const CameraFrame *frame);

CameraWork::CameraWork()
    : state(&CameraWork::fn_1_6D2C), unknown44(-1), itemCount(0), stateCounter(0)
{
    CameraFrame position;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 1.0f,
                             5000.0f, 1.2f);
    Hu3DBGColorSet(0, 0, 0);
    set_camera_frame_from_pose(&static_cast<CameraFrame &>(*this), lbl_1_rodata_340);
    set_camera_frame_from_pose(&this->nextFrame.frame, &lbl_1_rodata_340[1]);

    position.position.x = -30.0f;
    position.position.y = 1250.45f;
    position.position.z = 140.0f;
    position.target.x = -30.0f;
    position.target.y = 1249.72f;
    position.target.z = 139.32f;
    position.up.x = 0.0f;
    position.up.y = 1.0f;
    position.up.z = 0.0f;
    copyDebugCameraPose(&position);
    this->registerChannel(0);
}

void fn_1_7414(void *owner);

template <class T> inline T Square(const T &value)
{
    return value * value;
}

#include "REL/framework/scene_shadow.h"

struct Sphere;

typedef std::vector<Sphere *> ShadowSpheres;

float fitCameraToSpheres(CameraFrame *, CameraPerspective *, ShadowSpheres *);

static inline void setCameraState(CameraWork *object, CameraWork::State next)
{ object->state = next; }
CameraWork::~CameraWork() {}

void CameraWork::flagEnabled(int eventType)
{
    switch (eventType) {
    case 0:
    {
        fn_1_7408(this);
        setCameraState(this, &CameraWork::fn_1_6B94);
        return;
    }
    case 1:
        fn_1_7414(this);
        this->flags &= ~(1 << eventType);
        return;
    case 2:
    case 3:
    case 4:
        this->flags &= ~(1 << eventType);
    default:
        return;
    }
}

void CameraWork::update()
{
    (this->*this->state)();
    this->stateCounter++;
    debugResetHook(&this->nextFrame.frame, 0, 1);
}

void CameraWork::receive(const EventPacket *event)
{
    switch (event->type) {
    case 3:
    {
        this->stateCounter = 0;
        {
            setCameraState(this, &CameraWork::fn_1_6D5C);
        }
        return;
    }
    case 5:
    {
        this->stateCounter = 0;
        {
            setCameraState(this, &CameraWork::fn_1_6F8C);
        }

    }
    }
}

void CameraWork::fn_1_6B94(void)
{
    s32 duration;
    f64 zoomAmount;
    const CameraPose *startPose;
    const CameraPose *endPose;

    duration = secondsToFrames<long>(3);
    zoomAmount = Square(1.0 - cos(M_PI *
                                 (f64)this->stateCounter / duration)) /
                 4.0;
    startPose = lbl_1_rodata_340;
    endPose = &lbl_1_rodata_340[1];
    interpolate_camera_poses(&static_cast<CameraFrame &>(*this), startPose, endPose, (f32)zoomAmount);
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), 1);
    if (this->stateCounter >= duration) {
        this->flags &= ~1;
        setCameraState(this, &CameraWork::fn_1_6D2C);
    }
}

void CameraWork::fn_1_6D2C()
{
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), 1);
}

static inline f64 minimumValue(f64 first, f64 second)
{
    return first < second ? first : second;
}

void CameraWork::fn_1_6D5C(void)
{
    CameraPose previousPose;
    CameraPose currentPose;
    f64 normalizedProgress;

    get_camera_pose_from_frame(&static_cast<CameraFrame &>(*this), &previousPose);
    target.x = position.x;
    cameraTiltToAngle(&static_cast<CameraFrame &>(*this), -60.0f);
    fn_1_7220();
    get_camera_pose_from_frame(&static_cast<CameraFrame &>(*this), &currentPose);
    normalizedProgress = minimumValue(1.0,
        (f64)this->stateCounter / secondsToFrames(1.0));
    interpolate_camera_poses(&static_cast<CameraFrame &>(*this), &previousPose, &currentPose, (f32)(0.1 * normalizedProgress));
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), 1);
    if (normalizedProgress >= 1.0) {
        setCameraState(this, &CameraWork::fn_1_6F00);
    }
}

void CameraWork::fn_1_6F00(void)
{
    Vec current = position;
    if (this->fn_1_6FB4() != 0) {
        setCameraState(this, &CameraWork::fn_1_6D2C);
    }
}

void CameraWork::fn_1_6F8C(void)
{
    this->fn_1_6FB4();
}

/* The framing includes a margin above the goal.
 * It is measured in world units.
 */
static inline f32 maximumValue(f32 first, f32 second)
{

/* Retain assertion source location.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
    ASSERT(FALSE);
    return first > second ? first : second;
}

u8 CameraWork::fn_1_6FB4()
{
    CameraPose previousPose;
    CameraPose currentPose;
    s32 changed;
    f32 currentHeight;
    f32 sceneHeight;
    f64 progress;

    get_camera_pose_from_frame(&static_cast<CameraFrame &>(*this), &previousPose);
    target.x = position.x;
    cameraRotateAroundRightAndWorldYAxis(&static_cast<CameraFrame &>(*this),
               (f32)framesToSeconds(200.0), 0.0f);
    changed = fn_1_7220();
    sceneHeight = Goal::getChecked().fn_1_3470()->y + cmToWorld(100.0f);
    currentHeight = position.y;
    position.y = maximumValue(currentHeight, sceneHeight);
    get_camera_pose_from_frame(&static_cast<CameraFrame &>(*this), &currentPose);
    if ((u8)changed != 0) {
        stateCounter = 0;
    }
    progress = minimumValue(1.0, (f64)stateCounter /
                         secondsToFrames(2.5));
    interpolate_camera_poses(&static_cast<CameraFrame &>(*this), &previousPose, &currentPose, (f32)(0.1 * progress));
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), 1);
    return changed;
}

s32 CameraWork::fn_1_7220(void)
{
    CameraPerspective context;
    context.read(1);
    ShadowSpheres spheres;
    collectCategoryValues(&spheres, 1);
    s32 itemCount = spheres.size();
    if (itemCount > 0) fitCameraToSpheres(&static_cast<CameraFrame &>(*this), &context, &spheres);
    bool changed = itemCount != this->itemCount;
    this->itemCount = itemCount;
    return changed;
}

void fn_1_7408(CameraWork *camera) {
    camera->stateCounter = 0;
}

void fn_1_7414(void *owner) {}

extern "C" {

// The game never reads these bytes.
u8 lbl_1_bss_90[8];
}
