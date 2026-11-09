#ifndef LINKNODE_H
#define LINKNODE_H

struct Node;

// The two links used by chain splitting and merging, without node lifetime operations.
struct NodeLinks {
    Node *link0;
    Node *link4;
};

struct Node : NodeLinks {

    Node();
    ~Node();
    void unlink();
    void insert0(Node *other);
    void insert4(Node *other);
};

#endif
