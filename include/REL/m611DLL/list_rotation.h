#ifndef REL_M611DLL_LIST_ROTATION_H
#define REL_M611DLL_LIST_ROTATION_H

#include "REL/framework/list.h"

// Moves the first node to the end of a nonempty list.
template <class T>
inline void rotateListFront(ListRoot<T> &list)
{
    T *node = list.front();
    if (node != list.end_node()) {
        node->unlink();
        node->insert4(&list);
    }
}

// Deletes every object on an intrusive list.
template <class T>
inline void deleteEffects(ListRoot<T> &list)
{
    while (!list.empty()) {
        delete list.back();
    }
}

#endif
