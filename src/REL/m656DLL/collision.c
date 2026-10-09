/* Builds bounds and tests sphere, capsule, triangle, and segment collisions. */
#include "REL/m656/m656.h"

/* Sets a stage-space sphere when gameplay creates a collision shape. */
void fn_1_22FC(M656Sphere *sphere, f32 centerX, f32 centerY, f32 centerZ, f32 radius)
{
    sphere->center.x = centerX;
    sphere->center.y = centerY;
    sphere->center.z = centerZ;
    sphere->radius = radius;
}

/* Computes a capsule's stage-space broad-phase bounds before overlap tests. */
void fn_1_2310(M656Bounds *bounds, M656Capsule *capsule)
{
    HuVecF endpoints[2];
    endpoints[0] = capsule->segment.start;
    fn_1_4A8(&endpoints[1], &capsule->segment.start, &capsule->segment.delta);
    bounds->min.x =
        (endpoints[0].x < endpoints[1].x ? endpoints[0].x : endpoints[1].x) - capsule->radius;
    bounds->max.x =
        capsule->radius + (endpoints[0].x >= endpoints[1].x ? endpoints[0].x : endpoints[1].x);
    bounds->min.y =
        (endpoints[0].y < endpoints[1].y ? endpoints[0].y : endpoints[1].y) - capsule->radius;
    bounds->max.y =
        capsule->radius + (endpoints[0].y >= endpoints[1].y ? endpoints[0].y : endpoints[1].y);
    bounds->min.z =
        (endpoints[0].z < endpoints[1].z ? endpoints[0].z : endpoints[1].z) - capsule->radius;
    bounds->max.z =
        capsule->radius + (endpoints[0].z >= endpoints[1].z ? endpoints[0].z : endpoints[1].z);
}

/* fn_1_67E8 uses these bounds to reject meteors before its sphere-contact test. */
void fn_1_24C8(M656Bounds *bounds, M656Sphere *sphere)
{
    bounds->min = sphere->center;
    bounds->min.x -= sphere->radius;
    bounds->min.y -= sphere->radius;
    bounds->min.z -= sphere->radius;
    bounds->max = sphere->center;
    bounds->max.x += sphere->radius;
    bounds->max.y += sphere->radius;
    bounds->max.z += sphere->radius;
}

/* Computes triangle bounds for broad-phase collision checks. */
void fn_1_255C(M656Bounds *bounds, M656Triangle *triangle)
{
    HuVecF vertices[3];
    int vertexIndex;
    vertices[0] = triangle->start;
    fn_1_4A8(&vertices[1], &triangle->start, &triangle->edge1);
    fn_1_4A8(&vertices[2], &triangle->start, &triangle->edge2);
    bounds->min = vertices[0];
    bounds->max = bounds->min;
    for (vertexIndex = 1; vertexIndex < 3; vertexIndex++) {
        bounds->min.x =
            bounds->min.x > vertices[vertexIndex].x ? vertices[vertexIndex].x : bounds->min.x;
        bounds->max.x =
            bounds->max.x <= vertices[vertexIndex].x ? vertices[vertexIndex].x : bounds->max.x;
        bounds->min.y =
            bounds->min.y > vertices[vertexIndex].y ? vertices[vertexIndex].y : bounds->min.y;
        bounds->max.y =
            bounds->max.y <= vertices[vertexIndex].y ? vertices[vertexIndex].y : bounds->max.y;
        bounds->min.z =
            bounds->min.z > vertices[vertexIndex].z ? vertices[vertexIndex].z : bounds->min.z;
        bounds->max.z =
            bounds->max.z <= vertices[vertexIndex].z ? vertices[vertexIndex].z : bounds->max.z;
    }
}

/* fn_1_67E8 rejects separated player and meteor bounds before testing sphere contact. */
s32 fn_1_27C0(M656Bounds *firstBounds, M656Bounds *secondBounds)
{
    if ((firstBounds->min.z < secondBounds->max.z) && (firstBounds->max.z > secondBounds->min.z) &&
        (firstBounds->min.x < secondBounds->max.x) && (firstBounds->max.x > secondBounds->min.x) &&
        (firstBounds->min.y < secondBounds->max.y) && (firstBounds->max.y > secondBounds->min.y)) {
        return 1;
    }
    return 0;
}

/* Predicts a moving sphere collision during the current frame and reports its contact time. */
s32 fn_1_2830(M656Sphere *firstSphere, s32 velocityWordA, M656Sphere *secondSphere,
              s32 velocityWordB, float maxTime, float *hitTime, HuVecF *contactPointOut)
{
    HuVecF relativeVelocity, centerDelta, contactPoint;
    float relativeSpeedSquared, centerDistanceSquared, combinedRadius, combinedRadiusSquared;
    float velocityDotOffset, separationTerm, collisionDiscriminant;
    fn_1_4E0(&relativeVelocity, (HuVecF *)&velocityWordB, (HuVecF *)&velocityWordA);
    relativeSpeedSquared = fn_1_410(&relativeVelocity);
    fn_1_4E0(&centerDelta, &secondSphere->center, &firstSphere->center);
    centerDistanceSquared = fn_1_410(&centerDelta);
    combinedRadius = firstSphere->radius + secondSphere->radius;
    combinedRadiusSquared = combinedRadius * combinedRadius;
    if (relativeSpeedSquared > 0.0f) {
        velocityDotOffset = fn_1_440(&centerDelta, &relativeVelocity);
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
                        fn_1_4A8(contactPointOut, &firstSphere->center, &secondSphere->center);
                        fn_1_6F8(contactPointOut, contactPointOut, 0.5f);
                    } else {
                        *hitTime = -(velocityDotOffset + sqrtf(collisionDiscriminant)) /
                                   relativeSpeedSquared;
                        if (*hitTime < 0.0f) *hitTime = 0.0f;
                        else if (*hitTime > maxTime) *hitTime = maxTime;
                        fn_1_6F8(&contactPoint, &relativeVelocity, *hitTime);
                        /* This branch updates a temporary contact point but leaves the output
                         * untouched. */
                        fn_1_4A8(&contactPoint, &contactPoint, &centerDelta);
                    }
                    return 1;
                }
            }
            return 0;
        }
    }
    if (centerDistanceSquared <= combinedRadiusSquared) {
        *hitTime = 0.0f;
        fn_1_4A8(contactPointOut, &firstSphere->center, &secondSphere->center);
        fn_1_6F8(contactPointOut, contactPointOut, 0.5f);
        return 1;
    }
    return 0;
}

/* fn_1_67E8 compares max(0, centerDistanceSquared - radiusASquared - radiusBSquared) with zero. */
float fn_1_2BE8(M656Sphere *firstSphere, M656Sphere *secondSphere)
{
    HuVecF centerOffset;
    float gapSquared;
    fn_1_4E0(&centerOffset, &firstSphere->center, &secondSphere->center);
    gapSquared = fn_1_410(&centerOffset);
    gapSquared -=
        firstSphere->radius * firstSphere->radius + secondSphere->radius * secondSphere->radius;
    if (gapSquared < 0.0f) gapSquared = 0.0f;
    return gapSquared;
}

/* Returns sqrt(max(0, centerDistanceSquared - radiusASquared - radiusBSquared)) for gameplay
 * thresholds. */
float fn_1_2C8C(M656Sphere *sphereA, M656Sphere *sphereB)
{
    return sqrtf(fn_1_2BE8(sphereA, sphereB));
}

/* Returns the squared gap to the closest triangle point and optionally outputs its barycentric
 * weights. */
f32 fn_1_2E18(Point3d *queryPoint, M656Triangle *triangle, f32 *barycentricSOut,
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
    f32 edge1;
    f32 edge0;

    fn_1_4E0(&pointOffset, &triangle->start, queryPoint);
    edge1LengthSquared = fn_1_410(&triangle->edge1);
    edgeDot = fn_1_440(&triangle->edge1, &triangle->edge2);
    edge2LengthSquared = fn_1_410(&triangle->edge2);
    pointOffsetDotEdge1 = fn_1_440(&pointOffset, &triangle->edge1);
    pointOffsetDotEdge2 = fn_1_440(&pointOffset, &triangle->edge2);
    offsetLengthSquared = fn_1_410(&pointOffset);
    gramDeterminant = fabsf((edge1LengthSquared * edge2LengthSquared) - (edgeDot * edgeDot));
    barycentricS = (edgeDot * pointOffsetDotEdge2) - (edge2LengthSquared * pointOffsetDotEdge1);
    barycentricT = (edgeDot * pointOffsetDotEdge1) - (edge1LengthSquared * pointOffsetDotEdge2);
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
        edge0 = edgeDot + pointOffsetDotEdge1;
        edge1 = edge2LengthSquared + pointOffsetDotEdge2;
        if (edge1 > edge0) {
            edgeParameterNumerator = edge1 - edge0;
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
            if (edge1 <= 0.0f) {
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
        edge0 = edgeDot + pointOffsetDotEdge2;
        edge1 = edge1LengthSquared + pointOffsetDotEdge1;
        if (edge1 > edge0) {
            edgeParameterNumerator = edge1 - edge0;
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
            if (edge1 <= 0.0f) {
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

/* Takes the square root of the triangle gap for callers that need distance units. */
float fn_1_36BC(HuVecF *queryPoint, M656Triangle *triangle, float *barycentricSOut,
                float *barycentricTOut)
{
    return sqrtf(fn_1_2E18(queryPoint, triangle, barycentricSOut, barycentricTOut));
}

/* Returns the squared gap between two segments and optionally outputs the closest-point
 * parameters. */
f32 fn_1_37E8(M656Segment *firstSegment, M656Segment *secondSegment, f32 *parameterAOut,
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

    fn_1_4E0(&startOffset, &firstSegment->start, &secondSegment->start);
    lengthSquaredA = fn_1_410(&firstSegment->delta);
    negativeDirectionDot = -fn_1_440(&firstSegment->delta, &secondSegment->delta);
    lengthSquaredB = fn_1_410(&secondSegment->delta);
    offsetDotDirectionA = fn_1_440(&startOffset, &firstSegment->delta);
    offsetLengthSquared = fn_1_410(&startOffset);
    gramDeterminant =
        fabsf((lengthSquaredA * lengthSquaredB) - (negativeDirectionDot * negativeDirectionDot));
    if (gramDeterminant >= 1.1920929e-7f) {
        negativeOffsetDotDirectionB = -fn_1_440(&startOffset, &secondSegment->delta);
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
            negativeOffsetDotDirectionB = -fn_1_440(&startOffset, &secondSegment->delta);
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
        negativeOffsetDotDirectionB = -fn_1_440(&startOffset, &secondSegment->delta);
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

/* Takes the square root of the segment gap for callers that need distance units. */
float fn_1_43E4(M656Segment *firstSegment, M656Segment *secondSegment, float *parameterAOut,
                float *parameterBOut)
{
    return sqrtf(fn_1_37E8(firstSegment, secondSegment, parameterAOut, parameterBOut));
}

/* Returns the squared gap to the nearest point on a segment, clamping the projection to its
 * endpoints. */
float fn_1_4510(HuVecF *queryPoint, M656Segment *lineSegment)
{
    HuVecF pointOffset, projectedOffset;
    float segmentParameter, segmentLengthSquared;
    fn_1_4E0(&pointOffset, queryPoint, &lineSegment->start);
    segmentParameter = fn_1_440(&pointOffset, &lineSegment->delta);
    if (segmentParameter <= 0.0f) {
        segmentParameter = 0.0f;
    } else {
        segmentLengthSquared = fn_1_410(&lineSegment->delta);
        if (segmentParameter >= segmentLengthSquared) {
            segmentParameter = 1.0f;
            fn_1_4E0(&pointOffset, &pointOffset, &lineSegment->delta);
        } else {
            segmentParameter /= segmentLengthSquared;
            fn_1_6F8(&projectedOffset, &lineSegment->delta, segmentParameter);
            fn_1_4E0(&pointOffset, &pointOffset, &projectedOffset);
        }
    }
    return fn_1_410(&pointOffset);
}

/* Takes the square root of the point-to-segment gap for callers that need distance units. */
float fn_1_4608(HuVecF *queryPoint, M656Segment *lineSegment)
{
    return sqrtf(fn_1_4510(queryPoint, lineSegment));
}

/* Returns max(0, pointSegmentDistanceSquared - sphereRadiusSquared) for collision checks. */
float fn_1_47EC(M656Sphere *movingSphere, M656Segment *lineSegment)
{
    float gapSquared;
    gapSquared = fn_1_4510(&movingSphere->center, lineSegment);
    gapSquared -= movingSphere->radius * movingSphere->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

/* Returns sqrt(max(0, pointSegmentDistanceSquared - sphereRadiusSquared)) in stage units. */
double fn_1_4948(M656Sphere *movingSphere, M656Segment *lineSegment)
{
    return sqrtf(fn_1_47EC(movingSphere, lineSegment));
}

/* Returns max(0, pointSegmentDistanceSquared - sphereRadiusSquared - capsuleRadiusSquared) for
 * collision checks. */
float fn_1_4B8C(M656Sphere *movingSphere, M656Capsule *lineCapsule)
{
    float gapSquared;
    gapSquared = fn_1_4510(&movingSphere->center, &lineCapsule->segment);
    gapSquared -=
        movingSphere->radius * movingSphere->radius + lineCapsule->radius * lineCapsule->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

/* Returns sqrt(max(0, pointSegmentDistanceSquared - sphereRadiusSquared - capsuleRadiusSquared)) in
 * stage units. */
double fn_1_4CF8(M656Sphere *movingSphere, M656Capsule *lineCapsule)
{
    return sqrtf(fn_1_4B8C(movingSphere, lineCapsule));
}
