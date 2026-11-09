#ifndef REL_M681DLL_PLAYER_INPUT_H
#define REL_M681DLL_PLAYER_INPUT_H
#include "dolphin/types.h"
#include "REL/framework/event_packet.h"
struct PlayerObject;

struct ModeBase : EventListener {
    s32 value;
    s32 count;
    f32 duration;
    ModeBase();
    virtual ~ModeBase();
    virtual void receive(const EventPacket *);
    virtual void update(PlayerObject *) = 0;
};

struct ModeToggle : ModeBase {
    u8 active;
    ModeToggle();
    virtual ~ModeToggle();
    virtual void update(PlayerObject *);
};

struct ModeObject : ModeBase {
    u8 active;
    s32 modeValue;
    s32 mode;
    ModeObject(s32 mode);
    virtual ~ModeObject();
    virtual void update(PlayerObject *);
};

#endif
