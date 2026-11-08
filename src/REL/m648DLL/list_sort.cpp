#include "REL/m648DLL/framework/linknode.h"
/* Target-consumed records, not claims of original class names or complete layouts. */

extern "C" {
#include "game/hu3d.h"
#include "game/hsfex.h"
#include "game/charman.h"
#include "game/mg/actman.h"
#include "game/mg/timer.h"
#include "dolphin/gx.h"
#include "dolphin/os.h"
}

#include "REL/m648DLL/framework/model.h"
#include "REL/m648DLL/framework/thread.h"
#include "REL/m648DLL/motion_lists.h"



typedef bool (*Predicate)(Node *, Node *);
NodeLinks *fn_1_1BF28(Predicate, NodeLinks *);
NodeLinks *fn_1_1C5D0(NodeLinks *);
Node *fn_1_1C624(Predicate, Node *, Node *);

void Node::fn_1_1BE70(Predicate predicate) {
    bool isEmpty = false;
    if (link0 == this) {
        if (link0 == link4) {
            isEmpty = true;
        }
    }

    if (!isEmpty) {
        link4->link0 = 0;
        Node *first = static_cast<Node *>(fn_1_1BF28(predicate, link0));
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

NodeLinks *fn_1_1BF28(Predicate predicate, NodeLinks *list)
{
    if (list == 0) return 0;
    if (static_cast<Node *>(list)->link0 == 0) return list;
    NodeLinks *first = list;
    NodeLinks *second = fn_1_1C5D0(first);
    first = fn_1_1BF28(predicate, first);
    second = fn_1_1BF28(predicate, second);
    return fn_1_1C624(predicate, static_cast<Node *>(first), static_cast<Node *>(second));
}

NodeLinks *fn_1_1C5D0(NodeLinks *node) {
    NodeLinks *fast=node->link0; if(fast) fast=fast->link0;
    while(fast) { node=node->link0; fast=fast->link0; if(fast) fast=fast->link0; }
    fast=node->link0; node->link0=0; return fast;
}

Node *fn_1_1C624(Predicate predicate, Node *first, Node *second) {
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
    if (first == 0)
        *tail = second;
    else
        *tail = first;
    return head.link0;
}
