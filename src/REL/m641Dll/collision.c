/* Mole-it! collision and geometry helpers. */
#include <math.h>

#include "game/main.h"

#include "game/hu3d.h"

#include "game/frand.h"

extern s32 lbl_1_bss_11C;

extern s32 lbl_1_bss_8;

extern s32 lbl_1_bss_1218;

extern s32 lbl_1_bss_10C;

extern s32 lbl_1_bss_FC;

/* Test two spheres during collision checks; point is replaced by their separation vector. */
s32 fn_1_0(Point3d *point, f32 firstRadius, Point3d *center,
           f32 secondRadius)
{
    firstRadius += secondRadius;
    firstRadius *= firstRadius;

    PSVECSubtract(point, center, point);
    secondRadius = PSVECSquareMag(point);
    if (secondRadius < firstRadius) {
        return 1;
    }
    return 0;
}

extern Point3d lbl_1_data_180[14];

extern Point3d lbl_1_bss_0;

extern Point3d lbl_1_bss_C;

extern s32 fn_1_E10(const Point3d *firstPosition, const Point3d *firstDisplacement,
                    f32 firstRadius, const Point3d *secondPosition,
                    const Point3d *secondDisplacement, f32 secondRadius,
                    f32 *result);

extern void fn_1_146B8(Point3d *source, Point3d *destination);

typedef struct M641LightConfiguration {
    HU3D_LIGHTID primaryLight;
    s16 padding;
    HuVecF primaryPosition;
    HuVecF primaryDirection;
    GXColor primaryColor;
    HU3D_LIGHTID alternateLight;
    s16 alternatePadding;
    HuVecF alternatePosition;
    HuVecF alternateDirection;
    GXColor alternateColor;
} M641LightConfiguration;

extern s16 lbl_1_bss_7DC;

extern f32 lbl_1_bss_20;

extern s16 lbl_1_bss_24[6];

extern f32 lbl_1_bss_558[2];

extern Point3d lbl_1_rodata_78;

extern Point3d lbl_1_rodata_84;

extern Point3d lbl_1_rodata_90;

extern void fn_1_1DD4(void);

/* fn_1_540 resolves swept contact here, modifying both positions and displacement vectors. */
s32 fn_1_8C(Point3d *first, Point3d *firstDisplacement, f32 firstRadius,
            Point3d *second, Point3d *secondDisplacement, f32 secondRadius) {
    Point3d projected;
    Point3d firstExtent;
    Point3d secondExtent;
    Point3d difference;
    f32 remainingRange;
    f32 firstProjectionLength;
    f32 closestPoint;
    s32 atUpperLimit;
    s32 atLowerLimit;
    if (fn_1_E10(first, firstDisplacement, firstRadius, second, secondDisplacement, secondRadius,
                 &closestPoint) == 0) {
        PSVECSubtract(first, second, & projected);
        firstProjectionLength = PSVECSquareMag(& projected);
        if (firstProjectionLength > ((firstRadius + secondRadius) * (firstRadius + secondRadius)))
        {
            return 0;
        }
        closestPoint = 0.0f;
    }
    atUpperLimit = 0;
    if (-0.0001f <= closestPoint && closestPoint <= 0.0f)
    {
        atUpperLimit = 1;
    }
    atUpperLimit ? (closestPoint = 0.0f) : 0;
    atLowerLimit = 0;
    if (1.0001f >= closestPoint && closestPoint >= 1.0f)
    {
        atLowerLimit = 1;
    }
    atLowerLimit ? (closestPoint = 1.0f) : 0;
    if (! (0.0f <= closestPoint) || ! (closestPoint <= 1.0f))
    {
        PSVECSubtract(first, second, & projected);
        firstProjectionLength = PSVECSquareMag(& projected);
        if (firstProjectionLength > ((firstRadius + secondRadius) * (firstRadius + secondRadius)))
        {
            return 0;
        }
        closestPoint = 0.0f;
    }
    remainingRange = 1.0f;
     firstExtent = * firstDisplacement;
     PSVECScale(firstDisplacement, firstDisplacement, closestPoint);
    PSVECScale(& firstExtent, & firstExtent, 1.0f - closestPoint);
     PSVECAdd(first, firstDisplacement, first);
     secondExtent = * secondDisplacement;
     PSVECScale(secondDisplacement, secondDisplacement, closestPoint);
    PSVECScale(& secondExtent, & secondExtent, 1.0f - closestPoint);
     PSVECAdd(second, secondDisplacement, second);
    PSVECAdd(first, second, & lbl_1_bss_0);
    PSVECScale(& lbl_1_bss_0, & lbl_1_bss_0, 0.5f);
    PSVECSubtract(first, second, & difference);
    fn_1_146B8(& difference, & difference);
    firstProjectionLength = PSVECDotProduct(& difference, & firstExtent);
    projected = difference;
    PSVECScale(& projected, & projected, firstProjectionLength);
    PSVECSubtract(& firstExtent, & projected, & projected);
    if (firstProjectionLength >= 0.0f)
    {
         PSVECAdd(firstDisplacement, & firstExtent, firstDisplacement);
    }
    else
    {
        PSVECScale(& projected, & projected, remainingRange);
         PSVECAdd(firstDisplacement, & projected, firstDisplacement);
    }
    PSVECScale(& difference, & difference, -1.0f);
    firstProjectionLength = PSVECDotProduct(& difference, & secondExtent);
    projected = difference;
    PSVECScale(& projected, & projected, firstProjectionLength);
    PSVECSubtract(& secondExtent, & projected, & projected);
    if (firstProjectionLength >= 0.0f)
    {
         PSVECAdd(secondDisplacement, & secondExtent, secondDisplacement);
    }
    else
    {
        PSVECScale(& projected, & projected, remainingRange);
         PSVECAdd(secondDisplacement, & projected, secondDisplacement);
    }
    return 1;
}

/* fn_1_8400 and fn_1_B108 use two vertical contact tests to resolve moving capsule collisions. */
s32 fn_1_540(Point3d *first, f32 firstRadius, f32 firstExtent, Point3d *firstDisplacement,
             Point3d *second, f32 secondRadius, f32 secondExtent, Point3d *secondDisplacement) {
    Point3d firstOriginal;
    Point3d secondOriginal;
    Point3d firstCandidate;
    Point3d secondCandidate;
    Point3d firstContact;
    Point3d secondContact;
    Point3d *secondCandidatePointer;
    Point3d *firstCandidatePointer;
    Point3d *secondContactPointer;
    Point3d *firstContactPointer;
    f32 sharedCenter;

    firstOriginal = *first;
    secondOriginal = *second;
    secondOriginal.y += secondExtent;
    sharedCenter = (firstOriginal.y + secondOriginal.y) / 2;
    secondOriginal.y = sharedCenter;
    firstOriginal.y = sharedCenter;
    if (first->y + firstRadius > sharedCenter) {
        firstOriginal.y = first->y + firstRadius;
    } else if (first->y + firstExtent - firstRadius < sharedCenter) {
        firstOriginal.y = (first->y + firstExtent) - firstRadius;
    }
    if (second->y + secondRadius > sharedCenter) {
        secondOriginal.y = second->y + secondRadius;
    } else if (second->y + secondExtent - secondRadius < sharedCenter) {
        secondOriginal.y = (second->y + secondExtent) - secondRadius;
    }
    secondCandidate = secondOriginal;
    secondCandidatePointer = &secondCandidate;
    firstCandidate = firstOriginal;
    firstCandidatePointer = &firstCandidate;
    if (fn_1_8C(firstCandidatePointer, firstDisplacement, firstRadius,
                secondCandidatePointer, secondDisplacement, secondRadius) != 0) {
        lbl_1_bss_C = lbl_1_bss_0;
        return 1;
    }

    firstOriginal = *first;
    secondOriginal = *second;
    firstOriginal.y += firstExtent;
    sharedCenter = (firstOriginal.y + secondOriginal.y) / 2;
    secondOriginal.y = sharedCenter;
    firstOriginal.y = sharedCenter;
    if (first->y + firstRadius > sharedCenter) {
        firstOriginal.y = first->y + firstRadius;
    } else if (first->y + firstExtent - firstRadius < sharedCenter) {
        firstOriginal.y = (first->y + firstExtent) - firstRadius;
    }
    if (second->y + secondRadius > sharedCenter) {
        secondOriginal.y = second->y + secondRadius;
    } else if (second->y + secondExtent - secondRadius < sharedCenter) {
        secondOriginal.y = (second->y + secondExtent) - secondRadius;
    }
    secondContact = secondOriginal;
    secondContactPointer = &secondContact;
    firstContact = firstOriginal;
    firstContactPointer = &firstContact;
    if (fn_1_8C(firstContactPointer, firstDisplacement, firstRadius,
                secondContactPointer, secondDisplacement, secondRadius) != 0) {
        lbl_1_bss_C = lbl_1_bss_0;
        return 1;
    }
    return 0;
}

/* Tests capsule overlap at two vertical contact positions; no module function calls it. */
s32 fn_1_8C8(Point3d *first, f32 firstRadius, f32 firstExtent,
             Point3d *second, f32 secondRadius, f32 secondExtent) {
    Point3d firstOriginal;
    Point3d secondOriginal;
    Point3d firstCandidate;
    Point3d secondCandidate;
    Point3d firstCandidateAgain;
    Point3d secondCandidateAgain;
    f32 firstDistanceSquared;
    f32 secondDistanceSquared;
    f32 sharedCenter;
    f32 firstRadiusSum;
    f32 secondRadiusSum;
    s32 firstOverlap;
    s32 secondOverlap;

    firstOriginal = *first;
    secondOriginal = *second;
    secondOriginal.y += secondExtent;
    sharedCenter = (firstOriginal.y + secondOriginal.y) / 2;
    secondOriginal.y = sharedCenter;
    firstOriginal.y = sharedCenter;
    if (first->y + firstRadius > sharedCenter) {
        firstOriginal.y = first->y + firstRadius;
    } else if (first->y + firstExtent - firstRadius < sharedCenter) {
        firstOriginal.y = (first->y + firstExtent) - firstRadius;
    }
    if (second->y + secondRadius > sharedCenter) {
        secondOriginal.y = second->y + secondRadius;
    } else if (second->y + secondExtent - secondRadius < sharedCenter) {
        secondOriginal.y = (second->y + secondExtent) - secondRadius;
    }
    firstCandidate = firstOriginal;
    secondCandidate = secondOriginal;
    firstDistanceSquared = secondRadius;
    firstRadiusSum = firstRadius;
    firstRadiusSum += firstDistanceSquared;
    firstRadiusSum *= firstRadiusSum;
    PSVECSubtract(&firstCandidate, &secondCandidate, &firstCandidate);
    firstDistanceSquared = PSVECSquareMag(&firstCandidate);
    if (firstDistanceSquared < firstRadiusSum) {
        firstOverlap = 1;
    } else {
        firstOverlap = 0;
    }
    if (firstOverlap != 0) {
        return 1;
    }

    firstOriginal = *first;
    secondOriginal = *second;
    firstOriginal.y += firstExtent;
    sharedCenter = (firstOriginal.y + secondOriginal.y) / 2;
    secondOriginal.y = sharedCenter;
    firstOriginal.y = sharedCenter;
    if (first->y + firstRadius > sharedCenter) {
        firstOriginal.y = first->y + firstRadius;
    } else if (first->y + firstExtent - firstRadius < sharedCenter) {
        firstOriginal.y = (first->y + firstExtent) - firstRadius;
    }
    if (second->y + secondRadius > sharedCenter) {
        secondOriginal.y = second->y + secondRadius;
    } else if (second->y + secondExtent - secondRadius < sharedCenter) {
        secondOriginal.y = (second->y + secondExtent) - secondRadius;
    }
    firstCandidateAgain = firstOriginal;
    secondCandidateAgain = secondOriginal;
    secondDistanceSquared = secondRadius;
    secondRadiusSum = firstRadius;
    secondRadiusSum += secondDistanceSquared;
    secondRadiusSum *= secondRadiusSum;
    PSVECSubtract(&firstCandidateAgain, &secondCandidateAgain, &firstCandidateAgain);
    secondDistanceSquared = PSVECSquareMag(&firstCandidateAgain);
    if (secondDistanceSquared < secondRadiusSum) {
        secondOverlap = 1;
    } else {
        secondOverlap = 0;
    }
    if (secondOverlap != 0) {
        return 1;
    }
    return 0;
}

/* Solves the closest parameters for two lines; no function in this module calls it. */
s32 fn_1_C80(Point3d *firstStart, Point3d *firstDirection, Point3d *secondStart,
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
    if (fabsf(denominator) < 1e-06) {
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

/* fn_1_8C obtains the first intersection parameter for two moving spheres here. */
s32 fn_1_E10(const Point3d *firstPosition, const Point3d *firstDisplacement,
             f32 firstRadius, const Point3d *secondPosition,
             const Point3d *secondDisplacement, f32 secondRadius,
             f32 *result) {
    Point3d firstDirection;
    Point3d secondDirection;
    f32 directionDot;
    f32 firstLengthSquared;
    f32 secondLengthSquared;
    f32 discriminant;

    PSVECSubtract(firstPosition, secondPosition, &firstDirection);
    PSVECSubtract(firstDisplacement, secondDisplacement, &secondDirection);
    directionDot = PSVECDotProduct(&firstDirection, &secondDirection);
    secondLengthSquared = PSVECSquareMag(&secondDirection);
    firstLengthSquared = PSVECSquareMag(&firstDirection);

    if (-1e-06 < secondLengthSquared && secondLengthSquared < 1e-06) {
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
