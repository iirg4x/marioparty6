#ifndef REL_M628DLL_RANGE_RECORD_H
#define REL_M628DLL_RANGE_RECORD_H
#include "dolphin/mtx.h"
#include <vector>
#include "REL/framework/vector3.h"
// Stores the position, classification and last hit of a collision record.
struct RangeRecordBase
{
    Vector3 position;
    s32 shapeType;
    s32 hitState;
    MtxPtr matrix;
    s32 collisionClass;
    u8 active;
    // Clears the hit state and starts this collision record inactive.
    RangeRecordBase(s32 shapeTypeValue = 0)
    {
        shapeType = shapeTypeValue;
        hitState = -1;
        collisionClass = -1;
        active = 0;
    }
    // Destroys this RangeRecordBase and its members.
    virtual ~RangeRecordBase() {}
    // Returns the collision shape type.
    s32 getType() const
    {
        return shapeType;
    }
    // Returns the collision class.
    s32 getClass() const
    {
        return collisionClass;
    }
    // Returns the collision orientation matrix.
    MtxPtr getOrientation() const
    {
        return matrix;
    }
    // Returns the last collision hit state.
    s32 getHitState() const
    {
        return hitState;
    }
    // Returns whether collision testing is enabled.
    u8 isActive()
    {
        return active;
    }
    // Returns the stored position.
    Vec &getPosition()
    {
        return position;
    }
    // Copies the supplied position into the stored vector.
    Vec &setPosition(const Vec &value)
    {
        Vec &destination = position;
        destination = value;
        return getPosition();
    }
};

extern std::vector<RangeRecordBase *> lbl_1_bss_34;

// Registers a spherical collision record with its radius.
struct ActorRangeRecord : RangeRecordBase
{
    f32 radius;
    // Sets the sphere radius and registers this collision record.
    explicit ActorRangeRecord(f32 value = 50.0f) : RangeRecordBase(1)
    {
        radius = value;
        lbl_1_bss_34.push_back(this);
    }
    // Destroys this ActorRangeRecord and its members.
    virtual ~ActorRangeRecord() {}
    // Returns the collision radius.
    f32 getRadius() const
    {
        return radius;
    }
};

void fn_1_47FC();
#endif
