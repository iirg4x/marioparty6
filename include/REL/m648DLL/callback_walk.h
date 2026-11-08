#ifndef REL_M648_CALLBACK_WALK_H
#define REL_M648_CALLBACK_WALK_H

#include "REL/m648DLL/framework/linknode.h"
#include "REL/framework/model.h"
#include "REL/m648DLL/multi_model_owner.h"

typedef OwnedModel ReleasedModel;

struct CallbackListEntry : Node {
    CallbackListEntry *next_node() { return static_cast<CallbackListEntry *>(link0); }
    ReleasedModel model0;
    ReleasedModel model1;
    ReleasedModel model2;
    ReleasedModel model3;
    ReleasedModel model4;
    ReleasedModel model5;
    ReleasedModel model6;
};

struct CallbackListRoot : Node {
    bool empty() const { return link0 == this && link0 == link4; }
    Node *last() { return link4; }
    CallbackListEntry *back() { return static_cast<CallbackListEntry *>(last()); }
    Node *first_node() { return link0; }
    Node *end_node() { return this; }
    CallbackListEntry *front() { return static_cast<CallbackListEntry *>(first_node()); }
};

typedef MotionHookModels CountedCallback;

void fn_1_58B8(CallbackListRoot *, void (*)(CountedCallback *));
void fn_1_5918(Node *, Node *, void (*)(CountedCallback *));

#endif
