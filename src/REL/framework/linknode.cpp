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
