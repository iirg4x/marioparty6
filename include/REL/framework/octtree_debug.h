// Selected-leaf sets and model callbacks used to draw octree triangle outlines.
#ifndef FRAMEWORK_OCTTREE_DEBUG_H
#define FRAMEWORK_OCTTREE_DEBUG_H
#include "REL/framework/tree_node.h"

#include "REL/framework/octtree_man.h"
#include "REL/framework/model.h"

extern "C" {
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/stdio.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/abort_exit.h"
}

void *operator new(unsigned long);
void operator delete(void *);
inline void *operator new(unsigned long, void *storage) { return storage; }

#define OCTREE_SET_PARENT_FLAG 0x1UL

struct OctreeSetNode;
// Common link for set nodes and the sentinel whose left link holds the root.
typedef TreeAnchor OctreeSetAnchor;
// Parent link with a flag packed into its otherwise unused low address bit.
typedef TreeParent OctreeSetParent;
// A selection-set entry links the ordered tree and stores one octree leaf pointer.
struct OctreeSetNode : TreeNodeBase {
    OctreeDebugNode *value; // Selected leaf; its geometry is not owned by this entry.
};
// Constructs and destroys the stored pointer without deleting the selected leaf.
struct OctreeValueAllocator {
    typedef OctreeDebugNode *Value;
    OctreeValueAllocator() {}
    OctreeValueAllocator(const OctreeValueAllocator &) {}
    void destroy(Value *value) { value->~Value(); }
    void construct(Value *storage, const Value &value) { new(storage) Value(value); }
};
// Allocates selection-tree entries with the framework's global allocation functions.
struct OctreeSetAllocator {
    OctreeSetAllocator() {}
    OctreeSetAllocator(const OctreeSetAllocator &) {}
    void deallocate(OctreeSetNode *node, unsigned long) { ::operator delete(node); }
    // Tree insertion requests one entry; allocation failure terminates the program.
    OctreeSetNode *allocate(unsigned long count) {
        OctreeSetNode *node = (OctreeSetNode *)::operator new(count * sizeof(OctreeSetNode));
        if (!node) { fprintf(stderr, "Memory allocation failure"); abort(); }
        return node;
    }
};
// Stores the set size alongside the allocator for its leaf-pointer values.
struct OctreeSetCount : OctreeValueAllocator {
    unsigned long count; // Number of entries currently queued for drawing.
    OctreeSetCount(const OctreeSetAllocator &) : count(0) {}
    OctreeValueAllocator &allocator() { return *this; }
    unsigned long &second() { return count; }
    const unsigned long &second() const { return count; }
};
// Holds the empty-tree sentinel and the allocator for selection-tree entries.
struct OctreeSetRoot : OctreeSetAllocator {
    OctreeSetAnchor value; // Sentinel whose left link stores the selection tree's root.
    OctreeSetRoot(const OctreeSetAllocator &allocator) : OctreeSetAllocator(allocator) {}
    OctreeSetAnchor &second() { return value; }
    OctreeSetAllocator &allocator() { return *this; }
};
// Argument and result traits for comparing selected leaf addresses.
struct OctreePointerComparison {
    typedef OctreeDebugNode *first_argument_type;
    typedef OctreeDebugNode *second_argument_type;
    typedef bool result_type;
};
// Orders selections by address so repeated requests for the same leaf share an entry.
struct OctreePointerLess : OctreePointerComparison {
    bool operator()(OctreeDebugNode *const &leftLeaf, OctreeDebugNode *const &rightLeaf) const {
        return bool((unsigned long)leftLeaf < (unsigned long)rightLeaf);
    }
};
// Stores the first traversal entry with the pointer-order comparison used by insertion.
struct OctreeSetBegin : OctreePointerLess {
    OctreeSetAnchor *value; // Leftmost entry, or the sentinel when the set is empty.
    OctreeSetBegin(const OctreePointerLess &less) : OctreePointerLess(less) {}
    OctreeSetAnchor *&second() { return value; }
};
// Types exposed by iterators over the selection set's stored leaf pointers.
struct OctreeSetIteratorTraits {
    typedef OctreeDebugNode *value_type;
    typedef long difference_type;
    typedef OctreeDebugNode *const *pointer;
    typedef OctreeDebugNode *const &reference;
};
// Traverses tree entries while exposing their selected leaf pointers.
struct OctreeSetIterator : OctreeSetIteratorTraits {
    TreeNodeBase *node; // Current tree entry, or the end sentinel after traversal.
    OctreeSetIterator(OctreeSetNode *entryNode) : node(entryNode) {}
    OctreeDebugNode *const &operator*() const { return static_cast<OctreeSetNode *>(node)->value; }
    OctreeSetIterator &operator++() { advanceTreeNode(node); return *this; }
    bool operator!=(const OctreeSetIterator &other) const { return node != other.node; }
};
// Returns the matching entry and whether insertion created it.
struct OctreeSetResult {
    OctreeSetIterator first; // Entry for the submitted leaf, new or already present.
    bool second; // True only when a new entry was added.
    OctreeSetResult(const OctreeSetIterator &iterator, const bool &inserted)
        : first(iterator), second(inserted)
    {
    }
};
// Read-only selection iterator used by the debug drawing traversal.
struct OctreeConstSetIterator : OctreeSetIterator {
    OctreeConstSetIterator(const OctreeSetIterator &iterator) : OctreeSetIterator(iterator) {}
    OctreeDebugNode *const &operator*() const { return static_cast<OctreeSetNode *>(node)->value; }
    OctreeConstSetIterator &operator++() { advanceTreeNode(node); return *this; }
    bool operator!=(const OctreeConstSetIterator &other) const { return node != other.node; }
};
// Read-only insertion result exposed by the selected-leaf set.
struct OctreeConstSetResult {
    OctreeConstSetIterator first; // Entry for the submitted leaf, new or already present.
    bool second; // True only when a new entry was added.
    OctreeConstSetResult(const OctreeConstSetIterator &iterator, const bool &inserted)
        : first(iterator), second(inserted) {}
};
// Maximum entry count representable by the selection tree's unsigned size.
struct OctreeSetSizeLimit { static unsigned long max() { return (unsigned long)-1; } };
// Ordered selection storage whose nodes hold leaf pointers without owning their geometry.
struct OctreeNodeTree {
    OctreeSetCount size; // Entry count and stored-pointer allocator.
    OctreeSetRoot nodes; // Root sentinel and tree-entry allocator.
    OctreeSetBegin first; // Pointer ordering and first traversal entry.
    OctreeNodeTree(const OctreePointerLess &, const OctreeSetAllocator &);
    ~OctreeNodeTree() { if (root()) erase(root()); }
    OctreeSetNode *&root() { return (OctreeSetNode *&)nodes.second().left; }
    OctreeSetAnchor *&begin_node() { return first.second(); }
    OctreeSetAnchor *end_node() { return &nodes.second(); }
    OctreeSetIterator begin() { return (OctreeSetNode *)begin_node(); }
    OctreeSetIterator end() { return (OctreeSetNode *)end_node(); }
    void erase(OctreeSetNode *);
    OctreeSetAllocator &node_allocator() { return nodes.allocator(); }
    OctreeValueAllocator &value_allocator() { return size.allocator(); }
    void clear();
    unsigned long sizeCount() const { return size.second(); }
    unsigned long maxSize() const { return OctreeSetSizeLimit::max(); }
    OctreeSetResult insert(const OctreeDebugNode *);
    OctreeSetResult insert(OctreeDebugNode *const &);
    OctreeSetNode *insert_node(OctreeSetNode *, bool, bool, OctreeDebugNode *const &);
};
// Creates the selected-leaf tree with its pointer comparator and default allocator.
struct OctreeNodeSetBase : OctreeNodeTree {
    OctreeNodeSetBase() : OctreeNodeTree(OctreePointerLess(), OctreeSetAllocator()) {}
};
// Selection interface that prevents callers from changing the stored leaf pointers.
struct OctreeNodeSet : OctreeNodeSetBase {
    OctreeNodeSet() {}
    OctreeConstSetIterator begin() { return OctreeNodeTree::begin(); }
    OctreeConstSetIterator end() { return OctreeNodeTree::end(); }
    // select adds a leaf through the tree and exposes the result as a read-only iterator.
    OctreeConstSetResult insert(OctreeDebugNode *const &leaf) {
        OctreeSetResult result = OctreeNodeTree::insert(leaf);
        return OctreeConstSetResult(OctreeConstSetIterator(result.first), result.second);
    }
};
// Owns the Hu3D drawing hook installed by enableDebug; Model releases it on destruction.
struct OctreeRenderCallback : Model {
    OctreeRenderCallback(void (*)(void *, Mtx));
};
#endif
