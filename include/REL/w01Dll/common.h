// Board curve evaluation and path movement, and the event timer: the board's common code, compiled
// after the board's own (as w04Dll's REL/w04Dll/common.h).
#ifndef REL_W01DLL_COMMON_H
#define REL_W01DLL_COMMON_H

float fn_1_14D34(float pointA, float pointB, float pointC, float pointD,
    float t);

// Integrate curve speed from zero to t with ten trapezoid intervals.
// fn_1_147DC uses this estimate while correcting a curve parameter.
float fn_1_14108(W01CurveEval speedAtParameter, HuVecF *curvePointA,
    HuVecF *curvePointB, HuVecF *curvePointC, HuVecF *curvePointD,
    float targetParameter)
{
    int sampleIndex;
    int sampleCount;
    float accumulatedSpeed;
    float baseT;
    float sampleParameter;
    float parameterStep;

    sampleCount = 10;
    baseT = 0.0f;
    parameterStep = (targetParameter - baseT) / sampleCount;
    sampleParameter = baseT;
    accumulatedSpeed = 0.0f;
    for (sampleIndex = 0; sampleIndex < sampleCount - 1; sampleIndex++) {
        sampleParameter += parameterStep;
        accumulatedSpeed += speedAtParameter(curvePointA, curvePointB,
            curvePointC, curvePointD, sampleParameter);
    }
    accumulatedSpeed = parameterStep * 0.5
        * (speedAtParameter(curvePointA, curvePointB, curvePointC,
                curvePointD, baseT)
            + speedAtParameter(curvePointA, curvePointB, curvePointC,
                curvePointD, targetParameter)
            + (2.0 * accumulatedSpeed));
    return accumulatedSpeed;
}

// Integrate curve speed from zero to t with refined midpoint samples.
// fn_1_144C0 uses this estimate while correcting a curve parameter.
float fn_1_142B0(W01CurveEval speedAtParameter, HuVecF *curvePointA,
    HuVecF *curvePointB, HuVecF *curvePointC, HuVecF *curvePointD,
    float targetParameter)
{
    int refinementCount = 10;
    float baseT = 0.0f;
    float integratedLength = 0.0f;
    float parameterSpan = targetParameter - baseT;
    float endpointEstimate = parameterSpan
        * (speedAtParameter(curvePointA, curvePointB, curvePointC,
                curvePointD, baseT)
            + speedAtParameter(curvePointA, curvePointB, curvePointC,
                curvePointD, targetParameter))
        * 0.5f;
    float midpointEstimate;
    int sampleIndex;
    int refinement;

    for (refinement = 1; refinement <= refinementCount; refinement *= 2) {
        midpointEstimate = 0.0f;
        for (sampleIndex = 1; sampleIndex <= refinement; sampleIndex++) {
            midpointEstimate += speedAtParameter(curvePointA, curvePointB,
                curvePointC, curvePointD,
                baseT + parameterSpan * (sampleIndex - 0.5f));
        }
        midpointEstimate *= parameterSpan;
        integratedLength = (1.0f / 3.0f)
            * (endpointEstimate + (2.0f * midpointEstimate));
        parameterSpan *= 0.5f;
        endpointEstimate = (endpointEstimate + midpointEstimate) * 0.5f;
    }
    return integratedLength;
}

// Estimate the curve parameter for targetDistance using refined length integration.
// Always correct once, then stop when t stabilizes or maxStep is reached.
// Speeds below 0.1 in magnitude use 1.0 as the correction divisor.
float fn_1_144C0(W01CurveEval speedAtParameter, HuVecF *curvePointA,
    HuVecF *curvePointB, HuVecF *curvePointC, HuVecF *curvePointD,
    float t, float targetDistance, int maxStep)
{
    int iteration;
    float speed;
    float lengthError;
    float previousT;
    float minimumSpeed;

    minimumSpeed = 0.1f;
    iteration = 0;
    do {
        lengthError = fn_1_142B0(speedAtParameter, curvePointA, curvePointB,
            curvePointC, curvePointD, t) - targetDistance;
        if (fabs(speed = speedAtParameter(curvePointA, curvePointB,
                curvePointC, curvePointD, t)) < minimumSpeed) {
            speed = 1.0f;
        }
        previousT = t;
        t -= lengthError / speed;
        iteration++;
    } while (t != previousT && iteration < maxStep);
    return t;
}

// Estimate the curve parameter using the fixed ten-interval length estimate.
// Always correct once, then stop when t stabilizes or maxStep is reached.
// Speeds below 0.1 in magnitude use 1.0 as the correction divisor.
float fn_1_147DC(W01CurveEval speedAtParameter, HuVecF *curvePointA,
    HuVecF *curvePointB, HuVecF *curvePointC, HuVecF *curvePointD,
    float t, float targetDistance, int maxStep)
{
    int iteration;
    float speed;
    float lengthError;
    float previousT;
    float minimumSpeed;

    minimumSpeed = 0.1f;
    iteration = 0;
    do {
        lengthError = fn_1_14108(speedAtParameter, curvePointA, curvePointB,
            curvePointC, curvePointD, t) - targetDistance;
        if (fabs(speed = speedAtParameter(curvePointA, curvePointB,
                curvePointC, curvePointD, t)) < minimumSpeed) {
            speed = 1.0f;
        }
        previousT = t;
        t -= lengthError / speed;
        iteration++;
    } while (t != previousT && iteration < maxStep);
    return t;
}

// Return the magnitude of the quadratic Bezier tangent at t.
// The board-path curve routines pass this function as their speed evaluator.
float fn_1_14A90(HuVecF *controlPointA, HuVecF *controlPointB,
    HuVecF *controlPointC, float t)
{
    HuVecF slope;

    slope.x = mbBezierCalcSlope(controlPointA->x, controlPointB->x,
        controlPointC->x, t);
    slope.y = mbBezierCalcSlope(controlPointA->y, controlPointB->y,
        controlPointC->y, t);
    slope.z = mbBezierCalcSlope(controlPointA->z, controlPointB->z,
        controlPointC->z, t);
    return VECMag(&slope);
}

// Return the magnitude of the board curve tangent expression on each vector axis.
float fn_1_14B34(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, float t)
{
    HuVecF slope;

    slope.x = fn_1_14D34(pointA->x, pointB->x, pointC->x, pointD->x, t);
    slope.y = fn_1_14D34(pointA->y, pointB->y, pointC->y, pointD->y, t);
    slope.z = fn_1_14D34(pointA->z, pointB->z, pointC->z, pointD->z, t);
    return VECMag(&slope);
}

// Return the magnitude of the Hermite curve tangent at t.
// Curve-distance routines use this to estimate distance along board motion.
float fn_1_14BF0(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, float t)
{
    HuVecF slope;

    slope.x = mbHermiteCalcSlope(pointA->x, pointB->x, pointC->x, pointD->x, t);
    slope.y = mbHermiteCalcSlope(pointA->y, pointB->y, pointC->y, pointD->y, t);
    slope.z = mbHermiteCalcSlope(pointA->z, pointB->z, pointC->z, pointD->z, t);
    return VECMag(&slope);
}

// Evaluate a cubic Bezier coordinate from its four scalar control values.
float fn_1_14CAC(float pointA, float pointB, float pointC, float pointD,
    float t)
{
    float invT = 1.0f - t;

    return (pointA * (invT * invT * invT))
        + (pointB * (3.0f * t * invT * invT))
        + (pointC * (3.0f * t * t * invT))
        + (pointD * (t * t * t));
}

// Evaluate the board curve tangent polynomial for one coordinate at t.
// Its first control term keeps the negative linear coefficient used by this board.
float fn_1_14D34(float pointA, float pointB, float pointC, float pointD,
    float t)
{
    float t2 = t * t;

    return (pointA * ((-3.0f * t2) - (6.0f * t) - 3.0f))
        + (pointB * ((9.0f * t2) - (12.0f * t) + 3.0f))
        + (pointC * ((-9.0f * t2) + (6.0f * t)))
        + ((3.0f * t2) * pointD);
}

// Evaluate a cubic Bezier position independently on each vector axis.
void fn_1_14E0C(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, HuVecF *position, float t)
{
    position->x = fn_1_14CAC(pointA->x, pointB->x, pointC->x, pointD->x, t);
    position->y = fn_1_14CAC(pointA->y, pointB->y, pointC->y, pointD->y, t);
    position->z = fn_1_14CAC(pointA->z, pointB->z, pointC->z, pointD->z, t);
}

// Evaluate the board's tangent polynomial on each axis, including the negative linear term
// for the first control value.
void fn_1_150CC(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, HuVecF *slope, float t)
{
    slope->x = fn_1_14D34(pointA->x, pointB->x, pointC->x, pointD->x, t);
    slope->y = fn_1_14D34(pointA->y, pointB->y, pointC->y, pointD->y, t);
    slope->z = fn_1_14D34(pointA->z, pointB->z, pointC->z, pointD->z, t);
}

// Evaluate the supplied Bezier control points with De Casteljau interpolation.
void fn_1_1547C(HuVecF *points, int count, HuVecF *position, float t)
{
    HuVecF *workPoints = HuMemDirectMallocNum(
        HEAP_HEAP, count * sizeof(HuVecF), HU_MEMNUM_OVL);
    HuVecF *interpolationPoints = workPoints;
    int level;
    int pointIndex;

    memcpy(interpolationPoints, points, count * sizeof(HuVecF));
    for (level = 1; level < count; level++) {
        for (pointIndex = 0; pointIndex < count - level; pointIndex++) {
            interpolationPoints[pointIndex].x = interpolationPoints[pointIndex].x
                + (t * (interpolationPoints[pointIndex + 1].x - interpolationPoints[pointIndex].x));
            interpolationPoints[pointIndex].y = interpolationPoints[pointIndex].y
                + (t * (interpolationPoints[pointIndex + 1].y - interpolationPoints[pointIndex].y));
            interpolationPoints[pointIndex].z = interpolationPoints[pointIndex].z
                + (t * (interpolationPoints[pointIndex + 1].z - interpolationPoints[pointIndex].z));
        }
    }
    *position = interpolationPoints[0];
    HuMemDirectFree(interpolationPoints);
}

// Evaluate one recursive basis weight for the board's cubic spline path.
float fn_1_155EC(int basisIndex, int degree, float t)
{
    float leftWeight;
    float rightWeight;
    float divisor;

    if (degree == 0) {
        if (t >= lbl_1_bss_1620[basisIndex]
            && t < lbl_1_bss_1620[basisIndex + 1]) {
            return 1.0f;
        }
        return 0.0f;
    }
    divisor = lbl_1_bss_1620[basisIndex + degree]
        - lbl_1_bss_1620[basisIndex];
    if (divisor > 0.0f) {
        leftWeight = ((t - lbl_1_bss_1620[basisIndex])
                     * fn_1_155EC(basisIndex, degree - 1, t))
            / divisor;
    } else {
        leftWeight = 0.0f;
    }
    divisor = lbl_1_bss_1620[basisIndex + degree + 1]
        - lbl_1_bss_1620[basisIndex + 1];
    if (divisor > 0.0f) {
        rightWeight = ((lbl_1_bss_1620[basisIndex + degree + 1] - t)
                     * fn_1_155EC(basisIndex + 1, degree - 1, t))
            / divisor;
    } else {
        rightWeight = 0.0f;
    }
    return leftWeight + rightWeight;
}

// Evaluate the cubic spline path, clamping parameters outside its endpoints.
void fn_1_15B60(HuVecF *points, int count, HuVecF *outPosition, float t)
{
    int knotIndex;
    int degree = 3;
    int pointIndex;
    HuVecF curvePosition;
    float basisWeight;
    float *knotStorage;
    float *allocatedKnots;

    if (t < 0.0f) {
        *outPosition = points[0];
        return;
    }
    if (t >= 1.0f) {
        *outPosition = points[count - 1];
        return;
    }
    allocatedKnots = HuMemDirectMallocNum(HEAP_HEAP,
        (count + degree + 1) * sizeof(float), HU_MEMNUM_OVL);
    lbl_1_bss_1620 = allocatedKnots;
    knotIndex = 0;
    for (pointIndex = 0; pointIndex <= degree; pointIndex++) {
        lbl_1_bss_1620[knotIndex++] = 0.0f;
    }
    for (pointIndex = 0; pointIndex < (count - degree) - 1; pointIndex++) {
        lbl_1_bss_1620[knotIndex++] = (float)(pointIndex + 1) / (count - degree);
    }
    for (pointIndex = 0; pointIndex <= degree; pointIndex++) {
        lbl_1_bss_1620[knotIndex++] = 1.0f;
    }
    curvePosition.x = curvePosition.y = curvePosition.z = 0.0f;
    for (pointIndex = 0; pointIndex < count; pointIndex++) {
        basisWeight = fn_1_155EC(pointIndex, degree, t);
        curvePosition.x += basisWeight * points[pointIndex].x;
        curvePosition.y += basisWeight * points[pointIndex].y;
        curvePosition.z += basisWeight * points[pointIndex].z;
    }
    knotStorage = lbl_1_bss_1620;
    HuMemDirectFree(knotStorage);
    *outPosition = curvePosition;
}

// Select four Bezier controls for one board path segment, repeating endpoints at the end.
void fn_1_15E1C(HuVecF *points, int pointIndex, int pointCount,
    HuVecF *pointA, HuVecF *pointB, HuVecF *pointC, HuVecF *pointD)
{
    HuVecF *currentPoint;
    HuVecF nextPoint;
    HuVecF secondNextPoint;
    HuVecF thirdNextPoint;

    if (pointIndex > pointCount - 1) {
        pointIndex = pointCount - 1;
    }
    currentPoint = &points[pointIndex];
    if (pointIndex == pointCount - 1) {
        nextPoint = currentPoint[0];
        secondNextPoint = nextPoint;
        thirdNextPoint = nextPoint;
    } else if (pointIndex == pointCount - 2) {
        nextPoint = currentPoint[1];
        secondNextPoint = nextPoint;
        thirdNextPoint = nextPoint;
    } else if (pointIndex == pointCount - 3) {
        nextPoint = currentPoint[1];
        secondNextPoint = currentPoint[2];
        thirdNextPoint = secondNextPoint;
    } else {
        nextPoint = currentPoint[1];
        secondNextPoint = currentPoint[2];
        thirdNextPoint = currentPoint[3];
    }
    *pointA = currentPoint[0];
    *pointB = nextPoint;
    *pointC = secondNextPoint;
    *pointD = thirdNextPoint;
}

// Select two Hermite endpoints and their averaged tangents for a board path segment.
void fn_1_15FF8(HuVecF *points, int pointIndex, int pointCount,
    HuVecF *pointA, HuVecF *pointB, HuVecF *tangentA, HuVecF *tangentB)
{
    HuVecF *currentPoint;
    HuVecF firstTangent;
    HuVecF secondTangent;

    if (pointIndex > pointCount - 1) {
        pointIndex = pointCount - 1;
    }
    currentPoint = &points[pointIndex];
    if (pointIndex == 0) {
        VECSubtract(&currentPoint[1], &currentPoint[0], &firstTangent);
    } else {
        VECSubtract(&currentPoint[1], &currentPoint[-1], &firstTangent);
    }
    if (pointIndex == pointCount - 2) {
        VECSubtract(&currentPoint[1], &currentPoint[0], &secondTangent);
    } else {
        VECSubtract(&currentPoint[2], &currentPoint[0], &secondTangent);
    }
    VECScale(&firstTangent, &firstTangent, 0.5f);
    VECScale(&secondTangent, &secondTangent, 0.5f);
    *pointA = currentPoint[0];
    *pointB = currentPoint[1];
    *tangentA = firstTangent;
    *tangentB = secondTangent;
}

// Linearly interpolate between two positions for board path movement.
void fn_1_16148(HuVecF *startPos, HuVecF *endPos, HuVecF *position, float t)
{
    HuVecF delta;

    VECSubtract(endPos, startPos, &delta);
    VECScale(&delta, &delta, t);
    VECAdd(startPos, &delta, position);
}

// Animate a player along a straight board segment with a sinusoidal jump arc.
void fn_1_161AC(int playerNo, HuVecF *dstPos, float dstRotY,
    float jumpHeight, int maxTime)
{
    HuVecF startPos;
    HuVecF playerPos;
    HuVecF delta;
    int frame;
    float startRotY;
    float progress;
    float rotationProgress;

    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    mbPlayerPosGet(playerNo, &startPos);
    startRotY = mbPlayerRotYGet(playerNo);
    VECSubtract(dstPos, &startPos, &delta);
    for (frame = 0; frame <= maxTime; frame++) {
        progress = frame / (float)maxTime;
        if ((u32)frame == maxTime - 6) {
            mbPlayerMotionShiftSet(
                playerNo, 5, 0.0f, 2.0f, HU3D_MOTATTR_NONE);
        }
        playerPos.x = startPos.x + (progress * delta.x);
        playerPos.y = startPos.y + (progress * delta.y)
            + (jumpHeight * HuSin(180.0f * progress));
        playerPos.z = startPos.z + (progress * delta.z);
        mbPlayerPosSetV(playerNo, &playerPos);
        rotationProgress = frame / 6.0f;
        if (rotationProgress > 1.0f) {
            rotationProgress = 1.0f;
        }
        mbPlayerRotYSet(playerNo,
            mbAngleLerp(startRotY, dstRotY, rotationProgress));
        mbPlayerWorkGet(playerNo)->_unk08 = maxTime - frame;
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);
}

void fn_1_1644C(void)
{
    lbl_1_bss_161C = OSGetTick();
}

OSTick fn_1_16478(void)
{
    return OSGetTick() - lbl_1_bss_161C;
}

#endif
