#ifndef REL_M603DLL_LINKNODE_H
#define REL_M603DLL_LINKNODE_H

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
    void fn_1_19FEC(Node *other);
    void fn_1_1A074(bool (*predicate)(Node *, Node *));
};

#endif
