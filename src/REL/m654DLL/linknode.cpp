// Circular links, merge sorting and completion flags for registered objects.
#include "REL/m654DLL/flag_nodes.h"

typedef bool (*SortPredicate)(void *, void *);
void *fn_1_18A18(SortPredicate, void *);
void *fn_1_190C0(void *);
void *fn_1_19114(SortPredicate, void *, void *);

void Node::fn_1_18960(SortPredicate predicate)
{
    bool isEmpty = false;
    if (link0 == this) {
        if (link0 == link4) isEmpty = true;
    }
    if (!isEmpty) {
        link4->link0 = 0;
        Node *first = static_cast<Node *>(fn_1_18A18(predicate, link0));
        link0 = first;
        Node *last = this;
        while (first != 0) {
            first->link4 = last;
            last = first;
            first = first->link0;
        }
        link4 = last;
        last->link0 = this;
    }
}

void *fn_1_18A18(SortPredicate predicate, void *list)
{
    if (list == 0) return 0;
    if (static_cast<NodeLinks *>(list)->link0 == 0) return list;
    void *first = list;
    void *second = fn_1_190C0(first);
    first = fn_1_18A18(predicate, first);
    second = fn_1_18A18(predicate, second);
    return fn_1_19114(predicate, first, second);
}

void *fn_1_190C0(void *node)
{
    void *fast = static_cast<NodeLinks *>(node)->link0;
    if (fast) fast = static_cast<NodeLinks *>(fast)->link0;
    while (fast) {
        node = static_cast<NodeLinks *>(node)->link0;
        fast = static_cast<NodeLinks *>(fast)->link0;
        if (fast) fast = static_cast<NodeLinks *>(fast)->link0;
    }
    fast = static_cast<NodeLinks *>(node)->link0;
    static_cast<NodeLinks *>(node)->link0 = 0;
    return fast;
}

void *fn_1_19114(SortPredicate predicate, void *first, void *second)
{
    NodeLinks head;
    Node **tail = &head.link0;
    while (first != 0 && second != 0) {
        if (predicate(first, second)) {
            *tail = static_cast<Node *>(first);
            tail = &static_cast<NodeLinks *>(first)->link0;
            first = static_cast<NodeLinks *>(first)->link0;
        } else {
            *tail = static_cast<Node *>(second);
            tail = &static_cast<NodeLinks *>(second)->link0;
            second = static_cast<NodeLinks *>(second)->link0;
        }
    }
    if (first == 0) *tail = static_cast<Node *>(second);
    else *tail = static_cast<Node *>(first);
    return head.link0;
}

void fn_1_191C0(int bit)
{
    FlagNode *node = lbl_1_bss_10C.front();
    Node *end = lbl_1_bss_10C.end_node();
    lbl_1_bss_114 = bit;
    for (; node != end; node = node->next_node()) {
        node->flags |= 1U << bit;
        node->flagEnabled(bit);
    }
}

bool fn_1_19258()
{
    FlagNode *node = lbl_1_bss_10C.front();
    Node *end = lbl_1_bss_10C.end_node();
    for (; node != end; node = node->next_node()) {
        unsigned long bit = lbl_1_bss_114;
        if ((node->flags & (1 << bit)) != 0) return false;
    }
    return true;
}

void fn_1_192E4()
{
    FlagNode *node = lbl_1_bss_10C.front();
    Node *end = lbl_1_bss_10C.end_node();
    for (; node != end; node = node->next_node()) node->update();
}

void fn_1_19354()
{
    while (!lbl_1_bss_10C.empty()) {
        delete lbl_1_bss_10C.back();
    }
}

int fn_1_1941C() { return lbl_1_bss_114; }


FlagNodeList lbl_1_bss_10C;
unsigned long lbl_1_bss_114;
