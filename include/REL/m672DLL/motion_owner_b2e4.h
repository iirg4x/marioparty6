#ifndef REL_M672DLL_MOTION_OWNER_B2E4_H
#define REL_M672DLL_MOTION_OWNER_B2E4_H
#include "REL/framework/game_object.h"
#include "REL/m672DLL/position_bf64.h"
class OwnedModel;
struct VirtualPrefix { unsigned long unknown00,unknown04; };
typedef WorldPosition PositionB2E4;

struct VirtualPointee : VirtualPrefix { virtual ~VirtualPointee(); };
struct ChildPointer {
    VirtualPointee *value;
    ~ChildPointer();
    ChildPointer &operator=(VirtualPointee *next) {
        if (value != next) { delete value; value = next; }
        return *this;
    }
};
struct Owner {
    OwnedModel *value;
    ~Owner();
};
struct ChainTarget;
struct HangingCage : GameObject {
    PositionB2E4 position;
    Owner motion;
    Owner secondModel;
    ChildPointer child;
    f32 angle;
    f32 rotation;
    u32 frame;
    f32 fade;
    HangingCage(s32 type, const PositionB2E4 *position);
    virtual ~HangingCage();
    virtual void update();
    void *getModel() const { return motion.value; }
    ChainTarget *getTarget() const { return (ChainTarget *)child.value; }
};

#endif
