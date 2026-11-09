#ifndef LINKNODE_H
#define LINKNODE_H

struct Node;

// The two links alone; the list sort's merge helpers walk them without a node's lifetime.
struct NodeLinks {
    Node *link0;
    Node *link4;
};

/* Two-link circular node. Class and method names describe the observed
 * operations; the original C++ identifiers are not preserved in retail. */
struct Node : NodeLinks {
    Node();
    ~Node();
    void unlink();
    void insert0(Node *other);
    void insert4(Node *other);
    void moveAllTo(Node *other);
    // Merge-sorts the list; the predicate is true when its first node goes before its second.
    void sort(bool (*predicate)(Node *, Node *));
};

#endif
