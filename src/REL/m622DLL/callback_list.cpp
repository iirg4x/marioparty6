#include "REL/m622DLL/root_list.h"

class VirtualNode : public Node {
public:
    virtual ~VirtualNode();
    virtual void virtualAt0C(int value) = 0;
    virtual void virtualAt10();
    int flags;
    VirtualNode *next_node() { return static_cast<VirtualNode *>(link0); }
};

inline VirtualNode *LinkRoot1AB40::front()
{
    return static_cast<VirtualNode *>(first_node());
}

extern LinkRoot1AB40 lbl_1_bss_11C;
extern "C" unsigned long lbl_1_bss_124;

void fn_1_E348(int bit)
{
    VirtualNode *node = lbl_1_bss_11C.front();
    Node *end = lbl_1_bss_11C.end_node();
    lbl_1_bss_124 = bit;
    for (; node != end; node = node->next_node()) {
        node->flags |= 1 << bit;
        node->virtualAt0C(bit);
    }
}

bool fn_1_E3E0()
{
    VirtualNode *node = lbl_1_bss_11C.front();
    Node *end = lbl_1_bss_11C.end_node();
    for (; node != end; node = node->next_node()) {
        unsigned long bit = lbl_1_bss_124;
        if ((node->flags & (1 << bit)) != 0) {
            return false;
        }
    }
    return true;
}

void fn_1_E46C()
{
    VirtualNode *node = lbl_1_bss_11C.front();
    Node *end = lbl_1_bss_11C.end_node();
    for (; node != end; node = node->next_node()) {
        node->virtualAt10();
    }
}

inline VirtualNode *LinkRoot1AB40::back()
{
    return static_cast<VirtualNode *>(last_node());
}

void fn_1_E4DC()
{
    while (!lbl_1_bss_11C.empty()) {
        delete lbl_1_bss_11C.back();
    }
}

int fn_1_E5A4()
{
    return lbl_1_bss_124;
}

LinkRoot1AB40 lbl_1_bss_11C;
unsigned long lbl_1_bss_124;
