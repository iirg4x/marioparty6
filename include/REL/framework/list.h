#ifndef REL_FRAMEWORK_LIST_H
#define REL_FRAMEWORK_LIST_H

// Typed circular lists built on Node. Retail keeps one copy of the node base's destructor per element class, and one
// copy of the head's destructor and of each visit per list, at the end of the files that use them, where GC/1.3.2
// puts template instances: both are class templates on the element type. The template and member names are
// descriptive; retail carries no symbol for them.
#include "REL/framework/linknode.h"

// The node base of a list element.
template <class T> struct ListNode : Node {
    ~ListNode() {}
    // The node after this one; the list's head when this is the last.
    T *next() { return static_cast<T *>(link0); }
    // Calls visitor on each node from current up to end.
    static void visitNodes(T *current, Node *end, void (*visitor)(T *));
};

template <class T> void ListNode<T>::visitNodes(T *current, Node *end, void (*visitor)(T *))
{
    while (current != end) {
        visitor(current);
        T *next = static_cast<T *>(current->link0);
        current = next;
    }
}

// The head of a list of T: an empty list is the head linked to itself.
template <class T> struct ListRoot : Node {
    ~ListRoot() {}
    Node *first_node() { return link0; }
    Node *last_node() { return link4; }
    Node *end_node() { return this; }
    T *front() { return static_cast<T *>(first_node()); }
    T *back() { return static_cast<T *>(last_node()); }
    bool empty() const { return link0 == this && link0 == link4; }

    // GC/1.3.2 emits one list's members at the end of a file in the reverse of this order (m624 node_callbacks.cpp:
    // visit, visitMoved, the head's destructor), and the walks of two lists after both visits, the later list's
    // first (m603 draw_dispatch.cpp), which a static member gives and a function template does not.

    // Calls visitor on each node once, in order, while the visitor may add nodes to the list or remove them.
    void visitMoved(void (*visitor)(T *));
    // Calls visitor on each node in order.
    void visit(void (*visitor)(T *));
};

template <class T> void ListRoot<T>::visit(void (*visitor)(T *))
{
    ListNode<T>::visitNodes(front(), end_node(), visitor);
}

template <class T> void ListRoot<T>::visitMoved(void (*visitor)(T *))
{
    if (!empty()) {
        ListRoot<T> pending;
        moveAllTo(&pending);
        while (!pending.empty()) {
            T *node = pending.front();
            node->unlink();
            node->insert4(this);
            visitor(node);
        }
    }
}

#endif
