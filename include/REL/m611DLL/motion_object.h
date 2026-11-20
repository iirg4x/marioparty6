#ifndef REL_M611DLL_MOTION_OBJECT_H
#define REL_M611DLL_MOTION_OBJECT_H
#include "dolphin/types.h"
#include "REL/framework/vector3.h"
#include "REL/framework/list.h"
#include "REL/framework/model.h"

// Owns a moving model and the callback and target values used to advance it.
struct MotionObject : ListNode<MotionObject> {
    void (MotionObject::*callback)();
    Vector3 position;
    Vec target;
    OwnedModel model;
    f32 heading;
    f32 speed;
    f32 targetSpeed;
    s32 duration;
    // The game's code never reads this field.
    s32 unk40;
    f32 turnRate;
    f32 targetTurnRate;

    MotionObject(const OwnedModel &modelSource, Vector3 *position);
    ~MotionObject() {}
    void fn_1_25A0();
    void fn_1_23F0();
};

void fn_1_246C(MotionObject *object);
void fn_1_27AC(MotionObject *object);

// Live moving-model objects.
extern ListRoot<MotionObject> lbl_1_bss_78;

// Updates the moving model.
void fn_1_23B0(MotionObject *object);

#endif
