/* Shared ordered-tree links for channel receivers, event callbacks and selected octree leaves. */
#ifndef FRAMEWORK_TREE_NODE_H
#define FRAMEWORK_TREE_NODE_H

/* Tree links use aligned addresses, leaving the low bit available for the node's color. */
#define TREE_PARENT_TAG_MASK 0x1UL
struct TreeNodeBase;
/* The end anchor stores the root in the same first word as a node's left child. */
struct TreeAnchor {
    void *left; /* Left child, or the root on an end anchor; null when absent. */
    TreeAnchor() : left(0) {}
};
/* The parent address shares its low bit with the red/black tree color. */
struct TreeParent {
    unsigned long parentBits; /* Parent address; its low-bit color survives parent assignment. */
    operator TreeNodeBase *() const;
    TreeNodeBase *operator->() const;
    /* Insertion and tree copying reconnect the parent without changing the node's color. */
    TreeParent& operator=(TreeAnchor *parent) {
        parentBits = (unsigned long)parent | (parentBits & TREE_PARENT_TAG_MASK);
        return *this;
    }
};
/* Value-independent links used by every ordered-tree traversal and balancing operation. */
struct TreeNodeBase : TreeAnchor {
    TreeNodeBase *right; /* Right child, or null when absent. */
    TreeParent parent; /* Parent link and color; the root's parent is its tree's end anchor. */
};
/* Tree traversal reads the parent address with its color tag removed. */
inline TreeParent::operator TreeNodeBase *() const {
    return (TreeNodeBase *)(parentBits & ~TREE_PARENT_TAG_MASK);
}
/* Tree traversal accesses the parent's links without treating the color tag as an address bit. */
inline TreeNodeBase *TreeParent::operator->() const {
    return (TreeNodeBase *)(parentBits & ~TREE_PARENT_TAG_MASK);
}
/* Iterator increments advance a real entry to its successor or end anchor; never pass end. */
void advanceTreeNode(TreeNodeBase *&node);
/* Tree insertion restores the red/black invariants after linking the new node. */
void rebalanceTreeAfterInsert(TreeNodeBase *insertedNode, TreeNodeBase *rootNode);
#endif
