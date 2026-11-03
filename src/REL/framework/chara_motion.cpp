// Loads character motions on demand and manages their playback and transitions.
#include <algorithm>
#include <cstdio>
#include <functional>

#include "REL/framework/chara_motion.h"

static const char rcsid[] = "$Id: chara_motion.cpp,v 1.19 2004/09/09 12:24:38 saf Exp $";

HU3D_MOTIONID getCharacterMotion(CharacterMotion *, unsigned int);
template <class Function>
void applyMotionCleanup(MotionRecord *first, MotionRecord *last, Function cleanup);

// Character construction starts an empty motion list and closes motion and shared effect data.
MotionOwner::MotionOwner(unsigned int characterNo)
    : charNo(characterNo), currentDataNum(-1)
{
    fn_8005B4BC((s16)charNo);
}

// Character destruction requests cleanup for every handle, then closes motion and effect data.
// The engine may retain motions still attached to a model; its release result is not checked.
// The motions member then releases the record storage.
MotionOwner::~MotionOwner()
{
    applyMotionCleanup(motions.begin(), motions.end(),
                       std::bind1st(std::ptr_fun(fn_8005B420), charNo));
    fn_8005B4BC((s16)charNo);
}

// Character setup and lazy lookup load one motion and remember its returned engine handle.
// The motion and shared effect directories are closed after each load.
void createCharacterMotion(CharacterMotion *character, unsigned int dataNum)
{
    s16 motionId = fn_8005B0B8((s16)character->charNo, dataNum);
    character->motions.push_back(std::make_pair(dataNum, s16(motionId)));
    fn_8005B4BC((s16)character->charNo);
}

// Motion preload requests load every resource in the supplied array into this character's list.
void createCharacterMotions(CharacterMotion *character, unsigned int *dataNums, s32 count)
{
    unsigned int *last = dataNums + count;
    for (unsigned int *cursor = dataNums; cursor != last; ++cursor) {
        unsigned int dataNum = *cursor;
        s16 motionId = fn_8005B0B8((s16)character->charNo, dataNum);
        character->motions.push_back(std::make_pair(dataNum, s16(motionId)));
        fn_8005B4BC((s16)character->charNo);
    }
}

// Select the resource when it changes or the main motion reaches its endpoint.
// During a blend, the endpoint test still uses the main motion, not the incoming motion.
void selectCharacterMotion(CharacterMotion *character, unsigned int dataNum)
{
    if (dataNum != character->currentDataNum || fn_8005C240(character->charNo)) {
        fn_8005BE80(character->charNo, getCharacterMotion(character, dataNum));
        character->currentDataNum = dataNum;
    }
}

// Motion setup adds engine motion-slot attributes, loading the requested resource if absent.
void setCharacterMotionAttributes(CharacterMotion *character, unsigned int dataNum, u16 attributes)
{
    fn_8002DC74(getCharacterMotion(character, dataNum), attributes);
}

// setCharacterMotionTime seeks the main motion in frames; the engine clamps it to its duration.
void setMotionOwnerTime(CharacterMotion *character, float timeFrames)
{
    fn_8005C16C((s16)character->charNo, timeFrames);
}

// Animation timing queries use the incoming motion during a blend, otherwise the main motion.
float getMotionOwnerTransitionTime(s32 *characterNo)
{
    if (fn_8005C284((s16)*characterNo) >= 0) {
        // Incoming playback is reported one frame behind its engine clock.
        return fn_8005C34C((s16)*characterNo) - 1.0f;
    }
    return fn_8005C1B8((s16)*characterNo);
}

// Animation timing queries return the main motion's duration in frames, or zero with no motion.
float getMotionOwnerMaximumTime(s32 *characterNo)
{
    return fn_8005C1FC((s16)*characterNo);
}

// Animation completion queries test the main motion's endpoint, including reverse playback.
u8 characterMotionEnded(s32 *characterNo)
{
    return (u8)(fn_8005C240((s16)*characterNo) != 0);
}

// Animation transition queries return one when the engine has no incoming motion handle.
u32 characterMotionShiftInactive(s32 *characterNo)
{
    return (u32)fn_8005C284((s16)*characterNo) >> 31U;
}

// Blend when the requested resource changes or the main motion reaches its endpoint.
// blendFrames is the blend duration; startFrame is the incoming frame, with speed reset to one.
// Replacing an active blend promotes its incoming motion to main before starting the new blend.
void shiftCharacterMotion(CharacterMotion *character, unsigned int dataNum,
                          float blendFrames, float startFrame, u32 attributes)
{
    if (dataNum != character->currentDataNum || fn_8005C240(character->charNo)) {
        // Request a main-motion pause; replacing an active blend rebuilds main flags from
        // attributes.
        fn_8005C430(character->charNo, ((1 << 30) | (1 << 1)));
        fn_8005C2C8(character->charNo, getCharacterMotion(character, dataNum),
                           startFrame, blendFrames, attributes);
        character->currentDataNum = dataNum;
    }
}

// Overlay animation requests attach the cached or newly loaded motion at frame zero and speed one.
void setCharacterMotionOverlay(CharacterMotion *character, HU3D_MODELID modelId,
                               unsigned int dataNum)
{
    fn_8002CAA8(modelId, getCharacterMotion(character, dataNum));
}

// getCharacterMotion uses this predicate to find a record by its requested resource number.
bool motionRecordMatches(const MotionRecord &record, unsigned int dataNum)
{
    return record.first == dataNum;
}

// Motion selection, attributes and overlays look up a cached handle, loading it if absent.
// Failed load handles are cached too and returned on later lookups without retry.
HU3D_MOTIONID getCharacterMotion(CharacterMotion *character, unsigned int dataNum)
{
    MotionRecord *record = character->motions.begin();
    MotionRecord *last = character->motions.end();
    record = std::find_if(record, last, std::bind2nd(std::ptr_fun(motionRecordMatches), dataNum));
    if (record == last) {
        // A missed preload emits a warning but still loads and records the motion on demand.
        std::printf("****** Requested %s is not registered.\n", characterMotionNames[(u16)dataNum]);
        createCharacterMotion(character, dataNum);
        return character->motions.back().second;
    }
    return s16(record->second);
}

// MotionOwner destruction invokes the bound character cleanup for every stored motion handle.
// Retail places this after the strong functions, with the instantiated vector members.
template <class Function>
void applyMotionCleanup(MotionRecord *first, MotionRecord *last, Function cleanup)
{
    for (; first != last; ++first) {
        cleanup(first->second);
    }
}
