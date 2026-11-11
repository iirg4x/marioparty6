#ifndef REL_M648_FLYING_OBJECT_H
#define REL_M648_FLYING_OBJECT_H

#include "REL/framework/list.h"
#include "REL/framework/model.h"
#include "dolphin/mtx.h"

// The object each player keeps beside its sprite (0x68 bytes): it waits, flies toward the camera
// and bounces back. Every object is on one list; the first list object updates and resets them
// all, and deletes them with itself.
class FlyingObject : public ListNode<FlyingObject> {
public:
    typedef void (FlyingObject::*Callback)();
    Callback callback;
    OwnedModel modelA;
    OwnedModel modelB;
    s32 audioHandle;
    u8 stopped;
    u8 complete;
    Vec startPosition;
    Vec position;
    Vec movement;
    Vec resetPosition;
    f32 fade;
    f32 angle;
    s32 timer;
    s32 updateCount;
    s32 groundHeight;
    s32 updatesRemaining;

    FlyingObject(Vec *position);
    ~FlyingObject();
    void fn_1_758();
    void fn_1_7F0();
    void fn_1_8B4();
    void fn_1_BA8();
    void fn_1_BF0();
    void fn_1_E00();
    void fn_1_E4C();
};

extern "C" ListRoot<FlyingObject> lbl_1_bss_18;

void fn_1_984(FlyingObject *object, u8 stopped);
void fn_1_AB8(FlyingObject *object);

#endif
