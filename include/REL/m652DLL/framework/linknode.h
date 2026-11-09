#ifndef LINKNODE_H
#define LINKNODE_H

struct Node;

/* The merge head consumes the two links without node lifetime operations. */
struct NodeLinks {
    Node *link0;
    Node *link4;
};

/* A circular list node with links in both directions. */
struct Node : NodeLinks {
    Node();
    ~Node();
    void unlink();
    void insert0(Node *other);
    void insert4(Node *other);
};

#endif
