// A character model with cached motion playback.
#ifndef REL_FRAMEWORK_CHARA_H
#define REL_FRAMEWORK_CHARA_H

#include "REL/framework/chara_motion.h"
#include "REL/framework/model.h"

// Couples one character model handle with the motion owner for its character.
struct Character : MotionOwner, OwnedModel {
    Character(unsigned int characterNo, unsigned int modelNo);
    ~Character();
};

#endif
