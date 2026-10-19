/* Collision geometry for moving drawing points and player boundaries. */
#include "REL/m613Dll/geometry.h"
#include "game/main.h"
#include <math.h>

extern s32 fn_1_10C5C(Point3d *, Point3d *);
extern void fn_1_1138(f32, f32, f32, const Vec *, Vec *);
s32 fn_1_E8C(Point3d *firstPosition, const Point3d *firstDisplacement, f32 firstRadius,
             Point3d *secondPosition, const Point3d *secondDisplacement, f32 secondRadius,
             f32 *result);

/* Used by player separation and progress pieces to test whether two bounding spheres overlap. */
s32 fn_1_0(Point3d firstPosition, f32 firstRadius,
           Point3d secondPosition, f32 secondRadius)
{
    firstRadius += secondRadius;
    firstRadius *= firstRadius;

    PSVECSubtract(&firstPosition, &secondPosition, &firstPosition);
    secondRadius = PSVECSquareMag(&firstPosition);
    if (secondRadius < firstRadius) {
        return 1;
    }
    return 0;
}

/* Nudges equal X/Z coordinates. During overlap, removes negative motion along the chosen
 * contact direction and scales the remainder by one minus fallbackScale; near-zero separation
 * uses a random direction. */
s32 fn_1_8C(Point3d *position, Point3d *direction, f32 firstRadius,
            Point3d *center, f32 radius, f32 fallbackScale) {
    Point3d directionFromCenter;
    Point3d positionCopy;
    Point3d centerCopy;
    f32 radiusOrSeparationSquared;
    f32 projection;
    f32 combinedRadiusSquared;
    s32 intersects;

    if (position->x == center->x) {
        position->x += 0.001f;
    }
    if (position->z == center->z) {
        position->z += 0.001f;
    }
    positionCopy = *position;
    centerCopy = *center;

    radiusOrSeparationSquared = radius;
    combinedRadiusSquared = firstRadius;
    combinedRadiusSquared += radiusOrSeparationSquared;
    combinedRadiusSquared *= combinedRadiusSquared;
    PSVECSubtract(&positionCopy, &centerCopy, &positionCopy);
    radiusOrSeparationSquared = PSVECSquareMag(&positionCopy);
    if (radiusOrSeparationSquared < combinedRadiusSquared) {
        intersects = 1;
    } else {
        intersects = 0;
    }

    if (intersects != 0) {
        PSVECSubtract(position, center, &directionFromCenter);
        fn_1_10C5C(&directionFromCenter, &directionFromCenter);
        projection = PSVECDotProduct(&directionFromCenter, direction);
        PSVECScale(&directionFromCenter, &directionFromCenter, projection);
        PSVECSubtract(direction, &directionFromCenter, direction);
        if (projection >= 0.0f) {
            PSVECAdd(direction, &directionFromCenter, direction);
        } else {
            fallbackScale = 1.0f - fallbackScale;
            PSVECScale(direction, direction, fallbackScale);
        }
        return 1;
    }
    return 0;
}

/* Checks two moving spheres for contact and adjusts both directions, using a half-scale correction
 * for negative projections. */
s32 fn_1_28C(Point3d firstPosition, Point3d *firstDirection,
             f32 firstRadius, Point3d secondPosition,
             Point3d *secondDirection, f32 secondRadius) {
    Point3d contactVector;
    Point3d firstDirectionCopy;
    Point3d secondDirectionCopy;
    Point3d normal;
    f32 separationSquaredOrProjection;
    f32 parameter;
    f32 fallbackScale;
    s32 clampToStart;
    s32 clampToEnd;
    if (fn_1_E8C((&firstPosition), firstDirection, firstRadius, (&secondPosition), secondDirection,
                 secondRadius, &parameter) == 0) {
        PSVECSubtract((&firstPosition), (&secondPosition), & contactVector);
        separationSquaredOrProjection = PSVECSquareMag(& contactVector);
        if (separationSquaredOrProjection >
            ((firstRadius + secondRadius) * (firstRadius + secondRadius))) {
            return 0;
        }
        parameter = 0.0f;
    }
    clampToStart = 0;
    if ((-0.0001f <= parameter) && (parameter <= 0.0f))
    {
        clampToStart = 1;
    }
    clampToStart ? (parameter = 0.0f) : 0;
    clampToEnd = 0;
    if ((1.0001f >= parameter) && (parameter >= 1.0f))
    {
        clampToEnd = 1;
    }
    clampToEnd ? (parameter = 1.0f) : 0;
    if (! (0.0f <= parameter) || ! (parameter <= 1.0f))
    {
        PSVECSubtract((&firstPosition), (&secondPosition), & contactVector);
        separationSquaredOrProjection = PSVECSquareMag(& contactVector);
        if (separationSquaredOrProjection >
            ((firstRadius + secondRadius) * (firstRadius + secondRadius))) {
            return 0;
        }
        parameter = 0.0f;
    }
    fallbackScale = 0.5f;
    firstDirectionCopy = * firstDirection;
    PSVECScale(firstDirection, firstDirection, parameter);
    PSVECScale(& firstDirectionCopy, & firstDirectionCopy, 1.0f - parameter);
    PSVECAdd((&firstPosition), firstDirection, (&firstPosition));
    secondDirectionCopy = * secondDirection;
    PSVECScale(secondDirection, secondDirection, parameter);
    PSVECScale(& secondDirectionCopy, & secondDirectionCopy, 1.0f - parameter);
    PSVECAdd((&secondPosition), secondDirection, (&secondPosition));
    PSVECSubtract((&firstPosition), (&secondPosition), & normal);
    fn_1_10C5C(& normal, & normal);
    separationSquaredOrProjection = PSVECDotProduct(& normal, & firstDirectionCopy);
    contactVector = normal;
    PSVECScale(& contactVector, & contactVector, separationSquaredOrProjection);
    PSVECSubtract(& firstDirectionCopy, & contactVector, & contactVector);
    if (separationSquaredOrProjection >= 0.0f)
    {
        PSVECAdd(firstDirection, & firstDirectionCopy, firstDirection);
    }
    else
    {
        PSVECScale(& contactVector, & contactVector, fallbackScale);
        PSVECAdd(firstDirection, & contactVector, firstDirection);
    }
    PSVECScale(& normal, & normal, -1.0f);
    separationSquaredOrProjection = PSVECDotProduct(& normal, & secondDirectionCopy);
    contactVector = normal;
    PSVECScale(& contactVector, & contactVector, separationSquaredOrProjection);
    PSVECSubtract(& secondDirectionCopy, & contactVector, & contactVector);
    if (separationSquaredOrProjection >= 0.0f)
    {
        PSVECAdd(secondDirection, & secondDirectionCopy, secondDirection);
    }
    else
    {
        PSVECScale(& contactVector, & contactVector, fallbackScale);
        PSVECAdd(secondDirection, & contactVector, secondDirection);
    }
    return 1;
}

static const Point3d lbl_1_rodata_2C = {0.0f, 0.0f, 1.0f};
static const Point3d lbl_1_rodata_38 = {1.0f, 0.0f, 0.0f};

/* Checks two moving players for contact and adjusts the second direction when their paths
 * overlap. */
s32 fn_1_70C(Point3d firstPosition, f32 angle, f32 firstRadius,
             Point3d firstDirection, Point3d secondPosition,
             f32 secondRadius, Point3d *secondDirection) {
    Point3d distance;
    Point3d separation;
    Point3d firstAxis;
    Point3d secondAxis;
    Point3d firstEnd;
    Point3d secondEnd;
    Point3d secondDirectionCopy;
    Point3d rotatedFirstAxis;
    const Vec *rotatedFirstAxisPointer;
    Point3d rotatedSecondAxis;
    const Vec *rotatedSecondAxisPointer;
    Point3d contactDirectionCopy;
    const Vec *contactDirectionPointer;
    f32 secondProjection;
    f32 separationSquaredOrProjection;
    f32 correctionScale;
    s32 clampToStart;
    s32 clampToEnd;
    f32 travel;

    if (fn_1_E8C((&firstPosition), (&firstDirection), firstRadius,
                 (&secondPosition), secondDirection, secondRadius,
                 &travel) == 0) {
        PSVECSubtract((&firstPosition), (&secondPosition), &distance);
        separationSquaredOrProjection = PSVECSquareMag(&distance);
        if (separationSquaredOrProjection >
            ((firstRadius + secondRadius) * (firstRadius + secondRadius))) {
            return 0;
        }
        travel = 0.0f;
    }

    clampToStart = 0;
    if ((-0.0001f <= travel) && (travel <= 0.0f)) {
        clampToStart = 1;
    }
    clampToStart ? (travel = 0.0f) : 0;
    clampToEnd = 0;
    if ((1.0001f >= travel) && (travel >= 1.0f)) {
        clampToEnd = 1;
    }
    clampToEnd ? (travel = 1.0f) : 0;
    if (!(0.0f <= travel) || !(travel <= 1.0f)) {
        PSVECSubtract((&firstPosition), (&secondPosition), &distance);
        separationSquaredOrProjection = PSVECSquareMag(&distance);
        if (separationSquaredOrProjection >
            ((firstRadius + secondRadius) * (firstRadius + secondRadius))) {
            return 0;
        }
        travel = 0.0f;
    }

    firstAxis = lbl_1_rodata_2C;
    secondAxis = lbl_1_rodata_38;
    correctionScale = (f32)lbl_1_bss_0;
    PSVECScale((&firstDirection), (&firstDirection), travel);
    PSVECAdd((&firstPosition), (&firstDirection), &firstEnd);
    secondDirectionCopy = *secondDirection;
    PSVECScale(secondDirection, secondDirection, travel);
    PSVECScale(&secondDirectionCopy, &secondDirectionCopy, 1.0f - travel);
    PSVECAdd((&secondPosition), secondDirection, &secondEnd);
    PSVECSubtract(&secondEnd, &firstEnd, &separation);
    fn_1_10C5C(&separation, &separation);

    rotatedFirstAxis = firstAxis;
    rotatedFirstAxisPointer = (const Vec *)&rotatedFirstAxis;
    fn_1_1138(0.0f, 45.0f + angle, 0.0f,
              rotatedFirstAxisPointer, (Vec *)&firstAxis);
    rotatedSecondAxis = secondAxis;
    rotatedSecondAxisPointer = (const Vec *)&rotatedSecondAxis;
    fn_1_1138(0.0f, 45.0f + angle, 0.0f,
              rotatedSecondAxisPointer, (Vec *)&secondAxis);

    separationSquaredOrProjection = PSVECDotProduct(&firstAxis, &separation);
    secondProjection = PSVECDotProduct(&secondAxis, &separation);
    separation.x = separation.y = separation.z = 0.0f;
    if ((separationSquaredOrProjection > 0.0f) && (secondProjection < 0.0f)) {
        separation.z = 1.0f;
    } else if ((separationSquaredOrProjection > 0.0f) && (secondProjection > 0.0f)) {
        separation.x = 1.0f;
    } else if ((separationSquaredOrProjection < 0.0f) && (secondProjection < 0.0f)) {
        separation.x = -1.0f;
    } else {
        separation.z = -1.0f;
    }
    contactDirectionCopy = separation;
    contactDirectionPointer = (const Vec *)&contactDirectionCopy;
    fn_1_1138(0.0f, angle, 0.0f,
              contactDirectionPointer, (Vec *)&separation);
    travel = PSVECDotProduct(&separation, &secondDirectionCopy);
    PSVECScale(&separation, &separation, travel);
    PSVECSubtract(&secondDirectionCopy, &separation, &separation);
    if (travel >= 0.0f) {
        PSVECAdd(secondDirection, &secondDirectionCopy, secondDirection);
    } else {
        PSVECScale(&separation, &separation, correctionScale);
        PSVECAdd(secondDirection, &separation, secondDirection);
    }
    return 1;
}

/* Computes closest-approach parameters for two direction vectors; returns zero when near-parallel
 * or degenerate. */
s32 fn_1_CFC(Point3d *firstStart, Point3d *firstDirection, Point3d *secondStart,
             Point3d *secondDirection, f32 *firstAmount, f32 *secondAmount) {
    Point3d separation;
    f32 firstDirectionSquared;
    f32 secondDirectionSquared;
    f32 firstProjection;
    f32 secondProjection;
    f32 directionDot;
    f32 denominator;

    PSVECSubtract(secondStart, firstStart, &separation);
    firstDirectionSquared = PSVECDotProduct(firstDirection, firstDirection);
    directionDot = PSVECDotProduct(firstDirection, secondDirection);
    secondDirectionSquared = PSVECDotProduct(secondDirection, secondDirection);
    firstProjection = PSVECDotProduct(&separation, firstDirection);
    secondProjection = PSVECDotProduct(&separation, secondDirection);
    denominator = (firstDirectionSquared * secondDirectionSquared) - (directionDot * directionDot);
    if (fabsf(denominator) < 0.000001) {
        return 0;
    }
    *firstAmount =
        ((secondDirectionSquared * firstProjection) - (directionDot * secondProjection)) /
        denominator;
    *secondAmount =
        ((directionDot * firstProjection) - (firstDirectionSquared * secondProjection)) /
        denominator;
    return 1;
}

/* Computes a candidate travel fraction using relative movement and a position difference read from
 * the pointer parameters' storage; writes the result only on success. */
s32 fn_1_E8C(Point3d *firstPosition, const Point3d *firstDisplacement, f32 firstRadius,
             Point3d *secondPosition, const Point3d *secondDisplacement, f32 secondRadius,
             f32 *result) {
    Point3d firstDirection;
    Point3d secondDirection;
    f32 directionDot;
    f32 firstLengthSquared;
    f32 secondLengthSquared;
    f32 discriminant;

    PSVECSubtract((const Point3d *) &firstPosition, (const Point3d *) &secondPosition,
                  &firstDirection);
    PSVECSubtract(firstDisplacement, secondDisplacement, &secondDirection);
    directionDot = PSVECDotProduct(&firstDirection, &secondDirection);
    secondLengthSquared = PSVECSquareMag(&secondDirection);
    firstLengthSquared = PSVECSquareMag(&firstDirection);

    if (-0.000001 < secondLengthSquared && secondLengthSquared < 0.000001) {
        return 0;
    }

    discriminant = (directionDot * directionDot) -
                   (secondLengthSquared * (firstLengthSquared -
                    ((firstRadius + secondRadius) * (firstRadius + secondRadius))));
    if (discriminant < 0.0f) {
        return 0;
    }

    discriminant = sqrt(discriminant);
    *result = (-directionDot - discriminant) / secondLengthSquared;
    return 1;
}

/* Builds an X/Y/Z rotation matrix from the supplied angles and transforms source into
 * destination. */
void fn_1_1138(f32 angle_x, f32 angle_y, f32 angle_z,
               const Vec *source, Vec *destination)
{
    Mtx x_rotation;
    Mtx y_rotation;

    PSMTXRotRad(x_rotation, 'x', 0.017453292f * angle_x);
    PSMTXRotRad(y_rotation, 'y', 0.017453292f * angle_y);
    PSMTXConcat(y_rotation, x_rotation, x_rotation);
    PSMTXRotRad(y_rotation, 'z', 0.017453292f * angle_z);
    PSMTXConcat(y_rotation, x_rotation, x_rotation);
    PSMTXMultVecSR(x_rotation, source, destination);
}

s16 lbl_1_bss_0;
