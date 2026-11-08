#ifndef LINKNODE_H
#define LINKNODE_H

/* A node of a circular, doubly linked list. */
struct Node;

/* The sort consumes the forward-link base; its merge head also holds the backward link. */
struct ChainNode { Node *link0; };
struct NodeLinks : ChainNode { Node *link4; };

typedef bool (*NodePredicate)(Node *, Node *);

class Node : public NodeLinks {
public:

    Node();
    ~Node();
    void unlink();
    void insert0(Node *other);
    void insert4(Node *other);
    void moveAllTo(Node *other);
    void fn_1_DAE8(NodePredicate predicate);
};

#endif
