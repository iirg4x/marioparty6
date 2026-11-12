/* The collision list: spheres link themselves in, and each frame every overlapping active pair is reported. */
#include <vector>
#include "REL/m653DLL/collision_sphere.h"

CollisionSphere::CollisionSphere(s32 category, void *owner) : category(category), owner(owner), active(1)
{
    next = lbl_1_bss_30;
    lbl_1_bss_30 = this;
    radius = 1.0f;
}

CollisionSphere::~CollisionSphere()
{
    CollisionSphere **link = &lbl_1_bss_30;
    while (*link != this) {
        link = &(*link)->next;
    }
    *link = next;
}

void fn_1_4EFC(CollisionCallback callback)
{
    std::vector<CollisionSphere *> matches;
    CollisionSphere *node = lbl_1_bss_30;
    while (node) {
        if (node->active) matches.push_back(node);
        node = node->next;
    }
    s32 limit = matches.size();
    for (s32 firstIndex = 0; firstIndex < limit; ++firstIndex) {
        for (s32 secondIndex = 0; secondIndex < limit; ++secondIndex) {
            if (secondIndex > firstIndex) {
                CollisionSphere *first = matches[firstIndex];
                CollisionSphere *second = matches[secondIndex];
                if (spheresOverlap(first, second)) {
                    if (first->category < second->category) {
                        callback(first->category, first->owner,
                                 second->category, second->owner);
                    } else {
                        callback(second->category, second->owner,
                                 first->category, first->owner);
                    }
                }
            }
        }
    }
}

void CollisionSphere::deactivate() { active = 0; }

void CollisionSphere::activate() { active = 1; }

extern "C" { CollisionSphere *lbl_1_bss_30; }
