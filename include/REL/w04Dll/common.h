// Evaluate board curves and paths, and move players between their points.
#ifndef REL_W04DLL_COMMON_H
#define REL_W04DLL_COMMON_H

// Estimate curve length using ten trapezoid intervals.
float fn_1_DF78(W04CurveEval speedAtParameter, HuVecF *curvePointA,
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

// Refine the curve length estimate using midpoint samples.
float fn_1_E120(W04CurveEval speedAtParameter, HuVecF *curvePointA,
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

// Find a curve parameter for the requested travel distance.
float fn_1_E330(W04CurveEval speedAtParameter, HuVecF *curvePointA,
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
        lengthError = fn_1_E120(speedAtParameter, curvePointA, curvePointB,
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

// Correct the curve parameter using a fixed interval length estimate.
float fn_1_E64C(W04CurveEval speedAtParameter, HuVecF *curvePointA,
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
        lengthError = fn_1_DF78(speedAtParameter, curvePointA, curvePointB,
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

// Return the magnitude of the quadratic Bezier tangent at the supplied time.
f32 fn_1_E900(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC, f32 t)
{
    HuVecF slope;

    slope.x = mbBezierCalcSlope(pointA->x, pointB->x, pointC->x, t);
    slope.y = mbBezierCalcSlope(pointA->y, pointB->y, pointC->y, t);
    slope.z = mbBezierCalcSlope(pointA->z, pointB->z, pointC->z, t);
    return VECMag(&slope);
}

// Return the magnitude of the cubic Bezier tangent at the supplied time.
f32 fn_1_E9A4(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, f32 t)
{
    HuVecF slope;

    slope.x = fn_1_EBA4(pointA->x, pointB->x, pointC->x, pointD->x, t);
    slope.y = fn_1_EBA4(pointA->y, pointB->y, pointC->y, pointD->y, t);
    slope.z = fn_1_EBA4(pointA->z, pointB->z, pointC->z, pointD->z, t);
    return VECMag(&slope);
}

// Return the magnitude of the Hermite tangent at the supplied time.
f32 fn_1_EA60(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, f32 t)
{
    HuVecF slope;

    slope.x = mbHermiteCalcSlope(pointA->x, pointB->x, pointC->x, pointD->x, t);
    slope.y = mbHermiteCalcSlope(pointA->y, pointB->y, pointC->y, pointD->y, t);
    slope.z = mbHermiteCalcSlope(pointA->z, pointB->z, pointC->z, pointD->z, t);
    return VECMag(&slope);
}

// Evaluate one cubic Bezier coordinate at t.
f32 fn_1_EB1C(f32 pointA, f32 pointB, f32 pointC, f32 pointD, f32 t)
{
    f32 inverseT = 1.0f - t;

    return (pointA * (inverseT * inverseT * inverseT))
        + (pointB * (3.0f * t * inverseT * inverseT))
        + (pointC * (3.0f * t * t * inverseT))
        + (pointD * (t * t * t));
}

// Evaluate the cubic Bezier tangent polynomial for one coordinate at t.
f32 fn_1_EBA4(f32 pointA, f32 pointB, f32 pointC, f32 pointD, f32 t)
{
    f32 tSquared = t * t;

    return (pointA * ((-3.0f * tSquared) - (6.0f * t) - 3.0f))
        + (pointB * ((9.0f * tSquared) - (12.0f * t) + 3.0f))
        + (pointC * ((-9.0f * tSquared) + (6.0f * t)))
        + ((3.0f * tSquared) * pointD);
}

// Evaluate the cubic Bezier position on each vector axis.
void fn_1_EC7C(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, HuVecF *position, float t)
{
    position->x = fn_1_EB1C(pointA->x, pointB->x, pointC->x, pointD->x, t);
    position->y = fn_1_EB1C(pointA->y, pointB->y, pointC->y, pointD->y, t);
    position->z = fn_1_EB1C(pointA->z, pointB->z, pointC->z, pointD->z, t);
}

// Evaluate the cubic Bezier tangent on each vector axis.
void fn_1_EF3C(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, HuVecF *slope, float t)
{
    slope->x = fn_1_EBA4(pointA->x, pointB->x, pointC->x, pointD->x, t);
    slope->y = fn_1_EBA4(pointA->y, pointB->y, pointC->y, pointD->y, t);
    slope->z = fn_1_EBA4(pointA->z, pointB->z, pointC->z, pointD->z, t);
}

// Interpolate the supplied Bezier controls with De Casteljau subdivision.
void fn_1_F2EC(HuVecF *points, int count, HuVecF *position, float t)
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

// Evaluate one recursive basis weight for the cubic spline.
float fn_1_F45C(int basisIndex, int degree, float t)
{
    float leftWeight;
    float rightWeight;
    float divisor;

    if (degree == 0) {
        if (t >= lbl_1_bss_7310[basisIndex]
            && t < lbl_1_bss_7310[basisIndex + 1]) {
            return 1.0f;
        }
        return 0.0f;
    }
    divisor = lbl_1_bss_7310[basisIndex + degree]
        - lbl_1_bss_7310[basisIndex];
    if (divisor > 0.0f) {
        leftWeight = ((t - lbl_1_bss_7310[basisIndex])
                     * fn_1_F45C(basisIndex, degree - 1, t))
            / divisor;
    } else {
        leftWeight = 0.0f;
    }
    divisor = lbl_1_bss_7310[basisIndex + degree + 1]
        - lbl_1_bss_7310[basisIndex + 1];
    if (divisor > 0.0f) {
        rightWeight = ((lbl_1_bss_7310[basisIndex + degree + 1] - t)
                     * fn_1_F45C(basisIndex + 1, degree - 1, t))
            / divisor;
    } else {
        rightWeight = 0.0f;
    }
    return leftWeight + rightWeight;
}

// Evaluate the spline path and clamp its parameter at the endpoints.
void fn_1_F9D0(HuVecF *points, int count, HuVecF *outPosition, float t)
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
    lbl_1_bss_7310 = allocatedKnots;
    knotIndex = 0;
    for (pointIndex = 0; pointIndex <= degree; pointIndex++) {
        lbl_1_bss_7310[knotIndex++] = 0.0f;
    }
    for (pointIndex = 0; pointIndex < (count - degree) - 1; pointIndex++) {
        lbl_1_bss_7310[knotIndex++] = (float)(pointIndex + 1) / (count - degree);
    }
    for (pointIndex = 0; pointIndex <= degree; pointIndex++) {
        lbl_1_bss_7310[knotIndex++] = 1.0f;
    }
    curvePosition.x = curvePosition.y = curvePosition.z = 0.0f;
    for (pointIndex = 0; pointIndex < count; pointIndex++) {
        basisWeight = fn_1_F45C(pointIndex, degree, t);
        curvePosition.x += basisWeight * points[pointIndex].x;
        curvePosition.y += basisWeight * points[pointIndex].y;
        curvePosition.z += basisWeight * points[pointIndex].z;
    }
    knotStorage = lbl_1_bss_7310;
    HuMemDirectFree(knotStorage);
    *outPosition = curvePosition;
}

// Select consecutive Bezier controls, repeating the path endpoint.
void fn_1_FC8C(HuVecF *points, int index, int count, HuVecF *a, HuVecF *b,
    HuVecF *c, HuVecF *d)
{
    HuVecF *point;
    HuVecF pointB;
    HuVecF pointC;
    HuVecF pointD;

    if (index > count - 1) {
        index = count - 1;
    }
    point = &points[index];
    if (index == count - 1) {
        pointB = point[0];
        pointC = pointB;
        pointD = pointB;
    } else if (index == count - 2) {
        pointB = point[1];
        pointC = pointB;
        pointD = pointB;
    } else if (index == count - 3) {
        pointB = point[1];
        pointC = point[2];
        pointD = pointC;
    } else {
        pointB = point[1];
        pointC = point[2];
        pointD = point[3];
    }
    *a = point[0];
    *b = pointB;
    *c = pointC;
    *d = pointD;
}

// Select neighboring points and their averaged tangents for a board path.
void fn_1_FE68(HuVecF *points, s32 index, s32 count, HuVecF *pointA,
    HuVecF *pointB, HuVecF *tangentA, HuVecF *tangentB)
{
    HuVecF *currentPoint;
    HuVecF firstTangent;
    HuVecF secondTangent;

    if (index > count - 1) {
        index = count - 1;
    }
    currentPoint = &points[index];
    if (index == 0) {
        VECSubtract(&currentPoint[1], &currentPoint[0], &firstTangent);
    } else {
        VECSubtract(&currentPoint[1], &currentPoint[-1], &firstTangent);
    }
    if (index == count - 2) {
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

// Interpolate the position between two consecutive board path points.
void fn_1_FFB8(HuVecF *startPosition, HuVecF *endPosition,
    HuVecF *position, f32 t)
{
    HuVecF delta;

    VECSubtract(endPosition, startPosition, &delta);
    VECScale(&delta, &delta, t);
    VECAdd(startPosition, &delta, position);
}

// Move the player along a straight segment with a sinusoidal jump arc.
void fn_1_1001C(int playerNo, HuVecF *dstPos, float dstRotY,
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

void fn_1_102BC(void)
{
    lbl_1_bss_730C = OSGetTick();
}

OSTick fn_1_102E8(void)
{
    return OSGetTick() - lbl_1_bss_730C;
}

#endif
