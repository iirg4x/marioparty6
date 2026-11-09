#ifndef M654_FLAG_NODES_H
#define M654_FLAG_NODES_H
#include "REL/m654DLL/framework/linknode.h"
class FlagNode : public Node {
public:
    virtual ~FlagNode();
    virtual void flagEnabled(int) = 0;
    virtual void update();
    int flags;
    FlagNode *next_node() { return static_cast<FlagNode *>(link0); }
};

struct FlagNodeList : Node {
    FlagNodeList() {}
    ~FlagNodeList() {}
    Node *first_node() { return link0; }
    Node *end_node() { return this; }
    FlagNode *front() { return static_cast<FlagNode *>(first_node()); }
    bool empty() const { return link0 == this && link0 == link4; }
    Node *last_node() { return link4; }
    FlagNode *back() { return static_cast<FlagNode *>(last_node()); }
};

extern "C" FlagNodeList lbl_1_bss_10C;
extern "C" unsigned long lbl_1_bss_114;
#endif
