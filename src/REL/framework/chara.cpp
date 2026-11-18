// Creates character models and connects them to cached motion playback.
static const char rcsid[] = "$Id: chara.cpp,v 1.21 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/chara.h"
#include "REL/framework/particle.h"

// Runs when a game creates a Character; initializes its model and selects default motion 0.
Character::Character(unsigned int characterNo, unsigned int modelNo)
    : MotionOwner(characterNo), OwnedModel(characterNo, modelNo)
{
    // Enables attribute bits 0x40000001 on the character model.
    Model::setAttribute(0x40000001);
    // Also enables attribute bits 0x40000040.
    Model::setAttribute(0x40000040);
    Model::setShadow();
    createCharacterMotion(this, 0);
    selectCharacterMotion(this, 0);
}

// Runs when a game destroys a Character; releases its model before MotionOwner cleanup.
Character::~Character() {}

// Sets the character's playback position when a game seeks its current motion.
void MotionOwner::fn_1_DAF0(float timeFrames)
{
    setMotionOwnerTime(this, timeFrames);
}

// Returns the playback time of the character's motion (the incoming one during a blend).
float MotionOwner::fn_1_DB20()
{
    return getMotionOwnerTransitionTime(&charNo);
}

// Returns the length of the character's main motion in frames.
float MotionOwner::fn_1_DB48()
{
    return getMotionOwnerMaximumTime(&charNo);
}
