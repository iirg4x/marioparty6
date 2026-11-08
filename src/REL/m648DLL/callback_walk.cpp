#include "REL/m648DLL/callback_walk.h"

void fn_1_58B8(CallbackListRoot *list, void (*visitor)(CountedCallback *))
{
    fn_1_5918(list->front(), list->end_node(), visitor);
}

void fn_1_5918(Node *current, Node *sentinel, void (*callback)(CountedCallback *))
{
    while (current != sentinel) {
        callback(static_cast<CountedCallback *>(current));
        Node *next = current->link0;
        current = next;
    }
}
