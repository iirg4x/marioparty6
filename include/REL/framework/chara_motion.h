// Stores each character's requested motions as resource-number and engine-handle pairs.
#ifndef REL_FRAMEWORK_CHARA_MOTION_H
#define REL_FRAMEWORK_CHARA_MOTION_H

#include <utility>
#include <vector>

extern "C" {
#include "game/charman.h"
}

// Associates a requested resource number (first, used for lookup) with the engine handle returned
// for it (second); HU3D_MOTIONID_NONE means no motion handle was created.
typedef std::pair<unsigned int, HU3D_MOTIONID> MotionRecord;

// Tracks one character's cached motions and the resource most recently selected for playback.
struct MotionOwner {
    s32 charNo; // Character number used by the engine's character motion functions.
    std::vector<MotionRecord> motions; // Requested resources and returned handles, including failed
                                       // loads.
    unsigned int currentDataNum; // Selected resource number; unsigned -1 means none selected yet.
    MotionOwner(unsigned int characterNo);
    ~MotionOwner();
    void fn_1_DAF0(float timeFrames);
    float fn_1_DB20();
    float fn_1_DB48();
};

typedef MotionOwner CharacterMotion;

// Names indexed by the low 16-bit motion resource number, used for a missed-preload warning.
extern const char *characterMotionNames[];
void createCharacterMotion(CharacterMotion *character, unsigned int dataNum);
void selectCharacterMotion(CharacterMotion *character, unsigned int dataNum);
void setMotionOwnerTime(CharacterMotion *character, float timeFrames);
float getMotionOwnerTransitionTime(s32 *characterNo);
float getMotionOwnerMaximumTime(s32 *characterNo);

#endif
