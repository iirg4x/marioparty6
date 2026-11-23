// Rotates two joints of the primary scene model relative to the camera.

#include "msm_se.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "REL/m677DLL/camera_facing_scene_models.h"
#include "datadir_enum.h"

extern "C" {

#include "game/hu3d.h"
#include "game/hsfex.h"
}

// Model joints rotated by the rope scene.
static char *objectNames[] = { "seft_a", "seft1_b" };

// Loads the scene models and finds the joints that move.
CameraFacingSceneModels::CameraFacingSceneModels()
    : OwnedModel(DATANUM(_m677 << 16, 0xD)), secondaryModel(DATANUM(_m677 << 16, 0xE))
{
    seftATransform = &getObject(objectNames[0])->mesh.base;
    seft1BTransform = &getObject(objectNames[1])->mesh.base;
    angle = 0.0f;
    setLayer(4);
    secondaryModel.setLayer(5);
    // Queue the secondary model for drawing with depth testing disabled.
    secondaryModel.setAttribute(0x400000);
    secondaryModel.setAttribute(pauseMotion);
    registerChannel(0);
}

// Releases the two scene models.
CameraFacingSceneModels::~CameraFacingSceneModels() {}

// Applies an angle to a model joint using the supplied transform.
static void applyViewRotation(HSF_TRANSFORM *target, Mtx source, float angle)
{
    Mtx rotation;
    Vec angles;
    MTXRotDeg(rotation, 'z', angle);
    PSMTXConcat(source, rotation, rotation);
    Hu3DMtxRotGet(rotation, &angles);
    target->rot = angles;
}

// Advances the angle and updates both scene joints.
void CameraFacingSceneModels::updateJointRotations()
{
    angle -= framesToSeconds(60.0);
    Vec eye;
    Vec target;
    Vec up;
    fn_80027CD0(1, &eye, &up, &target);
    getObjectPosition(objectNames[0], &target);
    Mtx view;
    C_MTXLookAt(view, &eye, &up, &target);
    PSMTXTranspose(view, view);
    applyViewRotation(seftATransform, view, 2.0f * angle);
    applyViewRotation(seft1BTransform, view, angle);
    if (secondaryModel.isMotionEnd()) {
        secondaryModel.setAttribute(1);
    }
}

// On event 6, resumes the secondary model's motion and plays its cue.
void CameraFacingSceneModels::receive(const EventPacket *packet)
{
    if (packet->type == 6) {
        secondaryModel.resetAttribute(pauseMotion);
        soundPlay(MSM_SE_MG677_SCENE_MODEL_MOTION_START);
    }
}
