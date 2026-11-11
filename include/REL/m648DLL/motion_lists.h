#ifndef REL_M648_MOTION_LISTS_H
#define REL_M648_MOTION_LISTS_H
#include "REL/framework/linknode.h"
#include "REL/framework/model.h"
void operator delete(void *);
struct NodeOwner : Node { unsigned char ownerState[0x28]; ~NodeOwner() {} };
typedef Model ReleasedModel;
struct OwnedReleasedModel : ReleasedModel { ~OwnedReleasedModel() {} };
struct DrawNode : NodeOwner, OwnedReleasedModel { ~DrawNode() {} };
struct DrawRoot : Node {
    bool empty() const { return link0 == this && link0 == link4; }
    Node *last_node() { return link4; }
    DrawNode *back() { return static_cast<DrawNode *>(last_node()); }
    Node *first_node() { return link0; }
    Node *end_node() { return this; }
    DrawNode *front() { return static_cast<DrawNode *>(first_node()); }
};
#endif
