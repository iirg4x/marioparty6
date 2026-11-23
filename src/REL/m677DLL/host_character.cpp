// Owns the host model, its motions, and its voices.

#include "msm_se.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/m677DLL/host_character.h"
#include "REL/m677DLL/event_record.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "datadir_enum.h"

namespace {
// Motions loaded for the board model.
const unsigned long motionData[] = {
    DATANUM(_m677 << 16, 0x4), DATANUM(_m677 << 16, 0x5), DATANUM(_m677 << 16, 0x6),
    DATANUM(_m677 << 16, 0x7), DATANUM(_m677 << 16, 0x8), DATANUM(_m677 << 16, 0x9),
    DATANUM(_m677 << 16, 0xA), DATANUM(_m677 << 16, 0xB), DATANUM(_m677 << 16, 0xC)
};
}

// Sets up the board model, its motions, and its material colors.
HostCharacter::HostCharacter(Vec *position)
    : ModelMotion(DATANUM(_m677 << 16, 0x3)), materialColors(new MaterialColors(this))
{
    setPosition(position);
    setLayer(2);
    setShadow();
    addMotions(motionData, sizeof(motionData) / sizeof(motionData[0]));
    setMotionAttribute(DATANUM(_m677 << 16, 0x6), 1);
    setMotion(DATANUM(_m677 << 16, 0x4));
    setAttribute(loopMotion);
    setUniformScale(1.2f);
    materialColors->addTextureFilter("g000m001");
}

// Releases the board model and its material color state.
HostCharacter::~HostCharacter()
{
}

// Starts the looping model motion and optionally plays a sound.
void HostCharacter::fn_1_188BC(int sound)
{
    shiftMotion(DATANUM(_m677 << 16, 0x7), 16.0f, 0.0f, loopMotion);
    if (sound != -1) soundPlay(sound);
}

// Blends to the model's other looping motion.
void HostCharacter::fn_1_18928()
{
    shiftMotion(DATANUM(_m677 << 16, 0x4), 16.0f, 0.0f, loopMotion);
}

// Starts model motion 6 and plays its sound.
void HostCharacter::fn_1_18978()
{
    shiftMotion(DATANUM(_m677 << 16, 0x6), 8.0f, 0.0f, 0);
    soundPlay(MSM_SE_GUIDE_KOOPA_ANNOUNCE);
}

// Starts model motion 11 and plays its sound.
void HostCharacter::fn_1_189CC()
{
    shiftMotion(DATANUM(_m677 << 16, 0xB), 8.0f, 0.0f, 0);
    soundPlay(MSM_SE_GUIDE_KOOPA_QUIZ_CHEER);
}

// Starts model motion 9.
void HostCharacter::fn_1_18A20()
{
    shiftMotion(DATANUM(_m677 << 16, 0x9), 8.0f, 0.0f, 0);
}

// Spawns particles in front of the model's jaw joint.
void HostCharacter::spawnJawParticles()
{
    Vec position;
    getObjectPosition("ns_ago", &position);
    position.z = position.z + cmToWorld(50.0);
    ParticleAnimations *animations = &ParticleAnimations::getChecked();
    animations->createGrayImpactParticles(&position, 6, 0.3f, 3.0f, 1);
}

// Plays the motion and optional cue before the result message.
void HostCharacter::playBeforeResultMessageMotion()
{
    const EventRecord *record = getSelectedEventRecord();
    unsigned int motion = record->beforeResultMessageMotion;
    int sound = record->beforeResultMessageMotionSound;
    if (motion != DATANUM(_m677 << 16, 0xB)) {
        shiftMotion(motion, 16.0f, 0.0f, 0);
    }
    if (sound != -1) soundPlay(sound);
}

// Returns to the looping motion and plays the event's result sound.
void HostCharacter::playBetweenMotionSound()
{
    const EventRecord *record = getSelectedEventRecord();
    int sound = record->betweenMotionSound;
    shiftMotion(DATANUM(_m677 << 16, 0x7), 16.0f, 0.0f, loopMotion);
    if (sound != -1) soundPlay(sound);
}

// Plays the motion at its recorded start frame and the optional cue after the result message.
void HostCharacter::playAfterResultMessageMotion()
{
    const EventRecord *record = getSelectedEventRecord();
    int motion = record->afterResultMessageMotion;
    int sound = record->afterResultMessageMotionSound;
    shiftMotion(motion, 16.0f, (float)record->afterResultMessageMotionStart, 0);
    if (sound != -1) soundPlay(sound);
}

// Applies the blend to the host model's material colors.
void HostCharacter::setMaterialColorBlend(float value)
{
    materialColors->applyMaterialColorScale(value);
}
