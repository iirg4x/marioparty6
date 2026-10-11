// Collects selected octree leaves and draws their triangles as colored outlines.
static const char rcsid[] = "$Id: octtree_debug.cpp,v 1.22 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/octtree_debug.h"

#define OCTREE_PALETTE_ENTRY_BYTES 4
#define OCTREE_OUTLINE_VERTEX_COUNT 4

// A game's first enableDebug call creates the drawing hook and an empty leaf selection.
// Later calls preserve that hook and any leaves already queued for drawing.
void Octree::enableDebug()
{
    if (!renderer) {
        renderer = new OctreeRenderCallback(render);
        renderer->setHookData(this);
        selected = new OctreeNodeSet;
    }
}

// Selection requests accumulate unique leaves until the next debug drawing callback.
// Requests made before enableDebug are ignored; a null leaf is stored but draws nothing.
void Octree::select(OctreeDebugNode *node)
{
    OctreeNodeSet *selectedNodes = selected;
    if (selectedNodes) selectedNodes->insert(node);
}

// A game can submit a position instead of looking up its containing leaf itself.
// A position outside the root box submits a null leaf, which the drawing traversal skips.
void Octree::select(const Vec &position)
{
    select(nodeAt(position));
}

// Argument traits used by the member-call adapter in the debug drawing traversal.
template<class First, class Second, class Result> struct OctreeBinaryFunction {
    typedef First first_argument_type;
    typedef Second second_argument_type;
    typedef Result result_type;
};
// Argument traits for a drawing operation with its owning octree already bound.
template<class Argument, class Result> struct OctreeUnaryFunction {
    typedef Argument argument_type;
    typedef Result result_type;
};
// Wraps an octree member so the selected-node traversal can invoke it.
struct OctreeMemberFunction : OctreeBinaryFunction<Octree *, OctreeDebugNode *, void> {
    void (Octree::*method)(OctreeDebugNode *); // Member invoked for each selected leaf.
    explicit OctreeMemberFunction(void (Octree::*memberMethod)(OctreeDebugNode *))
        : method(memberMethod)
    {
    }
    void operator()(Octree *tree, OctreeDebugNode *node) const { (tree->*method)(node); }
};
inline OctreeMemberFunction octreeMemFun(void (Octree::*method)(OctreeDebugNode *))
{
    return OctreeMemberFunction(method);
}
// Stores the octree and drawing operation passed to the selected-node traversal.
template<class Operation>
struct OctreeFirstBinder : OctreeUnaryFunction<typename Operation::second_argument_type,
                                              typename Operation::result_type> {
    Operation operation; // Drawing operation invoked for a leaf.
    typename Operation::first_argument_type tree; // Octree that owns the selected leaves.
    // Before selection traversal, retain the drawing operation and its owning octree.
    OctreeFirstBinder(const Operation &boundOperation,
                      typename Operation::first_argument_type boundTree)
        : operation(boundOperation), tree(boundTree)
    {
    }
    // Each selected-leaf visit invokes the stored drawing operation with the bound octree.
    typename Operation::result_type
    operator()(const typename Operation::second_argument_type &node) const
    {
        operation(tree, node);
    }
};
// The render callback binds drawNode to its octree before traversing the selection.
template<class Operation, class Argument>
inline OctreeFirstBinder<Operation> octreeBindFirst(const Operation &function, const Argument &tree)
{
    typedef typename Operation::first_argument_type FirstArgument;
    return OctreeFirstBinder<Operation>(function, FirstArgument(tree));
}

// The render callback visits each selected leaf in set order with the bound drawing operation.
template<class Iterator, class Function>
inline
Function octreeForEach(Iterator current, Iterator end, Function operation)
{
    while (current != end) {
        operation(*current);
        ++current;
    }
    return operation;
}

// Hu3D calls this model hook to draw the selected leaves using its supplied matrix.
// The selection is consumed by this callback and must be submitted again for later draws.
void Octree::render(void *hookModel, Mtx transform)
{
    static const GXColor octreeDebugColors[] ATTRIBUTE_ALIGN(32) = {
        {255, 0, 0, 255}, {204, 204, 204, 255}, {204, 204, 0, 255},
        {136, 0, 0, 255}, {0, 255, 0, 255}, {255, 255, 255, 255},
        {0, 255, 255, 255}, {0, 0, 255, 255}, {204, 0, 204, 255},
        {255, 0, 255, 255}, {0, 255, 255, 255}, {102, 0, 0, 255},
        {102, 0, 102, 255}, {0, 102, 0, 255}, {0, 0, 102, 255}
    };
    static const GXColor material = {255, 255, 255, 0};
    static const GXColor ambient = {192, 192, 192, 0};
    GXLoadPosMtxImm(transform, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    GXSetArray(GX_VA_CLR0, (void *)octreeDebugColors, OCTREE_PALETTE_ENTRY_BYTES);
    GXSetNumChans(1);
    GXSetChanAmbColor(GX_COLOR0, ambient);
    GXSetChanMatColor(GX_COLOR0, material);
    GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
    // The outlines use the color channel without texture sampling.
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    // Hu3D passes its model to this hook; its hook data holds the octree from enableDebug.
    Octree *tree = static_cast<Octree *>(static_cast<HU3D_MODEL *>(hookModel)->hookData);
    OctreeNodeSet &selectedNodes = *tree->selected;
    octreeForEach(selectedNodes.begin(), selectedNodes.end(),
                  octreeBindFirst(octreeMemFun(&Octree::drawNode), tree));
    selectedNodes.clear();
}

// drawNode emits each outline point using the low eight bits of the triangle's palette index.
static inline void octreeVertex(const Vec *position, unsigned short paletteIndex)
{
    GXPosition3f32(position->x, position->y, position->z);
    GXColor1x8(paletteIndex);
}

// The render traversal draws every triangle referenced by one selected leaf; null leaves do
// nothing.
void Octree::drawNode(OctreeDebugNode *node)
{
    if (node) {
        OctreeTriangleIterator triangleIterator(this, node);
        while (triangleIterator.next()) {
            unsigned short paletteIndex = triangleIterator.color;
            // Reverse the last two vertices, then repeat the first to close the outline.
            GXBegin(GX_LINESTRIP, GX_VTXFMT0, OCTREE_OUTLINE_VERTEX_COUNT);
            octreeVertex(triangleIterator.firstVertex(), paletteIndex);
            octreeVertex(triangleIterator.thirdVertex(), paletteIndex);
            octreeVertex(triangleIterator.secondVertex(), paletteIndex);
            octreeVertex(triangleIterator.firstVertex(), paletteIndex);
        }
    }
}

// Selection clearing and tree destruction free child set nodes before their parent.
// The stored pointers are discarded; the octree's geometry remains owned by its archive.
void OctreeNodeTree::erase(OctreeSetNode *node)
{
    if (node->left) erase((OctreeSetNode *)node->left);
    if (node->right) erase(static_cast<OctreeSetNode *>(node->right));
    value_allocator().destroy(&node->value);
    node_allocator().deallocate(node, 1);
}

// After debug drawing, discard the set entries and restore the empty-tree sentinel links.
void OctreeNodeTree::clear()
{
    if (root()) {
        erase(root());
        size.second() = 0;
        root() = 0;
        begin_node() = end_node();
    }
}

// select inserts one leaf by pointer order, returning the existing entry for repeated requests.
OctreeSetResult OctreeNodeTree::insert(OctreeDebugNode *const &leaf)
{
    OctreeSetNode *predecessor = 0;
    OctreeSetNode *parent = (OctreeSetNode *)end_node();
    OctreeSetNode *node = root();
    bool insertLeft = true;
    bool becomesFirst = true;
    while (node) {
        parent = node;
        if (first(leaf, node->value)) {
            node = (OctreeSetNode *)node->left;
            insertLeft = true;
        } else {
            predecessor = parent;
            node = static_cast<OctreeSetNode *>(node->right);
            insertLeft = false;
            becomesFirst = false;
        }
    }
    // The predecessor detects equality after the search reaches an empty child link.
    if (!predecessor || first(predecessor->value, leaf)) {
        node = insert_node(parent, insertLeft, becomesFirst, leaf);
        OctreeSetIterator iterator(node);
        return OctreeSetResult(iterator, true);
    }
    OctreeSetIterator iterator(predecessor);
    return OctreeSetResult(iterator, false);
}

// Creating a debug selection starts its traversal at the empty-tree sentinel.
OctreeNodeTree::OctreeNodeTree(const OctreePointerLess &less, const OctreeSetAllocator &allocator)
    : size(allocator), nodes(allocator), first(less)
{
    begin_node() = end_node();
}

// Insert allocates a set entry, links it at the search position, and balances the tree.
// Allocation or size exhaustion aborts rather than dropping a requested leaf.
OctreeSetNode *OctreeNodeTree::insert_node(OctreeSetNode *parent, bool insertLeft,
                                           bool becomesFirst, OctreeDebugNode *const &leaf)
{
    if (sizeCount() > maxSize() - 1) {
        fprintf(stderr, "tree::insert length error\n");
        abort();
    }
    OctreeSetNode *node = node_allocator().allocate(1);
    value_allocator().construct(&node->value, leaf);
    node->left = node->right = 0;
    node->parent = parent;
    if (insertLeft) parent->left = node;
    else parent->right = node;
    ++size.second();
    rebalanceTreeAfterInsert(node, root());
    if (becomesFirst) begin_node() = node;
    return node;
}
