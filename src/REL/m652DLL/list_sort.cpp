
extern "C" {
#include "game/hu3d.h"
#include "game/hsfex.h"
#include "game/charman.h"
#include "game/mg/actman.h"
#include "game/mg/timer.h"
#include "dolphin/gx.h"
#include "dolphin/os.h"
}

#include "REL/m652DLL/framework/model.h"
#include "REL/m652DLL/framework/thread.h"

#include "REL/m652DLL/framework/linknode.h"

typedef bool (*Predicate)(Node *, Node *);
NodeLinks *fn_1_17778(Predicate, NodeLinks *);
NodeLinks *fn_1_17E20(NodeLinks *);
Node *fn_1_17E74(Predicate, Node *, Node *);

NodeLinks *fn_1_17778(Predicate predicate, NodeLinks *list)
{
    if (list == 0) return 0;
    if (static_cast<Node *>(list)->link0 == 0) return list;
    NodeLinks *first = list;
    NodeLinks *second = fn_1_17E20(first);
    first = fn_1_17778(predicate, first);
    second = fn_1_17778(predicate, second);
    return fn_1_17E74(predicate, static_cast<Node *>(first), static_cast<Node *>(second));
}

NodeLinks *fn_1_17E20(NodeLinks *node) {
    NodeLinks *fast=node->link0; if(fast) fast=fast->link0;
    while(fast) { node=node->link0; fast=fast->link0; if(fast) fast=fast->link0; }
    fast=node->link0; node->link0=0; return fast;
}

Node *fn_1_17E74(Predicate predicate, Node *first, Node *second) {
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
