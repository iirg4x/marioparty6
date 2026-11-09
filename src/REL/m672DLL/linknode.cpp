#include "REL/m672DLL/linknode.h"

NodeLinks *fn_1_1D2AC(NodePredicate, NodeLinks *);

void Node::sort(NodePredicate predicate) {
    bool isEmpty = false;
    if (link0 == this) {
        if (link0 == link4) {
            isEmpty = true;
        }
    }

    if (!isEmpty) {
        link4->link0 = 0;
        Node *first = static_cast<Node *>(fn_1_1D2AC(predicate, link0));
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

NodeLinks *fn_1_1D954(NodeLinks *);
Node *fn_1_1D9A8(NodePredicate, Node *, Node *);
NodeLinks *fn_1_1D2AC(NodePredicate predicate, NodeLinks *list)
{
    if (list == 0) return 0;
    if (static_cast<Node *>(list)->link0 == 0) return list;
    NodeLinks *first = list;
    NodeLinks *second = fn_1_1D954(first);
    first = fn_1_1D2AC(predicate, first);
    second = fn_1_1D2AC(predicate, second);
    return fn_1_1D9A8(predicate, static_cast<Node *>(first), static_cast<Node *>(second));
}
NodeLinks *fn_1_1D954(NodeLinks *node)
{
    NodeLinks *fast = node->link0;
    if (fast) fast = fast->link0;
    while (fast) {
        node = node->link0;
        fast = fast->link0;
        if (fast) fast = fast->link0;
    }
    fast = node->link0;
    node->link0 = 0;
    return fast;
}
Node *fn_1_1D9A8(NodePredicate predicate, Node *first, Node *second)
{
    NodeLinks head;
    Node **tail = &head.link0;
    while (first != 0 && second != 0) {
        if (predicate(first, second)) {
            *tail = first;
            tail = &first->link0;
            first = first->link0;
        } else {
            *tail = second;
            tail = &second->link0;
            second = second->link0;
        }
    }
    if (first == 0) *tail = second;
    else *tail = first;
    return head.link0;
}
