#ifndef REL_M648_ANIMATION_OWNER_17F8_H
#define REL_M648_ANIMATION_OWNER_17F8_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/m648DLL/owned_model.h"
#include "REL/framework/linknode.h"
class SetupAnimationOwner;
class PlayAnimationOwner;
// The nodes of the two update lists: each frame the list's visitor calls the owner's current step (callback).
struct SetupAnimationNode : Node {
    void (SetupAnimationOwner::*callback)();
    ~SetupAnimationNode();
};
struct PlayAnimationNode : Node {
    void (PlayAnimationOwner::*callback)();
    ~PlayAnimationNode();
};
typedef OwnedModel LinkedModel;
// One animated model of the setup list: the list's node, and the update its visitor runs each frame.
class SetupAnimationOwner : public SetupAnimationNode {
public:
    typedef void (SetupAnimationOwner::*Callback)();
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
    SetupAnimationOwner(const LinkedModel *model, s32 initialMode);
    void fn_1_1CAC();
    void fn_1_1D28();
    void fn_1_1F14();
    void fn_1_2290();
};
// One animated model of the play list: the list's node, and the update its visitor runs each frame.
class PlayAnimationOwner : public PlayAnimationNode {
public:
    typedef void (PlayAnimationOwner::*Callback)();
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
    PlayAnimationOwner(const LinkedModel *model, s32 initialMode);
    void fn_1_2C64();
    void fn_1_2CE0();
    void fn_1_2EDC();
    void fn_1_3268();
};
#endif
