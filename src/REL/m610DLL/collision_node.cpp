/* Collision-node registration and pair reporting. */
#include <vector>
#include "REL/framework/unregister_value.h"
#include "REL/m610DLL/scene_callback.h"
#include "REL/m610DLL/collision_node.h"

extern "C"
{
    /* Head of the active collision-node list. */
    CollisionNode *lbl_1_bss_18;
}

/* Register an active collision sphere for its owner and category. */
CollisionNode::CollisionNode(s32 nodeCategory, void *nodeOwner)
{
    category = nodeCategory;
    owner = nodeOwner;
    active = 1;
    next = lbl_1_bss_18;
    lbl_1_bss_18 = this;
    radius = 1.0f;
}

/* Unlink this sphere from the live collision list. */
CollisionNode::~CollisionNode()
{
    unlinkEntry(this, &lbl_1_bss_18);
}

/* Report overlapping collision-node pairs through the supplied callback. */
void fn_1_B914(SceneContactCallback callback)
{
    std::vector<CollisionNode *> matches;
    CollisionNode *node;
    s32 firstIndex;
    s32 secondIndex;
    s32 limit;
    CollisionNode *first;
    CollisionNode *second;

    node = lbl_1_bss_18;
    while (node)
    {
        if (node->active)
        {
            matches.push_back(node);
        }
        node = node->next;
    }

    limit = matches.size();
    firstIndex = 0;
    while (firstIndex < limit)
    {
        secondIndex = 0;
        while (secondIndex < limit)
        {
            if (secondIndex > firstIndex)
            {
                first = matches[firstIndex];
                second = matches[secondIndex];
                if (spheresOverlap(first, second))
                {
                    if (first->category < second->category)
                    {
                        callback(first->category, first->owner, second->category, second->owner);
                    }
                    else
                    {
                        callback(second->category, second->owner, first->category, first->owner);
                    }
                }
            }
            ++secondIndex;
        }
        ++firstIndex;
    }
}

/* Disable this sphere in collision-pair reporting. */
void CollisionNode::fn_1_BBC0()
{
    active = 0;
}
/* Enable this sphere in collision-pair reporting. */
void CollisionNode::fn_1_BBCC()
{
    active = 1;
}
