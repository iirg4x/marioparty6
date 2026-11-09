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

#include "REL/m603DLL/model.h"
#include "REL/framework/thread.h"

void operator delete(void *);

#include "REL/framework/linknode.h"

typedef bool (*Predicate)(Node *, Node *);
NodeLinks *fn_1_1A12C(Predicate, NodeLinks *);
NodeLinks *fn_1_1A7D4(NodeLinks *);
Node *fn_1_1A828(Predicate, Node *, Node *);

NodeLinks *fn_1_1A12C(Predicate predicate, NodeLinks *list)
{
    if (list == 0) return 0;
    if (static_cast<Node *>(list)->link0 == 0) return list;
    NodeLinks *first = list;
    NodeLinks *second = fn_1_1A7D4(first);
    first = fn_1_1A12C(predicate, first);
    second = fn_1_1A12C(predicate, second);
    return fn_1_1A828(predicate, static_cast<Node *>(first), static_cast<Node *>(second));
}

NodeLinks *fn_1_1A7D4(NodeLinks *node) {
    NodeLinks *fast=node->link0; if(fast) fast=fast->link0;
    while(fast) { node=node->link0; fast=fast->link0; if(fast) fast=fast->link0; }
    fast=node->link0; node->link0=0; return fast;
}

Node *fn_1_1A828(Predicate predicate, Node *first, Node *second) {
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
