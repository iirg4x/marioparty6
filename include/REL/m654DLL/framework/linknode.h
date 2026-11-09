#ifndef LINKNODE_H
#define LINKNODE_H
struct Node;
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
    void fn_1_188D8(Node *other);
    void fn_1_18960(bool (*predicate)(void *, void *));
};
#endif
