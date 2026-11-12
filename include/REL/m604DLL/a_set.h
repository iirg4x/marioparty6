// A list of every object of one kind (the players, the Noko), reached from anywhere through its one head.
// Constructing the head makes it the current one; getChecked() asserts that one exists.
//
//
//
#ifndef REL_M604DLL_A_SET_H
#define REL_M604DLL_A_SET_H

#undef NDEBUG // the build defines NDEBUG for every object; these asserts are compiled in
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "dolphin/types.h"
#include "REL/framework/list.h"

template <class T> class ASet : public ListRoot<T> {
public:
    // The current head: zero before construction.
    static ASet *sm_ptr;

    ASet()
    {
        assert(sm_ptr == NULL);
        sm_ptr = this;
    }

    // Deletes every object still on the list, the last first.
    ~ASet()
    {
        while (!this->empty()) {
            delete this->back();
        }
    }

    // Calls method on each object in order.
    void forEach(void (T::*method)())
    {
        T *node = this->front();
        Node *end = this->end_node();
        while (node != end) {
            (node->*method)();
            node = node->next();
        }
    }
    // Callers may run before the head is constructed.
    static ASet &getChecked()
    {
        assert(sm_ptr != 0);
        return *sm_ptr;
    }

    // Adds node at the end of the list.
    void insert(T *node) { node->insert4(this); }

    // The node after node.
    static T *next(T *const &node) { return static_cast<T *>(node->link0); }

    // Deletes every object on the list, the first first.
    void deleteAll()
    {
        T *node = this->front();
        Node *end = this->end_node();
        while (node != end) {
            delete node;
            node = next(node);
        }
    }
};


#endif
