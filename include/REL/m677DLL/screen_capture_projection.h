// Captures the framebuffer in an RGB565 texture and draws the transition geometry.
// A separate 4x4 Z24X8 texture filled with 0xFF supplies depth for the fan.

#ifndef REL_SEER_TERROR_SCREEN_CAPTURE_PROJECTION_H
#define REL_SEER_TERROR_SCREEN_CAPTURE_PROJECTION_H

#include "REL/framework/frame_interpolator.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"

#include <memory>

struct CaptureTextureTarget;

// Captures and draws the screen image used by the transition.
struct ScreenCaptureProjection : GameObject {
    ScreenCaptureProjection(Vec *, f32, f32);
    virtual ~ScreenCaptureProjection();
    virtual void update();
    void setProjectionScaleTarget(f32 value, f32 seconds);
    void captureFramebuffer();
    void loadProjectionModelView(Mtx source, Mtx output);
    void drawDepthTextureFan();
    void drawFramebufferTextureRing(Mtx view);
    // Host world position used to place the projection model.
    Vec effectPosition;
    FrameInterpolator<f32> projection;
    std::auto_ptr<CaptureTextureTarget> targetSmall;
    std::auto_ptr<CaptureTextureTarget> targetLarge;
    f32 perspective;
    OwnedModel smallModel;
    OwnedModel largeModel;
};

#endif
