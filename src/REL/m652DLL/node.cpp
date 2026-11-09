#include "REL/m652DLL/framework/linknode.h"

#include "REL/m652DLL/node_owner.h"

extern "C" {
FlagNodeList lbl_1_bss_EC;
unsigned long lbl_1_bss_F4;
}

void fn_1_17F20(int bit)
{
    FlagNode *node = lbl_1_bss_EC.front();
    Node *end = lbl_1_bss_EC.end_node();
    lbl_1_bss_F4 = bit;
    for (; node != end; node = node->next_node()) {
        node->flags |= 1U << bit;
        node->flagEnabled(bit);
    }
}

bool fn_1_17FB8()
{
    FlagNode *node = lbl_1_bss_EC.front();
    Node *end = lbl_1_bss_EC.end_node();
    for (; node != end; node = node->next_node()) {
        unsigned long bit = lbl_1_bss_F4;
        if ((node->flags & (1 << bit)) != 0) {
            return false;
        }
    }
    return true;
}

void fn_1_18044()
{
    FlagNode *node = lbl_1_bss_EC.front();
    Node *end = lbl_1_bss_EC.end_node();
    for (; node != end; node = node->next_node()) {
        node->update();
    }
}

void fn_1_180B4() {
    while (!lbl_1_bss_EC.empty()) {
        VirtualNode *node = lbl_1_bss_EC.back();
        if (node != 0) {
            node = node->virtualAt0C(1);
        }
    }
}
signed long fn_1_1817C() { return lbl_1_bss_F4; }
