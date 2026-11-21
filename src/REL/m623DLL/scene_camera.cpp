// The two player views and their shared camera transitions.

extern "C" {
#include "dolphin/mtx.h"
#include <math.h>
#include "game/hu3d.h"
#include "game/memory.h"
#include "dolphin/types.h"
#include "humath.h"
}

#include "REL/m623DLL/scene_camera_api.h"
#include "REL/framework/category_sphere.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/scene_shadow.h"
#include "REL/framework/model.h"
#include "REL/framework/debug.h"
#include "REL/framework/camera_frame.h"
#include "datadir_enum.h"
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"
#include <vector>
#include "REL/m623DLL/scene_camera_owner.h"
#include "REL/framework/range.h"

// Divides the screen between the two Hu3D cameras at each stored boundary.
class SplitView : public GameObject, public MinigameSingleton<SplitView> {
  public:
    SplitView();
    virtual ~SplitView();
    virtual void update();
    void setBoundary(unsigned long boundary, int position);
    void setPerspectiveShift(int shift);

    // Set when the camera viewports need to be refreshed.
    unsigned char changed : 1;
    // Right edge of each view, in screen pixels.
    std::vector<int> boundaries;
    // Horizontal shift used to calculate the view scissor.
    int perspectiveShift;
};

extern SplitView *lbl_1_bss_67C;
extern HU3D_MODELID lbl_1_bss_680;

f32 fn_1_CAD8(s32 index, s32 count, f32 spacing);

// Initializes two full-width split-view boundaries.
SplitView::SplitView() : GameObject(0x7FFF), changed(1), boundaries(2, 640), perspectiveShift(0)
{
    unsigned short cameraMask = 0;
    for (int cameraIndex = 0; cameraIndex < 2; cameraIndex++) {
        cameraMask |= 1 << cameraIndex;
    }
    Hu3DBGColorSet(0, 0, 0);
    Hu3DCameraCreate(cameraMask);
    update();
}

// Releases the split-view boundary vector.
SplitView::~SplitView()
{
}

// Updates one screen boundary and marks the view dirty.
void SplitView::setBoundary(unsigned long boundary, int position)
{
    if (boundary < boundaries.size() - 1) {
        boundaries[boundary] = position;
        changed = 1;
    }
}

// Updates the perspective shift and marks the view dirty.
void SplitView::setPerspectiveShift(int shift)
{
    perspectiveShift = shift / 2;
    changed = 1;
}

// Clamps the split boundary to the visible horizontal interval.
static inline s32 clampViewEdge(s32 value)
{
    // Leaves at least sixteen pixels for each view.
    if (value < 16) {
        return 16;
    }
    if (value > 624) {
        return 624;
    }
    return value;
}

// Interpolates the two values across the screen width.
static inline s32 interpolate(s32 position, s32 first, s32 last)
{
    return (s32) ((1.0 - ((f64) position / 640.0)) * (f64) first +
                  ((f64) position / 640.0) * (f64) last);
}

// Applies changed split boundaries to the two Hu3D cameras.
void SplitView::update()
{
    u32 boundaryCount = 0;
    while (boundaryCount < boundaries.size()) {
        boundaryCount++;
    }
    if (changed != 0) {
        s32 cameraMask = 1;
        s32 current = 0;
        s32 index = 0;
        while (index < 2) {
            s32 end = boundaries[index];
            s32 span = end - current;
            if (span <= 0) {
                Hu3DCameraKill(cameraMask);
            } else {
                s32 leftInset = rangeMaximum<s32>(0, 16 - current);
                s32 rightInset = rangeMaximum<s32>(0, end - 624);
                s32 inset = rangeMaximum<s32>(leftInset, rightInset);
                s32 left = clampViewEdge(current) - inset;
                s32 right = inset + clampViewEdge(end);
                s32 width = rangeMaximum<s32>(0, right - left);
                s32 viewLeft = perspectiveShift +
                               interpolate(current, -perspectiveShift, perspectiveShift + 640);
                s32 scissor =
                    interpolate(end, -perspectiveShift, perspectiveShift + 640) - perspectiveShift;
                u32 scissorWidth = rangeMaximum<s32>(1, scissor - viewLeft);
                Hu3DCameraViewportSet(cameraMask, (f32) left, 0.0f, (f32) width,
                                      480.0f, 0.0f, 1.0f);
                Hu3DCameraScissorSet(cameraMask, viewLeft, 0U, scissorWidth, 480U);
                Hu3DCameraPerspectiveSet(cameraMask, 55.0f, 100.0f, 100000.0f,
                                         (f32) ((1.2f / 640.0f) * (f64) width));
                current = end;
            }
            index++;
            cameraMask *= 2;
        }
        changed = 0;
    }
}

// Fills the camera frame with the initial view.
void fn_1_B958(CameraFrame *frame)
{
    frame->position.x = 0.0f;
    frame->position.y = 0.0f;
    frame->position.z = 0.0f;
    frame->target.x = 1.0f;
    // The initial view looks forward and down.
    frame->target.y = -0.8f;
    frame->target.z = 0.0f;
    frame->up.x = 0.0f;
    frame->up.y = 1.0f;
    frame->up.z = 0.0f;
}

// Converts the input to the camera's interpolation curve.
f32 fn_1_B9EC(f32 value)
{
    return (3.0f * (value * value)) - (2.0f * (value * (value * value)));
}

// Creates the split view and initializes the camera state and timers.
SceneCamera::SceneCamera() : GameObject(0x8000), introTimer(0.0f), blendTimer(0.0f)
{
    fn_1_B958(&view.frame);
    fn_1_B958(&cameraOne);
    fn_1_B958(&cameraTwo);

    SplitView *split = new SplitView;
    split->setBoundary(0, 320);
    split->setBoundary(1, 640);
    // Applies the initial perspective shift between the two views.
    split->setPerspectiveShift(4);
    lbl_1_bss_67C = split;

    Hu3DCameraCreate((1 << 0) | (1 << 1));
    blendTimer.set(0.0f, -1);
    HU3D_MOTIONID introMotion = fn_8002C4C0(fn_80008DD0(DATANUM(0x520000, 32), 0x10000000, 2));
    HU3D_MODELID introModel = fn_800283F4((s16) introMotion, 1);
    lbl_1_bss_680 = introModel;
    fn_800284C8(introModel, 1);
    fn_80026B8C(introModel, 0);
    setState(&SceneCamera::fn_1_C2E0);
}

// Releases the split view owned by the scene camera.
SceneCamera::~SceneCamera()
{
    delete SplitView::current();
}

// Advances the current camera callback and writes its view frame.
void SceneCamera::update()
{
    debugResetHook(&view.frame, 0, 1);
    if (view.highFlag != 0) {
        cameraApplyToHud(&view.frame, 2);
    } else {
        (this->*state)();
    }
}

// Starts the 1.5-second blend and snapshots the second camera frame.
void fn_1_C0C8(void)
{
    SceneCamera *self = &SceneCamera::getChecked();
    self->blendTimer.set(0.0f, 1.0f, (s32) secondsToFrames(1.5f));
    self->blendStart = self->cameraTwo;
}

// Selects the idle camera state and restores the first view's full width.
void fn_1_C1C4(Model *model)
{
    model->isInView(1);
    SceneCamera &self = SceneCamera::getChecked();
    self.setState(&SceneCamera::fn_1_C898);
    lbl_1_bss_67C->setBoundary(0, 640);
    lbl_1_bss_67C->setPerspectiveShift(0);
}

// Copies camera zero to camera one and waits for the introduction motion to finish.
void SceneCamera::fn_1_C2E0()
{
    HU3D_CAMERA *source = &Hu3DCamera[0];
    HU3D_CAMERA *destination = &Hu3DCamera[1];

    destination->fov = source->fov;
    destination->near = source->near;
    destination->far = source->far;
    destination->aspect = source->aspect;
    destination->upRot = source->upRot;
    destination->pos.x = source->pos.x;
    destination->pos.y = source->pos.y;
    destination->pos.z = source->pos.z;
    destination->up.x = source->up.x;
    destination->up.y = source->up.y;
    destination->up.z = source->up.z;
    destination->target.x = source->target.x;
    destination->target.y = source->target.y;
    destination->target.z = source->target.z;
    cameraReadFromHud(&introView, 1);
    if (fn_8002D75C(lbl_1_bss_680) != 0) {
        fn_8002858C(lbl_1_bss_680);
        setState(&SceneCamera::fn_1_C444);
        introTimer.set(0.0f, 1.0f, (s32) secondsToFrames(2.5f));
    }
}

// Unused camera position and orientation quaternion.
const f32 lbl_1_rodata_50C[7] = {
    42332.59f, 421.77472f, 3422.0432f, -0.113745674f, -0.60897046f, -0.088809945f, 0.77995497f,
};

// Blends from the introduction view to the two scene frames.
void SceneCamera::fn_1_C444()
{
    CameraPerspective perspective;

    perspective.width /= 2;
    std::vector<Sphere *> soloSpheres;
    std::vector<Sphere *> teamSpheres;
    collectCategoryValues(&soloSpheres, 0);
    collectCategoryValues(&teamSpheres, 1);
    perspective.read(1);
    fn_1_B958(&cameraOne);
    fitCameraToSpheres(&cameraOne, &perspective, &soloSpheres);
    cameraApplyToHud(&cameraOne, 1);
    copyDebugCameraFrame(&view.frame, &cameraOne);
    if (0 != teamSpheres.size()) {
        perspective.read(2);
        fn_1_B958(&cameraTwo);
        fitCameraToSpheres(&cameraTwo, &perspective, &teamSpheres);
        if (!blendTimer.isFinished()) {
            interpolate_camera_frames(&cameraTwo, &blendStart, &cameraTwo, blendTimer.update());
        }
        cameraApplyToHud(&cameraTwo, 2);
    }
    if (!introTimer.isFinished()) {
        f32 progress = introTimer.update();
        progress = fn_1_B9EC(progress);
        interpolate_camera_frames(&cameraOne, &introView, &cameraOne, progress);
        interpolate_camera_frames(&cameraTwo, &introView, &cameraTwo, progress);
        cameraApplyToHud(&cameraOne, 1);
        cameraApplyToHud(&cameraTwo, 2);
    }
}

// Leaves the completed camera state unchanged.
void SceneCamera::fn_1_C898()
{
}

// Places and turns the player model using the axes of camera 1.
void fn_1_C89C(s32 player, Model &model)
{
    CameraFrame view;
    Vec rightAxis;
    Vec up;
    Vec lookAxis;
    Vec position;
    Vec rightOffset;
    f32 heightDifference;
    f32 normalHeight;
    f32 groundY = 0.0f;
    f32 offset;
    f32 lookDistance;
    f32 side;

    cameraReadFromHud(&view, 1);
    cameraExtractBasis(&view, &rightAxis, &up, &lookAxis);
    heightDifference = view.position.y - groundY;
    normalHeight = (view.position.y - lookAxis.y) - groundY;
    PSVECScale(&lookAxis, &lookAxis,
               -((groundY - heightDifference) / (normalHeight - heightDifference)));
    PSVECAdd(&view.position, &lookAxis, &position);
    // Spaces the four placements along the camera-right axis.
    offset = fn_1_CAD8(player, 4, 230.0f);
    PSVECScale(&rightAxis, &rightOffset, offset);
    PSVECAdd(&rightOffset, &position, &position);
    Vec horizontalLook = lookAxis;
    horizontalLook.y = 0.0f;
    PSVECNormalize(&horizontalLook, &horizontalLook);
    // Draws the placed model back along the horizontal look direction.
    lookDistance = -500.0f;
    PSVECScale(&horizontalLook, &horizontalLook, lookDistance);
    PSVECAdd(&horizontalLook, &position, &position);
    // Shifts the solo and team models in opposite lateral directions.
    side = 50.0f;
    Vec lateralOffset;
    PSVECScale(&rightAxis, &lateralOffset, player == 0 ? -side : side);
    PSVECAdd(&lateralOffset, &position, &position);
    model.setPosition(&position);
    model.setRotationY((f32) ((atan2(-(f64) lookAxis.x, -(f64) lookAxis.z) / M_PI) * 180.0));
}

// Centers an indexed position using the supplied spacing.
f32 fn_1_CAD8(s32 index, s32 count, f32 spacing)
{
    --count;
    return spacing * index - spacing * count / 2.0f;
}

// The active split-view renderer.
SplitView *lbl_1_bss_67C;

// The intro motion model.
HU3D_MODELID lbl_1_bss_680;
