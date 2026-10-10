// Geometry tables and triangle traversal shared by the octree manager and debug drawing.
#ifndef FRAMEWORK_OCTTREE_MAN_H
#define FRAMEWORK_OCTTREE_MAN_H

extern "C" {
#include "dolphin/mtx.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/stdio.h"
}

void operator delete(void *);

// An archived spatial node describes its box, children, and leaf triangle-index range.
struct OctreeDebugNode {
    unsigned long triangleOffset; // First entry in the leaf's triangle-index range.
    unsigned long triangleCount; // Number of entries in that range.
    float bounds[6]; // Minimum and maximum coordinates of the node's box.
    OctreeDebugNode *children[8]; // Child pointers after the archive indices are resolved.
};
// A triangle stores three vertex-table indices and one debug palette index.
struct OctreeTriangle {
    unsigned short vertex[3]; // Indices into the archive's vertex table.
    unsigned short color; // Index into the debug renderer's color palette.
};
// Archive interface used to retrieve the named geometry tables and their counts.
struct OctreeArchive {
    virtual ~OctreeArchive();
    virtual void *find(const char *);
};

// Owns an archive or debug drawing object and deletes it on replacement or destruction.
template <class Value> struct OctreeOwned {
    Value *pointer; // Owned object, or null before it is assigned.
    OctreeOwned() : pointer(0) {}
    OctreeOwned(Value *value) : pointer(value) {}
    ~OctreeOwned() { delete pointer; }
    Value *get() const { return pointer; }
    Value *operator->() const { return pointer; }
    Value &operator*() const { return *get(); }
    bool operator!() const { return get() == 0; }
    operator Value *() const { return pointer; }
    static void destroy(Value *value) {
        delete value;
    }
    // Assignment deletes the previous object only when the incoming pointer differs.
    OctreeOwned &operator=(Value *value) {
        if (pointer != value) {
            destroy(pointer);
            pointer = value;
        }
        return *this;
    }
};

struct OctreeRenderCallback;
struct OctreeNodeSet;
void initializeOctreeNodes(OctreeDebugNode *, int);
OctreeDebugNode *findOctreeLeaf(OctreeDebugNode *, float, float, float);

// Holds archived spatial geometry and the optional selection used by debug drawing.
struct Octree {
    OctreeOwned<OctreeArchive> archive; // Archive owning the geometry tables below.
    OctreeDebugNode *nodes; // Node table; its first entry is the root for point lookup.
    OctreeTriangle *triangles; // Triangle records indexing the vertex table.
    long triangleCount; // Number of triangle records in the archive.
    Vec *vertices; // Positions shared by the archived triangles.
    long vertexCount; // Number of positions in the vertex table.
    unsigned short *triangleIndices; // Leaf ranges index the triangle table through this array.
    OctreeOwned<OctreeRenderCallback> renderer; // Null until enableDebug creates its drawing hook.
    OctreeOwned<OctreeNodeSet> selected; // Leaves queued for the next debug draw, or null before
                                         // enableDebug.
    Octree(OctreeArchive *);
    ~Octree();
    void enableDebug();
    void select(OctreeDebugNode *);
    void select(const Vec &);
    OctreeNodeSet *debugSet() const { return selected.get(); }
    // Point selection searches from the root; points outside its bounds return null.
    OctreeDebugNode *nodeAt(Vec point) {
        OctreeDebugNode *root = nodes;
        return findOctreeLeaf(root, point.x, point.y, point.z);
    }
    void initializeNodes(long count) { initializeOctreeNodes(nodes, count); }
    OctreeArchive *getArchive() const { return archive.get(); }
    OctreeTriangle *triangleTable() const { return triangles; }
    Vec *vertexTable() const { return vertices; }
    unsigned short *indexTable() const { return triangleIndices; }
    static void render(void *, Mtx);
    void drawNode(OctreeDebugNode *);
};

// Traverses one leaf's triangle references and exposes each triangle to the debug renderer.
struct OctreeTriangleIterator {
    const Vec *a, *b, *c; // First, second, and third vertex of the last successful next call.
    unsigned short color; // Debug palette index of that triangle.
    unsigned short *current, *end; // Next leaf triangle index and the end of its range.
    OctreeTriangle *triangles; // Shared triangle table addressed by each leaf index.
    Vec *vertices; // Shared position table addressed by the triangle's three vertex indices.
    OctreeTriangleIterator(Octree *, OctreeDebugNode *);
    bool next();
    const Vec *firstVertex() const { return a; }
    const Vec *secondVertex() const { return b; }
    const Vec *thirdVertex() const { return c; }
};

#endif
