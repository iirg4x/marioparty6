/* Sphere, segment, triangle and capsule collision queries for Clean Team. */
#include "REL/m645DLL/shared_types.h"
#include "math.h"
#include "game/object.h"

#include "REL/m645DLL.h"

void fn_1_8C0(M645Bounds *bounds, M645Triangle *triangle)
{
    HuVecF vertices[4];
    int vertexIndex;

    vertices[0] = triangle->start;
    fn_1_480(&vertices[1], &triangle->start, &triangle->edge1);
    fn_1_480(&vertices[2], &triangle->start, &triangle->edge2);
    fn_1_480(&vertices[3], &vertices[1], &triangle->edge2);
    bounds->min = vertices[0];
    bounds->max = bounds->min;
    for (vertexIndex = 1; vertexIndex < 4; vertexIndex++) {
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

void fn_1_22FC(M645Sphere *out, f32 x, f32 y, f32 z, f32 radius);

void fn_1_B34(M645Bounds *out, M645Sphere *sphere)
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

void fn_1_255C(M645Bounds *out, M645Triangle *triangle);

s32 fn_1_BC8(M645Bounds *a, M645Bounds *b)
{
    if ((a->min.z < b->max.z) && (a->max.z > b->min.z) && (a->min.x < b->max.x) && (a->max.x > b->min.x) && (a->min.y < b->max.y) && (a->max.y > b->min.y)) {
        return 1;
    }
    return 0;
}

extern float fn_1_C38(M645Sphere *, M645Triangle *, float *, float *);
extern float fn_1_3E8(const HuVecF *);
extern float fn_1_418(const HuVecF *, const HuVecF *);
extern void fn_1_4B8(HuVecF *, const HuVecF *, const HuVecF *);

f32 fn_1_C38(M645Sphere *sphere, M645Triangle *triangle,
             f32 *barycentricS, f32 *barycentricT)
{
    HuVecF difference;
    f32 edge1LengthSquared;
    f32 edge2LengthSquared;
    f32 edge1Projection;
    f32 edge2Projection;
    f32 edge1Weight;
    f32 edge2Weight;
    f32 distanceSquared;

    fn_1_4B8(&difference, &triangle->start, &sphere->center);
    edge1LengthSquared = fn_1_3E8(&triangle->edge1);
    edge2LengthSquared = fn_1_3E8(&triangle->edge2);
    edge1Projection = fn_1_418(&difference, &triangle->edge1);
    edge2Projection = fn_1_418(&difference, &triangle->edge2);
    edge1Weight = -edge1Projection;
    edge2Weight = -edge2Projection;
    distanceSquared = fn_1_3E8(&difference);
    if (edge1Weight < 0.0f) {
        edge1Weight = 0.0f;
    } else if (edge1Weight <= edge1LengthSquared) {
        edge1Weight /= edge1LengthSquared;
        distanceSquared += edge1Projection * edge1Weight;
    } else {
        edge1Weight = 1.0f;
        distanceSquared += edge1LengthSquared + (2.0f * edge1Projection);
    }
    if (edge2Weight < 0.0f) {
        edge2Weight = 0.0f;
    } else if (edge2Weight <= edge2LengthSquared) {
        edge2Weight /= edge2LengthSquared;
        distanceSquared += edge2Projection * edge2Weight;
    } else {
        edge2Weight = 1.0f;
        distanceSquared += edge2LengthSquared + (2.0f * edge2Projection);
    }
    if (barycentricS) {
        *barycentricS = edge1Weight;
    }
    if (barycentricT) {
        *barycentricT = edge2Weight;
    }
    return fabs((f64)distanceSquared);
}

float fn_1_E40(void *point, void *triangle, float *barycentricS, float *barycentricT)
{
    return sqrtf(fn_1_C38(point, triangle, barycentricS, barycentricT));
}

f32 fn_1_F6C(M645Sphere *sphere, M645Triangle *triangle,
             f32 *barycentricS, f32 *barycentricT)
{
    f32 gapSquared = fn_1_C38(sphere, triangle, barycentricS, barycentricT);
    gapSquared -= sphere->radius * sphere->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

f32 fn_1_FFC(M645Sphere *sphere, M645Triangle *triangle,
             f32 *barycentricS, f32 *barycentricT)
{
    f32 gapSquared;
    f32 radicand;

    gapSquared = fn_1_C38(sphere, triangle, barycentricS, barycentricT);
    gapSquared -= sphere->radius * sphere->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    radicand = gapSquared;
    return sqrtf(radicand);
}

f32 fn_1_1174(M645Segment *firstSegment, M645Segment *secondSegment, f32 *parameterAOut,
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

    fn_1_4B8(&startOffset, &firstSegment->start, &secondSegment->start);
    lengthSquaredA = fn_1_3E8(&firstSegment->delta);
    negativeDirectionDot = -fn_1_418(&firstSegment->delta, &secondSegment->delta);
    lengthSquaredB = fn_1_3E8(&secondSegment->delta);
    offsetDotDirectionA = fn_1_418(&startOffset, &firstSegment->delta);
    offsetLengthSquared = fn_1_3E8(&startOffset);
    gramDeterminant =
        fabsf((lengthSquaredA * lengthSquaredB) - (negativeDirectionDot * negativeDirectionDot));
    if (gramDeterminant >= 1.1920929e-7f) {
        negativeOffsetDotDirectionB = -fn_1_418(&startOffset, &secondSegment->delta);
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
            negativeOffsetDotDirectionB = -fn_1_418(&startOffset, &secondSegment->delta);
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
        negativeOffsetDotDirectionB = -fn_1_418(&startOffset, &secondSegment->delta);
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

f32 fn_1_1D70(M645Segment *queryPoint, M645Segment *triangle, f32 *barycentricSOut,
              f32 *barycentricTOut)
{
    return sqrtf(fn_1_1174(queryPoint, triangle, barycentricSOut, barycentricTOut));
}

void fn_1_6D0(HuVecF *dst, const HuVecF *src, f32 scale);

static inline f32 pointSegmentDistanceSquared(HuVecF *queryPoint, M645Segment *lineSegment)
{
    HuVecF pointOffset;
    HuVecF projectedOffset;
    f32 segmentParameter;
    f32 segmentLengthSquared;

    fn_1_4B8(&pointOffset, queryPoint, &lineSegment->start);
    segmentParameter = fn_1_418(&pointOffset, &lineSegment->delta);
    if (segmentParameter <= 0.0f) {
        segmentParameter = 0.0f;
    } else {
        segmentLengthSquared = fn_1_3E8(&lineSegment->delta);
        if (segmentParameter >= segmentLengthSquared) {
            segmentParameter = 1.0f;
            fn_1_4B8(&pointOffset, &pointOffset, &lineSegment->delta);
        } else {
            segmentParameter /= segmentLengthSquared;
            fn_1_6D0(&projectedOffset, &lineSegment->delta, segmentParameter);
            fn_1_4B8(&pointOffset, &pointOffset, &projectedOffset);
        }
    }
    return fn_1_3E8(&pointOffset);
}

f32 fn_1_1E9C(HuVecF *queryPoint, M645Segment *lineSegment)
{
    HuVecF pointOffset;
    HuVecF projectedOffset;
    f32 segmentParameter;
    f32 segmentLengthSquared;

    fn_1_4B8(&pointOffset, queryPoint, &lineSegment->start);
    segmentParameter = fn_1_418(&pointOffset, &lineSegment->delta);
    if (segmentParameter <= 0.0f) {
        segmentParameter = 0.0f;
    } else {
        segmentLengthSquared = fn_1_3E8(&lineSegment->delta);
        if (segmentParameter >= segmentLengthSquared) {
            segmentParameter = 1.0f;
            fn_1_4B8(&pointOffset, &pointOffset, &lineSegment->delta);
        } else {
            segmentParameter /= segmentLengthSquared;
            fn_1_6D0(&projectedOffset, &lineSegment->delta, segmentParameter);
            fn_1_4B8(&pointOffset, &pointOffset, &projectedOffset);
        }
    }
    return fn_1_3E8(&pointOffset);
}

f32 fn_1_1F94(HuVecF *queryPoint, M645Segment *lineSegment)
{
    return sqrtf(pointSegmentDistanceSquared(queryPoint, lineSegment));
}

f32 fn_1_2178(M645Sphere *movingSphere, const M645Segment *lineSegment)
{
    HuVecF projectedOffset;
    HuVecF pointOffset;
    f32 segmentParameter;
    f32 segmentLengthSquared;
    f32 distanceSquared;
    f32 gapSquared;

    fn_1_4B8(&pointOffset, &movingSphere->center, &lineSegment->start);
    segmentParameter = fn_1_418(&pointOffset, &lineSegment->delta);
    if (segmentParameter <= 0.0f) {
        segmentParameter = 0.0f;
    } else {
        segmentLengthSquared = fn_1_3E8(&lineSegment->delta);
        if (segmentParameter >= segmentLengthSquared) {
            segmentParameter = 1.0f;
            fn_1_4B8(&pointOffset, &pointOffset, &lineSegment->delta);
        } else {
            segmentParameter /= segmentLengthSquared;
            fn_1_6D0(&projectedOffset, &lineSegment->delta, segmentParameter);
            fn_1_4B8(&pointOffset, &pointOffset, &projectedOffset);
        }
    }
    distanceSquared = fn_1_3E8(&pointOffset);
    gapSquared = distanceSquared;
    gapSquared -= movingSphere->radius * movingSphere->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

static inline f32 sphereSegmentGapSquared(M645Sphere *movingSphere, M645Segment *lineSegment)
{
    f32 gapSquared;

    gapSquared = pointSegmentDistanceSquared(&movingSphere->center, lineSegment);
    gapSquared -= movingSphere->radius * movingSphere->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

f32 fn_1_22D4(M645Sphere *movingSphere, M645Segment *lineSegment)
{
    return sqrtf(sphereSegmentGapSquared(movingSphere, lineSegment));
}

f32 fn_1_2518(M645Sphere *firstSphere, M645Sphere *secondSphere)
{
    HuVecF centerOffset;
    f32 gapSquared;

    fn_1_4B8(&centerOffset, &firstSphere->center, &secondSphere->center);
    gapSquared = fn_1_3E8(&centerOffset);
    gapSquared -= firstSphere->radius * firstSphere->radius +
                  secondSphere->radius * secondSphere->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

f32 fn_1_25BC(M645Sphere *firstSphere, M645Sphere *secondSphere)
{
    return sqrtf(fn_1_2518(firstSphere, secondSphere));
}

f32 fn_1_2748(M645Capsule *firstCapsule, M645Capsule *secondCapsule)
{
    f32 gapSquared = fn_1_1174(&firstCapsule->segment, &secondCapsule->segment, 0, 0);
    gapSquared -= firstCapsule->radius * firstCapsule->radius +
                  secondCapsule->radius * secondCapsule->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    return gapSquared;
}

f32 fn_1_27E8(M645Capsule *firstCapsule, M645Capsule *secondCapsule)
{
    f32 gapSquared;
    f32 radicand;

    gapSquared = fn_1_1174(&firstCapsule->segment, &secondCapsule->segment,
                           0, 0);
    gapSquared -= firstCapsule->radius * firstCapsule->radius +
                  secondCapsule->radius * secondCapsule->radius;
    if (gapSquared < 0.0f) {
        gapSquared = 0.0f;
    }
    radicand = gapSquared;
    return sqrtf(radicand);
}
