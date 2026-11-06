/* Word Herd matrix operations, geometric bounds and collision-distance helpers. */
#include <math.h>
#include "humath.h"
#include "REL/m669DLL/vector.h"

/* Row-major matrix. */
typedef struct M669MatrixElements { float elements[16]; } M669MatrixElements;
typedef struct M669Sphere { HuVecF center; float radius; } M669Sphere;
typedef struct M669Segment { HuVecF start, delta; } M669Segment; /* End is start plus delta. */
typedef struct M669Capsule { M669Segment segment; float radius; } M669Capsule;
/* Vertices are start, start plus edge1, and start plus edge2. */
typedef struct M669Triangle { HuVecF start, edge1, edge2; } M669Triangle;
typedef struct M669Bounds { HuVecF min, max; } M669Bounds; /* Minimum and maximum on each axis. */

/* Multiplies two four-by-four matrices through a temporary, allowing the output to alias either
 * input. */
void fn_1_1E94(M669MatrixElements *out, M669MatrixElements *a, M669MatrixElements *b)
{
    M669MatrixElements result;
    float *left = a->elements;
    float *right = b->elements;
    result.elements[0] =
        left[3] * right[12] + (left[2] * right[8] + (left[0] * right[0] + left[1] * right[4]));
    result.elements[1] =
        left[3] * right[13] + (left[2] * right[9] + (left[0] * right[1] + left[1] * right[5]));
    result.elements[2] =
        left[3] * right[14] + (left[2] * right[10] + (left[0] * right[2] + left[1] * right[6]));
    result.elements[3] =
        left[3] * right[15] + (left[2] * right[11] + (left[0] * right[3] + left[1] * right[7]));
    result.elements[4] =
        left[7] * right[12] + (left[6] * right[8] + (left[4] * right[0] + left[5] * right[4]));
    result.elements[5] =
        left[7] * right[13] + (left[6] * right[9] + (left[4] * right[1] + left[5] * right[5]));
    result.elements[6] =
        left[7] * right[14] + (left[6] * right[10] + (left[4] * right[2] + left[5] * right[6]));
    result.elements[7] =
        left[7] * right[15] + (left[6] * right[11] + (left[4] * right[3] + left[5] * right[7]));
    result.elements[8] =
        left[11] * right[12] + (left[10] * right[8] + (left[8] * right[0] + left[9] * right[4]));
    result.elements[9] =
        left[11] * right[13] + (left[10] * right[9] + (left[8] * right[1] + left[9] * right[5]));
    result.elements[10] =
        left[11] * right[14] + (left[10] * right[10] + (left[8] * right[2] + left[9] * right[6]));
    result.elements[11] =
        left[11] * right[15] + (left[10] * right[11] + (left[8] * right[3] + left[9] * right[7]));
    result.elements[12] =
        left[15] * right[12] + (left[14] * right[8] + (left[12] * right[0] + left[13] * right[4]));
    result.elements[13] =
        left[15] * right[13] + (left[14] * right[9] + (left[12] * right[1] + left[13] * right[5]));
    result.elements[14] =
        left[15] * right[14] + (left[14] * right[10] + (left[12] * right[2] + left[13] * right[6]));
    result.elements[15] =
        left[15] * right[15] + (left[14] * right[11] + (left[12] * right[3] + left[13] * right[7]));
    *out = result;
}

/* Initializes a sphere from its center coordinates and radius. */
void fn_1_2338(M669Sphere *out, f32 x, f32 y, f32 z, f32 radius)
{
    out->center.x = x;
    out->center.y = y;
    out->center.z = z;
    out->radius = radius;
}


/* Builds axis-aligned bounds around both capsule endpoints, expanded by the capsule radius. */
void fn_1_234C(M669Bounds *out, M669Capsule *capsule)
{
    HuVecF points[2];
    points[0] = capsule->segment.start;
    M669VecAdd(&points[1], &capsule->segment.start, &capsule->segment.delta);
    out->min.x = (points[0].x < points[1].x ? points[0].x : points[1].x) - capsule->radius;
    out->max.x = capsule->radius + (points[0].x >= points[1].x ? points[0].x : points[1].x);
    out->min.y = (points[0].y < points[1].y ? points[0].y : points[1].y) - capsule->radius;
    out->max.y = capsule->radius + (points[0].y >= points[1].y ? points[0].y : points[1].y);
    out->min.z = (points[0].z < points[1].z ? points[0].z : points[1].z) - capsule->radius;
    out->max.z = capsule->radius + (points[0].z >= points[1].z ? points[0].z : points[1].z);
}


/* Builds axis-aligned bounds around a sphere for collision broad-phase tests. */
void fn_1_2504(M669Bounds *out, M669Sphere *sphere)
{
    out->min = sphere->center;
    out->min.x -= sphere->radius;
    out->min.y -= sphere->radius;
    out->min.z -= sphere->radius;
    out->max = sphere->center;
    out->max.x += sphere->radius;
    out->max.y += sphere->radius;
    out->max.z += sphere->radius;
}

f32 fn_1_2E54(Point3d *queryPoint, M669Triangle *triangle, f32 *barycentricSOut,
              f32 *barycentricTOut);

f32 fn_1_3824();



/* Builds axis-aligned bounds around the three triangle vertices. */
void fn_1_2598(M669Bounds *out, M669Triangle *triangle)
{
    HuVecF points[3];
    int index;
    points[0] = triangle->start;
    M669VecAdd(&points[1], &triangle->start, &triangle->edge1);
    M669VecAdd(&points[2], &triangle->start, &triangle->edge2);
    out->min = points[0];
    out->max = out->min;
    for (index = 1; index < 3; index++) {
        out->min.x = out->min.x > points[index].x ? points[index].x : out->min.x;
        out->max.x = out->max.x <= points[index].x ? points[index].x : out->max.x;
        out->min.y = out->min.y > points[index].y ? points[index].y : out->min.y;
        out->max.y = out->max.y <= points[index].y ? points[index].y : out->max.y;
        out->min.z = out->min.z > points[index].z ? points[index].z : out->min.z;
        out->max.z = out->max.z <= points[index].z ? points[index].z : out->max.z;
    }
}

/* Tests strict overlap on all three axes; bounds that only touch do not overlap. */
s32 fn_1_27FC(M669Bounds *a, M669Bounds *b)
{
    if ((a->min.z < b->max.z) && (a->max.z > b->min.z) && (a->min.x < b->max.x) &&
        (a->max.x > b->min.x) && (a->min.y < b->max.y) && (a->max.y > b->min.y)) {
        return 1;
    }
    return 0;
}

/* Tests moving-sphere contact through maxTime; existing overlaps write a midpoint, while future
 * hits leave the contact output untouched. */
s32 fn_1_286C(M669Sphere *firstSphere, HuVecF *firstVelocity, M669Sphere *secondSphere,
              HuVecF *secondVelocity, float maxTime, float *hitTime, HuVecF *contactPointOut)
{
    HuVecF relativeVelocity, centerDelta, contactPoint;
    float relativeSpeedSquared, centerDistanceSquared, combinedRadius, combinedRadiusSquared;
    float velocityDotOffset, separationTerm, collisionDiscriminant;
    /* The subtraction reads the pointer arguments' storage rather than the vectors they point
     * to. */
    M669VecSubtract(&relativeVelocity, (HuVecF *)&secondVelocity, (HuVecF *)&firstVelocity);
    relativeSpeedSquared = M669VecLengthSquared(&relativeVelocity);
    M669VecSubtract(&centerDelta, &secondSphere->center, &firstSphere->center);
    centerDistanceSquared = M669VecLengthSquared(&centerDelta);
    combinedRadius = firstSphere->radius + secondSphere->radius;
    combinedRadiusSquared = combinedRadius * combinedRadius;
    if (relativeSpeedSquared > 0.0f) {
        velocityDotOffset = M669VecDot(&centerDelta, &relativeVelocity);
        if (velocityDotOffset <= 0.0f) {
            if (-maxTime * relativeSpeedSquared <= velocityDotOffset ||
                centerDistanceSquared +
                        maxTime * (2.0f * velocityDotOffset + maxTime * relativeSpeedSquared) <=
                    combinedRadiusSquared) {
                separationTerm = centerDistanceSquared - combinedRadiusSquared;
                collisionDiscriminant =
                    velocityDotOffset * velocityDotOffset - relativeSpeedSquared * separationTerm;
                if (collisionDiscriminant >= 0.0f) {
                    if (separationTerm <= 0.0f) {
                        *hitTime = 0.0f;
                        M669VecAdd(contactPointOut, &firstSphere->center, &secondSphere->center);
                        M669VecScale(contactPointOut, contactPointOut, 0.5f);
                    } else {
                        *hitTime = -(velocityDotOffset + sqrtf(collisionDiscriminant)) /
                                   relativeSpeedSquared;
                        if (*hitTime < 0.0f) *hitTime = 0.0f;
                        else if (*hitTime > maxTime) *hitTime = maxTime;
                        M669VecScale(&contactPoint, &relativeVelocity, *hitTime);
                        /* This branch updates a temporary contact point but leaves the output
                         * untouched. */
                        M669VecAdd(&contactPoint, &contactPoint, &centerDelta);
                    }
                    return 1;
                }
            }
            return 0;
        }
    }
    if (centerDistanceSquared <= combinedRadiusSquared) {
        *hitTime = 0.0f;
        M669VecAdd(contactPointOut, &firstSphere->center, &secondSphere->center);
        M669VecScale(contactPointOut, contactPointOut, 0.5f);
        return 1;
    }
    return 0;
}

/* Returns squared center separation minus the sum of the squared radii, clamped to zero. */
float fn_1_2C24(M669Sphere *firstSphere, M669Sphere *secondSphere)
{
    HuVecF centerOffset;
    float gapSquared;
    M669VecSubtract(&centerOffset, &firstSphere->center, &secondSphere->center);
    gapSquared = M669VecLengthSquared(&centerOffset);
    gapSquared -=
        firstSphere->radius * firstSphere->radius + secondSphere->radius * secondSphere->radius;
    if (gapSquared < 0.0f) gapSquared = 0.0f;
    return gapSquared;
}

float M669SphereDistanceTerm(M669Sphere *firstSphere, M669Sphere *secondSphere);

/* Computes the clamped sphere term used by the square-root distance wrapper. */
static inline float M669SphereDistanceTerm(M669Sphere *firstSphere, M669Sphere *secondSphere)
{
    HuVecF centerOffset;
    float gapSquared;
    M669VecSubtract(&centerOffset, &firstSphere->center, &secondSphere->center);
    gapSquared = M669VecLengthSquared(&centerOffset);
    gapSquared -=
        firstSphere->radius * firstSphere->radius + secondSphere->radius * secondSphere->radius;
    if (gapSquared < 0.0f) gapSquared = 0.0f;
    return gapSquared;
}

/* Takes the square root of the clamped sphere term for collision-distance queries. */
float fn_1_2CC8(M669Sphere *sphereA, M669Sphere *sphereB)
{
    return sqrtf(M669SphereDistanceTerm(sphereA, sphereB));
}

/* Returns squared point-to-triangle distance and optionally writes the closest barycentric
 * coordinates. */
f32 fn_1_2E54(Point3d *queryPoint, M669Triangle *triangle, f32 *barycentricSOut,
              f32 *barycentricTOut)
{
    Point3d pointOffset;
    f32 gramDeterminant;
    f32 barycentricT;
    f32 barycentricS;
    f32 pointOffsetDotEdge1;
    f32 pointOffsetDotEdge2;
    f32 offsetLengthSquared;
    f32 distanceSquared;
    f32 edge2LengthSquared;
    f32 edge1LengthSquared;
    f32 edgeDot;
    f32 edgeParameterNumerator;
    f32 edgeParameterDenominator;
    f32 boundaryEdgeProjection;
    f32 crossEdgeProjection;

    M669VecSubtract(&pointOffset, &triangle->start, queryPoint);
    edge1LengthSquared = M669VecLengthSquared(&triangle->edge1);
    edgeDot = M669VecDot(&triangle->edge1, &triangle->edge2);
    edge2LengthSquared = M669VecLengthSquared(&triangle->edge2);
    pointOffsetDotEdge1 = M669VecDot(&pointOffset, &triangle->edge1);
    pointOffsetDotEdge2 = M669VecDot(&pointOffset, &triangle->edge2);
    offsetLengthSquared = M669VecLengthSquared(&pointOffset);
    gramDeterminant = fabsf((edge1LengthSquared * edge2LengthSquared) - (edgeDot * edgeDot));
    barycentricS = (edgeDot * pointOffsetDotEdge2) - (edge2LengthSquared * pointOffsetDotEdge1);
    barycentricT = (edgeDot * pointOffsetDotEdge1) - (edge1LengthSquared * pointOffsetDotEdge2);
    /* Test the interior and boundary regions in scaled barycentric coordinates. */
    if ((barycentricS + barycentricT) <= gramDeterminant) {
        if (barycentricS < 0.0f) {
            if (barycentricT < 0.0f) {
                if (pointOffsetDotEdge1 < 0.0f) {
                    barycentricT = 0.0f;
                    if (-pointOffsetDotEdge1 >= edge1LengthSquared) {
                        barycentricS = 1.0f;
                        distanceSquared = offsetLengthSquared +
                                          (edge1LengthSquared + (2.0f * pointOffsetDotEdge1));
                    } else {
                        barycentricS = -pointOffsetDotEdge1 / edge1LengthSquared;
                        distanceSquared =
                            offsetLengthSquared + (pointOffsetDotEdge1 * barycentricS);
                    }
                } else {
                    barycentricS = 0.0f;
                    if (pointOffsetDotEdge2 >= 0.0f) {
                        barycentricT = 0.0f;
                        distanceSquared = offsetLengthSquared;
                    } else if (-pointOffsetDotEdge2 >= edge2LengthSquared) {
                        barycentricT = 1.0f;
                        distanceSquared = offsetLengthSquared +
                                          (edge2LengthSquared + (2.0f * pointOffsetDotEdge2));
                    } else {
                        barycentricT = -pointOffsetDotEdge2 / edge2LengthSquared;
                        distanceSquared =
                            offsetLengthSquared + (pointOffsetDotEdge2 * barycentricT);
                    }
                }
            } else {
                barycentricS = 0.0f;
                if (pointOffsetDotEdge2 >= 0.0f) {
                    barycentricT = 0.0f;
                    distanceSquared = offsetLengthSquared;
                } else if (-pointOffsetDotEdge2 >= edge2LengthSquared) {
                    barycentricT = 1.0f;
                    distanceSquared =
                        offsetLengthSquared + (edge2LengthSquared + (2.0f * pointOffsetDotEdge2));
                } else {
                    barycentricT = -pointOffsetDotEdge2 / edge2LengthSquared;
                    distanceSquared = offsetLengthSquared + (pointOffsetDotEdge2 * barycentricT);
                }
            }
        } else if (barycentricT < 0.0f) {
            barycentricT = 0.0f;
            if (pointOffsetDotEdge1 >= 0.0f) {
                barycentricS = 0.0f;
                distanceSquared = offsetLengthSquared;
            } else if (-pointOffsetDotEdge1 >= edge1LengthSquared) {
                barycentricS = 1.0f;
                distanceSquared =
                    offsetLengthSquared + (edge1LengthSquared + (2.0f * pointOffsetDotEdge1));
            } else {
                barycentricS = -pointOffsetDotEdge1 / edge1LengthSquared;
                distanceSquared = offsetLengthSquared + (pointOffsetDotEdge1 * barycentricS);
            }
        } else {
            f32 inverseGramDeterminant = 1.0f / gramDeterminant;
            barycentricS = barycentricS * inverseGramDeterminant;
            barycentricT = barycentricT * inverseGramDeterminant;
            distanceSquared =
                offsetLengthSquared +
                ((barycentricS *
                  ((2.0f * pointOffsetDotEdge1) +
                   ((edge1LengthSquared * barycentricS) + (edgeDot * barycentricT)))) +
                 (barycentricT *
                  ((2.0f * pointOffsetDotEdge2) +
                   ((edgeDot * barycentricS) + (edge2LengthSquared * barycentricT)))));
        }
    } else if (barycentricS < 0.0f) {
        crossEdgeProjection = edgeDot + pointOffsetDotEdge1;
        boundaryEdgeProjection = edge2LengthSquared + pointOffsetDotEdge2;
        if (boundaryEdgeProjection > crossEdgeProjection) {
            edgeParameterNumerator = boundaryEdgeProjection - crossEdgeProjection;
            edgeParameterDenominator = edge2LengthSquared + (edge1LengthSquared - (2.0f * edgeDot));
            if (edgeParameterNumerator >= edgeParameterDenominator) {
                barycentricS = 1.0f;
                barycentricT = 0.0f;
                distanceSquared =
                    offsetLengthSquared + (edge1LengthSquared + (2.0f * pointOffsetDotEdge1));
            } else {
                barycentricS = edgeParameterNumerator / edgeParameterDenominator;
                barycentricT = 1.0f - barycentricS;
                distanceSquared =
                    offsetLengthSquared +
                    ((barycentricS *
                      ((2.0f * pointOffsetDotEdge1) +
                       ((edge1LengthSquared * barycentricS) + (edgeDot * barycentricT)))) +
                     (barycentricT *
                      ((2.0f * pointOffsetDotEdge2) +
                       ((edgeDot * barycentricS) + (edge2LengthSquared * barycentricT)))));
            }
        } else {
            barycentricS = 0.0f;
            if (boundaryEdgeProjection <= 0.0f) {
                barycentricT = 1.0f;
                distanceSquared =
                    offsetLengthSquared + (edge2LengthSquared + (2.0f * pointOffsetDotEdge2));
            } else if (pointOffsetDotEdge2 >= 0.0f) {
                barycentricT = 0.0f;
                distanceSquared = offsetLengthSquared;
            } else {
                barycentricT = -pointOffsetDotEdge2 / edge2LengthSquared;
                distanceSquared = offsetLengthSquared + (pointOffsetDotEdge2 * barycentricT);
            }
        }
    } else if (barycentricT < 0.0f) {
        crossEdgeProjection = edgeDot + pointOffsetDotEdge2;
        boundaryEdgeProjection = edge1LengthSquared + pointOffsetDotEdge1;
        if (boundaryEdgeProjection > crossEdgeProjection) {
            edgeParameterNumerator = boundaryEdgeProjection - crossEdgeProjection;
            edgeParameterDenominator = edge2LengthSquared + (edge1LengthSquared - (2.0f * edgeDot));
            if (edgeParameterNumerator >= edgeParameterDenominator) {
                barycentricT = 1.0f;
                barycentricS = 0.0f;
                distanceSquared =
                    offsetLengthSquared + (edge2LengthSquared + (2.0f * pointOffsetDotEdge2));
            } else {
                barycentricT = edgeParameterNumerator / edgeParameterDenominator;
                barycentricS = 1.0f - barycentricT;
                distanceSquared =
                    offsetLengthSquared +
                    ((barycentricS *
                      ((2.0f * pointOffsetDotEdge1) +
                       ((edge1LengthSquared * barycentricS) + (edgeDot * barycentricT)))) +
                     (barycentricT *
                      ((2.0f * pointOffsetDotEdge2) +
                       ((edgeDot * barycentricS) + (edge2LengthSquared * barycentricT)))));
            }
        } else {
            barycentricT = 0.0f;
            if (boundaryEdgeProjection <= 0.0f) {
                barycentricS = 1.0f;
                distanceSquared =
                    offsetLengthSquared + (edge1LengthSquared + (2.0f * pointOffsetDotEdge1));
            } else if (pointOffsetDotEdge1 >= 0.0f) {
                barycentricS = 0.0f;
                distanceSquared = offsetLengthSquared;
            } else {
                barycentricS = -pointOffsetDotEdge1 / edge1LengthSquared;
                distanceSquared = offsetLengthSquared + (pointOffsetDotEdge1 * barycentricS);
            }
        }
    } else {
        edgeParameterNumerator =
            ((edge2LengthSquared + pointOffsetDotEdge2) - edgeDot) - pointOffsetDotEdge1;
        if (edgeParameterNumerator <= 0.0f) {
            barycentricS = 0.0f;
            barycentricT = 1.0f;
            distanceSquared =
                offsetLengthSquared + (edge2LengthSquared + (2.0f * pointOffsetDotEdge2));
        } else {
            edgeParameterDenominator = edge2LengthSquared + (edge1LengthSquared - (2.0f * edgeDot));
            if (edgeParameterNumerator >= edgeParameterDenominator) {
                barycentricS = 1.0f;
                barycentricT = 0.0f;
                distanceSquared =
                    offsetLengthSquared + (edge1LengthSquared + (2.0f * pointOffsetDotEdge1));
            } else {
                barycentricS = edgeParameterNumerator / edgeParameterDenominator;
                barycentricT = 1.0f - barycentricS;
                distanceSquared =
                    offsetLengthSquared +
                    ((barycentricS *
                      ((2.0f * pointOffsetDotEdge1) +
                       ((edge1LengthSquared * barycentricS) + (edgeDot * barycentricT)))) +
                     (barycentricT *
                      ((2.0f * pointOffsetDotEdge2) +
                       ((edgeDot * barycentricS) + (edge2LengthSquared * barycentricT)))));
            }
        }
    }
    if (barycentricSOut) {
        *barycentricSOut = barycentricS;
    }
    if (barycentricTOut) {
        *barycentricTOut = barycentricT;
    }
    return fabsf(distanceSquared);
}

/* Returns point-to-triangle distance and optionally forwards the closest barycentric
 * coordinates. */
float fn_1_36F8(HuVecF *queryPoint, M669Triangle *triangle, float *barycentricSOut,
                float *barycentricTOut)
{
    return sqrtf(fn_1_2E54(queryPoint, triangle, barycentricSOut, barycentricTOut));
}

/* Returns squared distance between finite segments and optionally writes their closest
 * parameters. */
f32 fn_1_3824(M669Segment *firstSegment, M669Segment *secondSegment, f32 *parameterAOut,
              f32 *parameterBOut)
{
    Point3d startOffset;
    f32 parameterBValue;
    f32 offsetDotDirectionA;
    f32 parameterAValue;
    f32 offsetLengthSquared;
    f32 distanceSquared;
    f32 boundaryProjection;
    f32 lengthSquaredA;
    f32 negativeOffsetDotDirectionB;
    f32 lengthSquaredB;
    f32 negativeDirectionDot;
    f32 inverseGramDeterminant;
    f32 gramDeterminant;

    M669VecSubtract(&startOffset, &firstSegment->start, &secondSegment->start);
    lengthSquaredA = M669VecLengthSquared(&firstSegment->delta);
    negativeDirectionDot = -M669VecDot(&firstSegment->delta, &secondSegment->delta);
    lengthSquaredB = M669VecLengthSquared(&secondSegment->delta);
    offsetDotDirectionA = M669VecDot(&startOffset, &firstSegment->delta);
    offsetLengthSquared = M669VecLengthSquared(&startOffset);
    gramDeterminant =
        fabsf((lengthSquaredA * lengthSquaredB) - (negativeDirectionDot * negativeDirectionDot));
    /* Use the two-parameter solution unless the segment directions are nearly parallel. */
    if (gramDeterminant >= 1.1920929e-7f) {
        negativeOffsetDotDirectionB = -M669VecDot(&startOffset, &secondSegment->delta);
        parameterAValue = (negativeDirectionDot * negativeOffsetDotDirectionB) -
                          (lengthSquaredB * offsetDotDirectionA);
        parameterBValue = (negativeDirectionDot * offsetDotDirectionA) -
                          (lengthSquaredA * negativeOffsetDotDirectionB);
        if (parameterAValue >= 0.0f) {
            if (parameterAValue <= gramDeterminant) {
                if (parameterBValue >= 0.0f) {
                    if (parameterBValue <= gramDeterminant) {
                        inverseGramDeterminant = 1.0f / gramDeterminant;
                        parameterAValue = parameterAValue * inverseGramDeterminant;
                        parameterBValue = parameterBValue * inverseGramDeterminant;
                        distanceSquared =
                            offsetLengthSquared +
                            ((parameterAValue * ((2.0f * offsetDotDirectionA) +
                                                 ((lengthSquaredA * parameterAValue) +
                                                  (negativeDirectionDot * parameterBValue)))) +
                             (parameterBValue * ((2.0f * negativeOffsetDotDirectionB) +
                                                 ((negativeDirectionDot * parameterAValue) +
                                                  (lengthSquaredB * parameterBValue)))));
                    } else {
                        parameterBValue = 1.0f;
                        boundaryProjection = negativeDirectionDot + offsetDotDirectionA;
                        if (boundaryProjection >= 0.0f) {
                            parameterAValue = 0.0f;
                            distanceSquared =
                                offsetLengthSquared +
                                (lengthSquaredB + (2.0f * negativeOffsetDotDirectionB));
                        } else if (-boundaryProjection >= lengthSquaredA) {
                            parameterAValue = 1.0f;
                            distanceSquared =
                                offsetLengthSquared + (lengthSquaredA + lengthSquaredB) +
                                (2.0f * (negativeOffsetDotDirectionB + boundaryProjection));
                        } else {
                            parameterAValue = -boundaryProjection / lengthSquaredA;
                            distanceSquared =
                                offsetLengthSquared +
                                ((2.0f * negativeOffsetDotDirectionB) +
                                 (lengthSquaredB + (boundaryProjection * parameterAValue)));
                        }
                    }
                } else {
                    parameterBValue = 0.0f;
                    if (offsetDotDirectionA >= 0.0f) {
                        parameterAValue = 0.0f;
                        distanceSquared = offsetLengthSquared;
                    } else if (-offsetDotDirectionA >= lengthSquaredA) {
                        parameterAValue = 1.0f;
                        distanceSquared =
                            offsetLengthSquared + (lengthSquaredA + (2.0f * offsetDotDirectionA));
                    } else {
                        parameterAValue = -offsetDotDirectionA / lengthSquaredA;
                        distanceSquared =
                            offsetLengthSquared + (offsetDotDirectionA * parameterAValue);
                    }
                }
            } else if (parameterBValue >= 0.0f) {
                if (parameterBValue <= gramDeterminant) {
                    parameterAValue = 1.0f;
                    boundaryProjection = negativeDirectionDot + negativeOffsetDotDirectionB;
                    if (boundaryProjection >= 0.0f) {
                        parameterBValue = 0.0f;
                        distanceSquared =
                            offsetLengthSquared + (lengthSquaredA + (2.0f * offsetDotDirectionA));
                    } else if (-boundaryProjection >= lengthSquaredB) {
                        parameterBValue = 1.0f;
                        distanceSquared = offsetLengthSquared + (lengthSquaredA + lengthSquaredB) +
                                          (2.0f * (offsetDotDirectionA + boundaryProjection));
                    } else {
                        parameterBValue = -boundaryProjection / lengthSquaredB;
                        distanceSquared =
                            offsetLengthSquared +
                            ((2.0f * offsetDotDirectionA) +
                             (lengthSquaredA + (boundaryProjection * parameterBValue)));
                    }
                } else {
                    boundaryProjection = negativeDirectionDot + offsetDotDirectionA;
                    if (-boundaryProjection <= lengthSquaredA) {
                        parameterBValue = 1.0f;
                        if (boundaryProjection >= 0.0f) {
                            parameterAValue = 0.0f;
                            distanceSquared =
                                offsetLengthSquared +
                                (lengthSquaredB + (2.0f * negativeOffsetDotDirectionB));
                        } else {
                            parameterAValue = -boundaryProjection / lengthSquaredA;
                            distanceSquared =
                                offsetLengthSquared +
                                ((2.0f * negativeOffsetDotDirectionB) +
                                 (lengthSquaredB + (boundaryProjection * parameterAValue)));
                        }
                    } else {
                        parameterAValue = 1.0f;
                        boundaryProjection = negativeDirectionDot + negativeOffsetDotDirectionB;
                        if (boundaryProjection >= 0.0f) {
                            parameterBValue = 0.0f;
                            distanceSquared = offsetLengthSquared +
                                              (lengthSquaredA + (2.0f * offsetDotDirectionA));
                        } else if (-boundaryProjection >= lengthSquaredB) {
                            parameterBValue = 1.0f;
                            distanceSquared = offsetLengthSquared +
                                              (lengthSquaredA + lengthSquaredB) +
                                              (2.0f * (offsetDotDirectionA + boundaryProjection));
                        } else {
                            parameterBValue = -boundaryProjection / lengthSquaredB;
                            distanceSquared =
                                offsetLengthSquared +
                                ((2.0f * offsetDotDirectionA) +
                                 (lengthSquaredA + (boundaryProjection * parameterBValue)));
                        }
                    }
                }
            } else if (-offsetDotDirectionA < lengthSquaredA) {
                parameterBValue = 0.0f;
                if (offsetDotDirectionA >= 0.0f) {
                    parameterAValue = 0.0f;
                    distanceSquared = offsetLengthSquared;
                } else {
                    parameterAValue = -offsetDotDirectionA / lengthSquaredA;
                    distanceSquared = offsetLengthSquared + (offsetDotDirectionA * parameterAValue);
                }
            } else {
                parameterAValue = 1.0f;
                boundaryProjection = negativeDirectionDot + negativeOffsetDotDirectionB;
                if (boundaryProjection >= 0.0f) {
                    parameterBValue = 0.0f;
                    distanceSquared =
                        offsetLengthSquared + (lengthSquaredA + (2.0f * offsetDotDirectionA));
                } else if (-boundaryProjection >= lengthSquaredB) {
                    parameterBValue = 1.0f;
                    distanceSquared = offsetLengthSquared + (lengthSquaredA + lengthSquaredB) +
                                      (2.0f * (offsetDotDirectionA + boundaryProjection));
                } else {
                    parameterBValue = -boundaryProjection / lengthSquaredB;
                    distanceSquared = offsetLengthSquared +
                                      ((2.0f * offsetDotDirectionA) +
                                       (lengthSquaredA + (boundaryProjection * parameterBValue)));
                }
            }
        } else if (parameterBValue >= 0.0f) {
            if (parameterBValue <= gramDeterminant) {
                parameterAValue = 0.0f;
                if (negativeOffsetDotDirectionB >= 0.0f) {
                    parameterBValue = 0.0f;
                    distanceSquared = offsetLengthSquared;
                } else if (-negativeOffsetDotDirectionB >= lengthSquaredB) {
                    parameterBValue = 1.0f;
                    distanceSquared = offsetLengthSquared +
                                      (lengthSquaredB + (2.0f * negativeOffsetDotDirectionB));
                } else {
                    parameterBValue = -negativeOffsetDotDirectionB / lengthSquaredB;
                    distanceSquared =
                        offsetLengthSquared + (negativeOffsetDotDirectionB * parameterBValue);
                }
            } else {
                boundaryProjection = negativeDirectionDot + offsetDotDirectionA;
                if (boundaryProjection < 0.0f) {
                    parameterBValue = 1.0f;
                    if (-boundaryProjection >= lengthSquaredA) {
                        parameterAValue = 1.0f;
                        distanceSquared =
                            offsetLengthSquared + (lengthSquaredA + lengthSquaredB) +
                            (2.0f * (negativeOffsetDotDirectionB + boundaryProjection));
                    } else {
                        parameterAValue = -boundaryProjection / lengthSquaredA;
                        distanceSquared =
                            offsetLengthSquared +
                            ((2.0f * negativeOffsetDotDirectionB) +
                             (lengthSquaredB + (boundaryProjection * parameterAValue)));
                    }
                } else {
                    parameterAValue = 0.0f;
                    if (negativeOffsetDotDirectionB >= 0.0f) {
                        parameterBValue = 0.0f;
                        distanceSquared = offsetLengthSquared;
                    } else if (-negativeOffsetDotDirectionB >= lengthSquaredB) {
                        parameterBValue = 1.0f;
                        distanceSquared = offsetLengthSquared +
                                          (lengthSquaredB + (2.0f * negativeOffsetDotDirectionB));
                    } else {
                        parameterBValue = -negativeOffsetDotDirectionB / lengthSquaredB;
                        distanceSquared =
                            offsetLengthSquared + (negativeOffsetDotDirectionB * parameterBValue);
                    }
                }
            }
        } else if (offsetDotDirectionA < 0.0f) {
            parameterBValue = 0.0f;
            if (-offsetDotDirectionA >= lengthSquaredA) {
                parameterAValue = 1.0f;
                distanceSquared =
                    offsetLengthSquared + (lengthSquaredA + (2.0f * offsetDotDirectionA));
            } else {
                parameterAValue = -offsetDotDirectionA / lengthSquaredA;
                distanceSquared = offsetLengthSquared + (offsetDotDirectionA * parameterAValue);
            }
        } else {
            parameterAValue = 0.0f;
            if (negativeOffsetDotDirectionB >= 0.0f) {
                parameterBValue = 0.0f;
                distanceSquared = offsetLengthSquared;
            } else if (-negativeOffsetDotDirectionB >= lengthSquaredB) {
                parameterBValue = 1.0f;
                distanceSquared =
                    offsetLengthSquared + (lengthSquaredB + (2.0f * negativeOffsetDotDirectionB));
            } else {
                parameterBValue = -negativeOffsetDotDirectionB / lengthSquaredB;
                distanceSquared =
                    offsetLengthSquared + (negativeOffsetDotDirectionB * parameterBValue);
            }
        }
    } else if (negativeDirectionDot > 0.0f) {
        if (offsetDotDirectionA >= 0.0f) {
            parameterAValue = 0.0f;
            parameterBValue = 0.0f;
            distanceSquared = offsetLengthSquared;
        } else if (-offsetDotDirectionA <= lengthSquaredA) {
            parameterAValue = -offsetDotDirectionA / lengthSquaredA;
            parameterBValue = 0.0f;
            distanceSquared = offsetLengthSquared + (offsetDotDirectionA * parameterAValue);
        } else {
            negativeOffsetDotDirectionB = -M669VecDot(&startOffset, &secondSegment->delta);
            parameterAValue = 1.0f;
            boundaryProjection = lengthSquaredA + offsetDotDirectionA;
            if (-boundaryProjection >= negativeDirectionDot) {
                parameterBValue = 1.0f;
                distanceSquared = offsetLengthSquared + (lengthSquaredA + lengthSquaredB) +
                                  (2.0f * (negativeOffsetDotDirectionB +
                                           (negativeDirectionDot + offsetDotDirectionA)));
            } else {
                parameterBValue = -boundaryProjection / negativeDirectionDot;
                distanceSquared = offsetLengthSquared +
                                  (lengthSquaredA + (2.0f * offsetDotDirectionA)) +
                                  (parameterBValue *
                                   ((lengthSquaredB * parameterBValue) +
                                    (2.0f * (negativeDirectionDot + negativeOffsetDotDirectionB))));
            }
        }
    } else if (-offsetDotDirectionA >= lengthSquaredA) {
        parameterAValue = 1.0f;
        parameterBValue = 0.0f;
        distanceSquared = offsetLengthSquared + (lengthSquaredA + (2.0f * offsetDotDirectionA));
    } else if (offsetDotDirectionA <= 0.0f) {
        parameterAValue = -offsetDotDirectionA / lengthSquaredA;
        parameterBValue = 0.0f;
        distanceSquared = offsetLengthSquared + (offsetDotDirectionA * parameterAValue);
    } else {
        negativeOffsetDotDirectionB = -M669VecDot(&startOffset, &secondSegment->delta);
        parameterAValue = 0.0f;
        if (offsetDotDirectionA >= -negativeDirectionDot) {
            parameterBValue = 1.0f;
            distanceSquared =
                offsetLengthSquared + (lengthSquaredB + (2.0f * negativeOffsetDotDirectionB));
        } else {
            parameterBValue = -offsetDotDirectionA / negativeDirectionDot;
            distanceSquared =
                offsetLengthSquared + (parameterBValue * ((2.0f * negativeOffsetDotDirectionB) +
                                                          (lengthSquaredB * parameterBValue)));
        }
    }
    if (parameterAOut) {
        *parameterAOut = parameterAValue;
    }
    if (parameterBOut) {
        *parameterBOut = parameterBValue;
    }
    return fabsf(distanceSquared);
}

/* Passes its inputs to the segment-distance routine and returns the square root of that result. */
float fn_1_4420(HuVecF *queryPoint, M669Triangle *triangle, float *barycentricSOut,
                float *barycentricTOut)
{
    f32 fn_1_3824();
    return sqrtf(fn_1_3824(queryPoint, triangle, barycentricSOut, barycentricTOut));
}

/* Returns squared distance from a point to the nearest position on a finite segment. */
float fn_1_454C(HuVecF *queryPoint, M669Segment *lineSegment)
{
    HuVecF pointOffset, projectedOffset;
    float segmentParameter, segmentLengthSquared;
    M669VecSubtract(&pointOffset, queryPoint, &lineSegment->start);
    segmentParameter = M669VecDot(&pointOffset, &lineSegment->delta);
    if (segmentParameter <= 0.0f) {
        segmentParameter = 0.0f;
    } else {
        segmentLengthSquared = M669VecLengthSquared(&lineSegment->delta);
        if (segmentParameter >= segmentLengthSquared) {
            segmentParameter = 1.0f;
            M669VecSubtract(&pointOffset, &pointOffset, &lineSegment->delta);
        } else {
            segmentParameter /= segmentLengthSquared;
            M669VecScale(&projectedOffset, &lineSegment->delta, segmentParameter);
            M669VecSubtract(&pointOffset, &pointOffset, &projectedOffset);
        }
    }
    return M669VecLengthSquared(&pointOffset);
}

float fn_1_4510_inlined(HuVecF *queryPoint, M669Segment *lineSegment);

/* Computes the clamped point-to-segment squared distance for the distance wrappers. */
static inline float fn_1_4510_inlined(HuVecF *queryPoint, M669Segment *lineSegment)
{
    HuVecF pointOffset, projectedOffset;
    float segmentParameter, segmentLengthSquared;
    M669VecSubtract(&pointOffset, queryPoint, &lineSegment->start);
    segmentParameter = M669VecDot(&pointOffset, &lineSegment->delta);
    if (segmentParameter <= 0.0f) {
        segmentParameter = 0.0f;
    } else {
        segmentLengthSquared = M669VecLengthSquared(&lineSegment->delta);
        if (segmentParameter >= segmentLengthSquared) {
            segmentParameter = 1.0f;
            M669VecSubtract(&pointOffset, &pointOffset, &lineSegment->delta);
        } else {
            segmentParameter /= segmentLengthSquared;
            M669VecScale(&projectedOffset, &lineSegment->delta, segmentParameter);
            M669VecSubtract(&pointOffset, &pointOffset, &projectedOffset);
        }
    }
    return M669VecLengthSquared(&pointOffset);
}

/* Returns point-to-segment distance by taking the square root of its squared-distance helper. */
float fn_1_4644(HuVecF *queryPoint, M669Segment *lineSegment)
{
    return sqrtf(fn_1_4510_inlined(queryPoint, lineSegment));
}

/* Returns the clamped squared point-to-segment term after subtracting the sphere radius squared. */
float fn_1_4828(M669Sphere *movingSphere, M669Segment *lineSegment)
{
    float gapSquared;
    gapSquared = fn_1_454C(&movingSphere->center, lineSegment);
    gapSquared -= movingSphere->radius * movingSphere->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

float fn_1_47EC_inlined(M669Sphere *movingSphere, M669Segment *lineSegment);

/* Computes the clamped sphere-to-segment term for its square-root wrapper. */
static inline float fn_1_47EC_inlined(M669Sphere *movingSphere, M669Segment *lineSegment)
{
    float gapSquared;
    gapSquared = fn_1_4510_inlined(&movingSphere->center, lineSegment);
    gapSquared -= movingSphere->radius * movingSphere->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

/* Returns the square root of the clamped sphere-to-segment term. */
double fn_1_4984(M669Sphere *movingSphere, M669Segment *lineSegment)
{
    return sqrtf(fn_1_47EC_inlined(movingSphere, lineSegment));
}

/* Returns the segment-distance term after subtracting both sphere and capsule squared radii,
 * clamped to zero. */
float fn_1_4BC8(M669Sphere *movingSphere, M669Capsule *lineCapsule)
{
    float gapSquared;
    gapSquared = fn_1_454C(&movingSphere->center, &lineCapsule->segment);
    gapSquared -=
        movingSphere->radius * movingSphere->radius + lineCapsule->radius * lineCapsule->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

float fn_1_4B8C_inlined(M669Sphere *movingSphere, M669Capsule *lineCapsule);

/* Computes the clamped sphere-to-capsule term for its square-root wrapper. */
static inline float fn_1_4B8C_inlined(M669Sphere *movingSphere, M669Capsule *lineCapsule)
{
    float gapSquared;
    gapSquared = fn_1_4510_inlined(&movingSphere->center, &lineCapsule->segment);
    gapSquared -=
        movingSphere->radius * movingSphere->radius + lineCapsule->radius * lineCapsule->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

/* Returns the square root of the clamped sphere-to-capsule term. */
double fn_1_4D34(M669Sphere *movingSphere, M669Capsule *lineCapsule)
{
    return sqrtf(fn_1_4B8C_inlined(movingSphere, lineCapsule));
}
