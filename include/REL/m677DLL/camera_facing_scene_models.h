// Rotates two joints of the primary scene model relative to the camera.

#ifndef REL_SEER_TERROR_CAMERA_FACING_SCENE_MODELS_H
#define REL_SEER_TERROR_CAMERA_FACING_SCENE_MODELS_H

#include "REL/framework/event_packet.h"
#include "REL/framework/model.h"

extern "C" {
#include "game/hsfformat.h"
}

// Draws the scene model and updates its two camera-facing objects.
class CameraFacingSceneModels : public OwnedModel, public EventListener {
  public:
    CameraFacingSceneModels();
    virtual ~CameraFacingSceneModels();
    virtual void receive(const EventPacket *packet);
    void updateJointRotations();

  private:
    OwnedModel secondaryModel;
    // Joint transforms rotated relative to the current camera.
    HSF_TRANSFORM *seftATransform;
    HSF_TRANSFORM *seft1BTransform;
    float angle;
};

#endif
