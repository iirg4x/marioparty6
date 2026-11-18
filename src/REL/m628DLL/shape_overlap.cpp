// The list of every collision shape and the frame's overlap test over it.
#include "dolphin/types.h"
extern "C"
{
#include "dolphin/mtx.h"
}
#include <vector>

#include "REL/m628DLL/render_effect.h"
#include "REL/framework/vector3.h"

// Tests a sphere against an oriented cylindrical collision record.
s32 fn_1_4640(ActorRangeRecord *first, const RangeRecord *second)
{
    Mtx inverse;
    Vector3 firstPosition = first->position;
    Vector3 secondPosition = second->position;
    f32 firstRadius = first->getRadius();
    f32 secondRadius = second->getRadius();
    f32 halfHeight = second->getHalfHeight();
    f32 reach = firstRadius + halfHeight;
    MtxPtr firstOrientation = first->getOrientation();
    MtxPtr orientation = second->getOrientation();

    PSVECSubtract(&firstPosition, &secondPosition, &firstPosition);
    PSMTXTranspose(orientation, inverse);
    PSMTXMultVec(inverse, &firstPosition, &firstPosition);
    firstPosition.z -= 660.0f;
    if (firstPosition.z > reach)
    {
        return 0;
    }
    firstPosition.z = 0.0f;
    f32 distance = firstPosition.length();
    if (firstPosition.length() > firstRadius + secondRadius)
    {
        return 0;
    }
    return 1;
}

std::vector<RangeRecordBase *> lbl_1_bss_34;

// Tests whether two spherical collision records overlap.
static inline s32 sphericalRecordsOverlap(RangeRecordBase *first, RangeRecordBase *second)
{
    Vector3 firstPosition = first->position;
    Vector3 secondPosition = second->position;
    Vector3 difference;
    f32 firstRadius = static_cast<const ActorRangeRecord *>(first)->getRadius();
    f32 secondRadius = static_cast<const ActorRangeRecord *>(second)->getRadius();

    PSVECSubtract(&firstPosition, &secondPosition, &difference);
    if (difference.length() > firstRadius + secondRadius)
    {
        return 0;
    }
    return 1;
}

// Updates collision hit states for active records.
void fn_1_47FC(void)
{
    s32 checkedType;
    s32 otherType;
    s32 count = lbl_1_bss_34.size();
    s32 shapeIndex = 0;
    for (; shapeIndex < count; shapeIndex++)
    {
        RangeRecordBase *shape = lbl_1_bss_34.at(shapeIndex);
        u8 active = shape->active;
        if (active != 0)
        {
            shape->hitState = -1;
            checkedType = shape->getType();
            if (checkedType == 1)
            {
                s32 otherIndex = 0;
                for (; otherIndex < count; otherIndex++)
                {
                    RangeRecordBase *other = lbl_1_bss_34.at(otherIndex);
                    otherType = other->getType();
                    s32 checkedClass = other->getClass();
                    if ((shapeIndex != otherIndex) && (checkedClass == 0) && (otherType == 1))
                    {
                        if ((u8)sphericalRecordsOverlap(shape, other) != 0)
                        {
                            shape->hitState = 1;
                        }
                    }
                }
            }
        }
    }
    s32 ownerIndex = 0;
    for (; ownerIndex < count; ownerIndex++)
    {
        s32 initialState = -1;
        RangeRecordBase *owner = lbl_1_bss_34.at(ownerIndex);
        if (owner->isActive() != 0)
        {
            if (owner->getHitState() == -1)
            {
                owner->hitState = -1;
                s32 checkedType = owner->getType();
                if (checkedType == 1)
                {
                    s32 otherIndex = 0;
                    for (; otherIndex < count; otherIndex++)
                    {
                        RangeRecordBase *other = lbl_1_bss_34.at(otherIndex);
                        s32 otherType = other->getType();
                        if ((ownerIndex != otherIndex) && (otherType != 1))
                        {
                            if ((u8)fn_1_4640(static_cast<ActorRangeRecord *>(owner),
                                              static_cast<RangeRecord *>(other)) != 0)
                            {
                                owner->hitState = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}
