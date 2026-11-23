// Defines the model that marks the selected rope.

#ifndef REL_SEER_TERROR_ICON_RENDERER_H
#define REL_SEER_TERROR_ICON_RENDERER_H

#include "REL/framework/model.h"
#include "REL/framework/game_object.h"
#include "REL/framework/frame_interpolator.h"

// Positions the selection icon using the camera transform.
class IconRenderer : public GameObject, public OwnedModel {
  public:
    Vec position;
    // Grows the icon toward scale 0.5 or shrinks it toward zero while moving it laterally.
    FrameInterpolator<float> transition;
    IconRenderer();
    virtual ~IconRenderer();
    void setSlot(int position);
    void beginGrow();
    void beginShrink();
    virtual void update();
    void updatePosition(Mtx matrix);
    void faceCamera(Mtx matrix);
};

#endif
