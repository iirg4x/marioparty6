extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/debug.h"
#include "REL/m652DLL/time_conversion.h"
#include "REL/m652DLL/node_owner.h"
#include <vector>
#include "dolphin/types.h"
#include "REL/framework/game_object.h"
#include "REL/m652DLL/framework/camera_frame.h"
#include "REL/framework/event_packet.h"
#include "REL/m652DLL/camera_split.h"
#include "REL/m652DLL/camera_perspective.h"
#include "humath.h"

static inline int CameraFrames(int value) { return value * 60; }

extern "C" {
unsigned int lbl_1_bss_68 = fn_1_2178(5);
unsigned int lbl_1_bss_6C = CameraFrames(30);
float lbl_1_bss_70 = framesToSeconds(framesToSeconds(980.665));
}

extern "C" FlagNodeList lbl_1_bss_EC;

struct CameraSetup : CameraFrame {
    unsigned char flag7 : 1, flag6 : 1, flag5 : 1, unknown24 : 5;
    CameraSetup();
};

class PlayerCamera : public FlagNode, public CameraFrame,
                  public EventListener {
public:
    typedef void (PlayerCamera::*Action)();
    PlayerCamera(unsigned int index);
    virtual ~PlayerCamera();
    virtual void flagEnabled(int mask);
    virtual void update();
    virtual void receive(const EventPacket *event);
    void fn_1_4118(); void fn_1_3E2C(); void fn_1_4458(); void fn_1_414C();
    Action action;
    int count;
    unsigned short modelId;
    CameraPose position;
    int frame;
    CameraSetup state;
};
extern "C" const CameraPose lbl_1_rodata_1D0[3] = {
{{0.0f,925.32f,-3348.63f},{-0.13663656f,0.0f,-0.0f,0.99062127f}},
{{0.0f,1613.5106f,904.49774f},{-0.42033017f,0.0f,-0.0f,0.9073713f}},
{{0.0f,571.22656f,-4388.63f},{0.010465809f,0.0f,0.0f,0.9999453f}}
};
extern "C" {
extern void __msl_assertion_failed(const char *expression, const char *file,
                                  int line);
}

CameraSplit::CameraSplit() : GameObject(0), modified(1), positions(2, 0x280), screenOffset(0)
{
    unsigned short cameraMask;
    int cameraIndex;

    cameraMask = 0;
    for (cameraIndex = 0; cameraIndex < 2; cameraIndex++) {
        cameraMask |= 1u << cameraIndex;
    }
    Hu3DBGColorSet(0, 0, 0);
    Hu3DCameraCreate(cameraMask);
    update();
}

CameraSplit::~CameraSplit() {}

extern "C" {
void fn_1_345C(CameraSplit *owner, unsigned int index, int value);
void fn_1_34C0(CameraSplit *self, int value);
}

void fn_1_345C(CameraSplit *owner, unsigned int index, int value)
{
    if (index < owner->positions.size() - 1) {
        owner->positions[index] = value;
        owner->modified = 1;
    }
}

void fn_1_34C0(CameraSplit *self, int value)
{
    self->screenOffset = value / 2;
    self->modified = 1;
}

static inline int Maximum34E4(int first, int second)
{
    return first > second ? first : second;
}

static inline int Nonnegative34E4(int value)
{
    return value < 0 ? 0 : value;
}

static inline int ClipPosition34E4(int value)
{
    if (value < 16) return 16;
    if (value > 624) return 624;
    return value;
}

static inline double Aspect34E4(int width)
{
    return (double)0.6f > (double)0.0018750001f * (double)width
        ? (double)0.6f : (double)0.0018750001f * (double)width;
}

static inline int Edge34E4(int first, int last, int position)
{
    return (int)((1.0 - (double)position / 640.0) * (double)first +
                 ((double)position / 640.0) * (double)last);
}

static inline int ScissorWidth34E4(int width)
{
    return width < 1 ? 1 : width;
}

void CameraSplit::update()
{
    if (modified != 0) {
        int camera = 1;
        int previousPosition = 0;
        int index = 0;
        while (index < 2) {
            int targetPosition = positions[index];
            int positionDelta = targetPosition - previousPosition;
            int leftMargin;
            int rightMargin;
            int clip;
            int viewportStart;
            int viewportEnd;
            int viewportWidth;
            unsigned int scissorStart;
            int scissorEnd;
            int scissorWidth;
            if (positionDelta <= 0) {
                Hu3DCameraKill(camera);
            } else {
                leftMargin = Nonnegative34E4(16 - previousPosition);
                rightMargin = Nonnegative34E4(targetPosition - 624);
                clip = Maximum34E4(leftMargin, rightMargin);
                viewportStart = ClipPosition34E4(previousPosition) - clip;
                viewportEnd = clip + ClipPosition34E4(targetPosition);
                viewportWidth = Nonnegative34E4(viewportEnd - viewportStart);

                scissorStart = (unsigned int)screenOffset +
                    (unsigned int)Edge34E4(-screenOffset,
                                           screenOffset + 640, previousPosition);
                scissorEnd = Edge34E4(-screenOffset,
                                      screenOffset + 640, targetPosition) - screenOffset;
                scissorWidth = ScissorWidth34E4(scissorEnd - (int)scissorStart);

                Hu3DCameraViewportSet(camera, (float)viewportStart, 0.0f,
                                      (float)viewportWidth, 480.0f, 0.0f, 1.0f);
                Hu3DCameraScissorSet(camera, scissorStart, 0, scissorWidth,
                                     0x1E0);
                Hu3DCameraPerspectiveSet(camera, 45.0f, 100.0f, 10000.0f,
                                         (float)Aspect34E4(viewportWidth));
                previousPosition = targetPosition;
            }
            index++;
            camera *= 2;
        }
        modified = 0;
    }
}

void set_camera_frame_from_pose(CameraFrame *, const CameraPose *);
void get_camera_pose_from_frame(const CameraFrame *, CameraPose *);
void interpolate_camera_poses(CameraFrame *, const CameraPose *, const CameraPose *, float);

PlayerCamera::PlayerCamera(unsigned int index)
    : action(&PlayerCamera::fn_1_4118), count(index), modelId((unsigned short)(1 << index)), frame(0)
{
    set_camera_frame_from_pose(&static_cast<CameraFrame &>(*this), lbl_1_rodata_1D0);
    registerChannel(count + 1);
    set_camera_frame_from_pose(&state, &lbl_1_rodata_1D0[1]);
}

PlayerCamera::~PlayerCamera() {}

void PlayerCamera::flagEnabled(int mask) {
    switch (mask) {
    case 0: {
        PlayerCamera::Action temporary;
        frame = 0;
        temporary = &PlayerCamera::fn_1_3E2C;
        action = temporary;
        break;
    }
    default: {
        flags &= ~(1U << mask);
        break;
    }
    }
}

void PlayerCamera::receive(const EventPacket *event)
{
    if (event->type == 2) {
        PlayerCamera::Action temporary;

        frame = 0;
        get_camera_pose_from_frame(&static_cast<CameraFrame &>(*this), &position);
        temporary = &PlayerCamera::fn_1_4458;
        action = temporary;
    }
}

/* Dispatches the stored callback, increments its count, and updates its state. */
extern "C" void __msl_assertion_failed(const char *, const char *, int);
struct CountedCallback {
    u32 callbackObjectState[14];
    void (CountedCallback::*callback)();
    u32 callbackValue;
    u16 callbackFlags;
    u8 callbackData[0x1E];
    int count;
    u8 state[1];
};

void PlayerCamera::update()
{
 (this->*action)();
 frame++;
 debugResetHook(&state, count, modelId);
}

extern "C" {

void fn_1_41B4(PlayerCamera *object, CameraPose *pose);
double cos(double angle);
extern "C++" void cameraApplyToHud(CameraFrame *camera, s32 modelId);
void __msl_assertion_failed(const char *expression, const char *file,
                            int line);
}

template<class T> static inline T Square3E2C(const T &value)
{
    return value * value;
}

template<class T> static inline T Interpolate3E2C(T first, T last, double amount)
{
    return (1.0 - amount) * first + amount * last;
}

static inline bool CanWriteCameraValue3E2C(CameraSplit *owner,
                                           unsigned int index)
{
    return index < owner->positions.size() - 1;
}

static inline void WriteCameraValue3E2C(CameraSplit *owner,
                                        unsigned int index, int value)
{
    owner->positions[index] = value;
    owner->modified = 1;
}

static inline void SetPosition3E2C(CameraSplit *owner,
                                   unsigned int index, int value)
{
    if (CanWriteCameraValue3E2C(owner, index)) {
        WriteCameraValue3E2C(owner, index, value);
    }
}

void PlayerCamera::fn_1_3E2C()
{
    CameraSplit *current;
    int frameCount;
    int step;
    const CameraPose *defaultPose;
    double amount;
    double fraction;
    CameraPose cameraPose;

    frameCount = secondsToFrames<long>(5);
    amount = Square3E2C(1.0 -
        cos((M_PI * (double)this->frame) / (double)frameCount)) / 4.0;
    if (&CameraSplit::getUnchecked() != 0 && amount > 0.2) {
        current = &CameraSplit::getChecked();
        step = (((int)this->count + 1) * 640) / 2;
        fraction = (amount - 0.2) / 0.8;
        SetPosition3E2C(current, this->count, Interpolate3E2C(640, step, fraction));
        current->screenOffset = 2;
        current->modified = 1;
    }

    defaultPose = lbl_1_rodata_1D0;
    fn_1_41B4(this, &cameraPose);
    interpolate_camera_poses(&static_cast<CameraFrame &>(*this),
                             defaultPose, &cameraPose, (float)amount);
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), this->modelId);
    if (this->frame >= frameCount) {
        this->flags &= ~1u;
        PlayerCamera::Action next = &PlayerCamera::fn_1_414C;
        this->action = next;
    }
}

void PlayerCamera::fn_1_4118()
{
 cameraApplyToHud(&static_cast<CameraFrame &>(*this), modelId);
}
void PlayerCamera::fn_1_414C()
{
 CameraPose parameterState;
 CameraPose motionState;
 get_camera_pose_from_frame(&static_cast<CameraFrame &>(*this), &parameterState);
 fn_1_41B4(this, &motionState);
 interpolate_camera_poses(&static_cast<CameraFrame &>(*this),
                             &parameterState, &motionState, 0.1f);
 cameraApplyToHud(&static_cast<CameraFrame &>(*this), modelId);
}
/* The camera module's sphere; other files see it through its registered values. */
struct Sphere;

extern "C" void fn_1_DAD8(std::vector<Sphere *> *output, long modelId);

float fitCameraToSpheres(CameraFrame *camera, CameraPerspective *value, std::vector<Sphere *> *setup);

static inline float Maximum41B4(float value, float minimum)
{
    return value > minimum ? value : minimum;
}

void fn_1_41B4(PlayerCamera *object, CameraPose *pose)
{
    std::vector<Sphere *> setup;
    CameraFrame camera;

    setup.clear();
    fn_1_DAD8(&setup, object->count);
    if (setup.size() == 0) {
        get_camera_pose_from_frame(&static_cast<CameraFrame &>(*object), pose);
        return;
    }

    CameraPerspective calculation;
    calculation.read(object->modelId);
    calculation.width /= 2;
    set_camera_frame_from_pose(&camera, &lbl_1_rodata_1D0[1]);
    fitCameraToSpheres(&camera, &calculation, &setup);
    camera.position.y = Maximum41B4(camera.position.y, 1300.0f);
    get_camera_pose_from_frame(&camera, pose);
}

template<class T> static inline T Square4458(const T &value)
{
    return value * value;
}

template<class T> static inline T Interpolate4458(T first, T last, double amount)
{
    return (1.0 - amount) * first + amount * last;
}

void PlayerCamera::fn_1_4458()
{
    CameraSplit *current;
    int frameCount;
    int index;
    double amount;
    CameraPose *cameraTarget;
    const CameraPose *cameraDefault;

    frameCount = secondsToFrames<long>(3);
    amount = Square4458(1.0 -
        cos((M_PI * (double)this->frame) / (double)frameCount)) / 4.0;
    cameraTarget = &position;
    cameraDefault = &lbl_1_rodata_1D0[2];
    interpolate_camera_poses(&static_cast<CameraFrame &>(*this),
                             cameraTarget, cameraDefault, (float)amount);
    cameraApplyToHud(&static_cast<CameraFrame &>(*this), this->modelId);

    if (&CameraSplit::getUnchecked() != 0) {
        current = &CameraSplit::getChecked();
        index = 0;
        for (; index < 1; index++) {
            int step = ((index + 1) * 0x280) / 2;
            int end = (index >= this->count) ? 0x280 : 0;
            SetPosition3E2C(current, index, Interpolate4458(step, end, amount));
        }
    }

    if (this->frame >= frameCount) {
        PlayerCamera::Action next = &PlayerCamera::fn_1_4118;
        this->action = next;
    }
}
