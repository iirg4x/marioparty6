// Ray and swept-sphere queries against the triangles an octree holds in its leaves.
#ifndef REL_FRAMEWORK_TRI_COLLI_H
#define REL_FRAMEWORK_TRI_COLLI_H

// The order of <vector> and <set> is retail's: it sets the order of their members in the weak tail.
#include <algorithm>
#include <vector>
#include <set>

extern "C" {
#include "dolphin/mtx.h"
#include "dolphin/os.h"
}

#include "REL/framework/octtree_man.h"
#include "REL/framework/vector3.h"

// A plane's unit normal and its signed offset from the origin.
struct PlaneEquation {
    Vector3 normal;
    float distance;
};

// A plane built from triangle corners or from a normal and a point on it.
struct Plane : PlaneEquation {
    void setFromPoints(Vec *first, Vec *second, Vec *third);
    void setFromNormalAndPoint(Vec *normal, Vec *point);
};

// The octree as collision queries reach it.
struct CollisionTree : Octree {
    // The root of the search; a loaded tree always has one.
    OctreeDebugNode *root() const {
        ASSERT(nodes != 0);
        return nodes;
    }
    // Finds the leaf containing a position, or null outside the root's box.
    OctreeDebugNode *find(const Vector3 position) {
        OctreeDebugNode *node = root();
        return findOctreeLeaf(node, position.x, position.y, position.z);
    }
};

// A contact plane with the metadata of the triangle that produced it.
struct PlaneRecord : Plane {
    u16 metadata;
    PlaneRecord() {}
    PlaneRecord(const Plane &plane, u16 value)
        : Plane(plane), metadata(value) {}
};

// Contacts a sphere sweep collects, sorted by distance from the sweep's endpoint.
typedef std::vector<PlaneRecord> PlaneList;
// Vertices a sphere sweep has already tested; neighbouring triangles share them.
typedef std::set<Vector3 *> VertexSet;

// The octree's triangle traversal with the sphere contact tests a sweep runs on each triangle.
struct CollisionTriangle : OctreeTriangleIterator {
    CollisionTriangle(Octree *tree, OctreeDebugNode *node) : OctreeTriangleIterator(tree, node) {}
    Vector3 *pointA() const { return (Vector3 *)a; }
    Vector3 *pointB() const { return (Vector3 *)b; }
    Vector3 *pointC() const { return (Vector3 *)c; }
    bool intersectFace(const Plane *plane, float radius, const Vector3 *origin,
                       const Vector3 *direction, PlaneList *results) const;
    void intersectVertices(float radius, const Vector3 *point, PlaneList *results,
                           VertexSet *visited) const;
    void intersectEdges(float radius, const Vector3 *point, PlaneList *results) const;
};

bool comparePlanes(const PlaneRecord &first, const PlaneRecord &second);

// A collision query: its ray, and the nearest face it has reached.
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
    bool castNode(OctreeDebugNode *node);
    bool intersectSegment(Vector3 *point, const Vector3 *start, const Vector3 *end);
    void sweepSphere(const Vector3 *start, const Vector3 *end, float radius, PlaneList *results);
};

#endif
