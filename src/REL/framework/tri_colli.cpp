// Triangle collision state and the oriented-triangle tests used by ray queries.
#include "REL/framework/tri_colli.h"
extern "C" {
#include "REL/framework/assertion.h"
}

static const char rcsid[] = "$Id: tri_colli.cpp,v 1.22 2004/09/09 12:24:38 saf Exp $";

static bool closestPointOnSegment(const Vector3 &first, const Vector3 &second, const Vector3 &point,
                                  Vector3 *closest);
static double closestPointOnLine(const Vector3 &first, const Vector3 &second, const Vector3 &point,
                                 Vector3 *closest);

// The dot product of two vectors given by pointer.
static inline float vectorDot(const Vector3 *first, const Vector3 *second)
{
    return first->dot(*second);
}

// The dot product in double precision, for the edge fraction below.
static inline double vectorDotDouble(const Vector3 *first, const Vector3 *second)
{
    return (double)first->z * (double)second->z
        + ((double)first->x * (double)second->x + (double)first->y * (double)second->y);
}

// Stores the tree that ray queries will visit.
TriangleCollision *TriangleCollision::setCollisionTree(CollisionTree *tree)
{
    collisionTree = tree;
    return this;
}

// How far the turn from one corner direction to the next leans along the triangle's normal.
static inline float edgeAlignment(const Vector3 *first, const Vector3 *second, Vector3 *cross,
                                  const Vector3 *normal)
{
    PSVECCrossProduct(first, second, cross);
    return cross->dot(*normal);
}

// Tests whether all three oriented edges contain the supplied point.
bool point_in_oriented_triangle(const Vector3 *firstVertex, const Vector3 *secondVertex,
                                const Vector3 *thirdVertex, const Vector3 *origin,
                                const Vector3 *normal)
{
    Vector3 firstFromOrigin;
    Vector3 secondFromOrigin;
    Vector3 thirdFromOrigin;
    Vector3 crossProduct;

    PSVECSubtract(firstVertex, origin, &firstFromOrigin);
    PSVECSubtract(secondVertex, origin, &secondFromOrigin);
    PSVECSubtract(thirdVertex, origin, &thirdFromOrigin);
    return edgeAlignment(&firstFromOrigin, &secondFromOrigin, &crossProduct, normal) >= 0.0f
        && edgeAlignment(&secondFromOrigin, &thirdFromOrigin, &crossProduct, normal) >= 0.0f
        && edgeAlignment(&thirdFromOrigin, &firstFromOrigin, &crossProduct, normal) >= 0.0f;
}

// Distance along a ray to a plane as a fraction of its direction; -1 when the ray runs parallel.
static inline float rayIntersectionDistance(const Plane *plane, const Vector3 *origin,
                                            const Vector3 *direction)
{
    float projectedDistance;
    float positionDot;
    float alignment;
    float denominator;
    alignment = PSVECDotProduct(&plane->normal, direction);
    denominator = alignment;
    if (denominator == 0.0f) {
        return -1.0f;
    } else {
        positionDot = PSVECDotProduct(&plane->normal, origin);
        projectedDistance = plane->distance + positionDot;
        return -(projectedDistance / denominator);
    }
}

// Scales a direction into the supplied point and returns that point.
static inline Vec &scalePoint(Vec &point, const Vec &direction, float distance)
{
    PSVECScale(&direction, &point, distance);
    return point;
}

// Moves a point by an offset.
static inline void offsetPoint(const Vector3 *point, const Vector3 *offset, Vector3 *result)
{
    PSVECAdd(point, offset, result);
}

// Tests a direction-offset triangle point against its three oriented edges.
bool ray_point_in_triangle(const CollisionTriangle *triangle, const Vector3 *point,
                           const Vector3 *direction, float distance)
{
    Vector3 firstPoint;
    Vector3 secondPoint;
    Vector3 thirdPoint;
    Vector3 scaledDirection;

    PSVECScale(direction, &scaledDirection, distance);
    offsetPoint(triangle->pointA(), &scaledDirection, &firstPoint);
    offsetPoint(triangle->pointB(), &scaledDirection, &secondPoint);
    offsetPoint(triangle->pointC(), &scaledDirection, &thirdPoint);
    return point_in_oriented_triangle(&firstPoint, &secondPoint, &thirdPoint, point, direction);
}

// Starts a ray query in the tree cell containing its origin.
bool TriangleCollision::castRay(const Vector3 *start, const Vector3 *end)
{
    PSVECSubtract(end, start, &rayDirection);
    rayOrigin.x = start->x;
    rayOrigin.y = start->y;
    rayOrigin.z = start->z;
    OctreeDebugNode *node = collisionTree->find(*start);
    if (node == 0) {
        return false;
    }
    collisionTree->select(node);
    return castNode(node);
}

// Chooses the nearest forward intersection among the triangles visited in a cell.
bool TriangleCollision::castNode(OctreeDebugNode *node)
{
    bool found = false;
    CollisionTree *tree = collisionTree;
    CollisionTriangle triangles(tree, node);

    while (triangles.next()) {
        Plane plane;
        float distance;
        plane.setFromPoints(triangles.pointA(), triangles.pointB(), triangles.pointC());
        distance = rayIntersectionDistance(&plane, &rayOrigin, &rayDirection);
        if (!(distance < 0.0f) && (!found || !(distance >= maximumDistance))) {
            Vector3 intersection;
            const Vec &scaled = scalePoint(intersection, rayDirection, distance);
            PSVECAdd(&rayOrigin, &scaled, &intersection);
            if (point_in_oriented_triangle(triangles.pointA(), triangles.pointB(), triangles.pointC(),
                                           &intersection, &plane.normal)) {
                found = true;
                maximumDistance = distance;
                faceNormal.x = plane.normal.x;
                faceNormal.y = plane.normal.y;
                faceNormal.z = plane.normal.z;
                planeDistance = plane.distance;
                triangleMetadata = triangles.color;
            }
        }
    }
    return found;
}

// Reports the ray's intersection point and whether it lies within the segment.
bool TriangleCollision::intersectSegment(Vector3 *point, const Vector3 *start, const Vector3 *end)
{
    if (!castRay(start, end)) {
        return false;
    }
    const Vec &scaled = scalePoint(*point, rayDirection, maximumDistance);
    PSVECAdd(&rayOrigin, &scaled, point);
    return maximumDistance <= 1.0f;
}

// The sweep endpoint comparePlanes measures from; std::sort's comparison takes no context.
static Vector3 collisionSortOrigin;

// Collects triangle contacts for a sphere swept between the two endpoints.
void TriangleCollision::sweepSphere(const Vector3 *start, const Vector3 *end, float radius,
                                  PlaneList *results)
{
    PSVECSubtract(end, start, &rayDirection);
    rayOrigin.x = start->x;
    rayOrigin.y = start->y;
    rayOrigin.z = start->z;
    OctreeDebugNode *node = collisionTree->find(*start);
    if (node != 0) {
        collisionTree->select(node);
        VertexSet visited;
        CollisionTriangle triangles(collisionTree, node);
        while (triangles.next()) {
            Plane plane;
            plane.setFromPoints(triangles.pointA(), triangles.pointB(), triangles.pointC());
            float signedDistance = plane.distance + vectorDot(&plane.normal, end);
            float distance;
            if (signedDistance < 0.0f) {
                distance = -signedDistance;
            } else {
                distance = signedDistance;
            }
            if (distance > radius) {
                continue;
            }
            plane.distance -= radius;
            if (!triangles.intersectFace(&plane, radius, &rayOrigin, &rayDirection, results)) {
                triangles.intersectVertices(radius, end, results, &visited);
                triangles.intersectEdges(radius, end, results);
            }
        }
        collisionSortOrigin.x = end->x;
        collisionSortOrigin.y = end->y;
        collisionSortOrigin.z = end->z;
        std::sort(results->begin(), results->end(), comparePlanes);
    }
}

// Signed distance of a point from a plane.
static inline float signedPlaneDistance(const Plane *plane, const Vector3 *point)
{
    return plane->distance + vectorDot(&plane->normal, point);
}

// Moves a point along a plane's normal by its signed distance, onto that plane.
static inline void projectOntoPlane(const Plane *plane, const Vector3 *point, Vector3 *projection,
                                    float signedDistance)
{
    Vector3 displacement;
    PSVECScale(&plane->normal, &displacement, -signedDistance);
    PSVECAdd(point, &displacement, projection);
}

// Orders the collected planes by their signed distance from the sweep endpoint.
bool comparePlanes(const PlaneRecord &first, const PlaneRecord &second)
{
    return signedPlaneDistance(&first, &collisionSortOrigin)
        < signedPlaneDistance(&second, &collisionSortOrigin);
}

// Adds a face contact when the moving center or its plane projection meets the triangle.
bool CollisionTriangle::intersectFace(const Plane *plane, float radius,
                                      const Vector3 *origin, const Vector3 *direction,
                                      PlaneList *results) const
{
    Vector3 intersection;
    float distance = rayIntersectionDistance(plane, origin, direction);

    if (distance >= 0.0f && distance <= 1.0f) {
        const Vec &offset = scalePoint(intersection, *direction, distance);
        PSVECAdd(origin, &offset, &intersection);
        if (ray_point_in_triangle(this, &intersection, &plane->normal, radius)) {
            goto accepted;
        }
    }
    {
        float planeOffset = signedPlaneDistance(plane, origin);
        projectOntoPlane(plane, origin, &intersection, planeOffset);
        if (!ray_point_in_triangle(this, &intersection, &plane->normal, radius)) {
            return false;
        }
    }
accepted:
    results->push_back(PlaneRecord(*plane, color));
    return true;
}

// Adds radial contacts at triangle vertices, visiting each shared vertex once per query.
void CollisionTriangle::intersectVertices(float radius, const Vector3 *point, PlaneList *results,
                                          VertexSet *visited) const
{
    for (int index = 0; index < 3; ++index) {
        // The iterator's corner pointers stand together, so an index reaches each of them.
        Vector3 *visitedVertex = (Vector3 *)(&a)[index];
        if (visited->find(visitedVertex) == visited->end()) {
            visited->insert(visitedVertex);
            Vector3 displacement;
            Vector3 contactPoint;
            Vector3 *corner = visitedVertex;
            PSVECSubtract(point, corner, &displacement);
            float distance = displacement.length();
            if (distance != 0.0f && !(distance > radius)) {
                PSVECScale(&displacement, &displacement, radius / distance);
                Vector3 *contactVertex = visitedVertex;
                PSVECAdd(contactVertex, &displacement, &contactPoint);
                Plane plane;
                plane.setFromNormalAndPoint(&displacement, &contactPoint);
                results->push_back(PlaneRecord(plane, color));
            }
        }
    }
}

// Adds radial contacts where the center projects within one of the triangle's edges.
void CollisionTriangle::intersectEdges(float radius, const Vector3 *point, PlaneList *results) const
{
    for (int index = 0; index < 3; ++index) {
        int nextIndex = index == 2 ? 0 : index + 1;
        Vector3 closest;
        Vector3 displacement;
        const Vector3 &first = *(const Vector3 *)(&a)[index];
        const Vector3 &second = *(const Vector3 *)(&a)[nextIndex];
        if (closestPointOnSegment(first, second, *point, &closest)) {
            PSVECSubtract(point, &closest, &displacement);
            float distance = displacement.length();
            if (distance != 0.0f && !(distance > radius)) {
                PSVECScale(&displacement, &displacement, radius / distance);
                PSVECAdd(&closest, &displacement, &closest);
                Plane plane;
                plane.setFromNormalAndPoint(&displacement, &closest);
                results->push_back(PlaneRecord(plane, color));
            }
        }
    }
}

// Writes the infinite-line projection, then reports whether it falls within the edge.
static bool closestPointOnSegment(const Vector3 &first, const Vector3 &second, const Vector3 &point,
                                  Vector3 *closest)
{
    double fraction = closestPointOnLine(first, second, point, closest);
    bool within = false;
    if (fraction >= 0.0 && fraction <= 1.0) {
        within = true;
    }
    return within;
}

// Projects a point onto an edge's infinite line using double precision for its fraction.
static double closestPointOnLine(const Vector3 &first, const Vector3 &second, const Vector3 &point,
                                 Vector3 *closest)
{
    Vector3 edge;
    PSVECSubtract(&second, &first, &edge);
    double squaredLength = vectorDotDouble(&edge, &edge);
    squaredLength != 0.0 ? (void)0
        : __msl_assertion_failed("denom != 0", "tri_colli.cpp", 421);
    double fraction = (vectorDotDouble(&point, &edge) - vectorDotDouble(&first, &edge))
        / squaredLength;
    PSVECScale(&edge, &edge, (float)fraction);
    PSVECAdd(&first, &edge, closest);
    return fraction;
}
