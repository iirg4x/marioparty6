#ifndef M655DLL_CAMERA_WORK_H
#define M655DLL_CAMERA_WORK_H
#include "dolphin/types.h"
#include "REL/framework/singleton.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/list_object.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/debug.h"

struct CameraWork : MinigameSingleton<CameraWork>, RequestObject, CameraFrame, EventListener {
    typedef void (CameraWork::*State)();
    State state;
    s16 unknown44;
    u16 unk46; // The game never reads this member.
    s32 itemCount;
    s32 stateCounter;
    CameraSetup nextFrame;
    CameraWork();
    virtual ~CameraWork();
    virtual void flagEnabled(int eventType);
    virtual void update();
    virtual void receive(const EventPacket *);
    void fn_1_6D2C();
    void fn_1_6B94();
    void fn_1_6D5C();
    void fn_1_6F00();
    void fn_1_6F8C();
    s32 fn_1_7220();
    u8 fn_1_6FB4();
};
#endif
