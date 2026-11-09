#include "REL/m603DLL/linknode.h"

typedef bool (*Predicate)(Node *, Node *);
NodeLinks *fn_1_1A12C(Predicate, NodeLinks *);

void Node::fn_1_1A074(Predicate predicate)
{
    bool isEmpty = false;
    if (link0 == this) {
        if (link0 == link4) isEmpty = true;
    }
    if (!isEmpty) {
        link4->link0 = 0;
        Node *first = static_cast<Node *>(fn_1_1A12C(predicate, link0));
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
