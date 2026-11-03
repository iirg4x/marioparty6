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
    std::for_each(selectedNodes.begin(), selectedNodes.end(),
                  std::bind1st(std::mem_fun(&Octree::drawNode), tree));
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
