#ifndef LINKNODE_H
#define LINKNODE_H
class Node;
typedef bool (*NodePredicate)(Node *, Node *);
struct NodeLinks {
    Node *link0;
    Node *link4;
};
class Node : public NodeLinks {
public:
    Node();
    ~Node();
    void unlink();
    void insert0(Node *);
    void insert4(Node *);
    void splice(Node *);
    void sort(NodePredicate);
};
#endif
