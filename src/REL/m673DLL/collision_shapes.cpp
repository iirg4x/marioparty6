// Registers collision bodies and tests active pairs for overlap.

#include "humath.h"
#include <vector>
#include "REL/m673DLL/collision_body.h"
#include "REL/framework/vector3.h"
#include "REL/framework/game_object_man.h"

// Holds the collision bodies registered for contact checks.
std::vector<CollisionBody *> collisionShapes;

// Rejects identical X/Z origins. Otherwise the distance must be less than the sum of radii.
bool fn_1_0(CollisionBody *first, CollisionBody *second)
{
    Position3 firstPosition = first->position;
    Position3 secondPosition = second->position;
    Vector3 difference;
    f32 firstRadius = first->getRadius();
    f32 secondRadius = second->getRadius();
    HuSubVecF(&difference, &firstPosition.value, &secondPosition.value);
    if (difference.x == 0.0f && difference.z == 0.0f) {
        return false;
    }
    f32 distance = difference.length();
    if (distance >= firstRadius + secondRadius) {
        return false;
    }
    return true;
}

// Checks active collision bodies and records their contacts.
void fn_1_148()
{
    s32 count = collisionShapes.size();
    s32 firstKind;
    s32 secondKind;
    for (s32 i = 0; i < count; ++i) {
        CollisionBody *first = collisionShapes.at(i);
        firstKind = first->getRelation();
        if (first->isActive() == 0) {
            continue;
        }
        if (first->getContactRelation() != -1) {
            continue;
        }
        if (firstKind != 0) {
            continue;
        }
        for (s32 j = 0; j < count; ++j) {
            if (i == j) {
                continue;
            }
            CollisionBody *second = collisionShapes.at(j);
            secondKind = second->getRelation();
            if (secondKind != 1 && secondKind != 2) {
                continue;
            }
            if (second->isActive() == 0) {
                continue;
            }
            if (!fn_1_0(second, first)) {
                continue;
            }
            first->contactRelation = secondKind;
            second->contactRelation = firstKind;
            break;
        }
    }
}

// Runs the collision checks for this frame.
void fn_1_368()
{
    fn_1_148();
}

// Creates a collision body with the supplied radius.
CollisionBody::CollisionBody(f32 radius) : CollisionState(), radius(radius)
{
    collisionShapes.push_back(this);
}
