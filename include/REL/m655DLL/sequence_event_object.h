#ifndef REL_M655DLL_SEQUENCE_EVENT_OBJECT_H
#define REL_M655DLL_SEQUENCE_EVENT_OBJECT_H
#include "REL/framework/model_motion.h"
#include "REL/framework/game_object.h"
#include "REL/m655DLL/sequence_callback.h"
#include "REL/framework/category_sphere.h"
struct SequenceEventObject : GameObject, ModelMotion {
    explicit SequenceEventObject(int);
    virtual ~SequenceEventObject();
    virtual void update();
    typedef void (SequenceEventObject::*State)(void);
    EventHandlerManager channelZeroHandlers;
    EventHandlerManager indexedChannelHandlers;
    State state;
    u8 flag7 : 1;
    u8 flag6 : 1;
    u8 flags5_0 : 6;
    s32 index;
    RegisteredValue registeredValue;
    void fn_1_4048();
    void fn_1_4194();
    void fn_1_4218();
    void fn_1_4318();
    void fn_1_3F40(EventMessage *);
    void fn_1_3F54(EventMessage *);
    void fn_1_3F68(EventMessage *);
    void fn_1_3FBC(EventMessage *);
};
#endif
