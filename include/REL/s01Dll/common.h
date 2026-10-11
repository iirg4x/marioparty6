// Shared curve and player-movement helpers used by board RELs.
#ifndef REL_S01DLL_COMMON_H
#define REL_S01DLL_COMMON_H

#include "dolphin.h"
#include "humath.h"
#include "math.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/process.h"
#include "game/board/player.h"
#include "string.h"

typedef float (*S01CurveEval)();

extern float *lbl_1_bss_48;

float mbBezierCalcSlope(float startValue, float controlValue, float endValue, float parameterT);
float mbHermiteCalcSlope(float startValue, float endValue, float startTangent, float endTangent,
                         float parameterT);
float mbAngleLerp(float startAngle, float endAngle, float weight);

float fn_1_1EF8(float startValue, float firstControlValue, float secondControlValue, float endValue,
                float parameterT);

// Estimates curve length with ten intervals (eleven endpoint-inclusive samples).
// fn_1_19A0 uses this estimate for inverse lookup.
float fn_1_12CC(S01CurveEval curveEval, HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, float parameterT)
{
    int sampleIndex;
    int subdivisionCount;
    float estimatedLength;
    float startParameter;
    float sampleParameter;
    float parameterStep;

    subdivisionCount = 10;
    startParameter = 0.0f;
    parameterStep = (parameterT - startParameter) / subdivisionCount;
    sampleParameter = startParameter;
    estimatedLength = 0.0f;
    for (sampleIndex = 0; sampleIndex < subdivisionCount - 1; sampleIndex++) {
        sampleParameter += parameterStep;
        estimatedLength += curveEval(pointA, pointB, pointC, pointD, sampleParameter);
    }
    estimatedLength =
        parameterStep * 0.5 *
        (curveEval(pointA, pointB, pointC, pointD, startParameter) +
         curveEval(pointA, pointB, pointC, pointD, parameterT) + (2.0 * estimatedLength));
    return estimatedLength;
}

// Refines curve length by halving the sampling interval; fn_1_1684 uses it for inverse lookup.
float fn_1_1474(S01CurveEval curveEval, HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, float parameterT)
{
    int subdivisionCount = 10;
    float startParameter = 0.0f;
    float lengthEstimate = 0.0f;
    float parameterStep = parameterT - startParameter;
    float endpointEstimate = parameterStep *
                             (curveEval(pointA, pointB, pointC, pointD, startParameter) +
                              curveEval(pointA, pointB, pointC, pointD, parameterT)) *
                             0.5f;
    float estimatedLength;
    int intervalIndex;
    int sampleIndex;

    for (sampleIndex = 1; sampleIndex <= subdivisionCount; sampleIndex *= 2) {
        estimatedLength = 0.0f;
        for (intervalIndex = 1; intervalIndex <= sampleIndex; intervalIndex++) {
            estimatedLength += curveEval(pointA, pointB, pointC, pointD,
                startParameter + parameterStep * (intervalIndex - 0.5f));
        }
        estimatedLength *= parameterStep;
        lengthEstimate = (1.0f / 3.0f)
            * (endpointEstimate + (2.0f * estimatedLength));
        parameterStep *= 0.5f;
        endpointEstimate = (endpointEstimate + estimatedLength) * 0.5f;
    }
    return lengthEstimate;
}

// Uses Newton iteration with the refined length estimate to find a target-distance parameter.
// When the slope magnitude is below 0.1, it uses 1.0 as the denominator.
float fn_1_1684(S01CurveEval curveEval, HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, float parameterT, float targetDistance, int maximumSteps)
{
    int iteration;
    float slopeDenominator;
    float distanceError;
    float previousParameter;
    float minimumSlopeMagnitude;

    minimumSlopeMagnitude = 0.1f;
    iteration = 0;
    do {
        distanceError =
            fn_1_1474(curveEval, pointA, pointB, pointC, pointD, parameterT) - targetDistance;
        if (fabs(slopeDenominator = curveEval(pointA, pointB, pointC, pointD, parameterT)) <
            minimumSlopeMagnitude) {
            slopeDenominator = 1.0f;
        }
        previousParameter = parameterT;
        parameterT -= distanceError / slopeDenominator;
        iteration++;
    } while (parameterT != previousParameter && iteration < maximumSteps);
    return parameterT;
}

// Uses Newton iteration with the ten-interval length estimate to find a target-distance parameter.
// When the slope magnitude is below 0.1, it uses 1.0 as the denominator.
float fn_1_19A0(S01CurveEval curveEval, HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, float parameterT, float targetDistance, int maximumSteps)
{
    int iteration;
    float slopeDenominator;
    float distanceError;
    float previousParameter;
    float minimumSlopeMagnitude;

    minimumSlopeMagnitude = 0.1f;
    iteration = 0;
    do {
        distanceError =
            fn_1_12CC(curveEval, pointA, pointB, pointC, pointD, parameterT) - targetDistance;
        if (fabs(slopeDenominator = curveEval(pointA, pointB, pointC, pointD, parameterT)) <
            minimumSlopeMagnitude) {
            slopeDenominator = 1.0f;
        }
        previousParameter = parameterT;
        parameterT -= distanceError / slopeDenominator;
        iteration++;
    } while (parameterT != previousParameter && iteration < maximumSteps);
    return parameterT;
}

// Returns the magnitude of a quadratic Bezier curve's slope at parameter t.
float fn_1_1C54(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC, float parameterT)
{
    HuVecF curveSlope;
    curveSlope.x = mbBezierCalcSlope(pointA->x, pointB->x, pointC->x, parameterT);
    curveSlope.y = mbBezierCalcSlope(pointA->y, pointB->y, pointC->y, parameterT);
    curveSlope.z = mbBezierCalcSlope(pointA->z, pointB->z, pointC->z, parameterT);
    return VECMag(&curveSlope);
}

// Returns the magnitude of the vector formed by applying fn_1_1EF8's component expression
// to x, y, and z.
float fn_1_1CF8(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC, HuVecF *pointD, float parameterT)
{
    HuVecF curveSlope;
    curveSlope.x = fn_1_1EF8(pointA->x, pointB->x, pointC->x, pointD->x, parameterT);
    curveSlope.y = fn_1_1EF8(pointA->y, pointB->y, pointC->y, pointD->y, parameterT);
    curveSlope.z = fn_1_1EF8(pointA->z, pointB->z, pointC->z, pointD->z, parameterT);
    return VECMag(&curveSlope);
}

// Returns the magnitude of a Hermite curve's slope at parameter t.
float fn_1_1DB4(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC, HuVecF *pointD, float parameterT)
{
    HuVecF curveSlope;
    curveSlope.x = mbHermiteCalcSlope(pointA->x, pointB->x, pointC->x, pointD->x, parameterT);
    curveSlope.y = mbHermiteCalcSlope(pointA->y, pointB->y, pointC->y, pointD->y, parameterT);
    curveSlope.z = mbHermiteCalcSlope(pointA->z, pointB->z, pointC->z, pointD->z, parameterT);
    return VECMag(&curveSlope);
}

// Evaluates one coordinate; fn_1_1FD0 applies it to all coordinates of a cubic Bezier curve.
float fn_1_1E70(float pointA, float pointB, float pointC, float pointD, float parameterT)
{
    float inverseParameter = 1.0f - parameterT;

    return (pointA * (inverseParameter * inverseParameter * inverseParameter)) +
           (pointB * (3.0f * parameterT * inverseParameter * inverseParameter)) +
           (pointC * (3.0f * parameterT * parameterT * inverseParameter)) +
           (pointD * (parameterT * parameterT * parameterT));
}

// Evaluates the module's cubic Bezier component expression for one coordinate.
// Its pointA coefficient is -3t squared - 6t - 3.
float fn_1_1EF8(float pointA, float pointB, float pointC, float pointD, float parameterT)
{
    float parameterSquared = parameterT * parameterT;

    return (pointA * ((-3.0f * parameterSquared) - (6.0f * parameterT) - 3.0f)) +
           (pointB * ((9.0f * parameterSquared) - (12.0f * parameterT) + 3.0f)) +
           (pointC * ((-9.0f * parameterSquared) + (6.0f * parameterT))) +
           ((3.0f * parameterSquared) * pointD);
}

// Evaluates a cubic Bezier position for all three world coordinates.
void fn_1_1FD0(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC, HuVecF *pointD, HuVecF *result,
    float parameterT)
{
    result->x = fn_1_1E70(pointA->x, pointB->x, pointC->x, pointD->x, parameterT);
    result->y = fn_1_1E70(pointA->y, pointB->y, pointC->y, pointD->y, parameterT);
    result->z = fn_1_1E70(pointA->z, pointB->z, pointC->z, pointD->z, parameterT);
}

// Applies fn_1_1EF8's cubic Bezier component expression to all three coordinates.
void fn_1_2290(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC, HuVecF *pointD, HuVecF *result,
    float parameterT)
{
    result->x = fn_1_1EF8(pointA->x, pointB->x, pointC->x, pointD->x, parameterT);
    result->y = fn_1_1EF8(pointA->y, pointB->y, pointC->y, pointD->y, parameterT);
    result->z = fn_1_1EF8(pointA->z, pointB->z, pointC->z, pointD->z, parameterT);
}

// Evaluates a Bezier curve from its control points using repeated interpolation.
void fn_1_2640(HuVecF *controlPoints, int pointCount, HuVecF *result, float parameterT)
{
    HuVecF *controlPointBuffer = HuMemDirectMallocNum(
        HEAP_HEAP, pointCount * sizeof(HuVecF), HU_MEMNUM_OVL);
    HuVecF *workingPoints = controlPointBuffer;
    int sampleIndex;
    int intervalIndex;
    memcpy(workingPoints, controlPoints, pointCount * sizeof(HuVecF));
    for (sampleIndex = 1; sampleIndex < pointCount; sampleIndex++) {
        for (intervalIndex = 0; intervalIndex < pointCount - sampleIndex; intervalIndex++) {
            workingPoints[intervalIndex].x =
                workingPoints[intervalIndex].x + (parameterT * (workingPoints[intervalIndex + 1].x -
                                                                workingPoints[intervalIndex].x));
            workingPoints[intervalIndex].y =
                workingPoints[intervalIndex].y + (parameterT * (workingPoints[intervalIndex + 1].y -
                                                                workingPoints[intervalIndex].y));
            workingPoints[intervalIndex].z =
                workingPoints[intervalIndex].z + (parameterT * (workingPoints[intervalIndex + 1].z -
                                                                workingPoints[intervalIndex].z));
        }
    }
    *result = workingPoints[0];
    HuMemDirectFree(workingPoints);
}

// Evaluates a B-spline basis value recursively; fn_1_2D24 calls it for each control point.
float fn_1_27B0(int basisIndex, int splineDegree, float parameterT)
{
    float leftBasisValue;
    float rightBasisValue;
    float knotSpan;
    if (splineDegree == 0) {
        if (parameterT >= lbl_1_bss_48[basisIndex] && parameterT < lbl_1_bss_48[basisIndex + 1]) {
            return 1.0f;
        }
        return 0.0f;
    }
    knotSpan = lbl_1_bss_48[basisIndex + splineDegree] - lbl_1_bss_48[basisIndex];
    if (knotSpan > 0.0f) {
        leftBasisValue = ((parameterT - lbl_1_bss_48[basisIndex])
                     * fn_1_27B0(basisIndex, splineDegree - 1, parameterT)) / knotSpan;
    } else {
        leftBasisValue = 0.0f;
    }
    knotSpan = lbl_1_bss_48[basisIndex + splineDegree + 1] - lbl_1_bss_48[basisIndex + 1];
    if (knotSpan > 0.0f) {
        rightBasisValue = ((lbl_1_bss_48[basisIndex + splineDegree + 1] - parameterT)
                     * fn_1_27B0(basisIndex + 1, splineDegree - 1, parameterT)) / knotSpan;
    } else {
        rightBasisValue = 0.0f;
    }
    return leftBasisValue + rightBasisValue;
}

// Builds a clamped cubic knot table and evaluates the B-spline when a board path is sampled.
void fn_1_2D24(HuVecF *controlPoints, int pointCount, HuVecF *result, float parameterT)
{
    int knotIndex;
    int splineDegree = 3;
    int sampleIndex;
    HuVecF curvePosition;
    float value;
    float *knotBufferToFree;
    float *knotBuffer;
    if (parameterT < 0.0f) { *result = controlPoints[0]; return; }
    if (parameterT >= 1.0f) { *result = controlPoints[pointCount - 1]; return; }
    knotBuffer = HuMemDirectMallocNum(HEAP_HEAP,
        (pointCount + splineDegree + 1) * sizeof(float), HU_MEMNUM_OVL);
    lbl_1_bss_48 = knotBuffer;
    knotIndex = 0;
    for (sampleIndex = 0; sampleIndex <= splineDegree; sampleIndex++)
        lbl_1_bss_48[knotIndex++] = 0.0f;
    for (sampleIndex = 0; sampleIndex < (pointCount - splineDegree) - 1; sampleIndex++)
        lbl_1_bss_48[knotIndex++] = (float)(sampleIndex + 1) / (pointCount - splineDegree);
    for (sampleIndex = 0; sampleIndex <= splineDegree; sampleIndex++)
        lbl_1_bss_48[knotIndex++] = 1.0f;
    curvePosition.x = curvePosition.y = curvePosition.z = 0.0f;
    for (sampleIndex = 0; sampleIndex < pointCount; sampleIndex++) {
        value = fn_1_27B0(sampleIndex, splineDegree, parameterT);
        curvePosition.x += value * controlPoints[sampleIndex].x;
        curvePosition.y += value * controlPoints[sampleIndex].y;
        curvePosition.z += value * controlPoints[sampleIndex].z;
    }
    knotBufferToFree = lbl_1_bss_48;
    HuMemDirectFree(knotBufferToFree);
    *result = curvePosition;
}

// Selects four control points around a curve segment, repeating endpoints as needed.
void fn_1_2FE0(HuVecF *controlPoints, int basisIndex, int pointCount, HuVecF *pointA,
               HuVecF *pointB, HuVecF *pointC, HuVecF *pointD)
{
    HuVecF *point;
    HuVecF nextControlPoint, thirdControlPoint, fourthControlPoint;
    if (basisIndex > pointCount - 1) basisIndex = pointCount - 1;
    point = &controlPoints[basisIndex];
    if (basisIndex == pointCount - 1) {
        nextControlPoint = point[0];
        thirdControlPoint = nextControlPoint;
        fourthControlPoint = nextControlPoint;
    } else if (basisIndex == pointCount - 2) {
        nextControlPoint = point[1];
        thirdControlPoint = nextControlPoint;
        fourthControlPoint = nextControlPoint;
    } else if (basisIndex == pointCount - 3) {
        nextControlPoint = point[1];
        thirdControlPoint = point[2];
        fourthControlPoint = thirdControlPoint;
    } else {
        nextControlPoint = point[1];
        thirdControlPoint = point[2];
        fourthControlPoint = point[3];
    }
    *pointA = point[0];
    *pointB = nextControlPoint;
    *pointC = thirdControlPoint;
    *pointD = fourthControlPoint;
}

// Selects a Catmull-Rom segment and derives half-length tangents at its ends.
// High indices clamp to the last point, where the next two points are still read.
void fn_1_31BC(HuVecF *controlPoints, int basisIndex, int pointCount, HuVecF *pointA,
               HuVecF *pointB, HuVecF *pointC, HuVecF *pointD)
{
    HuVecF *segmentStart;
    HuVecF firstTangent, secondTangent;
    if (basisIndex > pointCount - 1) basisIndex = pointCount - 1;
    segmentStart = &controlPoints[basisIndex];
    if (basisIndex == 0) VECSubtract(&segmentStart[1], &segmentStart[0], &firstTangent);
    else VECSubtract(&segmentStart[1], &segmentStart[-1], &firstTangent);
    if (basisIndex == pointCount - 2)
        VECSubtract(&segmentStart[1], &segmentStart[0], &secondTangent);
    else VECSubtract(&segmentStart[2], &segmentStart[0], &secondTangent);
    VECScale(&firstTangent, &firstTangent, 0.5f);
    VECScale(&secondTangent, &secondTangent, 0.5f);
    *pointA = segmentStart[0];
    *pointB = segmentStart[1];
    *pointC = firstTangent;
    *pointD = secondTangent;
}

// Linearly interpolates between two world positions at parameter t.
void fn_1_330C(HuVecF *pointA, HuVecF *pointB, HuVecF *result, float parameterT)
{
    HuVecF delta;
    VECSubtract(pointB, pointA, &delta);
    VECScale(&delta, &delta, parameterT);
    VECAdd(pointA, &delta, result);
}

// Animates a board player along a jump arc, blending to the supplied heading over six frames.
void fn_1_3370(int playerIndex, HuVecF *destination, float destinationHeadingDegrees,
    float jumpArcHeight, int durationFrames)
{
    HuVecF startPosition, playerPosition, travelDelta;
    int elapsedFrame;
    float startHeadingDegrees, parameterT, headingBlend;
    mbPlayerMotionShiftSet(playerIndex, 4, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    mbPlayerPosGet(playerIndex, &startPosition);
    startHeadingDegrees = mbPlayerRotYGet(playerIndex);
    VECSubtract(destination, &startPosition, &travelDelta);
    for (elapsedFrame = 0; elapsedFrame <= durationFrames; elapsedFrame++) {
        parameterT = elapsedFrame / (float)durationFrames;
        if ((u32)elapsedFrame == durationFrames - 6)
            mbPlayerMotionShiftSet(playerIndex, 5, 0.0f, 2.0f, HU3D_MOTATTR_NONE);
        playerPosition.x = startPosition.x + (parameterT * travelDelta.x);
        playerPosition.y = startPosition.y + (parameterT * travelDelta.y) +
                           (jumpArcHeight * HuSin(180.0f * parameterT));
        playerPosition.z = startPosition.z + (parameterT * travelDelta.z);
        mbPlayerPosSetV(playerIndex, &playerPosition);
        headingBlend = elapsedFrame / 6.0f;
        if (headingBlend > 1.0f) headingBlend = 1.0f;
        mbPlayerRotYSet(playerIndex,
                        mbAngleLerp(startHeadingDegrees, destinationHeadingDegrees, headingBlend));
        mbPlayerWorkGet(playerIndex)->_unk08 = durationFrames - elapsedFrame;
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerIndex, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);
}

// Records the starting system tick for the elapsed-time helper.
void fn_1_3610(void)
{
    lbl_1_bss_44 = OSGetTick();
}

// Returns system ticks elapsed since fn_1_3610 recorded its timestamp.
s32 fn_1_363C(void)
{
    return OSGetTick() - lbl_1_bss_44;
}

#endif
