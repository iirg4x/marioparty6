#include "REL/framework/game_object_man.h"
#include "REL/m628DLL/act_player.h"

#include "REL/framework/game_object.h"

#include "REL/m628DLL/board_positions.h"

#include "REL/framework/event_hm.h"

#include <vector>

void fn_1_125DC(BoardPositions *owner);

// Clears the tracked positions, corner scores and player flags.
BoardPositions::BoardPositions() : GameObject(0x70)
{
    s32 primaryIndex;
    s32 secondaryIndex;
    f32 negativeX;
    f32 negativeZ;
    f32 positiveX;
    f32 positiveZ;

    for (primaryIndex = 0; primaryIndex < 4; ++primaryIndex)
    {
        static_cast<Vec *>(trackedPositions + primaryIndex)->x = 0.0f;
        static_cast<Vec *>(trackedPositions + primaryIndex)->y = 0.0f;
        static_cast<Vec *>(trackedPositions + primaryIndex)->z = 0.0f;
        lockedFlags[primaryIndex] = 0;
    }
    for (secondaryIndex = 0; secondaryIndex < 4; ++secondaryIndex)
    {
        static_cast<Vec *>(cornerPositions + secondaryIndex)->x = 0.0f;
        static_cast<Vec *>(cornerPositions + secondaryIndex)->y = 0.0f;
        static_cast<Vec *>(cornerPositions + secondaryIndex)->z = 0.0f;
        cornerStates[secondaryIndex] = 0;
    }
    unk50 = 0;
    negativeX = -340.0f;
    negativeZ = -340.0f;
    positiveX = -negativeX;
    positiveZ = -negativeZ;
    cornerPositions[0].x = negativeX;
    cornerPositions[0].z = negativeZ;
    cornerPositions[1].x = positiveX;
    cornerPositions[1].z = negativeZ;
    cornerPositions[2].x = negativeX;
    cornerPositions[2].z = positiveZ;
    cornerPositions[3].x = positiveX;
    cornerPositions[3].z = positiveZ;
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;
}

// Releases the tracked object list.
BoardPositions::~BoardPositions() {}

// Collects the player objects used by the board position tracker.
void fn_1_12474(BoardPositions *owner)
{
    std::vector<GameObject *> entries;
    s32 count;
    s32 index;

    collectGameObjects(0x40, &entries);
    count = entries.size();
    for (index = 0; index < count; index++)
    {
        owner->list.push_back(entries[index]);
    }
}

// Returns the tracked player object for a one-based selection.
ActPlayer *fn_1_12540(BoardPositions *object, s32 mode)
{
    s32 index = mode - 1;
    if (index >= 3)
    {
        return 0;
    }
    return static_cast<ActPlayer *>(object->list.at(index));
}

// Refreshes actor positions and their corner scores.
void BoardPositions::update()
{
    fn_1_12934(this);
    fn_1_125DC(this);
}

// Ranks the four corner areas from the first tracked position and classifies its velocity.
void fn_1_125DC(BoardPositions *owner)
{
    s32 ranks[4];
    f32 boundaryWidth;
    f32 halfWidth;
    s32 clearIndex;
    s32 rankIndex;
    s32 compareIndex;
    s32 updateIndex;

    for (clearIndex = 0; clearIndex < 4; clearIndex++)
    {
        ranks[clearIndex] = 0;
    }

    halfWidth = 400.0f;
    boundaryWidth = 2.0f * halfWidth;
    Vec sourcePosition = owner->trackedPositions[0];
    f32 areas[4];
    sourcePosition.x += halfWidth;
    sourcePosition.z += halfWidth;
    areas[0] = sourcePosition.x * sourcePosition.z;
    areas[1] = sourcePosition.z * (2.0f * halfWidth - sourcePosition.x);
    areas[2] = sourcePosition.x * (2.0f * halfWidth - sourcePosition.z);
    areas[3] = (2.0f * halfWidth - sourcePosition.x) * (2.0f * halfWidth - sourcePosition.z);

    for (rankIndex = 0; rankIndex < 4; rankIndex++)
    {
        for (compareIndex = 0; compareIndex < 4; compareIndex++)
        {
            if (rankIndex != compareIndex && areas[rankIndex] < areas[compareIndex])
            {
                ranks[rankIndex]++;
            }
        }
    }

    owner->unk50 = 0;
    for (updateIndex = 0; updateIndex < 4; updateIndex++)
    {
        if (owner->cornerStates[updateIndex] != ranks[updateIndex])
        {
            owner->unk50 = 1;
        }
        owner->cornerStates[updateIndex] = ranks[updateIndex];
    }

    Vec targetPosition = owner->velocity;
    if (targetPosition.x == 0.0f && targetPosition.y == 0.0f && targetPosition.z == 0.0f)
    {
        owner->unk54 = -1;
        return;
    }
    owner->unk54 = fn_1_1285C(owner, &targetPosition);
}

// Classifies a position by its signs on the board axes.
s32 fn_1_1285C(BoardPositions *context, const Vec *position)
{
    if (position->x < 0.0f)
    {
        if (position->z < 0.0f)
        {
            return 0;
        }
        return 2;
    }
    if (position->z < 0.0f)
    {
        return 1;
    }
    return 3;
}
