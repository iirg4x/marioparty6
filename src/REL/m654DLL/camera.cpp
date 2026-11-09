#include "math.h"
#include "REL/m654DLL/time_conversion.h"
#include "REL/m654DLL/unit_conversion.h"
#include "REL/framework/debug.h"
#include "dolphin/types.h"
#include "REL/m654DLL/framework/linknode.h"
#include "REL/m654DLL/camera_setup.h"
#include "REL/m654DLL/camera_transition.h"
#include "REL/m654DLL/camera_node_owner.h"
extern "C" {
#include "dolphin/mtx.h"
}
#include "humath.h"
extern "C" {
#include "game/hu3d.h"
#include "REL/m654DLL/pad_control.h"

double fn_1_11F4(double);
}

extern "C" {
extern void __dl__FPv(void *);
}

extern "C" {
extern const unsigned char __vt__16ManagerEventBase[];
extern const unsigned char __vt__13CameraManager[];

const CameraPosePair lbl_1_rodata_90[2] = {
    {{{0.0f, 923.52576f, 1120.26f}, {-0.17122246f, 0.0f, -0.0f, 0.9852325f}}, {{0.0f, 929.43555f, 980.26f}, {-0.43945006f, 0.0f, -0.0f, 0.8982671f}}},
    {{{0.0f, 1029.89f, 1700.26f}, {-0.24941446f, 0.0f, -0.0f, 0.96839684f}}, {{793.18365f, 997.13837f, 541.37195f}, {-0.100574255f, 0.0f, -0.0f, 0.9949296f}}}
};

extern Node lbl_1_bss_10C;
Node *__ct__4NodeFv(Node *object);
void insert4__4NodeFP4Node(Node *object, Node *link);
void __ct__11CameraSetupFv(CameraSetup *camera);

void fn_1_1132C(void *object, int value);
void fn_1_111CC(void *object);
void __msl_assertion_failed(const char *expression, const char *file,
                            int line);

}
CameraManager::CameraManager()
    : callback(&CameraManager::fn_1_1948), startPose(lbl_1_rodata_90[0].frame), targetPose(lbl_1_rodata_90[0].parameters), elapsed(0)
{
    Hu3DCameraCreate(1);
    Hu3DBGColorSet(0, 0, 0);
    set_camera_frame_from_pose(&static_cast<CameraFrame &>(*this), &startPose);
    registerChannel(0);
    set_camera_frame_from_pose(&finalCamera.frame, &lbl_1_rodata_90[1].frame);
}

extern "C" {
void __dt__13EventListenerFv(void *, short);
void __dt__4NodeFv(void *, short);
void fn_1_1978(CameraManager *, const CameraPose *, const CameraPose *, int);

int lbl_1_bss_48 = fn_1_B80(1);
float lbl_1_bss_4C = framesToSeconds(framesToSeconds(980.665));
}
void operator delete(void *);
typedef void (CameraManager::*CameraNodeCallback)();
template <class T> inline T Square(const T &value) { return value * value; }
CameraManager::~CameraManager() {}

void CameraManager::handleEvent(int state)
{
    this->activeBit = state;
    switch (state) {
    case 0:
        fn_1_1978(this, &lbl_1_rodata_90[0].frame, &lbl_1_rodata_90[0].parameters, secondsToFrames<long>(1));
        break;
    case 1:
        fn_1_1978(this, &lbl_1_rodata_90[0].parameters, &lbl_1_rodata_90[1].frame, secondsToFrames<long>(2));
        break;
    default:
        this->eventMask &= ~(1 << state);
        break;
    }
}

void CameraManager::advanceFrame()
{
    this->run();
    this->elapsed++;
    debugResetHook(&this->finalCamera.frame, 0, 1);
}

/* Offsets of the follow camera from the target position. */
static inline float FollowEyeDistance() { return 600.0f; }
static inline float FollowEyeHeight() { return 200.0f; }
static inline float FollowTargetHeight() { return 100.0f; }

void CameraManager::receive(const EventPacket *camera)
{
    if (camera->type == 4 && camera->context != 0) {
        Vec *targetPosition = static_cast<Vec *>(camera->context);
        double startX;
        double startY;
        double startZ;
        double endX;
        double endY;
        double endZ;
        CameraFrame path;
        startZ = targetPosition->z + FollowEyeDistance();
        startY = targetPosition->y + FollowEyeHeight();
        startX = targetPosition->x;
        path.eye.x = startX;
        path.eye.y = startY;
        path.eye.z = startZ;
        endZ = targetPosition->z;
        endY = targetPosition->y + FollowTargetHeight();
        endX = targetPosition->x;
        path.target.x = endX;
        path.target.y = endY;
        path.target.z = endZ;
        path.up.x = 0.0f;
        path.up.y = 1.0f;
        path.up.z = 0.0f;
        get_camera_pose_from_frame(&path, &this->targetPose);
        fn_1_1978(this, &lbl_1_rodata_90[1].frame, &this->targetPose, secondsToFrames<long>(3));
    }
}

void CameraManager::fn_1_1948() {
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), 1);
}

void fn_1_1978(CameraManager *owner, const CameraPose *first,
                           const CameraPose *second, int mode)
{
    owner->startPose = *first;
    owner->targetPose = *second;
    owner->duration = mode;
    owner->elapsed = 0;
    CameraNodeCallback callback = &CameraManager::fn_1_1A38;
    owner->callback = callback;
}

void CameraManager::fn_1_1A38()
{
    int mode = secondsToFrames<long>(3);
    double easedAmount =
        Square(1.0 -
               cos(M_PI * (double)elapsed / duration)) / 4.0;

    interpolate_camera_poses(&static_cast<CameraFrame &>(*this), &startPose, &targetPose,
              (float)easedAmount);
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), 1);
    if (elapsed >= duration) {
        startPose = targetPose;
        unsigned int bitIndex = activeBit;
        eventMask &= ~(1 << bitIndex);
        CameraNodeCallback callback = &CameraManager::fn_1_1948;
        this->callback = callback;
    }
}

void ManagerEventBase::advanceFrame() {}
