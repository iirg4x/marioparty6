#ifndef REL_M648_ANIMATION_OWNERS_H
#define REL_M648_ANIMATION_OWNERS_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include "REL/framework/list.h"
class SetupAnimationOwner;
class PlayAnimationOwner;
// One animated model of the setup list: the list's node, and the update its visitor runs each frame.
class SetupAnimationOwner : public ListNode<SetupAnimationOwner> {
public:
    typedef void (SetupAnimationOwner::*Callback)();
    // The current step, which the list's visitor calls each frame.
    Callback callback;
    s32 mode;
    Vec motionPosition;
    Vec destination;
    OwnedModel model;
    f32 angle;
    f32 radialValue;
    f32 radialTarget;
    u8 unknown40[8]; /* No code names this field. */
    s32 framesRemaining;
    f32 scale;
    f32 scaleTarget;
    SetupAnimationOwner(const OwnedModel *model, s32 initialMode);
    void fn_1_1CAC();
    void fn_1_1D28();
    void fn_1_1F14();
    void fn_1_2290();
};
// One animated model of the play list: the list's node, and the update its visitor runs each frame.
class PlayAnimationOwner : public ListNode<PlayAnimationOwner> {
public:
    typedef void (PlayAnimationOwner::*Callback)();
    // The current step, which the list's visitor calls each frame.
    Callback callback;
    s32 mode;
    Vec motionPosition;
    Vec destination;
    OwnedModel model;
    f32 angle;
    f32 radialValue;
    f32 radialTarget;
    u8 unknown40[8]; /* No code names this field. */
    s32 framesRemaining;
    f32 scale;
    f32 scaleTarget;
    PlayAnimationOwner(const OwnedModel *model, s32 initialMode);
    void fn_1_2C64();
    void fn_1_2CE0();
    void fn_1_2EDC();
    void fn_1_3268();
};
#endif
