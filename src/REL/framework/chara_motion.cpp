// Loads character motions on demand and manages their playback and transitions.
static const char rcsid[] = "$Id: chara_motion.cpp,v 1.19 2004/09/09 12:24:38 saf Exp $";

extern "C" {
#include <stdio.h>
}
#include "REL/framework/chara_motion.h"

void operator delete(void *);
void setMotionOwnerTime(CharacterMotion *, float);
float getMotionOwnerTime(s32 *);
float getMotionOwnerMaximumTime(s32 *);
float getMotionOwnerTransitionTime(s32 *);
HU3D_MOTIONID getCharacterMotion(CharacterMotion *, unsigned int);
template<class T>
void insertMotionRecords(Sequence<T> *, T *, unsigned int, const T *);
void applyMotionCleanup(MotionRecord *, MotionRecord *, MotionCleanup);

// Character construction starts an empty motion list and closes motion and shared effect data.
MotionOwner::MotionOwner(unsigned int characterNo)
    : charNo(characterNo), currentDataNum(-1)
{
    CharMotionDataClose((s16)charNo);
}

// Character destruction requests cleanup for every handle, then closes motion and effect data.
// The engine may retain motions still attached to a model; its release result is not checked.
// The motions member then releases the record storage.
MotionOwner::~MotionOwner()
{
    applyMotionCleanup(motions.begin(), motions.end(),
                       bindMotionCleanup(makeMotionFunction(CharMotionKill), charNo));
    CharMotionDataClose((s16)charNo);
}

// Character setup and lazy lookup load one motion and remember its returned engine handle.
// The motion and shared effect directories are closed after each load.
void createCharacterMotion(CharacterMotion *character, unsigned int dataNum)
{
    s16 motionId = CharMotionCreate((s16)character->charNo, dataNum);
    appendMotionRecord(character->motions, makeMotionRecord(dataNum, s16(motionId)));
    CharMotionDataClose((s16)character->charNo);
}

// Motion preload requests load every resource in the supplied array into this character's list.
void createCharacterMotions(CharacterMotion *character, unsigned int *dataNums, s32 count)
{
    unsigned int *last = dataNums + count;
    for (unsigned int *cursor = dataNums; cursor != last; ++cursor) {
        unsigned int dataNum = *cursor;
        s16 motionId = CharMotionCreate((s16)character->charNo, dataNum);
        appendMotionRecord(character->motions, makeMotionRecord(dataNum, s16(motionId)));
        CharMotionDataClose((s16)character->charNo);
    }
}

// Select the resource when it changes or the main motion reaches its endpoint.
// During a blend, the endpoint test still uses the main motion, not the incoming motion.
void selectCharacterMotion(CharacterMotion *character, unsigned int dataNum)
{
    if (dataNum != character->currentDataNum || CharMotionEndCheck(character->charNo)) {
        CharMotionSet(character->charNo, getCharacterMotion(character, dataNum));
        character->currentDataNum = dataNum;
    }
}

// Motion setup adds engine motion-slot attributes, loading the requested resource if absent.
void setCharacterMotionAttributes(CharacterMotion *character, unsigned int dataNum, u16 attributes)
{
    Hu3DMotionAttrSet(getCharacterMotion(character, dataNum), attributes);
}

// setCharacterMotionTime seeks the main motion in frames; the engine clamps it to its duration.
void setMotionOwnerTime(CharacterMotion *character, float timeFrames)
{
    CharMotionTimeSet((s16)character->charNo, timeFrames);
}

// Animation timing queries use the incoming motion during a blend, otherwise the main motion.
float getMotionOwnerTransitionTime(s32 *characterNo)
{
    if (CharMotionShiftIDGet((s16)*characterNo) >= 0) {
        // Incoming playback is reported one frame behind its engine clock.
        return CharMotionShiftTimeGet((s16)*characterNo) - 1.0f;
    }
    return CharMotionTimeGet((s16)*characterNo);
}

// Animation timing queries return the main motion's duration in frames, or zero with no motion.
float getMotionOwnerMaximumTime(s32 *characterNo)
{
    return CharMotionMaxTimeGet((s16)*characterNo);
}

// Animation completion queries test the main motion's endpoint, including reverse playback.
u8 characterMotionEnded(s32 *characterNo)
{
    return (u8)(CharMotionEndCheck((s16)*characterNo) != 0);
}

// Animation transition queries return one when the engine has no incoming motion handle.
u32 characterMotionShiftInactive(s32 *characterNo)
{
    return (u32)CharMotionShiftIDGet((s16)*characterNo) >> 31U;
}

// Blend when the requested resource changes or the main motion reaches its endpoint.
// blendFrames is the blend duration; startFrame is the incoming frame, with speed reset to one.
// Replacing an active blend promotes its incoming motion to main before starting the new blend.
void shiftCharacterMotion(CharacterMotion *character, unsigned int dataNum,
                          float blendFrames, float startFrame, u32 attributes)
{
    if (dataNum != character->currentDataNum || CharMotionEndCheck(character->charNo)) {
        // Request a main-motion pause; replacing an active blend rebuilds main flags from
        // attributes.
        CharModelAttrSet(character->charNo, HU3D_MOTATTR_PAUSE);
        CharMotionShiftSet(character->charNo, getCharacterMotion(character, dataNum),
                           startFrame, blendFrames, attributes);
        character->currentDataNum = dataNum;
    }
}

// Overlay animation requests attach the cached or newly loaded motion at frame zero and speed one.
void setCharacterMotionOverlay(CharacterMotion *character, HU3D_MODELID modelId,
                               unsigned int dataNum)
{
    Hu3DMotionOverlaySet(modelId, getCharacterMotion(character, dataNum));
}

// getCharacterMotion uses this predicate to find a record by its requested resource number.
bool motionRecordMatches(const MotionRecord &record, unsigned int dataNum)
{
    return record.dataNum == dataNum;
}

// Motion selection, attributes and overlays look up a cached handle, loading it if absent.
// Failed load handles are cached too and returned on later lookups without retry.
HU3D_MOTIONID getCharacterMotion(CharacterMotion *character, unsigned int dataNum)
{
    MotionRecord *record = character->motions.begin();
    MotionRecord *last = character->motions.end();
    record = findMatchingMotion(record, last,
        bindMotionMatcher(makeMotionMatchFunction(motionRecordMatches), dataNum));
    if (record == last) {
        // A missed preload emits a warning but still loads and records the motion on demand.
        printf("****** Requested %s is not registered.\n", characterMotionNames[(u16)dataNum]);
        createCharacterMotion(character, dataNum);
        return character->motions.back().motionId;
    }
    return s16(record->motionId);
}

// MotionOwner destruction invokes the bound character cleanup for every stored motion handle.
void applyMotionCleanup(MotionRecord *first, MotionRecord *last,
                        MotionCleanup cleanup)
{
    for (; first != last; ++first) {
        cleanup(first->motionId);
    }
}

// insertMotionRecords copies from the end so overlapping record moves preserve their values.
template<class T>
inline T *copyMotionRecordsBackwardRange(T *first, T *last, T *destination)
{
    while (last > first) *--destination = *--last;
    return destination;
}

// insertMotionRecords moves the existing tail toward the list's end before filling new slots.
template<class T>
inline T *copyMotionRecordsBackward(T *first, T *last, T *destination)
{
    return copyMotionRecordsBackwardRange(first, last, destination);
}

// appendMotionRecord inserts copies into the motion list, growing its allocation when needed.
template<class T>
void insertMotionRecords(Sequence<T> *sequence, T *position, unsigned int count,
                         const T *source)
{
    if (count != 0) {
        unsigned long maximumRecords = sequence->maximumCount();
        if (count > maximumRecords || sequence->count > maximumRecords - count) {
            fprintf(stderr, "vector::insert length error\n");
            abort();
        }
        if (sequence->count + count <= sequence->availableCapacity()) {
            T *last = sequence->end();
            int trailingRecords = last - position;
            const T *insertedRecord = source;
            if (count > trailingRecords) {
                T *destination = last;
                // Construct the new tail before assigning into the existing live slots.
                while (count > trailingRecords) {
                    sequence->construct(destination, *source);
                    ++destination;
                    --count;
                    ++sequence->count;
                }
                T *cursor = position;
                while (cursor < last) {
                    sequence->construct(destination, *cursor);
                    ++cursor;
                    ++destination;
                    ++sequence->count;
                }
            } else {
                T *cursor = last - count;
                T *destination = last;
                while (cursor < last) {
                    sequence->construct(destination, *cursor);
                    ++cursor;
                    ++destination;
                    ++sequence->count;
                }
                if (last - (trailingRecords - count) <= insertedRecord && insertedRecord < last) {
                    // An insertion value inside the moved tail now resides count slots later.
                    insertedRecord += count;
                }
                copyMotionRecordsBackward<T>(position, position + (trailingRecords - count), last);
            }
            fillMotionRecords(position, count, *insertedRecord);
            return;
        }
        SequenceStorage<T> replacement(sequence->allocator());
        unsigned long requiredRecords = sequence->count + count;
        unsigned long capacity = sequence->capacityValue() ? sequence->capacityValue() : 1;
        // Grow geometrically, stopping at the allocator's element limit before overflow.
        while (requiredRecords > capacity) {
            if (capacity < maximumRecords / 2) capacity *= 2;
            else capacity = maximumRecords;
        }
        replacement.storage = replacement.allocator().allocate(capacity);
        replacement.capacityValue() = capacity;
        T *destination = replacement.storage;
        T *cursor = sequence->begin();
        T *last = sequence->end();
        while (cursor < position) {
            replacement.construct(destination, *cursor);
            ++cursor;
            ++destination;
            ++replacement.count;
        }
        while (count != 0) {
            replacement.construct(destination, *source);
            ++destination;
            --count;
            ++replacement.count;
        }
        while (cursor < last) {
            replacement.construct(destination, *cursor);
            ++cursor;
            ++destination;
            ++replacement.count;
        }
        exchangeMotionStorage(replacement, static_cast<SequenceStorage<T> &>(*sequence));
        // replacement now owns the previous allocation and releases it on leaving this scope.
    }
}

// Motion list destruction and insertion's replaced allocation destroy records and free storage.
template<class T>
inline SequenceStorage<T>::~SequenceStorage()
{
    clear();
    if (storagePointer()) deallocate(storagePointer());
}

// Allocation replacement swaps each stored count or pointer through this helper.
template<class T>
inline void exchangeMotionValue(T &left, T &right)
{
    T savedValue = left;
    left = right;
    right = savedValue;
}

// SequenceStorage destruction destroys records from the end while keeping the reserved allocation.
template<class T>
void SequenceStorage<T>::clear()
{
    T *first = storagePointer();
    T *last = first + count;
    while (last > first) destroy(--last);
    count = 0;
}
