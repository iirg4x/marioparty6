#ifndef M648_PLAYER_ROOT_H
#define M648_PLAYER_ROOT_H
#include "REL/framework/linknode.h"
struct CallbackMotionOwner;
struct PlayerListRoot : Node {
    ~PlayerListRoot() {}
    Node *first_node() { return link0; }
    Node *end_node() { return this; }
    inline CallbackMotionOwner *begin();
    inline CallbackMotionOwner *front();
    bool empty() const { return link0 == this && link0 == link4; }
    Node *last_node() { return link4; }
    inline CallbackMotionOwner *back();
};
#endif
