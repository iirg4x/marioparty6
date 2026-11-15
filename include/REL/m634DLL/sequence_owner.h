#ifndef REL_M634DLL_SEQUENCE_OWNER_H
#define REL_M634DLL_SEQUENCE_OWNER_H
#include <memory>
#include "REL/framework/model.h"
#include "REL/m634DLL/collision_shape.h"

struct SequenceOwner : GameObject {
    typedef void (SequenceOwner::*Callback)();
    BoxShape box;
    Mtx rotation;
    s32 frame;
    s32 frameLimit;
    f32 frameRate;
    f32 elapsed;
    u8 initialized;

    u8 unk95[3]; /* No code names this field. */

    s32 sequenceType;
    f32 motionDuration;
    Callback callback;
    std::auto_ptr<OwnedModel> motionA;
    std::auto_ptr<OwnedModel> motionB;
    SequenceOwner(s32 first, s32 second, char *third, s32 mode, Vec position, f32 elapsed);
    virtual ~SequenceOwner();
    virtual void update();
    void fn_1_10744();
    void fn_1_108A0();
};
#endif
