#ifndef REL_FRAMEWORK_TRI_COLLI_H
#define REL_FRAMEWORK_TRI_COLLI_H

extern "C" {
#include "dolphin/mtx.h"
}

struct Vector3 : Vec {
    float dot(const Vector3 &other) const {
        return PSVECDotProduct(this, &other);
    }
    float length() const { return PSVECMag(this); }
};

struct CollisionTreeNode {
    CollisionTreeNode *find(float x, float y, float z);
};

struct CollisionTree {
    u32 header;
    CollisionTreeNode *rootNode;
    CollisionTreeNode *root() const { return rootNode; }
    CollisionTreeNode *find(const Vector3 position) {
        CollisionTreeNode *node = root();
        return node->find(position.x, position.y, position.z);
    }
    void prepare(CollisionTreeNode *node);
};

struct CollisionTriangle {
    Vector3 *vertices[3];
    u16 metadata;
    Vector3 *first() const { return vertices[0]; }
    Vector3 *second() const { return vertices[1]; }
    Vector3 *third() const { return vertices[2]; }
    Vector3 *vertex(long index) const { return vertices[index]; }
};

struct CollisionTriangleIterator : CollisionTriangle {
    CollisionTriangleIterator(CollisionTree *tree, CollisionTreeNode *node);
    bool next();
};

struct PlaneEquation {
    Vector3 normal;
    float distance;
};

struct CollisionPlane : PlaneEquation {
    CollisionPlane() {}
    CollisionPlane(const Vector3 *first, const Vector3 *second, const Vector3 *third);
    CollisionPlane(const Vector3 *normal, const Vector3 *point);
};

struct PlaneRecord : CollisionPlane {
    u16 metadata;
    PlaneRecord() {}
    PlaneRecord(const CollisionPlane &plane, u16 value)
        : CollisionPlane(plane), metadata(value) {}
};

struct VertexTreeNode;
struct VertexParentColor {
    u32 bits;
    VertexParentColor &operator=(VertexTreeNode *parent) {
        bits = (u32)parent | (bits & 1UL);
        return *this;
    }
};
void rebalanceVertexTree(VertexTreeNode *, VertexTreeNode *);
struct VertexIteratorBase {};
struct VertexIterator : VertexIteratorBase {
    VertexTreeNode *node;
    VertexIterator(VertexTreeNode *value) : node(value) {}
    VertexIterator(const VertexIterator &other) : node(other.node) {}
    bool operator==(const VertexIterator &other) const { return node == other.node; }
};
struct VertexInsertion {
    VertexIterator iterator;
    bool inserted;
    VertexInsertion(const VertexIterator &position, const bool &success)
        : iterator(position), inserted(success) {}
};
void operator delete(void *);
struct VertexValueAllocator {
    typedef u32 Value;
    void destroy(u32 *value) { value->~Value(); }
    void construct(u32 *place, const u32 &value);
};
struct VertexNodeAllocator {
    VertexNodeAllocator() {}
    VertexNodeAllocator(const VertexNodeAllocator &) {}
    void deallocate(VertexTreeNode *node, u32) { ::operator delete(node); }
    VertexTreeNode *allocate(u32 count, const void *hint = 0);
};
struct VertexCountHolder : VertexValueAllocator {
    u32 count;
    VertexCountHolder(const VertexNodeAllocator &) : count(0) {}
    VertexValueAllocator &first() { return *this; }
    u32 &second() { return count; }
    const u32 &second() const { return count; }
};
struct VertexRootAnchor {
    void *left;
    VertexRootAnchor() : left(0) {}
};
struct VertexTreeNode : VertexRootAnchor {
    VertexTreeNode *right;
    VertexParentColor parentColor;
    u32 key;
};
struct VertexLess {
    bool operator()(const u32 &first, const u32 &second) const {
        return bool(first < second);
    }
};
struct VertexSizeLimits {
    static u32 maximum() { return (u32)-1; }
};
struct VertexCompareHolder : VertexLess {
    VertexTreeNode *firstNode;
    VertexCompareHolder(const VertexLess &compare) : VertexLess(compare) {}
    VertexLess &first() { return *this; }
    VertexTreeNode *&second() { return firstNode; }
};
struct VertexRootHolder : VertexNodeAllocator {
    VertexRootAnchor value;
    VertexRootHolder(const VertexNodeAllocator &allocator) : VertexNodeAllocator(allocator) {}
    VertexRootAnchor &second() { return value; }
    VertexNodeAllocator &first() { return *this; }
};
struct VertexTree {
    VertexCountHolder size;
    VertexRootHolder nodes;
    VertexCompareHolder comparator;
    VertexTree(const VertexLess &compare, const VertexNodeAllocator &allocator)
        : size(allocator), nodes(allocator), comparator(compare) {
        comparator.second() = (VertexTreeNode *)&nodes.second();
    }
    VertexTreeNode *&root() { return (VertexTreeNode *&)nodes.second().left; }
    VertexValueAllocator &valueAllocator() { return size.first(); }
    VertexNodeAllocator &nodeAllocator() { return nodes.first(); }
    u32 maximumSize() const { return VertexSizeLimits::maximum(); }
    u32 length() const { return size.second(); }
    void destroySubtree(VertexTreeNode *node);
    ~VertexTree();
    VertexIterator find(const u32 &vertex);
    VertexInsertion insertUnique(const u32 *vertex);
    VertexTreeNode *insertNode(VertexTreeNode *parent, bool left, bool first, const u32 *vertex);
    VertexIterator end() {
        return (VertexTreeNode *)&nodes.second();
    }
};
struct VertexSet : VertexTree {
    VertexSet(const VertexLess &, const VertexNodeAllocator &);
    ~VertexSet();
};
struct PlaneRecordAllocator {
    void deallocate(PlaneRecord *records) { ::operator delete(records); }
};
struct PlaneList : PlaneRecordAllocator {
    u32 capacity;
    u32 count;
    PlaneRecord *entries;
    PlaneList() : capacity(0), count(0), entries(0) {}
    ~PlaneList();
    void clear();
    PlaneRecord *&storage() { return entries; }
    PlaneRecord *begin() { return storage(); }
    PlaneRecord *end() { return storage() + count; }
    void insert(PlaneRecord *where, u32 count, const PlaneRecord *plane);
};

bool comparePlanes(const PlaneRecord *first, const PlaneRecord *second);
void sortPlanes(PlaneRecord *begin, PlaneRecord *end,
                bool (*compare)(const PlaneRecord *, const PlaneRecord *));
bool intersectTriangleFace(const CollisionTriangle *triangle, const CollisionPlane *plane,
                           const Vector3 *origin, const Vector3 *direction, PlaneList *results,
                           float radius);
void intersectTriangleVertices(const CollisionTriangle *triangle, const Vector3 *point,
                               PlaneList *results, VertexSet *visited, float radius);
void intersectTriangleEdges(const CollisionTriangle *triangle, const Vector3 *point,
                            PlaneList *results, float radius);

struct TriangleCollision {
    Vector3 faceNormal;
    float planeDistance;
    // Metadata returned for the triangle selected by a collision query.
    u16 triangleMetadata;
    Vector3 rayOrigin;
    Vector3 rayDirection;
    float maximumDistance;
    CollisionTree *collisionTree;

    void setCollisionTree(CollisionTree *tree);
    bool castRay(const Vector3 *start, const Vector3 *end);
    bool castNode(CollisionTreeNode *node);
    bool intersectSegment(Vector3 *point, const Vector3 *start, const Vector3 *end);
    void sweepSphere(const Vector3 *start, const Vector3 *end, PlaneList *results, float radius);
};

#endif
