// Geometry helpers: a point pulled towards the reference distance from a pivot, the rotation between two directions,
// a rotation's axis, a reflection, planes, a lerp along a segment and a point-in-triangle test.
// Geometry support for the course and its players.
#include "REL/m604DLL/mtx64.h"

// firstPoint moved along the edge from secondPoint by radialScale times its shortfall from referenceLength plus
// projectionScale times the edge's projection of (thirdPoint - secondPoint) * sideScale over the edge length.
Vec fn_1_9D78(const Vec *firstPoint, const Vec *secondPoint, const Vec *thirdPoint,
              f32 sideScale, f32 radialScale, f32 projectionScale, f32 referenceLength)
{
    Vec firstEdge;
    Vec scaledSecondEdge;
    Vec output;
    f32 distance;
    f32 firstEdgeLength;
    f32 projection;
    f32 secondEdgeLength;
    PSVECSubtract(firstPoint, secondPoint, &firstEdge);
    scale(subtract(thirdPoint, secondPoint, &output), &scaledSecondEdge, sideScale);
    firstEdgeLength = PSVECMag(&firstEdge);
    projection = PSVECDotProduct(&firstEdge, &scaledSecondEdge);
    secondEdgeLength = PSVECMag(&firstEdge);
    distance = radialScale * (referenceLength - secondEdgeLength) + projectionScale * (projection / firstEdgeLength);
    PSVECNormalize(&firstEdge, &firstEdge);
    PSVECScale(&firstEdge, &firstEdge, distance);
    return *add(&firstEdge, firstPoint, &output);
}

/* Builds the quaternion that rotates sourceDirection onto targetDirection. */
void fn_1_9ECC(Quaternion *result, const Vec *sourceDirection, const Vec *targetDirection)
{
    Vec normalizedSource, normalizedTarget, crossProduct;
    double dot, magnitude, inverseMagnitude;
    PSVECNormalize(sourceDirection, &normalizedSource);
    PSVECNormalize(targetDirection, &normalizedTarget);
    dot = PSVECDotProduct(&normalizedSource, &normalizedTarget);
    magnitude = dot;
    inverseMagnitude = sqrt(2.0 + 2.0 * magnitude);
    result->w = (f32)(inverseMagnitude / 2.0);
    inverseMagnitude = 1.0 / inverseMagnitude;
    PSVECCrossProduct(&normalizedSource, &normalizedTarget, &crossProduct);
    result->x = (f32)((double)crossProduct.x * inverseMagnitude);
    result->y = (f32)((double)crossProduct.y * inverseMagnitude);
    result->z = (f32)((double)crossProduct.z * inverseMagnitude);
}
// The rotation's axis 'X', 'Y' or 'Z' (negative: the axis reversed) as a direction.
void fn_1_A228(Vec *result, const Quaternion *rotation, u32 axis)
{
    f32 x, y, z, w;
    f64 first, second, third;
    u32 negative;
    s32 absoluteAxis;
    x = rotation->x;
    y = rotation->y;
    z = rotation->z;
    w = rotation->w;
    negative = axis >> 31;
    if ((u8)negative != 0) {
        absoluteAxis = -(s32)axis;
    } else {
        absoluteAxis = (s32)axis;
    }
    switch (absoluteAxis) {
    case 'X':
        first = 1.0 + (-2.0 * (((f64)y * (f64)y) + ((f64)z * (f64)z)));
        second = 2.0 * (((f64)x * (f64)y) - ((f64)w * (f64)z));
        third = 2.0 * (((f64)x * (f64)z) + ((f64)w * (f64)y));
        break;
    case 'Y':
        first = 2.0 * (((f64)x * (f64)y) + ((f64)w * (f64)z));
        second = 1.0 + (-2.0 * (((f64)x * (f64)x) - ((f64)z * (f64)z)));
        third = 2.0 * (((f64)y * (f64)z) + ((f64)w * (f64)x));
        break;
    case 'Z':
        first = 2.0 * (((f64)x * (f64)z) + ((f64)w * (f64)y));
        second = 2.0 * (((f64)y * (f64)z) - ((f64)w * (f64)x));
        third = 1.0 + (-2.0 * (((f64)x * (f64)x) + ((f64)y * (f64)y)));
        break;
    default:
        assert(!"Invalid Axis");
        return;
    }

    if ((u8)negative != 0) {
        first = -first;
        second = -second;
        third = -third;
    }
    result->x = (f32)first;
    result->y = (f32)second;
    result->z = (f32)third;
}

typedef Vec64<double> Vec64d;

// A plane: its unit normal and its distance from the origin along it.
struct Plane64 {
    Vec64d normal;
    double distance;
};

void fn_1_A4B8(Vec *result, Vec *left, Vec *right)
{
    f32 scale;
    f32 dotProduct;
    dotProduct = PSVECDotProduct(left, right);
    scale = -2.0f * dotProduct;
    PSVECScale(left, result, scale);
    PSVECAdd(right, result, result);
}

void fn_1_A558(Plane64 *result, const Vec *start, const Vec *middle, const Vec *end)
{
    Vec64d firstDifference;
    Vec64d secondDifference;

    subtract(start, middle, &firstDifference);
    subtract(middle, end, &secondDifference);
    firstDifference.cross(secondDifference, &result->normal);
    result->normal.normalize(&result->normal);
    result->distance = -dot(result->normal, *start);
}

double fn_1_A5EC(const Plane64 *left, const Vec *right)
{
    return dot(left->normal, *right) + left->distance;
}

void fn_1_A62C(Vec *result, const Vec *middle, double start,
               const Vec *source, double end)
{
    Vec64d difference;
    double duration;

    duration = end - start;
    if (duration != 0.0) {
        subtract(source, middle, &difference);
        difference.scale(&difference, -start / duration);
        add(middle, &difference, &difference);
        result->x = (f32)difference.x;
        result->y = (f32)difference.y;
        result->z = (f32)difference.z;
        return;
    }

    result->x = source->x;
    result->y = source->y;
    result->z = source->z;
}

s32 fn_1_A724(const Vec *firstPoint, const Vec *secondPoint,
              const Vec *thirdPoint, const Vec *referencePoint,
              const Vec64d *normal)
{
    Vec64d firstDifference;
    Vec64d secondDifference;
    Vec64d thirdDifference;
    Vec64d orientation;
    s32 allOrientationsPositive;
    u8 secondOrientationPositive;

    subtract(firstPoint, referencePoint, &firstDifference);
    subtract(secondPoint, referencePoint, &secondDifference);
    subtract(thirdPoint, referencePoint, &thirdDifference);
    allOrientationsPositive = 0;
    secondOrientationPositive = 0;
    if (dot(*firstDifference.cross(secondDifference, &orientation), *normal) >= 0.0) {
        if (dot(*secondDifference.cross(thirdDifference, &orientation), *normal) >= 0.0) {
            secondOrientationPositive = 1;
        }
    }
    if (secondOrientationPositive != 0) {
        if (dot(*thirdDifference.cross(firstDifference, &orientation), *normal) >= 0.0) {
            allOrientationsPositive = 1;
        }
    }
    return allOrientationsPositive;
}
