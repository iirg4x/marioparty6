#ifndef REL_M652_NODE_OWNER_H
#define REL_M652_NODE_OWNER_H

#include "REL/m652DLL/framework/linknode.h"

struct FlagNodeLink : Node {
    ~FlagNodeLink() {}
};

struct FlagNode : FlagNodeLink {
    FlagNode();
    virtual ~FlagNode() {}
    virtual void flagEnabled(int bit) = 0;
    virtual void update() {}
    int flags;
    FlagNode *next_node() { return static_cast<FlagNode *>(link0); }
};
struct VirtualNode : Node {
    virtual VirtualNode *virtualAt0C(int value) = 0;
    virtual void virtualAt10();
};
struct FlagNodeList : Node {
    ~FlagNodeList() {}
    Node *first_node() { return link0; }
    Node *end_node() { return this; }
    FlagNode *front() { return static_cast<FlagNode *>(first_node()); }
    bool empty() const { return link0 == this && link0 == link4; }
    Node *last_node() { return link4; }
    VirtualNode *back() { return static_cast<VirtualNode *>(last_node()); }
};
extern "C" FlagNodeList lbl_1_bss_EC;
inline FlagNode::FlagNode() : flags(0) { insert4(&lbl_1_bss_EC); }
#endif
