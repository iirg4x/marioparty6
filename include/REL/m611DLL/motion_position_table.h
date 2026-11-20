#ifndef REL_M611DLL_MOTION_POSITION_TABLE_H
#define REL_M611DLL_MOTION_POSITION_TABLE_H
#include "REL/framework/list_object.h"
#include "REL/framework/model.h"
#include "REL/framework/singleton.h"
#include "dolphin/mtx.h"
#include "REL/framework/random.h"
// Chooses a random position on the horizontal plane.
inline void randomizeMotionPosition(Vec *position)
{
    position->x = 100.0f * (30.0f * (RandomFloat() - 0.5f));
    position->y = 0.0f;
    position->z = 100.0f * (30.0f * (RandomFloat() - 0.5f));
}
// Owns the three scene models and their positions and motion frames.
struct MotionPositionTable : RequestObject, MinigameSingleton<MotionPositionTable> {
    // Day or night model resources for slots 0, 1, and 2.
    OwnedModel slot0Model;
    OwnedModel slot1Model;
    OwnedModel slot2Model;
    Vec positions[3];
    s32 motionFrames[3];
    MotionPositionTable(bool dayMode);
    virtual ~MotionPositionTable();
    virtual void flagEnabled(int);
    virtual void update();
};

#endif
