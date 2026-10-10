// Tests sphere overlaps, separates their centers, and finds contact along moving paths.
static const char rcsid[] = "$Id: sphere.cpp,v 1.8.2.1 2004/06/04 03:56:51 shohyama Exp $";

#include <dolphin/math.h>
#include <dolphin/mtx.h>

// Wraps a displacement passed to the sphere center correction helpers.
struct Vector3 {
    // Center correction in the same coordinate units as the spheres.
    Point3d vector;

    Point3d *getVector() { return &vector; }
    operator Point3d *() { return getVector(); }
};

// Describes a collision sphere in the caller's coordinate system.
struct Sphere {
    // Center position in the caller's coordinate units.
    Point3d center;
    // Radius in the same units as the center coordinates.
    f32 radius;

    f32 getRadius() const { return radius; }
    void translate(Point3d *offset) { PSVECAdd(offset, &center, &center); }
};

inline f32 vectorMagnitude(Point3d *vector) { return PSVECMag(vector); }
inline f32 vectorDot(Point3d *first, Point3d *second) {
    return PSVECDotProduct(first, second);
}
inline f32 square(const f32 &value) { return value * value; }

// Used for minigame sphere collision checks; touching boundaries do not count as overlap.
bool spheresOverlap(Sphere *first, Sphere *second)
{
    Point3d centerDelta;
    f32 radiusSum;
    f32 secondRadius;
    f32 firstRadius;
    f32 centerDistance;

    PSVECSubtract(&first->center, &second->center, &centerDelta);
    secondRadius = second->radius;
    firstRadius = first->radius;
    radiusSum = firstRadius + secondRadius;
    centerDistance = PSVECMag(&centerDelta);
    return centerDistance < radiusSum;
}

// Used for minigame collision response to move overlapping centers apart.
// firstShare divides the correction between the spheres; correctionScale scales that correction.
// Returns 0 without moving separated or just-touching spheres, and 1 after applying a correction.
s32 resolveSphereContact(Sphere *first, Sphere *second, f32 firstShare, f32 correctionScale)
{
    Point3d separation;
    Vector3 correction;
    f32 firstCorrectionShare;
    f32 secondCorrectionShare;
    f32 combinedRadius;
    f32 centerDistance;

    combinedRadius = first->getRadius() + second->getRadius();
    PSVECSubtract(&second->center, &first->center, &separation);
    centerDistance = vectorMagnitude(&separation);
    if (centerDistance >= combinedRadius)
        return 0;

    firstCorrectionShare = firstShare;
    secondCorrectionShare = 1.0f - firstShare;
    if (centerDistance < 0.0000001f) {
        // Use the X axis when the centers are too close to define a separation direction.
        // The forced distance also makes the correction use combinedRadius minus one unit.
        separation.x = 1.0f;
        separation.y = 0.0f;
        separation.z = 0.0f;
        centerDistance = 1.0f;
    }

    PSVECScale(&separation, &separation,
               (correctionScale * (combinedRadius - centerDistance)) / centerDistance);
    PSVECScale(&separation, &correction.vector, -firstCorrectionShare);
    first->translate(correction);
    PSVECScale(&separation, &correction.vector, secondCorrectionShare);
    second->translate(correction);
    return 1;
}

// Used for swept sphere checks from saved start centers to the spheres' current centers.
// Writes the earlier candidate contact parameter, where zero is the start and one is the end.
// Returns 0 only for negligible relative motion; a return of 1 does not validate an intersection.
s32 intersectSpherePaths(f32 *contactTime, Point3d *firstStart, Sphere *first,
                         Point3d *secondStart, Sphere *second)
{
    Point3d firstMotion;
    Point3d secondMotion;
    Point3d startDelta;
    Point3d relativeMotion;
    f32 relativeMotionSquared;
    f32 startProjection;
    f32 discriminant;

    PSVECSubtract(&first->center, firstStart, &firstMotion);
    PSVECSubtract(&second->center, secondStart, &secondMotion);
    PSVECSubtract(&firstMotion, &secondMotion, &relativeMotion);
    relativeMotionSquared = vectorDot(&relativeMotion, &relativeMotion);
    if ((relativeMotionSquared > -0.000001f) &&
        (relativeMotionSquared < 0.000001f))
        return 0;

    PSVECSubtract(firstStart, secondStart, &startDelta);
    startProjection = vectorDot(&startDelta, &relativeMotion);
    discriminant = square(startProjection) -
        (relativeMotionSquared *
         (vectorDot(&startDelta, &startDelta) -
          square(first->getRadius() + second->getRadius())));
    // sqrtf returns a nonpositive discriminant unchanged; neither it nor the time range is
    // rejected.
    *contactTime = (-startProjection - sqrtf(discriminant)) / relativeMotionSquared;
    return 1;
}
