// Loads character motions on demand and manages their playback and transitions.
#include <algorithm>
#include <cstdio>
#include <functional>

#include "REL/framework/chara_motion.h"

static const char rcsid[] = "$Id: chara_motion.cpp,v 1.19 2004/09/09 12:24:38 saf Exp $";

// Motion resource names by data number, for the missed-preload warning.
const char *characterMotionNames[] = {
    "c000m1_300", "c000m1_301", "c000m1_302", "c000m1_303", "c000m1_304", "c000m1_305",
    "c000m1_306", "c000m1_307", "c000m1_308", "c000m1_309", "c000m1_310", "c000m1_311",
    "c000m1_312", "c000m1_313", "c000m1_314", "c000m1_315", "c000m1_316", "c000m1_317",
    "c000m1_318", "c000m1_319", "c000m1_320", "c000m1_321", "c000m1_322", "c000m1_323",
    "c000m1_324", "c000m1_325", "c000m1_326", "c000m1_327", "c000m1_328", "c000m1_332",
    "c000m1_333", "c000m1_336", "c000m1_398", "c000m1_339", "c000m1_340", "c000m1_341",
    "c000m1_343", "c000m1_346", "c000m1_347", "c000m1_349", "c000m1_350", "c000m1_351",
    "c000m1_352", "c000m1_354", "c000m1_355", "c000m1_356", "c000m1_357", "c000m1_358",
    "c000m1_359", "c000m1_360", "c000m1_361", "c000m1_362", "c000m1_364", "c000m1_365",
    "c000m1_367", "c000m1_368", "c000m1_369", "c000m1_370", "c000m1_371", "c000m1_372",
    "c000m1_374", "c000m1_376", "c000m1_377", "c000m1_380", "c000m1_381", "c000m1_382",
    "c000m1_383", "c000m1_385", "c000m1_386", "c000m1_396", "c000m1_399", "c000m1_402",
    "c000m1_403", "c000m1_404", "c000m1_405", "c000m1_406", "c000m1_409", "c000m1_410",
    "c000m1_411", "c000m1_412", "c000m1_413", "c000m1_419", "c000m1_420", "c000m1_421",
    "c000m1_422", "c000m1_423", "c000m1_424", "c000m1_425", "c000m1_427", "c000m1_433",
    "c000m1_434", "c000m1_435", "c000m1_436", "c000m1_437", "c000m1_438", "c000m1_439",
    "c000m1_440", "c000m1_441", "c000m1_442", "c000m1_443", "c000m1_444", "c000m1_446",
    "c000m1_453", "c000m1_454", "c000m1_455", "c000m1_456", "c000m1_457", "c000m1_458",
    "c000m1_464", "c000m1_468", "c000m1_476", "c000m1_477", "c000m1_478", "c000m1_480",
    "c000m1_481", "c000m1_483", "c000m1_484", "c000m1_485", "c000m1_487", "c000m1_488",
    "c000m1_489", "c000m1_490", "c000m1_491", "c000m1_493", "c000m1_494", "c000m1_505",
    "c000m1_506", "c000m1_507", "c000m1_508", "c000m1_509", "c000m1_510", "c000m1_526",
    "c000m1_600", "c000m1_601", "c000m1_602", "c000m1_603", "c000m1_604", "c000m1_605",
    "c000m1_606", "c000m1_607", "c000m1_608", "c000m1_609", "c000m1_610", "c000m1_611",
    "c000m1_612", "c000m1_613", "c000m1_614", "c000m1_615", "c000m1_616", "c000m1_617",
    "c000m1_618", "c000m1_619", "c000m1_620", "c000m1_621", "c000m1_622", "c000m1_623",
    "c000m1_624", "c000m1_625", "c000m1_626", "c000m1_627", "c000m1_628", "c000m1_629",
    "c000m1_630", "c000m1_631", "c000m1_632", "c000m1_633", "c000m1_634", "c000m1_635",
    "c000m1_636", "c000m1_637", "c000m1_638", "c000m1_639", "c000m1_640", "c000m1_641",
    "c000m1_642", "c000m1_643", "c000m1_644", "c000m1_645", "c000m1_646", "c000m1_647",
    "c000m1_648", "c000m1_649", "c000m1_650", "c000m1_651", "c000m1_652", "c000m1_653",
    "c000m1_654", "c000m1_655", "c000m1_656", "c000m1_657", "c000m1_658", "c000m1_659",
    "c000m1_660", "c000m1_661", "c000m1_662", "c000m1_663", "c000m1_664", "c000m1_665",
    "c000m1_666", "c000m1_667", "c000m1_668", "c000m1_669", "c000m1_670", "c000m1_671",
    "c000m1_672", "c000m1_673", "c000m1_674", "c000m1_675", "c000m1_676", "c000m1_677",
    "c000m1_678", "c000m1_679", "c000m1_680", "c000m1_681", "c000m1_682", "c000m1_eye",
};

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
