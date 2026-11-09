#ifndef REL_M681DLL_WORK_6B44_H
#define REL_M681DLL_WORK_6B44_H

#include "dolphin/types.h"
#include "REL/m681DLL/texture_node.h"
extern "C" {
#include "game/charman.h"
}
#include "REL/m681DLL/character_motion.h"
#include "REL/m681DLL/transition.h"
#include "REL/m681DLL/framework/model.h"
#include "REL/m681DLL/model_resource.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"

struct ModePair { s32 values[2]; };
struct ModeBase;
struct Accessory533C;
struct OwnedModelOwner {
    OwnedModel *value;
    void reset(void *p) { value = static_cast<OwnedModel *>(p); }
    ~OwnedModelOwner();
};
struct ModelResourceOwner {
    ModelResource *value;
    operator ModelResource *() const { return value; }
    void reset(void *p) { value = static_cast<ModelResource *>(p); }
    ~ModelResourceOwner();
};
struct ModeOwner {
    ModeBase *value;
    void reset(void *p) { value = static_cast<ModeBase *>(p); }
    ~ModeOwner();
};
struct PlayerMotionState {
    s32 sequence;
    CharacterMotion motion;
};

struct PlayerConfigSelection : PlayerMotionState, Model {
    PlayerConfigSelection(s32 player, s32 mode);
    ~PlayerConfigSelection();
};

struct PlayerObject : GameObject, PlayerConfigSelection, EventListener {
    PlayerObject(s32 player, const PositionVector *initialState);
    virtual ~PlayerObject();
    virtual void update();
    virtual void receive(const EventPacket *event);
    typedef void (PlayerObject::*Callback)();
    ModeOwner resourceNode;
    ModelResourceOwner resource;
    ModePair modes;
    PositionVector state;
    Vec rotation;
    Transition firstAngle;
    Transition secondAngle;
    f32 fullAngle;
    OwnedModelOwner firstItem;
    OwnedModelOwner secondItem;
    u8 unknown8C[4]; /* No code names this field. */
    u32 elapsed;
    u32 fallFrames;
    f32 progress;
    f32 velocity;
    f32 firstValue;
    f32 secondValue;
    f32 animationScale;
    f32 thirdValue;
    f32 fourthValue;
    u8 unknownB4[4]; /* No code names this field. */
    s32 fifthValue;
    s32 count;
    u8 active;
    u8 sixthValue;
    char *firstItemHook;
    char *secondItemHook;
    Callback callback;
    void fn_1_5E90();
    void fn_1_5EF4();
    void fn_1_61B8();
    void fn_1_643C();
    void fn_1_6450();
    void fn_1_66F0();
    void fn_1_6824();
    void fn_1_6A50();
    void fn_1_7410();
    void fn_1_74E8();
    void fn_1_7608();
    void fn_1_77E4();
    void fn_1_5E8C();
};

typedef PlayerObject Work6B44;

#endif
