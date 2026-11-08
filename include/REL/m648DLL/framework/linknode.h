#ifndef LINKNODE_H
#define LINKNODE_H

struct Node;

/* The merge head consumes the two links without node lifetime operations. */
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
    void fn_1_1BE70(bool (*predicate)(Node *, Node *));
};

#endif
