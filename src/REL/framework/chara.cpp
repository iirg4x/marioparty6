// Creates character models and connects them to cached motion playback.
static const char rcsid[] = "$Id: chara.cpp,v 1.21 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/chara_motion.h"
#include "REL/framework/particle.h"

// These attributes are applied to a character model during construction.
#define CHARACTER_MODEL_INITIAL_FLAG 0x40000001
#define CHARACTER_MODEL_SECONDARY_FLAG 0x40000040
#define CHARACTER_MODEL_ID_OFFSET 20

// Couples one character model handle with the motion owner for its character.
struct Character : MotionOwner {
    s16 modelId; // Engine model ID initialized with this character's model data.
    Character(unsigned int characterNo, unsigned int modelNo);
    ~Character();
};

void initializeCharacterModel(s32 modelHandle, s32 characterNo, s32 modelNo);
void setCharacterModelAttribute(s32 modelHandle, s32 attributeFlags);
void finalizeCharacterModel(s32 modelHandle);
void createCharacterMotion(MotionOwner *character, unsigned int motion);
void selectCharacterMotion(MotionOwner *character, unsigned int motion);
void setMotionOwnerTime(MotionOwner *character, float time);

// Runs when a game creates a Character; initializes its model and selects default motion 0.
Character::Character(unsigned int characterNo, unsigned int modelNo)
    : MotionOwner(characterNo)
{
    // The engine model ID follows MotionOwner's state in this character object.
    initializeCharacterModel((s32)this + CHARACTER_MODEL_ID_OFFSET, characterNo, modelNo);
    setCharacterModelAttribute((s32)this + CHARACTER_MODEL_ID_OFFSET, CHARACTER_MODEL_INITIAL_FLAG);
    setCharacterModelAttribute((s32) this + CHARACTER_MODEL_ID_OFFSET,
                               CHARACTER_MODEL_SECONDARY_FLAG);
    finalizeCharacterModel((s32)this + CHARACTER_MODEL_ID_OFFSET);
    createCharacterMotion(this, 0);
    selectCharacterMotion(this, 0);
}

// Runs when a game destroys a Character; releases its model before MotionOwner cleanup.
Character::~Character()
{
    // Both checks compare the member address with null, so this always resets the stored model ID
    // to -1.
    if (&modelId != 0 && &modelId != 0) {
        ((Model *)&modelId)->Replace(-1);
    }
}

// Sets the character's playback position when a game seeks its current motion.
void setCharacterMotionTime(MotionOwner *characterOwner, float timeFrames)
{
    setMotionOwnerTime(characterOwner, timeFrames);
}
