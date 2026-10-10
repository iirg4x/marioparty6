// Manages keyed game object entries in an ordered table.
static const char rcsid[] = "$Id: game_object_man.cpp,v 1.21 2004/09/09 12:24:38 saf Exp $";

// The header anchor's left link points to the root node.
struct TreeAnchor { void *leftChild; /* Left child, or the root when this is the sentinel. */ };
// Each table entry associates an integer key with its managed object.
struct TreeValue {
    int key;      /* Integer used to order and find the entry. */
    void *object; /* Managed object stored for the key. */
};
// A tree node extends the anchor with its other links and mapped entry.
struct TreeNode : TreeAnchor {
    TreeNode *rightChild; /* Right child in the ordered tree. */
    unsigned long parentAndColor; /* Parent link and red-black color bit. */
    TreeValue entry; /* Key and associated object pointer. */
};
// Holds the sentinel anchor used to reach the root and represent end().
struct TreeRootHolder {
    TreeAnchor sentinel; /* Header node used as the tree's end marker. */
    TreeAnchor &root_anchor()
    {
        return sentinel;
    }
};
// Defines the ascending order for integer object keys.
struct TreeLess {
    bool operator()(const int &left, const int &right) const
    {
        return bool(left < right);
    }
};
// Adapts the key comparison used by the ordered table.
struct TreeCompare {
    TreeLess less; /* Integer ordering policy. */
    bool operator()(const int &left, const int &right) const
    {
        return less(left, right);
    }
};
// Stores the comparison policy and the table's first node pointer.
struct TreeCompareHolder {
    TreeCompare compare; /* Ordering policy used by the table. */
    TreeNode *firstNode; /* Pointer to the first tree node. */
    TreeCompare &first()
    {
        return compare;
    }
};
// Identifies a node in the ordered table; end() points at the sentinel.
struct TreeIterator {
    TreeNode *node; /* Current node, or the sentinel for end(). */
    TreeIterator(TreeNode *position) : node(position)
    {
    }
    bool operator==(const TreeIterator &other) const
    {
        return node == other.node;
    }
};
// Maintains object entries ordered by their integer keys.
struct ObjectTree {
    unsigned long nodeCount; /* Number of entries in the table. */
    TreeRootHolder rootHolder; /* Root link and end sentinel. */
    TreeCompareHolder comparator; /* Integer key ordering and first-node metadata. */
    TreeNode*& root() { return (TreeNode*&)rootHolder.root_anchor().leftChild; }
    TreeIterator end() { return (TreeNode*)&rootHolder.root_anchor(); }
    // Locates a matching key; erase() calls this before removing an entry.
    TreeIterator find(const int& key) {
        TreeNode *node = root();
        TreeNode *lowerBound = (TreeNode*)&rootHolder.root_anchor();
        while (node != 0) {
            if (!comparator.first()(node->entry.key, key)) {
                lowerBound = node;
                node = (TreeNode*)node->leftChild;
            } else {
                node = node->rightChild;
            }
        }
        if (lowerBound == (TreeNode *) &rootHolder.root_anchor() ||
            comparator.first()(key, lowerBound->entry.key))
            return end();
        return lowerBound;
    }
    bool erase(const int& key);
};

void eraseTreePosition(ObjectTree *, TreeIterator);

// Removes a keyed object entry when it exists; called by the manager's removal path.
bool ObjectTree::erase(const int& key)
{
    TreeIterator position = find(key);
    if (position == end()) return false;
    eraseTreePosition(this, position);
    return true;
}
