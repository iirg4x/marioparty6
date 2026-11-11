// Provides the circular links used to place game objects in collections.
#include "REL/framework/linknode.h"

static const char rcsid[] = "$Id: linknode.cpp,v 1.7 2004/02/17 09:22:00 hanamasu Exp $";

// Starts a new node as a one-node circular list when its owner creates it.
Node::Node()
{
    link0 = this;
    link4 = this;
}

// Reconnects the neighboring nodes when the owner destroys this node.
Node::~Node()
{
    link0->link4 = link4;
    link4->link0 = link0;
}

// Removes this node from its list and leaves it ready to be inserted again.
// A collection owner calls this before moving the node to another position.
void Node::unlink()
{
    link0->link4 = link4;
    link4->link0 = link0;
    link0 = this;
    link4 = this;
}

// Places this node immediately before the anchor in its circular list.
// A collection owner calls this while adding the node at that position.
void Node::insert0(Node *anchor)
{
    Node *neighbor = anchor;
    link4 = neighbor;
    link0 = neighbor->link0;
    neighbor->link0->link4 = this;
    neighbor->link0 = this;
}

// Places this node immediately after the anchor in its circular list.
// A collection owner calls this while adding the node at that position.
void Node::insert4(Node *anchor)
{
    Node *neighbor = anchor;
    link0 = neighbor;
    link4 = neighbor->link4;
    neighbor->link4->link0 = this;
    neighbor->link4 = this;
}

// Moves every node of this list to the end of the anchor's list and leaves this head empty.
// A collection owner calls this to walk its nodes while they may add to or leave the list.
void Node::moveAllTo(Node *anchor)
{
    bool isolated = false;
    if (link0 == this) {
        if (link0 == link4)
            isolated = true;
    }
    if (!isolated) {
        Node *neighbor = anchor;
        Node *first = link0;
        Node *last = link4;
        link0 = this;
        link4 = this;
        first->link4 = neighbor->link4;
        last->link0 = neighbor;
        neighbor->link4->link0 = first;
        neighbor->link4 = last;
    }
}

// The sort opens the circle into a chain on link0 that ends in 0, merge-sorts the chain, and then
// rebuilds the link4 side and closes the circle again.
NodeLinks *sortChain(bool (*predicate)(Node *, Node *), NodeLinks *chain);
NodeLinks *splitChain(NodeLinks *chain);
Node *mergeChains(bool (*predicate)(Node *, Node *), Node *first, Node *second);

// Orders this list by the predicate; a collection owner calls it to sort its members.
void Node::sort(bool (*predicate)(Node *, Node *))
{
    bool isEmpty = false;
    if (link0 == this) {
        if (link0 == link4)
            isEmpty = true;
    }
    if (!isEmpty) {
        link4->link0 = 0;
        Node *first = static_cast<Node *>(sortChain(predicate, link0));
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

// Sorts a 0-terminated chain by splitting it in two, sorting each half and merging them.
NodeLinks *sortChain(bool (*predicate)(Node *, Node *), NodeLinks *chain)
{
    if (chain == 0)
        return 0;
    if (static_cast<Node *>(chain)->link0 == 0)
        return chain;
    NodeLinks *first = chain;
    NodeLinks *second = splitChain(first);
    first = sortChain(predicate, first);
    second = sortChain(predicate, second);
    return mergeChains(predicate, static_cast<Node *>(first), static_cast<Node *>(second));
}

// Cuts a chain after its middle node and returns the second half.
NodeLinks *splitChain(NodeLinks *chain)
{
    NodeLinks *fast = chain->link0;
    if (fast)
        fast = fast->link0;
    while (fast) {
        chain = chain->link0;
        fast = fast->link0;
        if (fast)
            fast = fast->link0;
    }
    fast = chain->link0;
    chain->link0 = 0;
    return fast;
}

// Merges two sorted chains; the first chain's node goes first when the predicate says so.
Node *mergeChains(bool (*predicate)(Node *, Node *), Node *first, Node *second)
{
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
