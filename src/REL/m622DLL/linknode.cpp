#include "REL/m622DLL/framework/linknode.h"

ChainNode *fn_1_DBA0(NodePredicate, ChainNode *);
ChainNode *fn_1_E248(ChainNode *);
Node *fn_1_E29C(NodePredicate, Node *, Node *);

void Node::fn_1_DAE8(NodePredicate predicate)
{
    bool isEmpty = false;
    if (link0 == this) {
        if (link0 == link4) isEmpty = true;
    }
    if (!isEmpty) {
        link4->link0 = 0;
        Node *first = static_cast<Node *>(fn_1_DBA0(predicate, link0));
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

ChainNode *fn_1_DBA0(NodePredicate predicate, ChainNode *list)
{
    if (list == 0) return 0;
    if (static_cast<NodeLinks *>(list)->link0 == 0) return list;
    ChainNode *first = list;
    ChainNode *second = fn_1_E248(first);
    first = fn_1_DBA0(predicate, first);
    second = fn_1_DBA0(predicate, second);
    return fn_1_E29C(predicate, static_cast<Node *>(first), static_cast<Node *>(second));
}

ChainNode *fn_1_E248(ChainNode *node)
{
    ChainNode *fast = node->link0;
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

Node *fn_1_E29C(NodePredicate predicate, Node *first, Node *second)
{
    NodeLinks head;
    Node **tail = &head.link0;
    while (first != 0 && second != 0) {
        if (predicate(first, second)) {
            *tail = first; tail = &first->link0; first = first->link0;
        } else {
            *tail = second; tail = &second->link0; second = second->link0;
        }
    }
    if (first == 0) *tail = second;
    else *tail = first;
    return head.link0;
}
