// Runs the Stage rotation event and its result.

#include "msm_se.h"
#include "REL/m677DLL/motion_attributes.h"
#include "datanum/charmot.h"
#include "REL/framework/range.h"
#include <math.h>
#include "REL/framework/interpolation.h"
#include "REL/framework/sound.h"
#include "REL/m677DLL/stage_rotation_event.h"
#include "REL/m677DLL/seer_terror_map.h"
#include "REL/m677DLL/player_work.h"
#include "datadir_enum.h"
extern "C" {
#include "game/hsfformat.h"
}

// Character motions loaded for the stage rotation.
static const unsigned int motions[] = {
    CHARMOT_HSF_c000m1_325, CHARMOT_HSF_c000m1_336, CHARMOT_HSF_c000m1_321
};

// Joint names in rope selection order.
static char *jointObjectNames[] = { "futa_a", "futa_b", "futa_c", "futa_d" };

// Starts at the first character-motion state.
StageRotationEvent::StageRotationEvent()
    : state(&StageRotationEvent::startCharacterMotion), previousAngle(0.0f)
{
}

// Destroys the cover callback.
StageRotationEvent::~StageRotationEvent()
{
}

// Loads the cover joint motion and character motions.
void StageRotationEvent::begin()
{
    ModelMotion &joint = SeerTerrorMap::getJoint();
    joint.addMotion(DATANUM(_m677 << 16, 0x15));
    PlayerWork &character = PlayerWork::getChecked();
    createCharacterMotions(&static_cast<MotionOwner &>(character), motions, 3);
}

// Runs the current state and reports completion.
int StageRotationEvent::step()
{
    if (state) {
        (this->*state)();
    }
    return !state;
}

// Starts the first character motion and advances the state.
void StageRotationEvent::startCharacterMotion()
{
    startEventCharacterMotion();
    setState(&StageRotationEvent::startJointAfterCharacterMotion);
}

// Starts the joint motion and sound once the character motion ends.
void StageRotationEvent::startJointAfterCharacterMotion()
{
    PlayerWork &character = PlayerWork::getChecked();
    if (characterMotionEnded(&character.charNo)) {
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_336, 8.0f,
                             0.0f, loopMotion);
        int selected = character.selectedRope();
        ModelMotion &joint = SeerTerrorMap::getJoint();
        joint.setMotion(DATANUM(_m677 << 16, 0x15));
        for (int index = 0; index < 4; ++index) {
            if (index != selected) {
                joint.setObjectAttribute(jointObjectNames[index], 0x100);
            }
        }
        sound = soundPlay(MSM_SE_MG677_STAGE_ROTATION);
        setState(&StageRotationEvent::trackJointAngle);
    }
}

// Tracks the selected joint's angle and ends the sequence when its motion ends.
void StageRotationEvent::trackJointAngle()
{
    PlayerWork &character = PlayerWork::getChecked();
    int selected = character.selectedRope();
    ModelMotion &joint = SeerTerrorMap::getJoint();
    float &angle = joint.getObject(jointObjectNames[selected])->mesh.curr.rot.y;
    fn_1_1CEA8(&character, 180.0f + angle, true);
    float difference = fabsf(angle - previousAngle);
    const short pitch = interpolateValue<int>(0, 8191, rangeMinimum(difference / 44.0f, 1.0f));
    soundSetPitch(sound, pitch);
    previousAngle = angle;
    if (joint.isMotionEnd()) {
        soundStop(sound);
        shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_321, 16.0f,
                             0.0f, loopMotion);
        setState(0);
    }
}
