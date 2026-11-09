#include "REL/m652DLL/framework/linknode.h"

typedef bool (*Predicate)(Node *, Node *);

NodeLinks *fn_1_17778(Predicate predicate, NodeLinks *chain);

void fn_1_176C0(Node *self, Predicate predicate) {
    bool isEmpty = false;
    if (self->link0 == self) {
        if (self->link0 == self->link4) {
            isEmpty = true;
        }
    }

    if (!isEmpty) {
        self->link4->link0 = 0;
        Node *first = static_cast<Node *>(fn_1_17778(predicate, self->link0));
        self->link0 = first;

        Node *last = self;
        while (first != 0) {
            first->link4 = last;
            last = first;
            first = first->link0;
        }

        self->link4 = last;
        last->link0 = self;
    }
}
