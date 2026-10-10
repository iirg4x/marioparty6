#ifndef LINKNODE_H
#define LINKNODE_H

/* Two-link circular node. Class and method names describe the observed
 * operations; the original C++ identifiers are not preserved in retail. */
struct Node {
    Node *link0;
    Node *link4;

    Node();
    ~Node();
    void unlink();
    void insert0(Node *other);
    void insert4(Node *other);
};

#endif
