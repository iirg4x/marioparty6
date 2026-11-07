/* Collision geometry: bounds of capsules, triangles and boxes, and closest-point distances
 * between points, segments and triangles. */
#include "dolphin/types.h"
#include "humath.h"
#include "math.h"

typedef struct M615Bounds {
    HuVecF min;
    HuVecF max;
} M615Bounds;

typedef struct M615Segment {
    HuVecF start;
    HuVecF delta;
} M615Segment;

typedef struct M615Capsule {
    M615Segment segment;
    f32 radius;
} M615Capsule;

typedef struct M615Triangle {
    HuVecF start;
    HuVecF edge1;
    HuVecF edge2;
} M615Triangle;

typedef struct M615Box {
    HuVecF center;
    HuVecF axis[3];
    f32 halfExtents[3];
} M615Box;

void fn_1_444(HuVecF *out, f32 x, f32 y, f32 z);
f32 fn_1_574(const HuVecF *v);
f32 fn_1_5A4(const HuVecF *a, const HuVecF *b);
void fn_1_5D4(HuVecF *dst, const HuVecF *a, const HuVecF *b);
void fn_1_60C(HuVecF *dst, const HuVecF *a, const HuVecF *b);
void fn_1_644(HuVecF *dst, const HuVecF *a, const HuVecF *b);
f32 fn_1_67C(HuVecF *dst, const HuVecF *src);
void fn_1_85C(HuVecF *dst, const HuVecF *src, f32 scale);

f32 fn_1_14E8(HuVecF *queryPoint, M615Triangle *triangle,
              f32 *barycentricS, f32 *barycentricT);

void fn_1_A0C(M615Bounds *out, M615Capsule *capsule);
void fn_1_BC4(M615Bounds *bounds, M615Triangle *triangle);
void fn_1_E38(M615Bounds *out, M615Triangle *triangle);
void fn_1_109C(M615Bounds *bounds, const M615Box *transform);
s32 fn_1_1478(M615Bounds *a, M615Bounds *b);
f32 fn_1_16F0(HuVecF *queryPoint, M615Triangle *triangle, f32 *firstParameter, f32 *secondParameter);
f32 fn_1_181C(M615Segment *firstSegment, M615Segment *secondSegment, f32 *parameterAOut,
              f32 *parameterBOut);
f32 fn_1_2418(M615Segment *firstSegment, M615Segment *secondSegment, f32 *firstParameter, f32 *secondParameter);
f32 fn_1_2544(M615Segment *segment, M615Triangle *triangle, f32 *segmentTOut, f32 *triangleSOut, f32 *triangleTOut);
f32 fn_1_4198(M615Segment *segment, M615Triangle *triangle, f32 *segmentTime, f32 *firstParameter, f32 *secondParameter);
f32 fn_1_42CC(HuVecF *queryPoint, M615Triangle *triangle, f32 *barycentricSOut,
              f32 *barycentricTOut);
f32 fn_1_4B70(HuVecF *queryPoint, M615Triangle *triangle, f32 *firstParameter, f32 *secondParameter);
f32 fn_1_4C9C(M615Segment *segment, M615Triangle *triangle, f32 *segmentTOut,
              f32 *triangleSOut, f32 *triangleTOut);
f32 fn_1_6308(M615Segment *segment, M615Triangle *triangle, f32 *segmentTime, f32 *firstParameter, f32 *secondParameter);
float fn_1_643C(HuVecF *queryPoint, M615Segment *lineSegment);
f32 fn_1_6534(HuVecF *queryPoint, M615Segment *lineSegment);
f32 fn_1_6718(const HuVecF *position, const M615Box *box,
              f32 *closestX, f32 *closestY, f32 *closestZ);
f32 fn_1_6960(const HuVecF *position, const M615Box *box, f32 *closestX, f32 *closestY, f32 *closestZ);
void fn_1_6A94(int primaryAxis, int sideAxisA, int sideAxisB,
               HuVecF *position, const HuVecF *direction, const M615Box *box,
               f32 *offset, f32 *time, f32 *distanceSquared);
void fn_1_78B8(HuVecF *point, const HuVecF *direction,
               const M615Box *triangle, f32 *time, f32 *distanceSquared);
void fn_1_7AA0(s32 firstAxis, s32 secondAxis, s32 thirdAxis,
               HuVecF *point, const HuVecF *direction,
               const M615Box *triangle, f32 *correction,
               f32 *distanceSquared);
void fn_1_7EC8(s32 velocityAxis, s32 firstAxis, s32 secondAxis,
               f32 *positionInput, f32 *velocityInput,
               const M615Box *collision,
               f32 *time, f32 *distanceSquared);
void fn_1_80B8(HuVecF *position, const M615Box *box,
                            f32 *distanceSquared);
f32 fn_1_8230(const M615Segment *line, const M615Box *box, f32 *time,
              f32 *closestX, f32 *closestY, f32 *closestZ);
f64 fn_1_8BCC(const M615Segment *line, const M615Box *box, f32 *time,
                f32 *closestX, f32 *closestY, f32 *closestZ);
f32 fn_1_8D08(const M615Segment *line, const M615Box *box, f32 *time,
                 f32 *closestX, f32 *closestY, f32 *closestZ);
double fn_1_8EB0(const M615Segment *line, const M615Box *bounds, f32 *time,
                 f32 *closestX, f32 *closestY, f32 *closestZ);

void fn_1_A0C(M615Bounds *out, M615Capsule *capsule)
{
    HuVecF points[2];
    points[0] = capsule->segment.start;
    fn_1_60C(&points[1], &capsule->segment.start, &capsule->segment.delta);
    out->min.x = (points[0].x < points[1].x ? points[0].x : points[1].x) - capsule->radius;
    out->max.x = capsule->radius + (points[0].x >= points[1].x ? points[0].x : points[1].x);
    out->min.y = (points[0].y < points[1].y ? points[0].y : points[1].y) - capsule->radius;
    out->max.y = capsule->radius + (points[0].y >= points[1].y ? points[0].y : points[1].y);
    out->min.z = (points[0].z < points[1].z ? points[0].z : points[1].z) - capsule->radius;
    out->max.z = capsule->radius + (points[0].z >= points[1].z ? points[0].z : points[1].z);
}

void fn_1_BC4(M615Bounds *bounds, M615Triangle *triangle)
{
    HuVecF vertices[4];
    int vertexIndex;

    vertices[0] = triangle->start;
    fn_1_60C(&vertices[1], &triangle->start, &triangle->edge1);
    fn_1_60C(&vertices[2], &triangle->start, &triangle->edge2);
    fn_1_60C(&vertices[3], &vertices[1], &triangle->edge2);
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

void fn_1_E38(M615Bounds *out, M615Triangle *triangle)
{
    HuVecF points[3];
    int index;
    points[0] = triangle->start;
    fn_1_60C(&points[1], &triangle->start, &triangle->edge1);
    fn_1_60C(&points[2], &triangle->start, &triangle->edge2);
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

void fn_1_109C(M615Bounds *bounds, const M615Box *transform)
{
    HuVecF scaledZ;
    HuVecF scaledY;
    HuVecF scaledX;
    HuVecF vertices[8];
    f32 minimumX;
    f32 maximumX;
    f32 minimumY;
    f32 maximumY;
    f32 minimumZ;
    f32 maximumZ;
    s32 vertexIndex;

    fn_1_85C(&scaledX, &transform->axis[0], transform->halfExtents[0]);
    fn_1_85C(&scaledY, &transform->axis[1], transform->halfExtents[1]);
    fn_1_85C(&scaledZ, &transform->axis[2], transform->halfExtents[2]);

    fn_1_644(&vertices[0], &transform->center, &scaledX);
    fn_1_644(&vertices[0], &vertices[0], &scaledY);
    fn_1_644(&vertices[0], &vertices[0], &scaledZ);
    fn_1_60C(&vertices[1], &transform->center, &scaledX);
    fn_1_644(&vertices[1], &vertices[1], &scaledY);
    fn_1_644(&vertices[1], &vertices[1], &scaledZ);
    fn_1_60C(&vertices[2], &transform->center, &scaledX);
    fn_1_60C(&vertices[2], &vertices[2], &scaledY);
    fn_1_644(&vertices[2], &vertices[2], &scaledZ);
    fn_1_644(&vertices[3], &transform->center, &scaledX);
    fn_1_60C(&vertices[3], &vertices[3], &scaledY);
    fn_1_644(&vertices[3], &vertices[3], &scaledZ);
    fn_1_644(&vertices[4], &transform->center, &scaledX);
    fn_1_644(&vertices[4], &vertices[4], &scaledY);
    fn_1_60C(&vertices[4], &vertices[4], &scaledZ);
    fn_1_60C(&vertices[5], &transform->center, &scaledX);
    fn_1_644(&vertices[5], &vertices[5], &scaledY);
    fn_1_60C(&vertices[5], &vertices[5], &scaledZ);
    fn_1_60C(&vertices[6], &transform->center, &scaledX);
    fn_1_60C(&vertices[6], &vertices[6], &scaledY);
    fn_1_60C(&vertices[6], &vertices[6], &scaledZ);
    fn_1_644(&vertices[7], &transform->center, &scaledX);
    fn_1_60C(&vertices[7], &vertices[7], &scaledY);
    fn_1_60C(&vertices[7], &vertices[7], &scaledZ);

    bounds->min = vertices[0];
    bounds->max = bounds->min;
    for (vertexIndex = 1; vertexIndex < 8; vertexIndex++) {
        if (bounds->min.x > vertices[vertexIndex].x) {
            minimumX = vertices[vertexIndex].x;
        } else {
            minimumX = bounds->min.x;
        }
        bounds->min.x = minimumX;
        if (bounds->max.x <= vertices[vertexIndex].x) {
            maximumX = vertices[vertexIndex].x;
        } else {
            maximumX = bounds->max.x;
        }
        bounds->max.x = maximumX;
        if (bounds->min.y > vertices[vertexIndex].y) {
            minimumY = vertices[vertexIndex].y;
        } else {
            minimumY = bounds->min.y;
        }
        bounds->min.y = minimumY;
        if (bounds->max.y <= vertices[vertexIndex].y) {
            maximumY = vertices[vertexIndex].y;
        } else {
            maximumY = bounds->max.y;
        }
        bounds->max.y = maximumY;
        if (bounds->min.z > vertices[vertexIndex].z) {
            minimumZ = vertices[vertexIndex].z;
        } else {
            minimumZ = bounds->min.z;
        }
        bounds->min.z = minimumZ;
        if (bounds->max.z <= vertices[vertexIndex].z) {
            maximumZ = vertices[vertexIndex].z;
        } else {
            maximumZ = bounds->max.z;
        }
        bounds->max.z = maximumZ;
    }
}

s32 fn_1_1478(M615Bounds *a, M615Bounds *b)
{
    if ((a->min.z < b->max.z) && (a->max.z > b->min.z) && (a->min.x < b->max.x) && (a->max.x > b->min.x) && (a->min.y < b->max.y) && (a->max.y > b->min.y)) {
        return 1;
    }
    return 0;
}

f32 fn_1_14E8(HuVecF *queryPoint, M615Triangle *triangle,
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

    fn_1_644(&difference, &triangle->start, queryPoint);
    edge1LengthSquared = fn_1_574(&triangle->edge1);
    edge2LengthSquared = fn_1_574(&triangle->edge2);
    edge1Projection = fn_1_5A4(&difference, &triangle->edge1);
    edge2Projection = fn_1_5A4(&difference, &triangle->edge2);
    edge1Weight = -edge1Projection;
    edge2Weight = -edge2Projection;
    distanceSquared = fn_1_574(&difference);
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

f32 fn_1_16F0(HuVecF *queryPoint, M615Triangle *triangle, f32 *firstParameter, f32 *secondParameter)
{
    return sqrtf(fn_1_14E8(queryPoint, triangle, firstParameter, secondParameter));
}

f32 fn_1_181C(M615Segment *firstSegment, M615Segment *secondSegment, f32 *parameterAOut,
              f32 *parameterBOut)
{
    HuVecF startOffset;
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

    fn_1_644(&startOffset, &firstSegment->start, &secondSegment->start);
    lengthSquaredA = fn_1_574(&firstSegment->delta);
    negativeDirectionDot = -fn_1_5A4(&firstSegment->delta, &secondSegment->delta);
    lengthSquaredB = fn_1_574(&secondSegment->delta);
    offsetDotDirectionA = fn_1_5A4(&startOffset, &firstSegment->delta);
    offsetLengthSquared = fn_1_574(&startOffset);
    gramDeterminant =
        fabsf((lengthSquaredA * lengthSquaredB) - (negativeDirectionDot * negativeDirectionDot));
    if (gramDeterminant >= 1.1920929e-7f) {
        negativeOffsetDotDirectionB = -fn_1_5A4(&startOffset, &secondSegment->delta);
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
            negativeOffsetDotDirectionB = -fn_1_5A4(&startOffset, &secondSegment->delta);
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
        negativeOffsetDotDirectionB = -fn_1_5A4(&startOffset, &secondSegment->delta);
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

f32 fn_1_2418(M615Segment *firstSegment, M615Segment *secondSegment, f32 *firstParameter, f32 *secondParameter)
{
    return sqrtf(fn_1_181C(firstSegment, secondSegment, firstParameter, secondParameter));
}

f32 fn_1_2544(M615Segment *segment, M615Triangle *triangle, f32 *segmentTOut, f32 *triangleSOut, f32 *triangleTOut)
{
    M615Segment edge;
    HuVecF offset;
    f32 segmentLengthSquared, negativeSegmentEdge1Dot, negativeSegmentEdge2Dot;
    f32 edge1LengthSquared, edge2LengthSquared, negativeOffsetSegmentDot;
    f32 offsetEdge1Dot, offsetEdge2Dot, edgeProduct;
    f32 negativeSegmentEdge1Minor, negativeSegmentEdge2Minor, determinant;
    f32 inverseDeterminant, offsetParameter, edge1Parameter, edge2Parameter;
    f32 candidateSegmentT, candidateTriangleS, candidateTriangleT;
    f32 bestSegmentT, bestTriangleS, bestTriangleT, bestDistance, distance;
    f32 candidateSegmentTResult, candidateTriangleSResult, candidateTriangleTResult;
    HuVecF queryCenter;
    f32 intermediateValue;
    f32 edge1Minor, segmentEdgeProduct, edge2Minor, determinantMagnitude;
    fn_1_644(& offset, & triangle->start, & segment->start);
    segmentLengthSquared = fn_1_574(& segment->delta);
    negativeSegmentEdge1Dot = - fn_1_5A4(& segment->delta, & triangle->edge1);
    negativeSegmentEdge2Dot = - fn_1_5A4(& segment->delta, & triangle->edge2);
    edge1LengthSquared = fn_1_574(& triangle->edge1);
    edge2LengthSquared = fn_1_574(& triangle->edge2);
    negativeOffsetSegmentDot = - fn_1_5A4(& offset, & segment->delta);
    offsetEdge1Dot = fn_1_5A4(& offset, & triangle->edge1);
    offsetEdge2Dot = fn_1_5A4(& offset, & triangle->edge2);
    edgeProduct = edge1LengthSquared * edge2LengthSquared;
    negativeSegmentEdge1Minor = - negativeSegmentEdge1Dot * edge2LengthSquared;
    negativeSegmentEdge2Minor = - negativeSegmentEdge2Dot * edge1LengthSquared;
    determinant = (negativeSegmentEdge2Dot * negativeSegmentEdge2Minor) + ((segmentLengthSquared * edgeProduct) + (negativeSegmentEdge1Dot * negativeSegmentEdge1Minor));
    determinantMagnitude = fabs(determinant);
    if (determinantMagnitude >= 1.1920929e-7f)
    {
        edge1Minor = (segmentLengthSquared * edge2LengthSquared) - (negativeSegmentEdge2Dot * negativeSegmentEdge2Dot);
        segmentEdgeProduct = negativeSegmentEdge2Dot * negativeSegmentEdge1Dot;
        edge2Minor = (segmentLengthSquared * edge1LengthSquared) - (negativeSegmentEdge1Dot * negativeSegmentEdge1Dot);
        inverseDeterminant = 1.0f / determinant;
        offsetParameter = - negativeOffsetSegmentDot * inverseDeterminant;
        edge1Parameter = - offsetEdge1Dot * inverseDeterminant;
        edge2Parameter = - offsetEdge2Dot * inverseDeterminant;
        candidateSegmentT = (negativeSegmentEdge2Minor * edge2Parameter) + ((edgeProduct * offsetParameter) + (negativeSegmentEdge1Minor * edge1Parameter));
        candidateTriangleS = (segmentEdgeProduct * edge2Parameter) + ((negativeSegmentEdge1Minor * offsetParameter) + (edge1Minor * edge1Parameter));
        candidateTriangleT = (edge2Minor * edge2Parameter) + ((negativeSegmentEdge2Minor * offsetParameter) + (segmentEdgeProduct * edge1Parameter));
        if (candidateSegmentT < 0.0f)
        {
            if (candidateTriangleS < 0.0f)
            {
                if (candidateTriangleT < 0.0f)
                {
                    edge.start = triangle->start;
                    edge.delta = triangle->edge2;
                    bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                    candidateTriangleS = 0.0f;
                    edge.start = triangle->start;
                    edge.delta = triangle->edge1;
                    distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                    candidateTriangleTResult = 0.0f;
                    if (distance < bestDistance)
                    {
                        bestDistance = distance;
                        candidateSegmentT = candidateSegmentTResult;
                        candidateTriangleS = candidateTriangleSResult;
                        candidateTriangleT = candidateTriangleTResult;
                    }
                    distance = fn_1_14E8(&segment->start, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                    candidateSegmentTResult = 0.0f;
                    if (distance < bestDistance)
                    {
                        bestDistance = distance;
                        candidateSegmentT = candidateSegmentTResult;
                        candidateTriangleS = candidateTriangleSResult;
                        candidateTriangleT = candidateTriangleTResult;
                    }
                }
                else
                {
                    if (candidateTriangleT <= 1.0f)
                    {
                        edge.start = triangle->start;
                        edge.delta = triangle->edge2;
                        bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                        candidateTriangleS = 0.0f;
                        distance = fn_1_14E8(&segment->start, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                        candidateSegmentTResult = 0.0f;
                        if (distance < bestDistance)
                        {
                            bestDistance = distance;
                            candidateSegmentT = candidateSegmentTResult;
                            candidateTriangleS = candidateTriangleSResult;
                            candidateTriangleT = candidateTriangleTResult;
                        }
                    }
                    else
                    {
                        edge.start = triangle->start;
                        edge.delta = triangle->edge2;
                        bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                        candidateTriangleS = 0.0f;
                        fn_1_60C(& edge.start, & triangle->start, & triangle->edge2);
                        edge.delta = triangle->edge1;
                        distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                        candidateTriangleTResult = 1.0f;
                        if (distance < bestDistance)
                        {
                            bestDistance = distance;
                            candidateSegmentT = candidateSegmentTResult;
                            candidateTriangleS = candidateTriangleSResult;
                            candidateTriangleT = candidateTriangleTResult;
                        }
                        distance = fn_1_14E8(&segment->start, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                        candidateSegmentTResult = 0.0f;
                        if (distance < bestDistance)
                        {
                            bestDistance = distance;
                            candidateSegmentT = candidateSegmentTResult;
                            candidateTriangleS = candidateTriangleSResult;
                            candidateTriangleT = candidateTriangleTResult;
                        }
                    }
                }
            }
            else
            {
                if (candidateTriangleS <= 1.0f)
                {
                    if (candidateTriangleT < 0.0f)
                    {
                        edge.start = triangle->start;
                        edge.delta = triangle->edge1;
                        bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleS);
                        candidateTriangleT = 0.0f;
                        distance = fn_1_14E8(&segment->start, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                        candidateSegmentTResult = 0.0f;
                        if (distance < bestDistance)
                        {
                            bestDistance = distance;
                            candidateSegmentT = candidateSegmentTResult;
                            candidateTriangleS = candidateTriangleSResult;
                            candidateTriangleT = candidateTriangleTResult;
                        }
                    }
                    else
                    {
                        if (candidateTriangleT <= 1.0f)
                        {
                            bestDistance = fn_1_14E8(&segment->start, triangle, & candidateTriangleS, & candidateTriangleT);
                            candidateSegmentT = 0.0f;
                        }
                        else
                        {
                            fn_1_60C(& edge.start, & triangle->start, & triangle->edge2);
                            edge.delta = triangle->edge1;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleS);
                            candidateTriangleT = 1.0f;
                            distance = fn_1_14E8(&segment->start, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                            candidateSegmentTResult = 0.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                        }
                    }
                }
                else
                {
                    if (candidateTriangleT < 0.0f)
                    {
                        fn_1_60C(& edge.start, & triangle->start, & triangle->edge1);
                        edge.delta = triangle->edge2;
                        bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                        candidateTriangleS = 1.0f;
                        edge.start = triangle->start;
                        edge.delta = triangle->edge1;
                        distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                        candidateTriangleTResult = 0.0f;
                        if (distance < bestDistance)
                        {
                            bestDistance = distance;
                            candidateSegmentT = candidateSegmentTResult;
                            candidateTriangleS = candidateTriangleSResult;
                            candidateTriangleT = candidateTriangleTResult;
                        }
                        distance = fn_1_14E8(&segment->start, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                        candidateSegmentTResult = 0.0f;
                        if (distance < bestDistance)
                        {
                            bestDistance = distance;
                            candidateSegmentT = candidateSegmentTResult;
                            candidateTriangleS = candidateTriangleSResult;
                            candidateTriangleT = candidateTriangleTResult;
                        }
                    }
                    else
                    {
                        if (candidateTriangleT <= 1.0f)
                        {
                            fn_1_60C(& edge.start, & triangle->start, & triangle->edge1);
                            edge.delta = triangle->edge2;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                            candidateTriangleS = 1.0f;
                            distance = fn_1_14E8(&segment->start, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                            candidateSegmentTResult = 0.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                        }
                        else
                        {
                            fn_1_60C(& edge.start, & triangle->start, & triangle->edge1);
                            edge.delta = triangle->edge2;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                            candidateTriangleS = 1.0f;
                            fn_1_60C(& edge.start, & triangle->start, & triangle->edge2);
                            edge.delta = triangle->edge1;
                            distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                            candidateTriangleTResult = 1.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                            distance = fn_1_14E8(&segment->start, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                            candidateSegmentTResult = 0.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (candidateSegmentT <= 1.0f)
            {
                if (candidateTriangleS < 0.0f)
                {
                    if (candidateTriangleT < 0.0f)
                    {
                        edge.start = triangle->start;
                        edge.delta = triangle->edge2;
                        bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                        candidateTriangleS = 0.0f;
                        edge.start = triangle->start;
                        edge.delta = triangle->edge1;
                        distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                        candidateTriangleTResult = 0.0f;
                        if (distance < bestDistance)
                        {
                            bestDistance = distance;
                            candidateSegmentT = candidateSegmentTResult;
                            candidateTriangleS = candidateTriangleSResult;
                            candidateTriangleT = candidateTriangleTResult;
                        }
                    }
                    else
                    {
                        if (candidateTriangleT <= 1.0f)
                        {
                            edge.start = triangle->start;
                            edge.delta = triangle->edge2;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                            candidateTriangleS = 0.0f;
                        }
                        else
                        {
                            edge.start = triangle->start;
                            edge.delta = triangle->edge2;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                            candidateTriangleS = 0.0f;
                            fn_1_60C(& edge.start, & triangle->start, & triangle->edge2);
                            edge.delta = triangle->edge1;
                            distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                            candidateTriangleTResult = 1.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                        }
                    }
                }
                else
                {
                    if (candidateTriangleS <= 1.0f)
                    {
                        if (candidateTriangleT < 0.0f)
                        {
                            edge.start = triangle->start;
                            edge.delta = triangle->edge1;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleS);
                            candidateTriangleT = 0.0f;
                        }
                        else
                        {
                            if (candidateTriangleT <= 1.0f)
                            {
                                bestDistance = (candidateTriangleT * ((2.0f * offsetEdge2Dot) + ((negativeSegmentEdge2Dot * candidateSegmentT) + (edge2LengthSquared * candidateTriangleT)))) + ((candidateSegmentT * ((2.0f * negativeOffsetSegmentDot) + ((negativeSegmentEdge2Dot * candidateTriangleT) + ((segmentLengthSquared * candidateSegmentT) + (negativeSegmentEdge1Dot * candidateTriangleS))))) + (candidateTriangleS * ((2.0f * offsetEdge1Dot) + ((negativeSegmentEdge1Dot * candidateSegmentT) + (edge1LengthSquared * candidateTriangleS))))) + fn_1_574(& offset);
                            }
                            else
                            {
                                fn_1_60C(& edge.start, & triangle->start, & triangle->edge2);
                                edge.delta = triangle->edge1;
                                bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleS);
                                candidateTriangleT = 1.0f;
                            }
                        }
                    }
                    else
                    {
                        if (candidateTriangleT < 0.0f)
                        {
                            fn_1_60C(& edge.start, & triangle->start, & triangle->edge1);
                            edge.delta = triangle->edge2;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                            candidateTriangleS = 1.0f;
                            edge.start = triangle->start;
                            edge.delta = triangle->edge1;
                            distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                            candidateTriangleTResult = 0.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                        }
                        else
                        {
                            if (candidateTriangleT <= 1.0f)
                            {
                                fn_1_60C(& edge.start, & triangle->start, & triangle->edge1);
                                edge.delta = triangle->edge2;
                                bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                                candidateTriangleS = 1.0f;
                            }
                            else
                            {
                                fn_1_60C(& edge.start, & triangle->start, & triangle->edge1);
                                edge.delta = triangle->edge2;
                                bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                                candidateTriangleS = 1.0f;
                                fn_1_60C(& edge.start, & triangle->start, & triangle->edge2);
                                edge.delta = triangle->edge1;
                                distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                                candidateTriangleTResult = 1.0f;
                                if (distance < bestDistance)
                                {
                                    bestDistance = distance;
                                    candidateSegmentT = candidateSegmentTResult;
                                    candidateTriangleS = candidateTriangleSResult;
                                    candidateTriangleT = candidateTriangleTResult;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if (candidateTriangleS < 0.0f)
                {
                    if (candidateTriangleT < 0.0f)
                    {
                        edge.start = triangle->start;
                        edge.delta = triangle->edge2;
                        bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                        candidateTriangleS = 0.0f;
                        edge.start = triangle->start;
                        edge.delta = triangle->edge1;
                        distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                        candidateTriangleTResult = 0.0f;
                        if (distance < bestDistance)
                        {
                            bestDistance = distance;
                            candidateSegmentT = candidateSegmentTResult;
                            candidateTriangleS = candidateTriangleSResult;
                            candidateTriangleT = candidateTriangleTResult;
                        }
                        fn_1_60C(& queryCenter, & segment->start, & segment->delta);
                        distance = fn_1_14E8(&queryCenter, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                        candidateSegmentTResult = 1.0f;
                        if (distance < bestDistance)
                        {
                            bestDistance = distance;
                            candidateSegmentT = candidateSegmentTResult;
                            candidateTriangleS = candidateTriangleSResult;
                            candidateTriangleT = candidateTriangleTResult;
                        }
                    }
                    else
                    {
                        if (candidateTriangleT <= 1.0f)
                        {
                            edge.start = triangle->start;
                            edge.delta = triangle->edge2;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                            candidateTriangleS = 0.0f;
                            distance = fn_1_14E8(&segment->start, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                            fn_1_60C(& queryCenter, & segment->start, & segment->delta);
                            distance = fn_1_14E8(&queryCenter, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                            candidateSegmentTResult = 1.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                        }
                        else
                        {
                            edge.start = triangle->start;
                            edge.delta = triangle->edge2;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                            candidateTriangleS = 0.0f;
                            fn_1_60C(& edge.start, & triangle->start, & triangle->edge2);
                            edge.delta = triangle->edge1;
                            distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                            candidateTriangleTResult = 1.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                            fn_1_60C(& queryCenter, & segment->start, & segment->delta);
                            distance = fn_1_14E8(&queryCenter, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                            candidateSegmentTResult = 1.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                        }
                    }
                }
                else
                {
                    if (candidateTriangleS <= 1.0f)
                    {
                        if (candidateTriangleT < 0.0f)
                        {
                            edge.start = triangle->start;
                            edge.delta = triangle->edge1;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleS);
                            candidateTriangleT = 0.0f;
                            fn_1_60C(& queryCenter, & segment->start, & segment->delta);
                            distance = fn_1_14E8(&queryCenter, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                            candidateSegmentTResult = 1.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                        }
                        else
                        {
                            if (candidateTriangleT <= 1.0f)
                            {
                                fn_1_60C(& queryCenter, & segment->start, & segment->delta);
                                bestDistance = fn_1_14E8(&queryCenter, triangle, & candidateTriangleS, & candidateTriangleT);
                                candidateSegmentT = 1.0f;
                            }
                            else
                            {
                                fn_1_60C(& edge.start, & triangle->start, & triangle->edge2);
                                edge.delta = triangle->edge1;
                                bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleS);
                                candidateTriangleT = 1.0f;
                                fn_1_60C(& queryCenter, & segment->start, & segment->delta);
                                distance = fn_1_14E8(&queryCenter, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                                candidateSegmentTResult = 1.0f;
                                if (distance < bestDistance)
                                {
                                    bestDistance = distance;
                                    candidateSegmentT = candidateSegmentTResult;
                                    candidateTriangleS = candidateTriangleSResult;
                                    candidateTriangleT = candidateTriangleTResult;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (candidateTriangleT < 0.0f)
                        {
                            fn_1_60C(& edge.start, & triangle->start, & triangle->edge1);
                            edge.delta = triangle->edge2;
                            bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                            candidateTriangleS = 1.0f;
                            edge.start = triangle->start;
                            edge.delta = triangle->edge1;
                            distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                            candidateTriangleTResult = 0.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                            fn_1_60C(& queryCenter, & segment->start, & segment->delta);
                            distance = fn_1_14E8(&queryCenter, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                            candidateSegmentTResult = 1.0f;
                            if (distance < bestDistance)
                            {
                                bestDistance = distance;
                                candidateSegmentT = candidateSegmentTResult;
                                candidateTriangleS = candidateTriangleSResult;
                                candidateTriangleT = candidateTriangleTResult;
                            }
                        }
                        else
                        {
                            if (candidateTriangleT <= 1.0f)
                            {
                                fn_1_60C(& edge.start, & triangle->start, & triangle->edge1);
                                edge.delta = triangle->edge2;
                                bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                                candidateTriangleS = 1.0f;
                                fn_1_60C(& queryCenter, & segment->start, & segment->delta);
                                distance = fn_1_14E8(&queryCenter, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                                candidateSegmentTResult = 1.0f;
                                if (distance < bestDistance)
                                {
                                    bestDistance = distance;
                                    candidateSegmentT = candidateSegmentTResult;
                                    candidateTriangleS = candidateTriangleSResult;
                                    candidateTriangleT = candidateTriangleTResult;
                                }
                            }
                            else
                            {
                                fn_1_60C(& edge.start, & triangle->start, & triangle->edge1);
                                edge.delta = triangle->edge2;
                                bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleT);
                                candidateTriangleS = 1.0f;
                                fn_1_60C(& edge.start, & triangle->start, & triangle->edge2);
                                edge.delta = triangle->edge1;
                                distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
                                candidateTriangleTResult = 1.0f;
                                if (distance < bestDistance)
                                {
                                    bestDistance = distance;
                                    candidateSegmentT = candidateSegmentTResult;
                                    candidateTriangleS = candidateTriangleSResult;
                                    candidateTriangleT = candidateTriangleTResult;
                                }
                                fn_1_60C(& queryCenter, & segment->start, & segment->delta);
                                distance = fn_1_14E8(&queryCenter, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
                                candidateSegmentTResult = 1.0f;
                                if (distance < bestDistance)
                                {
                                    bestDistance = distance;
                                    candidateSegmentT = candidateSegmentTResult;
                                    candidateTriangleS = candidateTriangleSResult;
                                    candidateTriangleT = candidateTriangleTResult;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        edge.start = triangle->start;
        edge.delta = triangle->edge1;
        bestDistance = fn_1_181C(segment, & edge, & candidateSegmentT, & candidateTriangleS);
        candidateTriangleT = 0.0f;
        edge.delta = triangle->edge2;
        distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleTResult);
        candidateTriangleSResult = 0.0f;
        if (distance < bestDistance)
        {
            bestDistance = distance;
            candidateSegmentT = candidateSegmentTResult;
            candidateTriangleS = candidateTriangleSResult;
            candidateTriangleT = candidateTriangleTResult;
        }
        fn_1_60C(& edge.start, & triangle->start, & triangle->edge2);
        edge.delta = triangle->edge1;
        distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleSResult);
        candidateTriangleTResult = 1.0f;
        if (distance < bestDistance)
        {
            bestDistance = distance;
            candidateSegmentT = candidateSegmentTResult;
            candidateTriangleS = candidateTriangleSResult;
            candidateTriangleT = candidateTriangleTResult;
        }
        fn_1_60C(& edge.start, & triangle->start, & triangle->edge1);
        edge.delta = triangle->edge2;
        distance = fn_1_181C(segment, & edge, & candidateSegmentTResult, & candidateTriangleTResult);
        candidateTriangleSResult = 1.0f;
        if (distance < bestDistance)
        {
            bestDistance = distance;
            candidateSegmentT = candidateSegmentTResult;
            candidateTriangleS = candidateTriangleSResult;
            candidateTriangleT = candidateTriangleTResult;
        }
        distance = fn_1_14E8(&segment->start, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
        candidateSegmentTResult = 0.0f;
        if (distance < bestDistance)
        {
            bestDistance = distance;
            candidateSegmentT = candidateSegmentTResult;
            candidateTriangleS = candidateTriangleSResult;
            candidateTriangleT = candidateTriangleTResult;
        }
        fn_1_60C(& queryCenter, & segment->start, & segment->delta);
        distance = fn_1_14E8(&queryCenter, triangle, & candidateTriangleSResult, & candidateTriangleTResult);
        candidateSegmentTResult = 1.0f;
        if (distance < bestDistance)
        {
            bestDistance = distance;
            candidateSegmentT = candidateSegmentTResult;
            candidateTriangleS = candidateTriangleSResult;
            candidateTriangleT = candidateTriangleTResult;
        }
    }
    if (segmentTOut)
    {
        * segmentTOut = candidateSegmentT;
    }
    if (triangleSOut)
    {
        * triangleSOut = candidateTriangleS;
    }
    if (triangleTOut != 0)
    {
        * triangleTOut = candidateTriangleT;
    }
    return fabs(bestDistance);
}

f32 fn_1_4198(M615Segment *segment, M615Triangle *triangle, f32 *segmentTime, f32 *firstParameter, f32 *secondParameter)
{
    return sqrtf(fn_1_2544(segment, triangle, segmentTime, firstParameter, secondParameter));
}

f32 fn_1_42CC(HuVecF *queryPoint, M615Triangle *triangle, f32 *barycentricSOut,
              f32 *barycentricTOut)
{
    HuVecF pointOffset;
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

    fn_1_644(&pointOffset, &triangle->start, queryPoint);
    edge1LengthSquared = fn_1_574(&triangle->edge1);
    edgeDot = fn_1_5A4(&triangle->edge1, &triangle->edge2);
    edge2LengthSquared = fn_1_574(&triangle->edge2);
    pointOffsetDotEdge1 = fn_1_5A4(&pointOffset, &triangle->edge1);
    pointOffsetDotEdge2 = fn_1_5A4(&pointOffset, &triangle->edge2);
    offsetLengthSquared = fn_1_574(&pointOffset);
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

f32 fn_1_4B70(HuVecF *queryPoint, M615Triangle *triangle, f32 *firstParameter, f32 *secondParameter)
{
    return sqrtf(fn_1_42CC(queryPoint, triangle, firstParameter, secondParameter));
}

f32 fn_1_4C9C(M615Segment *segment, M615Triangle *triangle, f32 *segmentTOut,
              f32 *triangleSOut, f32 *triangleTOut)
{
    M615Segment edge;
    HuVecF segmentOffset;
    HuVecF segmentEnd;
    HuVecF oppositeEdge;
    f32 segmentLengthSquared, segmentEdge1Dot, segmentEdge2Dot;
    f32 triangleEdge1LengthSquared, edgeDot, triangleEdge2LengthSquared;
    f32 negativeOffsetDotSegment, offsetDotEdge1, offsetDotEdge2;
    f32 bestSegmentT, bestTriangleS, bestTriangleT;
    f32 segmentT, triangleS, triangleT;
    f32 oppositeEdgeLengthSquared, offsetDotOppositeEdge;
    f32 edgeGram, segmentEdge1Minor, segmentEdge2Minor;
    f32 edge1Minor, edge2Minor, mixedMinor, determinant;
    f32 inverseDeterminant, unconstrainedSegmentT, unconstrainedTriangleS, unconstrainedTriangleT;
    f32 distance, bestDistance;
    s32 lineIntersectsTrianglePlane;
    fn_1_644(&segmentOffset, &triangle->start, &segment->start);
    segmentLengthSquared = fn_1_574(&segment->delta);
    segmentEdge1Dot = -fn_1_5A4(&segment->delta, &triangle->edge1);
    segmentEdge2Dot = -fn_1_5A4(&segment->delta, &triangle->edge2);
    triangleEdge1LengthSquared = fn_1_574(&triangle->edge1);
    edgeDot = fn_1_5A4(&triangle->edge1, &triangle->edge2);
    triangleEdge2LengthSquared = fn_1_5A4(&triangle->edge2, &triangle->edge2);
    negativeOffsetDotSegment = -fn_1_5A4(&segmentOffset, &segment->delta);
    offsetDotEdge1 = fn_1_5A4(&segmentOffset, &triangle->edge1);
    offsetDotEdge2 = fn_1_5A4(&segmentOffset, &triangle->edge2);
    fn_1_5D4(&oppositeEdge, &triangle->edge1, &triangle->edge2);
    oppositeEdgeLengthSquared = fn_1_574(&oppositeEdge);
    offsetDotOppositeEdge = fn_1_5A4(&segment->delta, &oppositeEdge);
    lineIntersectsTrianglePlane = (offsetDotOppositeEdge * offsetDotOppositeEdge) >= (1.1920929e-07f * segmentLengthSquared * oppositeEdgeLengthSquared);
    if (lineIntersectsTrianglePlane) {
        edgeGram = (triangleEdge1LengthSquared * triangleEdge2LengthSquared) - (edgeDot * edgeDot);
        segmentEdge1Minor = (segmentEdge2Dot * edgeDot) - (segmentEdge1Dot * triangleEdge2LengthSquared);
        segmentEdge2Minor = (segmentEdge1Dot * edgeDot) - (segmentEdge2Dot * triangleEdge1LengthSquared);
        edge1Minor = (segmentLengthSquared * triangleEdge2LengthSquared) - (segmentEdge2Dot * segmentEdge2Dot);
        edge2Minor = (segmentEdge2Dot * segmentEdge1Dot) - (segmentLengthSquared * edgeDot);
        mixedMinor = (segmentLengthSquared * triangleEdge1LengthSquared) - (segmentEdge1Dot * segmentEdge1Dot);
        determinant = (segmentEdge2Dot * segmentEdge2Minor) + ((segmentLengthSquared * edgeGram) + (segmentEdge1Dot * segmentEdge1Minor));
        inverseDeterminant = (determinant == 0.0f) ? 0.0f : 1.0f / determinant;
        unconstrainedSegmentT = -negativeOffsetDotSegment * inverseDeterminant;
        unconstrainedTriangleS = -offsetDotEdge1 * inverseDeterminant;
        unconstrainedTriangleT = -offsetDotEdge2 * inverseDeterminant;
        bestSegmentT = (segmentEdge2Minor * unconstrainedTriangleT) + ((edgeGram * unconstrainedSegmentT) + (segmentEdge1Minor * unconstrainedTriangleS));
        bestTriangleS = (edge2Minor * unconstrainedTriangleT) + ((segmentEdge1Minor * unconstrainedSegmentT) + (edge1Minor * unconstrainedTriangleS));
        bestTriangleT = (mixedMinor * unconstrainedTriangleT) + ((segmentEdge2Minor * unconstrainedSegmentT) + (edge2Minor * unconstrainedTriangleS));
        if (bestSegmentT < 0.0f) {
            if ((bestTriangleS + bestTriangleT) <= 1.0f) {
                if (bestTriangleS < 0.0f) {
                    if (bestTriangleT < 0.0f) {
                        edge.start = triangle->start;
                        edge.delta = triangle->edge2;
                        bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
                        bestTriangleS = 0.0f;
                        edge.start = triangle->start;
                        edge.delta = triangle->edge1;
                        distance = fn_1_181C(segment, &edge, &segmentT, &triangleS);
                        triangleT = 0.0f;
                        if (distance < bestDistance) {
                            bestDistance = distance;
                            bestSegmentT = segmentT;
                            bestTriangleS = triangleS;
                            bestTriangleT = triangleT;
                        }
                        distance = fn_1_42CC(&segment->start, triangle, &triangleS, &triangleT);
                        segmentT = 0.0f;
                        if (distance < bestDistance) {
                            bestDistance = distance;
                            bestSegmentT = segmentT;
                            bestTriangleS = triangleS;
                            bestTriangleT = triangleT;
                        }
                    } else {
                        edge.start = triangle->start;
                        edge.delta = triangle->edge2;
                        bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
                        bestTriangleS = 0.0f;
                        distance = fn_1_42CC(&segment->start, triangle, &triangleS, &triangleT);
                        segmentT = 0.0f;
                        if (distance < bestDistance) {
                            bestDistance = distance;
                            bestSegmentT = segmentT;
                            bestTriangleS = triangleS;
                            bestTriangleT = triangleT;
                        }
                    }
                } else if (bestTriangleT < 0.0f) {
                    edge.start = triangle->start;
                    edge.delta = triangle->edge1;
                    bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleS);
                    bestTriangleT = 0.0f;
                    distance = fn_1_42CC(&segment->start, triangle, &triangleS, &triangleT);
                    segmentT = 0.0f;
                    if (distance < bestDistance) {
                        bestDistance = distance;
                        bestSegmentT = segmentT;
                        bestTriangleS = triangleS;
                        bestTriangleT = triangleT;
                    }
                } else {
                    bestDistance = fn_1_42CC(&segment->start, triangle, &bestTriangleS, &bestTriangleT);
                    bestSegmentT = 0.0f;
                }
            } else if (bestTriangleS < 0.0f) {
                edge.start = triangle->start;
                edge.delta = triangle->edge2;
                bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
                bestTriangleS = 0.0f;
                fn_1_60C(&edge.start, &triangle->start, &triangle->edge1);
                fn_1_644(&edge.delta, &triangle->edge2, &triangle->edge1);
                distance = fn_1_181C(segment, &edge, &segmentT, &triangleT);
                triangleS = 1.0f - triangleT;
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestSegmentT = segmentT;
                    bestTriangleS = triangleS;
                    bestTriangleT = triangleT;
                }
                distance = fn_1_42CC(&segment->start, triangle, &triangleS, &triangleT);
                segmentT = 0.0f;
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestSegmentT = segmentT;
                    bestTriangleS = triangleS;
                    bestTriangleT = triangleT;
                }
            } else if (bestTriangleT < 0.0f) {
                edge.start = triangle->start;
                edge.delta = triangle->edge1;
                bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleS);
                bestTriangleT = 0.0f;
                fn_1_60C(&edge.start, &triangle->start, &triangle->edge1);
                fn_1_644(&edge.delta, &triangle->edge2, &triangle->edge1);
                distance = fn_1_181C(segment, &edge, &segmentT, &triangleT);
                triangleS = 1.0f - triangleT;
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestSegmentT = segmentT;
                    bestTriangleS = triangleS;
                    bestTriangleT = triangleT;
                }
                distance = fn_1_42CC(&segment->start, triangle, &triangleS, &triangleT);
                segmentT = 0.0f;
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestSegmentT = segmentT;
                    bestTriangleS = triangleS;
                    bestTriangleT = triangleT;
                }
            } else {
                fn_1_60C(&edge.start, &triangle->start, &triangle->edge1);
                fn_1_644(&edge.delta, &triangle->edge2, &triangle->edge1);
                bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
                bestTriangleS = 1.0f - bestTriangleT;
                distance = fn_1_42CC(&segment->start, triangle, &triangleS, &triangleT);
                segmentT = 0.0f;
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestSegmentT = segmentT;
                    bestTriangleS = triangleS;
                    bestTriangleT = triangleT;
                }
            }
        } else if (bestSegmentT <= 1.0f) {
            if ((bestTriangleS + bestTriangleT) <= 1.0f) {
                if (bestTriangleS < 0.0f) {
                    if (bestTriangleT < 0.0f) {
                        edge.start = triangle->start;
                        edge.delta = triangle->edge2;
                        bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
                        bestTriangleS = 0.0f;
                        edge.start = triangle->start;
                        edge.delta = triangle->edge1;
                        distance = fn_1_181C(segment, &edge, &segmentT, &triangleS);
                        triangleT = 0.0f;
                        if (distance < bestDistance) {
                            bestDistance = distance;
                            bestSegmentT = segmentT;
                            bestTriangleS = triangleS;
                            bestTriangleT = triangleT;
                        }
                    } else {
                        edge.start = triangle->start;
                        edge.delta = triangle->edge2;
                        bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
                        bestTriangleS = 0.0f;
                    }
                } else if (bestTriangleT < 0.0f) {
                    edge.start = triangle->start;
                    edge.delta = triangle->edge1;
                    bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleS);
                    bestTriangleT = 0.0f;
                } else {
                    bestDistance = 0.0f;
                }
            } else if (bestTriangleS < 0.0f) {
                edge.start = triangle->start;
                edge.delta = triangle->edge2;
                bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
                bestTriangleS = 0.0f;
                fn_1_60C(&edge.start, &triangle->start, &triangle->edge1);
                fn_1_644(&edge.delta, &triangle->edge2, &triangle->edge1);
                distance = fn_1_181C(segment, &edge, &segmentT, &triangleT);
                triangleS = 1.0f - triangleT;
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestSegmentT = segmentT;
                    bestTriangleS = triangleS;
                    bestTriangleT = triangleT;
                }
            } else if (bestTriangleT < 0.0f) {
                edge.start = triangle->start;
                edge.delta = triangle->edge1;
                bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleS);
                bestTriangleT = 0.0f;
                fn_1_60C(&edge.start, &triangle->start, &triangle->edge1);
                fn_1_644(&edge.delta, &triangle->edge2, &triangle->edge1);
                distance = fn_1_181C(segment, &edge, &segmentT, &triangleT);
                triangleS = 1.0f - triangleT;
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestSegmentT = segmentT;
                    bestTriangleS = triangleS;
                    bestTriangleT = triangleT;
                }
            } else {
                fn_1_60C(&edge.start, &triangle->start, &triangle->edge1);
    fn_1_644(&edge.delta, &triangle->edge2, &triangle->edge1);
    bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
                bestTriangleS = 1.0f - bestTriangleT;
            }
        } else if ((bestTriangleS + bestTriangleT) <= 1.0f) {
            if (bestTriangleS < 0.0f) {
                if (bestTriangleT < 0.0f) {
                    edge.start = triangle->start;
                    edge.delta = triangle->edge2;
    bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
                    bestTriangleS = 0.0f;
                    edge.start = triangle->start;
                    edge.delta = triangle->edge1;
    distance = fn_1_181C(segment, &edge, &segmentT, &triangleS);
                    triangleT = 0.0f;
                    if (distance < bestDistance) {
                        bestDistance = distance;
                        bestSegmentT = segmentT;
                        bestTriangleS = triangleS;
                        bestTriangleT = triangleT;
                    }
    fn_1_60C(&segmentEnd, &segment->start, &segment->delta);
    distance = fn_1_42CC(&segmentEnd, triangle, &triangleS, &triangleT);
                    segmentT = 1.0f;
                    if (distance < bestDistance) {
                        bestDistance = distance;
                        bestSegmentT = segmentT;
                        bestTriangleS = triangleS;
                        bestTriangleT = triangleT;
                    }
                } else {
                    edge.start = triangle->start;
                    edge.delta = triangle->edge2;
    bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
                    bestTriangleS = 0.0f;
    fn_1_60C(&segmentEnd, &segment->start, &segment->delta);
    distance = fn_1_42CC(&segmentEnd, triangle, &triangleS, &triangleT);
                    segmentT = 1.0f;
                    if (distance < bestDistance) {
                        bestDistance = distance;
                        bestSegmentT = segmentT;
                        bestTriangleS = triangleS;
                        bestTriangleT = triangleT;
                    }
                }
            } else if (bestTriangleT < 0.0f) {
                edge.start = triangle->start;
                edge.delta = triangle->edge1;
    bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleS);
                bestTriangleT = 0.0f;
                fn_1_60C(&segmentEnd, &segment->start, &segment->delta);
    distance = fn_1_42CC(&segmentEnd, triangle, &triangleS, &triangleT);
                segmentT = 1.0f;
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestSegmentT = segmentT;
                    bestTriangleS = triangleS;
                    bestTriangleT = triangleT;
                }
            } else {
    fn_1_60C(&segmentEnd, &segment->start, &segment->delta);
    bestDistance = fn_1_42CC(&segmentEnd, triangle, &bestTriangleS, &bestTriangleT);
                bestSegmentT = 1.0f;
            }
        } else if (bestTriangleS < 0.0f) {
            edge.start = triangle->start;
            edge.delta = triangle->edge2;
    bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
            bestTriangleS = 0.0f;
    fn_1_60C(&edge.start, &triangle->start, &triangle->edge1);
    fn_1_644(&edge.delta, &triangle->edge2, &triangle->edge1);
    distance = fn_1_181C(segment, &edge, &segmentT, &triangleT);
            triangleS = 1.0f - triangleT;
            if (distance < bestDistance) {
                bestDistance = distance;
                bestSegmentT = segmentT;
                bestTriangleS = triangleS;
                bestTriangleT = triangleT;
            }
    fn_1_60C(&segmentEnd, &segment->start, &segment->delta);
    distance = fn_1_42CC(&segmentEnd, triangle, &triangleS, &triangleT);
            segmentT = 1.0f;
            if (distance < bestDistance) {
                bestDistance = distance;
                bestSegmentT = segmentT;
                bestTriangleS = triangleS;
                bestTriangleT = triangleT;
            }
        } else if (bestTriangleT < 0.0f) {
            edge.start = triangle->start;
            edge.delta = triangle->edge1;
    bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleS);
            bestTriangleT = 0.0f;
            fn_1_60C(&edge.start, &triangle->start, &triangle->edge1);
    fn_1_644(&edge.delta, &triangle->edge2, &triangle->edge1);
    distance = fn_1_181C(segment, &edge, &segmentT, &triangleT);
            triangleS = 1.0f - triangleT;
            if (distance < bestDistance) {
                bestDistance = distance;
                bestSegmentT = segmentT;
                bestTriangleS = triangleS;
                bestTriangleT = triangleT;
            }
    fn_1_60C(&segmentEnd, &segment->start, &segment->delta);
    distance = fn_1_42CC(&segmentEnd, triangle, &triangleS, &triangleT);
            segmentT = 1.0f;
            if (distance < bestDistance) {
                bestDistance = distance;
                bestSegmentT = segmentT;
                bestTriangleS = triangleS;
                bestTriangleT = triangleT;
            }
        } else {
    fn_1_60C(&edge.start, &triangle->start, &triangle->edge1);
    fn_1_644(&edge.delta, &triangle->edge2, &triangle->edge1);
    bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleT);
            bestTriangleS = 1.0f - bestTriangleT;
            fn_1_60C(&segmentEnd, &segment->start, &segment->delta);
    distance = fn_1_42CC(&segmentEnd, triangle, &triangleS, &triangleT);
            segmentT = 1.0f;
            if (distance < bestDistance) {
                bestDistance = distance;
                bestSegmentT = segmentT;
                bestTriangleS = triangleS;
                bestTriangleT = triangleT;
            }
        }
    } else {
        edge.start = triangle->start;
        edge.delta = triangle->edge1;
    bestDistance = fn_1_181C(segment, &edge, &bestSegmentT, &bestTriangleS);
        bestTriangleT = 0.0f;
        edge.delta = triangle->edge2;
    distance = fn_1_181C(segment, &edge, &segmentT, &triangleT);
        triangleS = 0.0f;
        if (distance < bestDistance) {
            bestDistance = distance;
            bestSegmentT = segmentT;
            bestTriangleS = triangleS;
            bestTriangleT = triangleT;
        }
    fn_1_60C(&edge.start, &triangle->start, &triangle->edge1);
    fn_1_644(&edge.delta, &triangle->edge2, &triangle->edge1);
    distance = fn_1_181C(segment, &edge, &segmentT, &triangleT);
        triangleS = 1.0f - triangleT;
        if (distance < bestDistance) {
            bestDistance = distance;
            bestSegmentT = segmentT;
            bestTriangleS = triangleS;
            bestTriangleT = triangleT;
        }
    distance = fn_1_42CC(&segment->start, triangle, &triangleS, &triangleT);
        segmentT = 0.0f;
        if (distance < bestDistance) {
            bestDistance = distance;
            bestSegmentT = segmentT;
            bestTriangleS = triangleS;
            bestTriangleT = triangleT;
        }
    fn_1_60C(&segmentEnd, &segment->start, &segment->delta);
    distance = fn_1_42CC(&segmentEnd, triangle, &triangleS, &triangleT);
        segmentT = 1.0f;
        if (distance < bestDistance) {
            bestDistance = distance;
            bestSegmentT = segmentT;
            bestTriangleS = triangleS;
            bestTriangleT = triangleT;
        }
    }
    if (segmentTOut) {
        *segmentTOut = bestSegmentT;
    }
    if (triangleSOut) {
        *triangleSOut = bestTriangleS;
    }
    if (triangleTOut) {
        *triangleTOut = bestTriangleT;
    }
    return bestDistance;
}

f32 fn_1_6308(M615Segment *segment, M615Triangle *triangle, f32 *segmentTime, f32 *firstParameter, f32 *secondParameter)
{
    return sqrtf(fn_1_4C9C(segment, triangle, segmentTime, firstParameter, secondParameter));
}

float fn_1_643C(HuVecF *queryPoint, M615Segment *lineSegment)
{
    HuVecF pointOffset, projectedOffset;
    float segmentParameter, segmentLengthSquared;
    fn_1_644(&pointOffset, queryPoint, &lineSegment->start);
    segmentParameter = fn_1_5A4(&pointOffset, &lineSegment->delta);
    if (segmentParameter <= 0.0f) {
        segmentParameter = 0.0f;
    } else {
        segmentLengthSquared = fn_1_574(&lineSegment->delta);
        if (segmentParameter >= segmentLengthSquared) {
            segmentParameter = 1.0f;
            fn_1_644(&pointOffset, &pointOffset, &lineSegment->delta);
        } else {
            segmentParameter /= segmentLengthSquared;
            fn_1_85C(&projectedOffset, &lineSegment->delta, segmentParameter);
            fn_1_644(&pointOffset, &pointOffset, &projectedOffset);
        }
    }
    return fn_1_574(&pointOffset);
}

f32 fn_1_6534(HuVecF *queryPoint, M615Segment *lineSegment)
{
    return sqrtf(fn_1_643C(queryPoint, lineSegment));
}

f32 fn_1_6718(const HuVecF *position, const M615Box *box,
              f32 *closestX, f32 *closestY, f32 *closestZ)
{
    HuVecF difference;
    HuVecF closestPoint;
    f32 distanceSquared = 0.0f;
    f32 differenceFromExtent;

    fn_1_644(&difference, position, &box->center);
    fn_1_444(&closestPoint,
             fn_1_5A4(&difference, &box->axis[0]),
             fn_1_5A4(&difference, &box->axis[1]),
             fn_1_5A4(&difference, &box->axis[2]));

    if (closestPoint.x < -box->halfExtents[0]) {
        differenceFromExtent = closestPoint.x + box->halfExtents[0];
        distanceSquared += differenceFromExtent * differenceFromExtent;
        closestPoint.x = -box->halfExtents[0];
    } else if (closestPoint.x > box->halfExtents[0]) {
        differenceFromExtent = closestPoint.x - box->halfExtents[0];
        distanceSquared += differenceFromExtent * differenceFromExtent;
        closestPoint.x = box->halfExtents[0];
    }

    if (closestPoint.y < -box->halfExtents[1]) {
        differenceFromExtent = closestPoint.y + box->halfExtents[1];
        distanceSquared += differenceFromExtent * differenceFromExtent;
        closestPoint.y = -box->halfExtents[1];
    } else if (closestPoint.y > box->halfExtents[1]) {
        differenceFromExtent = closestPoint.y - box->halfExtents[1];
        distanceSquared += differenceFromExtent * differenceFromExtent;
        closestPoint.y = box->halfExtents[1];
    }

    if (closestPoint.z < -box->halfExtents[2]) {
        differenceFromExtent = closestPoint.z + box->halfExtents[2];
        distanceSquared += differenceFromExtent * differenceFromExtent;
        closestPoint.z = -box->halfExtents[2];
    } else if (closestPoint.z > box->halfExtents[2]) {
        differenceFromExtent = closestPoint.z - box->halfExtents[2];
        distanceSquared += differenceFromExtent * differenceFromExtent;
        closestPoint.z = box->halfExtents[2];
    }

    if (closestX) {
        *closestX = closestPoint.x;
    }
    if (closestY) {
        *closestY = closestPoint.y;
    }
    if (closestZ) {
        *closestZ = closestPoint.z;
    }
    return distanceSquared;
}

f32 fn_1_6960(const HuVecF *position, const M615Box *box, f32 *closestX, f32 *closestY, f32 *closestZ)
{
    return sqrtf(fn_1_6718(position, box, closestX, closestY, closestZ));
}

void fn_1_6A94(int primaryAxis, int sideAxisA, int sideAxisB,
               HuVecF *position, const HuVecF *direction, const M615Box *box,
               f32 *offset, f32 *time, f32 *distanceSquared)
{
    f32 inverseAxisSpeed;
    f32 sideTime;
    f32 closestParameter;
    f32 projectedDistance;
    f32 axisMagnitude;
    f32 edgeProjection;
    f32 projectedPosition[3];
    f32 *projectedComponents = projectedPosition;
    f32 *positionComponents = &position->x;
    const f32 *directionComponents = &direction->x;
    f32 *offsetComponents = offset;
    projectedComponents[sideAxisA] = box->halfExtents[sideAxisA] + positionComponents[sideAxisA];
    projectedComponents[sideAxisB] = box->halfExtents[sideAxisB] + positionComponents[sideAxisB];    if ((directionComponents[primaryAxis] * projectedComponents[sideAxisA]) >= (directionComponents[sideAxisA] * offsetComponents[primaryAxis])) {
        if ((directionComponents[primaryAxis] * projectedComponents[sideAxisB]) >= (directionComponents[sideAxisB] * offsetComponents[primaryAxis])) {
            if (time) {
                positionComponents[primaryAxis] = box->halfExtents[primaryAxis];
                inverseAxisSpeed = 1.0f / directionComponents[primaryAxis];
                positionComponents[sideAxisA] -= inverseAxisSpeed * (directionComponents[sideAxisA] * offsetComponents[primaryAxis]);
                positionComponents[sideAxisB] -= inverseAxisSpeed * (directionComponents[sideAxisB] * offsetComponents[primaryAxis]);
                *time = inverseAxisSpeed * -offsetComponents[primaryAxis];
            }
        } else {

axisMagnitude = (directionComponents[primaryAxis] * directionComponents[primaryAxis]) + (directionComponents[sideAxisB] * directionComponents[sideAxisB]);
edgeProjection = (axisMagnitude * projectedComponents[sideAxisA]) - (directionComponents[sideAxisA] * ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisB] * projectedComponents[sideAxisB])));
            if (edgeProjection <= (2.0f * axisMagnitude * box->halfExtents[sideAxisA])) {
                sideTime = edgeProjection / axisMagnitude;
                axisMagnitude += directionComponents[sideAxisA] * directionComponents[sideAxisA];
                edgeProjection = projectedComponents[sideAxisA] - sideTime;
                projectedDistance = (edgeProjection * directionComponents[sideAxisA]) + (directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisB] * projectedComponents[sideAxisB]);
                closestParameter = -projectedDistance / axisMagnitude;
                *distanceSquared += (projectedDistance * closestParameter) + ((edgeProjection * edgeProjection) + (offsetComponents[primaryAxis] * offsetComponents[primaryAxis]) + (projectedComponents[sideAxisB] * projectedComponents[sideAxisB]));
                if (time) {
                    *time = closestParameter;
                    positionComponents[primaryAxis] = box->halfExtents[primaryAxis];
                    positionComponents[sideAxisA] = sideTime - box->halfExtents[sideAxisA];
                    positionComponents[sideAxisB] = -box->halfExtents[sideAxisB];
                }
            } else {
                axisMagnitude += directionComponents[sideAxisA] * directionComponents[sideAxisA];
                projectedDistance = (directionComponents[sideAxisB] * projectedComponents[sideAxisB]) + ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisA] * offsetComponents[sideAxisA]));
                closestParameter = -projectedDistance / axisMagnitude;
                *distanceSquared += (projectedDistance * closestParameter) + ((projectedComponents[sideAxisB] * projectedComponents[sideAxisB]) + ((offsetComponents[primaryAxis] * offsetComponents[primaryAxis]) + (offsetComponents[sideAxisA] * offsetComponents[sideAxisA])));
                if (time) {
                    *time = closestParameter;
                    positionComponents[primaryAxis] = box->halfExtents[primaryAxis];
                    positionComponents[sideAxisA] = box->halfExtents[sideAxisA];
                    positionComponents[sideAxisB] = -box->halfExtents[sideAxisB];
                }
            }
        }
    } else if ((directionComponents[primaryAxis] * projectedComponents[sideAxisB]) >= (directionComponents[sideAxisB] * offsetComponents[primaryAxis])) {

                axisMagnitude = (directionComponents[primaryAxis] * directionComponents[primaryAxis]) + (directionComponents[sideAxisA] * directionComponents[sideAxisA]);
                edgeProjection = (axisMagnitude * projectedComponents[sideAxisB]) - (directionComponents[sideAxisB] * ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisA] * projectedComponents[sideAxisA])));
        if (edgeProjection <= (2.0f * axisMagnitude * box->halfExtents[sideAxisB])) {
            sideTime = edgeProjection / axisMagnitude;
            axisMagnitude += directionComponents[sideAxisB] * directionComponents[sideAxisB];
            edgeProjection = projectedComponents[sideAxisB] - sideTime;
            projectedDistance = (edgeProjection * directionComponents[sideAxisB]) + ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisA] * projectedComponents[sideAxisA]));
            closestParameter = -projectedDistance / axisMagnitude;
            *distanceSquared += (projectedDistance * closestParameter) + ((edgeProjection * edgeProjection) + ((offsetComponents[primaryAxis] * offsetComponents[primaryAxis]) + (projectedComponents[sideAxisA] * projectedComponents[sideAxisA])));
            if (time) {
                *time = closestParameter;
                positionComponents[primaryAxis] = box->halfExtents[primaryAxis];
                positionComponents[sideAxisA] = -box->halfExtents[sideAxisA];
                positionComponents[sideAxisB] = sideTime - box->halfExtents[sideAxisB];
            }
        } else {
            axisMagnitude += directionComponents[sideAxisB] * directionComponents[sideAxisB];
            projectedDistance = (directionComponents[sideAxisB] * offsetComponents[sideAxisB]) + ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisA] * projectedComponents[sideAxisA]));
            closestParameter = -projectedDistance / axisMagnitude;
            *distanceSquared += (projectedDistance * closestParameter) + ((offsetComponents[sideAxisB] * offsetComponents[sideAxisB]) + ((offsetComponents[primaryAxis] * offsetComponents[primaryAxis]) + (projectedComponents[sideAxisA] * projectedComponents[sideAxisA])));
            if (time) {
                *time = closestParameter;
                positionComponents[primaryAxis] = box->halfExtents[primaryAxis];
                positionComponents[sideAxisA] = -box->halfExtents[sideAxisA];
                positionComponents[sideAxisB] = box->halfExtents[sideAxisB];
            }
        }
    } else {

        axisMagnitude = (directionComponents[primaryAxis] * directionComponents[primaryAxis]) + (directionComponents[sideAxisB] * directionComponents[sideAxisB]);
        edgeProjection = (axisMagnitude * projectedComponents[sideAxisA]) - (directionComponents[sideAxisA] * ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisB] * projectedComponents[sideAxisB])));
        if (edgeProjection >= 0.0f) {
            if (edgeProjection <= (2.0f * axisMagnitude * box->halfExtents[sideAxisA])) {
                sideTime = edgeProjection / axisMagnitude;
                axisMagnitude += directionComponents[sideAxisA] * directionComponents[sideAxisA];
                edgeProjection = projectedComponents[sideAxisA] - sideTime;
                projectedDistance = (edgeProjection * directionComponents[sideAxisA]) + (directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisB] * projectedComponents[sideAxisB]);
                closestParameter = -projectedDistance / axisMagnitude;
                *distanceSquared += (projectedDistance * closestParameter) + ((edgeProjection * edgeProjection) + (offsetComponents[primaryAxis] * offsetComponents[primaryAxis]) + (projectedComponents[sideAxisB] * projectedComponents[sideAxisB]));
                if (time) {
                    *time = closestParameter;
                    positionComponents[primaryAxis] = box->halfExtents[primaryAxis];
                    positionComponents[sideAxisA] = sideTime - box->halfExtents[sideAxisA];
                    positionComponents[sideAxisB] = -box->halfExtents[sideAxisB];
                }
            } else {
                axisMagnitude += directionComponents[sideAxisA] * directionComponents[sideAxisA];
                projectedDistance = (directionComponents[sideAxisB] * projectedComponents[sideAxisB]) + ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisA] * offsetComponents[sideAxisA]));
                closestParameter = -projectedDistance / axisMagnitude;
                *distanceSquared += (projectedDistance * closestParameter) + ((projectedComponents[sideAxisB] * projectedComponents[sideAxisB]) + ((offsetComponents[primaryAxis] * offsetComponents[primaryAxis]) + (offsetComponents[sideAxisA] * offsetComponents[sideAxisA])));
                if (time) {
                    *time = closestParameter;
                    positionComponents[primaryAxis] = box->halfExtents[primaryAxis];
                    positionComponents[sideAxisA] = box->halfExtents[sideAxisA];
                    positionComponents[sideAxisB] = -box->halfExtents[sideAxisB];
                }
            }
            return;
        }

        axisMagnitude = (directionComponents[primaryAxis] * directionComponents[primaryAxis]) + (directionComponents[sideAxisA] * directionComponents[sideAxisA]);
        edgeProjection = (axisMagnitude * projectedComponents[sideAxisB]) - (directionComponents[sideAxisB] * ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisA] * projectedComponents[sideAxisA])));
        if (edgeProjection >= 0.0f) {
            if (edgeProjection <= (2.0f * axisMagnitude * box->halfExtents[sideAxisB])) {
                sideTime = edgeProjection / axisMagnitude;
                axisMagnitude += directionComponents[sideAxisB] * directionComponents[sideAxisB];
                edgeProjection = projectedComponents[sideAxisB] - sideTime;
                projectedDistance = (edgeProjection * directionComponents[sideAxisB]) + ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisA] * projectedComponents[sideAxisA]));
                closestParameter = -projectedDistance / axisMagnitude;
                *distanceSquared += (projectedDistance * closestParameter) + ((edgeProjection * edgeProjection) + ((offsetComponents[primaryAxis] * offsetComponents[primaryAxis]) + (projectedComponents[sideAxisA] * projectedComponents[sideAxisA])));
                if (time) {
                    *time = closestParameter;
                    positionComponents[primaryAxis] = box->halfExtents[primaryAxis];
                    positionComponents[sideAxisA] = -box->halfExtents[sideAxisA];
                    positionComponents[sideAxisB] = sideTime - box->halfExtents[sideAxisB];
                }
            } else {
                axisMagnitude += directionComponents[sideAxisB] * directionComponents[sideAxisB];
                projectedDistance = (directionComponents[sideAxisB] * offsetComponents[sideAxisB]) + ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisA] * projectedComponents[sideAxisA]));
                closestParameter = -projectedDistance / axisMagnitude;
                *distanceSquared += (projectedDistance * closestParameter) + ((offsetComponents[sideAxisB] * offsetComponents[sideAxisB]) + ((offsetComponents[primaryAxis] * offsetComponents[primaryAxis]) + (projectedComponents[sideAxisA] * projectedComponents[sideAxisA])));
                if (time) {
                    *time = closestParameter;
                    positionComponents[primaryAxis] = box->halfExtents[primaryAxis];
                    positionComponents[sideAxisA] = -box->halfExtents[sideAxisA];
                    positionComponents[sideAxisB] = box->halfExtents[sideAxisB];
                }
            }
            return;
        }
        axisMagnitude += directionComponents[sideAxisB] * directionComponents[sideAxisB];
        projectedDistance = (directionComponents[sideAxisB] * projectedComponents[sideAxisB]) + ((directionComponents[primaryAxis] * offsetComponents[primaryAxis]) + (directionComponents[sideAxisA] * projectedComponents[sideAxisA]));
        closestParameter = -projectedDistance / axisMagnitude;
        *distanceSquared += (projectedDistance * closestParameter) + ((projectedComponents[sideAxisB] * projectedComponents[sideAxisB]) + ((offsetComponents[primaryAxis] * offsetComponents[primaryAxis]) + (projectedComponents[sideAxisA] * projectedComponents[sideAxisA])));
        if (time) {
            *time = closestParameter;
            positionComponents[primaryAxis] = box->halfExtents[primaryAxis];
            positionComponents[sideAxisA] = -box->halfExtents[sideAxisA];
            positionComponents[sideAxisB] = -box->halfExtents[sideAxisB];
        }
    }

}

void fn_1_78B8(HuVecF *point, const HuVecF *direction,
               const M615Box *triangle, f32 *time, f32 *distanceSquared)
{
    HuVecF offset;
    f32 firstRight;
    f32 firstLeft;
    f32 secondRight;
    f32 secondLeft;
    f32 thirdRight;
    f32 thirdLeft;

    offset.x = point->x - triangle->halfExtents[0];
    offset.y = point->y - triangle->halfExtents[1];
    offset.z = point->z - triangle->halfExtents[2];
    firstRight = direction->x * offset.y;
    firstLeft = direction->y * offset.x;
    if (firstLeft >= firstRight) {
        secondRight = direction->z * offset.x;
        secondLeft = direction->x * offset.z;
        if (secondRight >= secondLeft) {
            fn_1_6A94(0, 1, 2, point, direction, triangle, &offset.x, time, distanceSquared);
            return;
        }
        fn_1_6A94(2, 0, 1, point, direction, triangle, &offset.x, time, distanceSquared);
        return;
    }
    thirdRight = direction->z * offset.y;
    thirdLeft = direction->y * offset.z;
    if (thirdRight >= thirdLeft) {
        fn_1_6A94(1, 2, 0, point, direction, triangle, &offset.x, time, distanceSquared);
        return;
    }
    fn_1_6A94(2, 0, 1, point, direction, triangle, &offset.x, time, distanceSquared);
}

void fn_1_7AA0(s32 firstAxis, s32 secondAxis, s32 thirdAxis,
               HuVecF *point, const HuVecF *direction,
               const M615Box *triangle, f32 *correction,
               f32 *distanceSquared)
{
    f32 *pointComponents = &point->x;
    const f32 *directionComponents = &direction->x;
    f32 firstDelta;
    f32 secondDelta;
    f32 firstCross;
    f32 secondCross;
    f32 edgeOffset;
    f32 inverseLengthSquared;
    f32 inverseDirection;

    firstDelta = pointComponents[firstAxis] -
        triangle->halfExtents[firstAxis];
    secondDelta = pointComponents[secondAxis] -
        triangle->halfExtents[secondAxis];
    firstCross = firstDelta * directionComponents[secondAxis];
    secondCross = secondDelta * directionComponents[firstAxis];
    if (firstCross >= secondCross) {
        f32 firstSideOffset;

        pointComponents[firstAxis] =
            triangle->halfExtents[firstAxis];
        firstSideOffset =
            triangle->halfExtents[secondAxis] +
            pointComponents[secondAxis];
        edgeOffset = firstCross - firstSideOffset * directionComponents[firstAxis];
        if (edgeOffset >= 0.0f) {
            inverseLengthSquared = 1.0f /
                ((directionComponents[firstAxis] * directionComponents[firstAxis]) +
                 (directionComponents[secondAxis] * directionComponents[secondAxis]));
            *distanceSquared += inverseLengthSquared * (edgeOffset * edgeOffset);
            if (correction) {
                pointComponents[secondAxis] = -triangle->halfExtents[secondAxis];
                *correction = inverseLengthSquared * -
                    ((firstDelta * directionComponents[firstAxis]) +
                     (firstSideOffset * directionComponents[secondAxis]));
            }
        } else if (correction) {
            inverseDirection = 1.0f / directionComponents[firstAxis];
            pointComponents[secondAxis] -= firstCross * inverseDirection;
            *correction = -firstDelta * inverseDirection;
        }
    } else {
        f32 secondSideOffset;

        pointComponents[secondAxis] =
            triangle->halfExtents[secondAxis];
        secondSideOffset =
            triangle->halfExtents[firstAxis] +
            pointComponents[firstAxis];
        edgeOffset = secondCross - secondSideOffset * directionComponents[secondAxis];
        if (edgeOffset >= 0.0f) {
            inverseLengthSquared = 1.0f /
                ((directionComponents[firstAxis] * directionComponents[firstAxis]) +
                 (directionComponents[secondAxis] * directionComponents[secondAxis]));
            *distanceSquared += inverseLengthSquared * (edgeOffset * edgeOffset);
            if (correction) {
                pointComponents[firstAxis] = -triangle->halfExtents[firstAxis];
                *correction = inverseLengthSquared * -
                    ((secondSideOffset * directionComponents[firstAxis]) +
                     (secondDelta * directionComponents[secondAxis]));
            }
        } else if (correction) {
            inverseDirection = 1.0f / directionComponents[secondAxis];
            pointComponents[firstAxis] -= secondCross * inverseDirection;
            *correction = -secondDelta * inverseDirection;
        }
    }

    if (pointComponents[thirdAxis] < -triangle->halfExtents[thirdAxis]) {
        edgeOffset = triangle->halfExtents[thirdAxis] + pointComponents[thirdAxis];
        *distanceSquared += edgeOffset * edgeOffset;
        pointComponents[thirdAxis] = -triangle->halfExtents[thirdAxis];
        return;
    }
    if (pointComponents[thirdAxis] > triangle->halfExtents[thirdAxis]) {
        edgeOffset = pointComponents[thirdAxis] - triangle->halfExtents[thirdAxis];
        *distanceSquared += edgeOffset * edgeOffset;
        pointComponents[thirdAxis] = triangle->halfExtents[thirdAxis];
    }
}

void fn_1_7EC8(s32 velocityAxis, s32 firstAxis, s32 secondAxis,
               f32 *positionInput, f32 *velocityInput,
               const M615Box *collision,
               f32 *time, f32 *distanceSquared)
{
    f32 *position;
    f32 *velocity;
    f32 difference;

    position = positionInput;
    velocity = velocityInput;
    if (time) {
        *time = (collision->halfExtents[velocityAxis] - position[velocityAxis]) /
                velocity[velocityAxis];
    }
    position[velocityAxis] = collision->halfExtents[velocityAxis];

    if (position[firstAxis] < -collision->halfExtents[firstAxis]) {
        difference = collision->halfExtents[firstAxis] + position[firstAxis];
        *distanceSquared += difference * difference;
        position[firstAxis] = -collision->halfExtents[firstAxis];
    } else if (position[firstAxis] > collision->halfExtents[firstAxis]) {
        difference = position[firstAxis] - collision->halfExtents[firstAxis];
        *distanceSquared += difference * difference;
        position[firstAxis] = collision->halfExtents[firstAxis];
    }

    if (position[secondAxis] < -collision->halfExtents[secondAxis]) {
        difference = collision->halfExtents[secondAxis] + position[secondAxis];
        *distanceSquared += difference * difference;
        position[firstAxis] = -collision->halfExtents[secondAxis];
        return;
    }
    if (position[secondAxis] > collision->halfExtents[secondAxis]) {
        difference = position[secondAxis] - collision->halfExtents[secondAxis];
        *distanceSquared += difference * difference;
        position[secondAxis] = collision->halfExtents[secondAxis];
    }
}

void fn_1_80B8(HuVecF *position, const M615Box *box,
                            f32 *distanceSquared)
{
    f32 difference;

    if (position->x < -box->halfExtents[0]) {
        difference = position->x + box->halfExtents[0];
        *distanceSquared += difference * difference;
        position->x = -box->halfExtents[0];
    } else if (position->x > box->halfExtents[0]) {
        difference = position->x - box->halfExtents[0];
        *distanceSquared += difference * difference;
        position->x = box->halfExtents[0];
    }
    if (position->y < -box->halfExtents[1]) {
        difference = position->y + box->halfExtents[1];
        *distanceSquared += difference * difference;
        position->y = -box->halfExtents[1];
    } else if (position->y > box->halfExtents[1]) {
        difference = position->y - box->halfExtents[1];
        *distanceSquared += difference * difference;
        position->y = box->halfExtents[1];
    }
    if (position->z < -box->halfExtents[2]) {
        difference = position->z + box->halfExtents[2];
        *distanceSquared += difference * difference;
        position->z = -box->halfExtents[2];
        return;
    }
    if (position->z > box->halfExtents[2]) {
        difference = position->z - box->halfExtents[2];
        *distanceSquared += difference * difference;
        position->z = box->halfExtents[2];
    }
}

f32 fn_1_8230(const M615Segment *line, const M615Box *box, f32 *time,
              f32 *closestX, f32 *closestY, f32 *closestZ)
{
    HuVecF offset;
    HuVecF closest;
    HuVecF direction;
    f32 * closestComponents;
    f32 * directionComponents;
    s32 reflected[3];
    f32 distanceSquared;
    s32 axisIndex;
    fn_1_644(& offset, & line->start, & box->center);
    closest.x = fn_1_5A4(& offset, & box->axis[0]);
    closest.y = fn_1_5A4(& offset, & box->axis[1]);
    closest.z = fn_1_5A4(& offset, & box->axis[2]);
    direction.x = fn_1_5A4(& line->delta, & box->axis[0]);
    direction.y = fn_1_5A4(& line->delta, & box->axis[1]);
    direction.z = fn_1_5A4(& line->delta, & box->axis[2]);
    directionComponents = &direction.x;
    closestComponents = &closest.x;
    for (axisIndex = 0; axisIndex < 3; axisIndex++)
    {
        if (directionComponents[axisIndex] < 0.0f)
        {
            closestComponents[axisIndex] = - closestComponents[axisIndex];
            directionComponents[axisIndex] = - directionComponents[axisIndex];
            reflected[axisIndex] = 1;
        }
        else
        {
            reflected[axisIndex] = 0;
        }
    }
    distanceSquared = 0.0f;
    if (direction.x > 0.0f)
    {
        if (direction.y > 0.0f)
        {
            if (direction.z > 0.0f)
            {
                fn_1_78B8(& closest, & direction, box, time, & distanceSquared);
            }
            else
            {
                fn_1_7AA0(0, 1, 2, & closest, & direction, box, time, & distanceSquared);
            }
        }
        else
        {
            if (direction.z > 0.0f)
            {
                fn_1_7AA0(0, 2, 1, & closest, & direction, box, time, & distanceSquared);
            }
            else
            {
                fn_1_7EC8(0, 1, 2, &closest.x, &direction.x, box, time, &distanceSquared);
            }
        }
    }
    else
    {
        if (direction.y > 0.0f)
        {
            if (direction.z > 0.0f)
            {
                fn_1_7AA0(1, 2, 0, & closest, & direction, box, time, & distanceSquared);
            }
            else
            {
                fn_1_7EC8(1, 0, 2, &closest.x, &direction.x, box, time, &distanceSquared);
            }
        }
        else
        {
            if (direction.z > 0.0f)
            {
                fn_1_7EC8(2, 0, 1, &closest.x, &direction.x, box, time, &distanceSquared);
            }
            else
            {
                fn_1_80B8(&closest, box, &distanceSquared);
                if (time) {
                    *time = 0.0f;
                }
            }
        }
    }
    for (axisIndex = 0; axisIndex < 3; ++axisIndex)
    {
        if (reflected[axisIndex] != 0)
        {
        closestComponents[axisIndex] = - closestComponents[axisIndex];
        }
    }
    if (closestX)
    {
        * closestX = closest.x;
    }
    if (closestY)
    {
        * closestY = closest.y;
    }
    if (closestZ)
    {
        * closestZ = closest.z;
    }
    return distanceSquared;
}

f64 fn_1_8BCC(const M615Segment *line, const M615Box *box, f32 *time,
                f32 *closestX, f32 *closestY, f32 *closestZ)
{
    return sqrtf(fn_1_8230(line, box, time, closestX, closestY, closestZ));
}

f32 fn_1_8D08(const M615Segment *line, const M615Box *box, f32 *time,
                 f32 *closestX, f32 *closestY, f32 *closestZ)
{
    M615Segment segment;
    HuVecF segmentEnd;
    f32 timeResult;
    f32 closestXResult;
    f32 closestYResult;
    f32 closestZResult;
    f32 distance;

    segment.start = line->start;
    segment.delta = line->delta;
    distance = fn_1_8230(&segment, box,
                         &timeResult,
                         &closestXResult, &closestYResult, &closestZResult);
    if (timeResult >= 0.0f) {
        if (timeResult <= 1.0f) {
            if (time) {
                *time = timeResult;
            }
            if (closestX) {
                *closestX = closestXResult;
            }
            if (closestY) {
                *closestY = closestYResult;
            }
            if (closestZ) {
                *closestZ = closestZResult;
            }
            return distance;
        }
        fn_1_60C(&segmentEnd, &line->start,
                 &line->delta);
        distance = fn_1_6718(&segmentEnd, box, closestX, closestY, closestZ);
        if (time) {
            *time = 1.0f;
        }
        return distance;
    }
    distance = fn_1_6718(&line->start, box, closestX, closestY, closestZ);
    if (time) {
        *time = 0.0f;
    }
    return distance;
}

double fn_1_8EB0(const M615Segment *line, const M615Box *bounds, f32 *time,
                 f32 *closestX, f32 *closestY, f32 *closestZ)
{
    return sqrtf(fn_1_8D08(line, bounds, time, closestX,
                                  closestY, closestZ));
}
