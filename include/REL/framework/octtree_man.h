// Geometry tables and triangle traversal shared by the octree manager and debug drawing.
#ifndef FRAMEWORK_OCTTREE_MAN_H
#define FRAMEWORK_OCTTREE_MAN_H

#include "REL/framework/hu_archive.h"

#include <memory>
#include "REL/framework/vector3.h"

extern "C" {
#include "dolphin/mtx.h"
}

void operator delete(void *);

// An archived spatial node describes its box, children, and leaf triangle-index range.
struct OctreeDebugNode {
    unsigned long triangleOffset; // First entry in the leaf's triangle-index range.
    unsigned long triangleCount; // Number of entries in that range.
    float bounds[6]; // Minimum X/Y/Z followed by maximum X/Y/Z in the vertex table's coordinates.
    OctreeDebugNode *children[8]; // Child pointers after the archive indices are resolved.
};
// A triangle stores three vertex-table indices and one debug palette index.
struct OctreeTriangle {
    unsigned short vertex[3]; // Indices into the archive's vertex table.
    unsigned short color; // Index into the debug renderer's color palette.
};

struct OwnedModel;
struct Hu3DModel_s;
struct OctreeNodeSet;
void initializeOctreeNodes(OctreeDebugNode *, int);
OctreeDebugNode *findOctreeLeaf(OctreeDebugNode *, float, float, float);

// Holds archived spatial geometry and the optional selection used by debug drawing.
struct Octree {
    std::auto_ptr<Archive> archive; // Archive owning the geometry tables below.
    OctreeDebugNode *nodes; // Node table; its first entry is the root for point lookup.
    OctreeTriangle *triangles; // Triangle records indexing the vertex table.
    long triangleCount; // Number of triangle records in the archive.
    Vector3 *vertices; // Positions shared by the archived triangles.
    long vertexCount; // Number of positions in the vertex table.
    unsigned short *triangleIndices; // Leaf ranges index the triangle table through this array.
    std::auto_ptr<OwnedModel> renderer; // Null until enableDebug creates its drawing hook model.
    std::auto_ptr<OctreeNodeSet> selected; // Leaves queued for the next debug draw, or null before
                                           // enableDebug.
    // root() and find() are defined inline in tri_colli.h, beside their only user.
    OctreeDebugNode *root() const;
    OctreeDebugNode *find(const Vector3 position);
    Octree(Archive *);
    ~Octree();
    void enableDebug();
    void select(OctreeDebugNode *);
    void select(const Vec &);
    // Returns the selected nodes used for debug drawing.
    OctreeNodeSet *debugSet() const { return selected.get(); }
    // Point selection searches from the root; points outside its bounds return null.
    OctreeDebugNode *nodeAt(Vec point) {
        OctreeDebugNode *root = nodes;
        return findOctreeLeaf(root, point.x, point.y, point.z);
    }
    // Initializes the given number of nodes in the node array.
    void initializeNodes(long count) { initializeOctreeNodes(nodes, count); }
    // Returns the archive that owns the octree data.
    Archive *getArchive() const { return archive.get(); }
    OctreeTriangle *triangleTable() const { return triangles; }
    Vector3 *vertexTable() const { return vertices; }
    unsigned short *indexTable() const { return triangleIndices; }
    static void render(Hu3DModel_s *, Mtx *);
    void drawNode(OctreeDebugNode *);
};

// Traverses one leaf's triangle references and exposes each triangle to the debug renderer.
struct OctreeTriangleIterator {
    Vector3 *a[3]; // First, second, and third vertex of the last successful next call.
    unsigned short color; // Debug palette index of that triangle.
    unsigned short *current, *end; // Next leaf triangle index and the end of its range.
    OctreeTriangle *triangles; // Shared triangle table addressed by each leaf index.
    Vector3 *vertices; // Shared position table addressed by the triangle's three vertex indices.
    OctreeTriangleIterator(Octree *, OctreeDebugNode *);
    bool next();
    // Returns the first triangle vertex.
    const Vec *firstVertex() const { return a[0]; }
    // Returns the second triangle vertex.
    const Vec *secondVertex() const { return a[1]; }
    // Returns the third triangle vertex.
    const Vec *thirdVertex() const { return a[2]; }
};

#endif
