#ifndef REL_FRAMEWORK_LIST_H
#define REL_FRAMEWORK_LIST_H

// Typed circular lists built on Node, with traversal helpers for their element type.
#include "REL/framework/linknode.h"

// The node base of a list element.
template <class T> struct ListNode : Node {
    ~ListNode() {}
    // The node after this one; the list's head when this is the last.
    T *next() { return static_cast<T *>(link0); }
    // Calls visitor on each node from current up to end.
    static void visitNodes(T *current, Node *end, void (*visitor)(T *));
};

// Calls the visitor on each node until the end node is reached.
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
    // Returns the head used as the end marker.
    Node *end_node() { return this; }
    // Returns the first node as a pointer to the element type.
    T *front() { return static_cast<T *>(first_node()); }
    // Returns the last node as a pointer to the element type.
    T *back() { return static_cast<T *>(last_node()); }
    // Tests whether both links point to the list head.
    bool empty() const { return link0 == this && link0 == link4; }

    // Traversal can either visit the list directly or move each visited node through a local head.

    // Calls visitor on each node once, in order, allowing nodes to be added or removed.
    void visitMoved(void (*visitor)(T *));
    // Calls visitor on each node in order.
    void visit(void (*visitor)(T *));
};

// Calls the visitor on each node in list order.
template <class T> void ListRoot<T>::visit(void (*visitor)(T *))
{
    ListNode<T>::visitNodes(front(), end_node(), visitor);
}

// Moves each original node back from a pending list before visiting it.
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
