// Runs the Jump up! event and its result.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "REL/m677DLL/jump_up_event.h"
#include "REL/m677DLL/seer_terror_camera.h"
#include "REL/m677DLL/seer_terror_map.h"
#include "REL/m677DLL/player_work.h"
#include "datadir_enum.h"
#include <memory>

extern "C" {
#include "game/audio.h"
}

static char *jointObjectNames[] = { "spling_a", "spling_b", "spling_c", "spling_d" };

// Sets the spring model's render layer and initial attributes.
JumpUpEvent::JumpUpEvent()
    : state(&JumpUpEvent::startSelectedJointMotion), countdown(0),
        model(new OwnedModel(DATANUM(_m677 << 16, 0x19))),
      sound(-1)
{
    model->setLayer(4);
    model->setAttribute(1);
    model->setAttribute(pauseMotion);
}

// Releases the spring model.
JumpUpEvent::~JumpUpEvent()
{
}

// Adds the joint motion and loads the character motions.
void JumpUpEvent::begin()
{
    ModelMotion &joint = SeerTerrorMap::getJoint();
    joint.addMotion(DATANUM(_m677 << 16, 0x18));
    PlayerWork &character = PlayerWork::getChecked();
    static const unsigned int motions[] = { CHARMOT_HSF_c000m1_325, CHARMOT_HSF_c000m1_336,
                                            CHARMOT_HSF_c000m1_321 };
    createCharacterMotions(&static_cast<MotionOwner &>(character), motions, 3);
}

// Runs the active state and reports completion.
int JumpUpEvent::step()
{
    if (state) {
        (this->*state)();
    }
    return !state;
}

// Starts the joint motion for the selected rope.
void JumpUpEvent::startSelectedJointMotion()
{
    PlayerWork *character = &PlayerWork::getChecked();
    int selected = character->selectedRope();
    ModelMotion &joint = SeerTerrorMap::getJoint();
    joint.setMotion(DATANUM(_m677 << 16, 0x18));
    for (int index = 0; index < 4; ++index) {
        if (index != selected) {
            // Hold the unselected joints at their base Y position and Y scale.
            joint.setObjectAttribute(jointObjectNames[index], 0x200020);
        }
    }

    soundPlay(MSM_SE_MG677_JUMP_LAUNCH);
    sound = CharFXPlay(character->characterNo(), MSM_SE_CHARVOICE_HIT_YELP);
    Vec launchVelocity = { 0.0f, 0.0f, 0.0f };
    launchVelocity.y = framesToSeconds<int>(2000);
    character->offset = launchVelocity;
    character->rumble(1, 24);
    setState(&JumpUpEvent::reactAtHeight);
}

// Triggers the character reaction at the height threshold.
void JumpUpEvent::reactAtHeight()
{
    PlayerWork *character = &PlayerWork::getChecked();
    Vec position = character->getPosition();
    if (!(position.y < 1000.0f)) {
        fn_1_1CF40(character, false);
        character->rumble(2, 24);
        SeerTerrorCamera::getChecked().startShake(0.1f, 0.5f);
        soundPlay(MSM_SE_MG677_JUMP_HEIGHT_REACTION);
        soundStop(sound);
        position.y = 0.0f;
        model->setPosition(&position);
        setCountdown(secondsToFrames<double>(0.6));
        setState(&JumpUpEvent::revealModelAfterCountdown);
    }
}

// Reveals the model after the countdown.
void JumpUpEvent::revealModelAfterCountdown()
{
    if (::updateCountdown(countdown)) {
        model->resetAttribute(1);
        model->resetAttribute(pauseMotion);
        setState(&JumpUpEvent::waitForModelMotion);
    }
}

// Ends the callback when the model motion ends.
void JumpUpEvent::waitForModelMotion()
{
    if (model->isMotionEnd()) {
        setState(0);
    }
}
