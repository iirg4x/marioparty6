// Runs the Pitfall event and its result.

#include "msm_se.h"
#include "REL/m677DLL/motion_attributes.h"
#include "datanum/charmot.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "REL/m677DLL/pitfall_event.h"
#include "REL/m677DLL/seer_terror_map.h"
#include "REL/m677DLL/player_work.h"
#include "datadir_enum.h"

static char *jointObjectNames[] = { "spling_a", "spling_b", "spling_c", "spling_d" };
float pitfallFloorHeight = cmToWorld(-1000.0f);

// Starts the callback in its initial state.
PitfallEvent::PitfallEvent() : state(&PitfallEvent::startReaction), countdown(0)
{
}

// Releases the callback's owned state.
PitfallEvent::~PitfallEvent()
{
}

// Adds the joint motion and loads the character motions.
void PitfallEvent::begin()
{
    ModelMotion &joint = SeerTerrorMap::getJoint();
    joint.addMotion(DATANUM(_m677 << 16, 0x1B));
    PlayerWork &character = PlayerWork::getChecked();
    static const unsigned int motions[] = { CHARMOT_HSF_c000m1_325, CHARMOT_HSF_c000m1_344 };
    createCharacterMotions(&static_cast<MotionOwner &>(character), motions, 2);
}

// Runs the active state and reports completion.
int PitfallEvent::step()
{
    if (state) {
        (this->*state)();
    }
    return !state;
}

// Starts the character's reaction motion and rumble.
void PitfallEvent::startReaction()
{
    PlayerWork *character = &PlayerWork::getChecked();
    shiftCharacterMotion(&static_cast<MotionOwner &>(*character), CHARMOT_HSF_c000m1_325, 8.0f,
                         0.0f, 0);
    character->rumble(1, 24);
    character->resetShadow();
    int selected = character->selectedRope();

    ModelMotion &joint = SeerTerrorMap::getJoint();
    joint.setMotion(DATANUM(_m677 << 16, 0x1B));
    for (int index = 0; index < 4; ++index) {
        if (index != selected) {
            joint.setObjectAttribute(jointObjectNames[index], 0x20);
        }
    }
    setCountdown(secondsToFrames<double>(0.5));
    setState(&PitfallEvent::waitForReaction);
}

// Waits for the reaction and its countdown before changing the motion.
void PitfallEvent::waitForReaction()
{
    PlayerWork &character = PlayerWork::getChecked();
    if (characterMotionEnded(&character.charNo)) {
        if (::updateCountdown(countdown)) {
            setCountdown(secondsToFrames<double>(0.5));
            shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_344,
                                 secondsToFrames<double>(0.2), 0.0f, loopMotion);
            setState(&PitfallEvent::lowerFloor);
        }
    }
}

// Applies the height threshold and plays the sound after the countdown.
void PitfallEvent::lowerFloor()
{
    if (::updateCountdown(countdown)) {
        playerFloorHeight = pitfallFloorHeight;
        soundPlay(MSM_SE_MG677_PITFALL_OPEN);
        setState(&PitfallEvent::waitForFall);
    }
}

// Completes the callback when the character reaches the height threshold.
void PitfallEvent::waitForFall()
{
    PlayerWork *character = &PlayerWork::getChecked();
    if (!(character->getPositionPtr()->y > pitfallFloorHeight)) {
        setState(0);
    }
}
