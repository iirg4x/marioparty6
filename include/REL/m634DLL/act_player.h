#ifndef REL_M634DLL_ACT_PLAYER_H
#define REL_M634DLL_ACT_PLAYER_H
#include <functional>
#include "REL/framework/vector3.h"
#include "REL/m634DLL/pad_control.h"
#include <vector>
#include <memory>
#include "REL/m634DLL/collision_shape.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/charman.h"
#include "game/gamework.h"
#include "game/mg/actman.h"
}
struct ActPlayer;
typedef void (ActPlayer::*CallbackMethod)(const EventPacket &);
typedef void (ActPlayer::*ActState)();

typedef std::binder1st<std::mem_fun1_t<void, ActPlayer, const EventPacket &> > CallbackPayload;

struct NightScale {
    f32 x, y, z;
};

// The object a player's input comes from: a pad, or one of the two computer levels.
#include "REL/m634DLL/player_part.h"

// Descriptive class names. The new-expressions in fn_1_3A0 establish extents;
// fn_1_67B8, fn_1_6A68 and fn_1_7568 initialize the fields shown here.
struct PlayerPadPart : ActPlayerPart {
    s32 padNo;
    explicit PlayerPadPart(ActPlayer *);
    virtual ~PlayerPadPart() {}
    virtual void readInput(StickState *output);
};
struct PlayerChoicePart : ActPlayerPart {
    PadControl padControl;
    Vec position;
    s32 selectedIndex;
    s32 idle;
    u32 updateCount;
    s32 buttonDelay;
    s32 difficulty;
    typedef void (PlayerChoicePart::*Callback)(StickState *);
    Callback callback;
    PlayerChoicePart(ActPlayer *, s32);
    virtual ~PlayerChoicePart();
    virtual void readInput(StickState *output);
    virtual void reset();
    void setCallback(Callback next) { callback = next; }
    void setPosition(const Vec &target) { position.x = target.x; position.y = target.y; position.z = target.z; }
    void fn_1_6CB0(StickState *);
    void fn_1_6CB4(StickState *);
    void fn_1_6F08(StickState *);
    void fn_1_70D8(StickState *);
};
struct PlayerMotionPart : ActPlayerPart {
    PadControl padControl;
    Vector3 position;
    s32 selectedIndex;
    s32 selectedMotion;
    s32 selectedMotionTime;
    s32 unknown5C;
    s32 transitionCount;
    s32 selectionStrategy;
    typedef void (PlayerMotionPart::*Callback)(StickState *);
    Callback callback;
    PlayerMotionPart(ActPlayer *, s32);
    virtual ~PlayerMotionPart();
    virtual void readInput(StickState *);
    virtual void reset();
    void setCallback(Callback next) { callback = next; }
    ActPlayer *getOwner() const { return owner; }
    void setPosition(const Vec &next) { position.x = next.x; position.y = next.y; position.z = next.z; }
    void fn_1_779C(StickState *);
    void fn_1_77A0(StickState *);
    void fn_1_7A68(StickState *);
    void fn_1_7DC0(StickState *);
    void fn_1_82C8(StickState *);
};

int fn_1_3BBC(COL_NARROW_PARAM *first, COL_NARROW_PARAM *second);

void fn_1_4A8(HU3D_MODEL *, Mtx *);
extern "C" {
extern Vec lbl_1_data_8C[4];
extern unsigned int lbl_1_data_48[0x11];
}

struct ActPlayer : GameObject {
    Vector3 position;
    Vector3 getPosition() const { return position; }
    s32 getSelections() const { return playerMode; }
    s32 getProgress() const { return unknownC8; }
    void resetProgress() { unknownC4 = 0; unknownC8 = 0; }
    s32 getPlayerNo() const { return playerMode; }
    Vec rotation;
    Vec velocity;
    CrossShape shape;
    f32 unknown70;
    s32 unknown74;
    s32 playerIndex;
    std::auto_ptr<OwnedModel> bodyModel;
    s16 secondaryLightId;
    std::vector<GXColor> firstColors;
    std::vector<GXColor> secondColors;
    MGPLAYER *player;
    std::auto_ptr<ActPlayerPart> part;
    u8 unknownA4;
    s16 modelId;
    s16 characterNumber;
    u8 unkAA[2]; /* No code names this field. */
    f32 angle;
    u8 unknownB0;
    s32 unknownB4;
    s32 playerMode;
    s32 transitionStage;
    s32 transitionStep;
    u8 unknownC4;
    s32 unknownC8;
    std::auto_ptr<OwnedModel> models[2];
    u8 transitionReady;
    u8 unkD5[3]; /* No code names this field. */
    s32 shownModel;
    f32 elapsedTime;
    EventHandlerManager firstHandlers;
    EventHandlerManager secondHandlers;
    ActState state;
    s32 motionId;
    std::vector<HU3D_MOTIONID> m_motion_id;

    ActPlayer(s32 index, s32 mode, s32 category);
    virtual ~ActPlayer();
    virtual void update();
    void loadMotion(u32 dataNum);
    void loadMotions();
    void loadColors();
    void setShapeKind(int kind) { shape.shapeKind = kind; }
    void setState(ActState next) { state = next; }
    void fn_1_1814();
    void fn_1_194C(s32 index, u32 value);
    void fn_1_1A34();
    void fn_1_1A84();
    void fn_1_1DE8();
    void fn_1_1F9C(f32 redScale, f32 greenScale, f32 blueScale);
    void fn_1_1B90();
    void fn_1_1C8C();
    void fn_1_21C4();
    void fn_1_21F8();
    void fn_1_222C();
    void fn_1_2340();
    void fn_1_2A20();
    void fn_1_2A24();
    void fn_1_353C(const EventPacket &);
    void fn_1_3C74();
    void fn_1_3D18(const EventPacket &);
    void fn_1_33F8(u8);
    void fn_1_3A28(const EventPacket &);
    void fn_1_2614();
    void fn_1_32BC();
    void fn_1_3638(const EventPacket &);
    void fn_1_37FC(const EventPacket &);
    void fn_1_32B0();
    void fn_1_36D4(const EventPacket &);
    void shiftMotion(u32 id, u8 mode, u8 loop, f32 blend)
    {
        s16 motion;
        if (motionId != id || mode != 0) {
            motion = m_motion_id.at(id);
            fn_8005C430(characterNumber, ((1 << 30) | (1 << 1)));
            if (mode == 1) {
                fn_8005BE80(characterNumber, motion);
            } else {
                fn_8005C2C8(characterNumber, motion, 0.0f, blend,
                           loop == 0 ? ((1 << 30) | (1 << 0)) : 0);
            }
            fn_8005C390(characterNumber, 1.0f);
            motionId = id;
        }
    }
};
#endif
