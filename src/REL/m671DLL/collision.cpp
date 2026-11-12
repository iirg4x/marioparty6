#include "REL/m671DLL/animation_owner.h"
#include "REL/framework/model.h"
#include "REL/framework/sound.h"
/* Collision shapes: every shape built registers itself in one module-wide list, and each frame the
 * list and the players are pushed apart. */
#include <vector>

#include "dolphin/types.h"
#include "dolphin/mtx.h"
extern "C" {
#include "game/charman.h"
}

#include "REL/framework/game_object_man.h"
#include "REL/framework/event_manager.h"
#include "REL/m671DLL/collision.h"

std::vector<CollisionShape *> lbl_1_bss_C;

s32 fn_1_0(CollisionObject *first, CollisionObject *second)
{
    Vec firstPosition = first->value;
    Vec secondPosition = second->value;
    Vec difference;
    f32 firstRadiusForComparison;
    f32 secondRadiusForComparison;
    f32 distanceCopy;
    f32 firstRadius;
    f32 secondRadius;
    f32 distance;

    firstRadius = first->radius;
    firstRadiusForComparison = firstRadius;
    secondRadius = second->radius;
    secondRadiusForComparison = secondRadius;
    PSVECSubtract(&firstPosition, &secondPosition, &difference);
    if (difference.x == 0.0f && difference.z == 0.0f) {
        return 0;
    }
    distance = PSVECMag(&difference);
    distanceCopy = distance;
    if (distanceCopy >= firstRadiusForComparison + secondRadiusForComparison) {
        return 0;
    }
    return 1;
}

static inline s32 spheresIntersect(CollisionObject *first, CollisionObject *second)
{
    Vec firstPosition = first->value;
    Vec secondPosition = second->value;
    Vec difference;
    f32 distance;
    f32 secondRadius;
    f32 firstRadius;
    f32 firstRadiusForComparison;
    f32 secondRadiusForComparison;
    f32 distanceCopy;

    firstRadius = first->radius;
    firstRadiusForComparison = firstRadius;
    secondRadius = second->radius;
    secondRadiusForComparison = secondRadius;
    PSVECSubtract(&firstPosition, &secondPosition, &difference);
    if (difference.x == 0.0f && difference.z == 0.0f) {
        return 0;
    }
    distance = PSVECMag(&difference);
    distanceCopy = distance;
    if (distanceCopy >= firstRadiusForComparison + secondRadiusForComparison) {
        return 0;
    }
    return 1;
}

void fn_1_148(void)
{
    s32 count = lbl_1_bss_C.size();
    s32 firstRelation;
    s32 secondRelation;
    s32 firstIndex;

    for (firstIndex = 0; firstIndex < count; firstIndex++) {
        CollisionObject *first = (CollisionObject *)lbl_1_bss_C.at(firstIndex);
        s32 secondIndex;
        firstRelation = first->getRelation();
        if (first->isActive() == 0) {
            continue;
        }
        if (first->getState() != -1) {
            continue;
        }
        if (firstRelation != 0) {
            continue;
        }

        for (secondIndex = 0; secondIndex < count; secondIndex++) {
            CollisionObject *second;

            if (firstIndex == secondIndex) {
                continue;
            }
            second = (CollisionObject *)lbl_1_bss_C.at(secondIndex);
            secondRelation = second->getRelation();
            if (secondRelation != 1) {
                continue;
            }
            if (second->isActive() == 0) {
                continue;
            }

            if ((u8)spheresIntersect(second, first) == 0) {
                continue;
            }
            first->state = secondRelation;
            second->state = firstRelation;
            break;
        }
    }
}

static inline f32 VectorDot(Vec *normal, Vec *point)
{
    return PSVECDotProduct(normal, point);
}

void fn_1_360(Vec *normal, Vec *point)
{
    float scale;
    Vec scaledNormal;

    scale = -2.0f * VectorDot(normal, point);
    PSVECScale(normal, &scaledNormal, scale);
    PSVECAdd(point, &scaledNormal, &scaledNormal);
    point->x = scaledNormal.x;
    point->y = scaledNormal.y;
    point->z = scaledNormal.z;
}

extern void fn_1_5754(AnimationOwner *, const Vec *);
extern void fn_1_5F00(AnimationOwner *, const Vec *);

static inline Vec ObjectPosition(AnimationOwner *object)
{
    return object->position.value;
}

static inline Vec ObjectVelocity(AnimationOwner *object)
{
    return object->previousPosition.value;
}

static inline u8 ObjectState(AnimationOwner *object)
{
    return object->position.active;
}

static inline u8 ObjectFlag(AnimationOwner *object)
{
    return object->preserveSpeed;
}

static inline bool ObjectActive(AnimationOwner *object)
{
    return ObjectState(object) == (u8)1;
}

static inline f32 VectorLength(Vec *vector)
{
    return PSVECMag(vector);
}

void fn_1_40C(void)
{
    std::vector<GameObject *> list;
    u8 processed[4];
    s32 count;
    s32 processedIndex;
    s32 firstIndex;

    collectGameObjects(1, &list);
    count = list.size();
    for (processedIndex = 0; processedIndex < count; processedIndex++) {
        processed[processedIndex] = 0;
    }

    for (firstIndex = 0; firstIndex < count; firstIndex++) {
        AnimationOwner *first;

        first = (AnimationOwner *)list[firstIndex];
        if (processed[firstIndex] == 1) {
            continue;
        }
        if (ObjectActive(first) != false) {
            s32 secondIndex;
            Vec firstPosition = ObjectPosition(first);
            Vec firstVelocity = ObjectVelocity(first);

            firstVelocity.y = 0.0f;
            for (secondIndex = 0; secondIndex < count; secondIndex++) {
                AnimationOwner *second;

                if (firstIndex == secondIndex) {
                    continue;
                }
                second = (AnimationOwner *)list[secondIndex];
                if (ObjectActive(second) == false) {
                    continue;
                }
                Vec secondPosition = ObjectPosition(second);
                Vec secondVelocity = ObjectVelocity(second);
                Vec pushSecond;
                Vec opposingPush;
                Vec pushFirst;
                Vec difference;
                f32 distance;
                secondVelocity.y = 0.0f;
                PSVECSubtract(&firstPosition, &secondPosition, &difference);
                distance = VectorLength(&difference);
                if (distance >= 200.0f) {
                    continue;
                }
                processed[secondIndex] = 1;
                PSVECScale(&difference, &pushFirst, (200.0f - distance) / distance);
                PSVECScale(&pushFirst, &pushSecond, 0.55f);
                PSVECScale(&pushFirst, &opposingPush, -0.55f);
                PSVECAdd(&pushSecond, &firstPosition, &firstPosition);
                PSVECAdd(&opposingPush, &secondPosition, &secondPosition);
                if (difference.x == 0.0f && difference.z == 0.0f) {
                    difference.z = 1.0f;
                }
                Vec normal = difference;
                normal.y = 0.0f;
                PSVECNormalize(&normal, &normal);
                if (firstPosition.y <= 100.0f && secondPosition.y <= 100.0f) {
                    if (ObjectFlag(first) == 0) {
                        PSVECNormalize(&firstVelocity, &firstVelocity);
                        PSVECScale(&firstVelocity, &firstVelocity, 6.6666665f);
                    }
                    if (ObjectFlag(second) == 0) {
                        PSVECNormalize(&secondVelocity, &secondVelocity);
                        PSVECScale(&secondVelocity, &secondVelocity, 6.6666665f);
                    }
                    fn_1_360(&normal, &firstVelocity);
                    Vec reflectedSecond = normal;
                    PSVECScale(&reflectedSecond, &reflectedSecond, -1.0f);
                    fn_1_360(&reflectedSecond, &secondVelocity);
                } else {
                    PSVECScale(&normal, &firstVelocity, 6.6666665f);
                    PSVECScale(&normal, &secondVelocity, -6.6666665f);
                }
                fn_1_5F00(first, &firstVelocity);
                fn_1_5F00(second, &secondVelocity);
                fn_1_5754(first, &firstPosition);
                fn_1_5754(second, &secondPosition);
                Vec effectPosition = difference;
                PSVECScale(&effectPosition, &effectPosition, -0.5f);
                Vec effectDirection = effectPosition;
                PSVECAdd(&firstPosition, &effectPosition, &effectPosition);
                PSVECNormalize(&effectDirection, &effectDirection);
                {
                    f64 tangentX;
                    f64 tangentZ;
                    tangentZ = -effectDirection.x;
                    tangentX = effectDirection.z;
                    effectDirection.x = tangentX;
                    effectDirection.y = 0.0f;
                    effectDirection.z = tangentZ;
                }
                fn_8005D864(1, &effectPosition, &effectDirection);
                soundPlayAt(1, 0x89E, &effectPosition);
            }
        }
    }
}

void fn_1_C58(void)
{
    fn_1_148();
    fn_1_40C();
}

CollisionObject::CollisionObject(float radius) : radius(radius)
{
    lbl_1_bss_C.push_back(this);
}
