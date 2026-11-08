#include "REL/m648DLL/framework/linknode.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/m648DLL.h"

struct VirtualNode : Node {
    virtual ~VirtualNode();
    virtual void virtualAt0C(int value) = 0;
    virtual void virtualAt10();
    int flags;
    VirtualNode *next_node() { return static_cast<VirtualNode *>(link0); }
};

struct RequestListRoot : Node {
    RequestListRoot() {}
    ~RequestListRoot() {}
    bool empty() const { return link0 == this && link0 == link4; }
    Node *last_node() { return link4; }
    VirtualNode *back() { return static_cast<VirtualNode *>(last_node()); }
    Node *first_node() { return link0; }
    Node *end_node() { return this; }
    VirtualNode *front() { return static_cast<VirtualNode *>(first_node()); }
};

extern "C" { RequestListRoot lbl_1_bss_63C; }
extern "C" { int lbl_1_bss_644; }

void fn_1_1C6D0(int bit)
{
    VirtualNode *node = lbl_1_bss_63C.front();
    Node *end = lbl_1_bss_63C.end_node();
    lbl_1_bss_644 = bit;
    for (; node != end; node = node->next_node()) {
        node->flags |= 1U << bit;
        node->virtualAt0C(bit);
    }
}

bool fn_1_1C768() {
    VirtualNode *node = lbl_1_bss_63C.front();
    Node *end = lbl_1_bss_63C.end_node();
    for (; node != end; node = node->next_node()) {
        unsigned long bit = lbl_1_bss_644;
        if ((node->flags & (1 << bit)) != 0) {
            return false;
        }
    }
    return true;
}

void fn_1_1C7F4() {
    VirtualNode *node = lbl_1_bss_63C.front();
    Node *end = lbl_1_bss_63C.end_node();
    for (; node != end; node = node->next_node()) {
        node->virtualAt10();
    }
}

void fn_1_1C864() {
    while (!lbl_1_bss_63C.empty()) delete lbl_1_bss_63C.back();
}

s32 fn_1_1C92C(void) { return lbl_1_bss_644; }
