#include "REL/m656/m656.h"

void fn_1_22FC(M656Sphere *out, f32 x, f32 y, f32 z, f32 radius)
{
    out->center.x = x;
    out->center.y = y;
    out->center.z = z;
    out->radius = radius;
}

void fn_1_2310(M656Bounds *out, M656Capsule *capsule)
{
    HuVecF points[2];
    points[0] = capsule->segment.start;
    fn_1_4A8(&points[1], &capsule->segment.start, &capsule->segment.delta);
    out->min.x = (points[0].x < points[1].x ? points[0].x : points[1].x) - capsule->radius;
    out->max.x = capsule->radius + (points[0].x >= points[1].x ? points[0].x : points[1].x);
    out->min.y = (points[0].y < points[1].y ? points[0].y : points[1].y) - capsule->radius;
    out->max.y = capsule->radius + (points[0].y >= points[1].y ? points[0].y : points[1].y);
    out->min.z = (points[0].z < points[1].z ? points[0].z : points[1].z) - capsule->radius;
    out->max.z = capsule->radius + (points[0].z >= points[1].z ? points[0].z : points[1].z);
}

void fn_1_24C8(M656Bounds *out, M656Sphere *sphere)
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

void fn_1_255C(M656Bounds *out, M656Triangle *triangle)
{
    HuVecF points[3];
    int index;
    points[0] = triangle->start;
    fn_1_4A8(&points[1], &triangle->start, &triangle->edge1);
    fn_1_4A8(&points[2], &triangle->start, &triangle->edge2);
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

s32 fn_1_27C0(M656Bounds *a, M656Bounds *b)
{
    if ((a->min.z < b->max.z) && (a->max.z > b->min.z) && (a->min.x < b->max.x) && (a->max.x > b->min.x) && (a->min.y < b->max.y) && (a->max.y > b->min.y)) {
        return 1;
    }
    return 0;
}

/* The unused retail entry takes single-word arguments here, then reads their
 * stack homes as vectors. The following words are not initialized by this
 * function. Preserve that observed behavior; the original signature is unknown
 * and this entry is not portable-safe for callers. */
s32 fn_1_2830(M656Sphere *a, s32 wordA, M656Sphere *b, s32 wordB,
    float maxTime, float *time, HuVecF *out)
{
    HuVecF relative, delta, point;
    float speedSquared, distanceSquared, radiusSum, radiusSquared;
    float dot, c, discriminant;
    fn_1_4E0(&relative, (HuVecF *)&wordB, (HuVecF *)&wordA);
    speedSquared = fn_1_410(&relative);
    fn_1_4E0(&delta, &b->center, &a->center);
    distanceSquared = fn_1_410(&delta);
    radiusSum = a->radius + b->radius;
    radiusSquared = radiusSum * radiusSum;
    if (speedSquared > 0.0f) {
        dot = fn_1_440(&delta, &relative);
        if (dot <= 0.0f) {
            if (-maxTime * speedSquared <= dot ||
                distanceSquared + maxTime * (2.0f * dot + maxTime * speedSquared) <= radiusSquared) {
                c = distanceSquared - radiusSquared;
                discriminant = dot * dot - speedSquared * c;
                if (discriminant >= 0.0f) {
                    if (c <= 0.0f) {
                        *time = 0.0f;
                        fn_1_4A8(out, &a->center, &b->center);
                        fn_1_6F8(out, out, 0.5f);
                    } else {
                        *time = -(dot + sqrtf(discriminant)) / speedSquared;
                        if (*time < 0.0f) *time = 0.0f;
                        else if (*time > maxTime) *time = maxTime;
                        fn_1_6F8(&point, &relative, *time);
                        /* Retail computes this local point without copying it
                         * to out on this branch. Do not silently repair it. */
                        fn_1_4A8(&point, &point, &delta);
                    }
                    return 1;
                }
            }
            return 0;
        }
    }
    if (distanceSquared <= radiusSquared) {
        *time = 0.0f;
        fn_1_4A8(out, &a->center, &b->center);
        fn_1_6F8(out, out, 0.5f);
        return 1;
    }
    return 0;
}

float fn_1_2BE8(M656Sphere *a, M656Sphere *b)
{
    HuVecF delta;
    float distance;
    fn_1_4E0(&delta, &a->center, &b->center);
    distance = fn_1_410(&delta);
    distance -= a->radius * a->radius + b->radius * b->radius;
    if (distance < 0.0f) distance = 0.0f;
    return distance;
}

float fn_1_2C8C(M656Sphere *a, M656Sphere *b)
{
    return sqrtf(fn_1_2BE8(a, b));
}

f32 fn_1_2E18(Point3d *arg0, M656Triangle *arg1, f32 *arg2, f32 *arg3)
{
    Point3d offset;
    f32 determinant;
    f32 t;
    f32 s;
    f32 b0;
    f32 b1;
    f32 c;
    f32 distanceSquared;
    f32 a11;
    f32 a00;
    f32 a01;
    f32 numerator;
    f32 denominator;
    f32 edge1;
    f32 edge0;

    fn_1_4E0(&offset, &arg1->start, arg0);
    a00 = fn_1_410(&arg1->edge1);
    a01 = fn_1_440(&arg1->edge1, &arg1->edge2);
    a11 = fn_1_410(&arg1->edge2);
    b0 = fn_1_440(&offset, &arg1->edge1);
    b1 = fn_1_440(&offset, &arg1->edge2);
    c = fn_1_410(&offset);
    determinant = fabsf((a00 * a11) - (a01 * a01));
    s = (a01 * b1) - (a11 * b0);
    t = (a01 * b0) - (a00 * b1);
    if ((s + t) <= determinant) {
        if (s < 0.0f) {
            if (t < 0.0f) {
                if (b0 < 0.0f) {
                    t = 0.0f;
                    if (-b0 >= a00) {
                        s = 1.0f;
                        distanceSquared = c + (a00 + (2.0f * b0));
                    } else {
                        s = -b0 / a00;
                        distanceSquared = c + (b0 * s);
                    }
                } else {
                    s = 0.0f;
                    if (b1 >= 0.0f) {
                        t = 0.0f;
                        distanceSquared = c;
                    } else if (-b1 >= a11) {
                        t = 1.0f;
                        distanceSquared = c + (a11 + (2.0f * b1));
                    } else {
                        t = -b1 / a11;
                        distanceSquared = c + (b1 * t);
                    }
                }
            } else {
                s = 0.0f;
                if (b1 >= 0.0f) {
                    t = 0.0f;
                    distanceSquared = c;
                } else if (-b1 >= a11) {
                    t = 1.0f;
                    distanceSquared = c + (a11 + (2.0f * b1));
                } else {
                    t = -b1 / a11;
                    distanceSquared = c + (b1 * t);
                }
            }
        } else if (t < 0.0f) {
            t = 0.0f;
            if (b0 >= 0.0f) {
                s = 0.0f;
                distanceSquared = c;
            } else if (-b0 >= a00) {
                s = 1.0f;
                distanceSquared = c + (a00 + (2.0f * b0));
            } else {
                s = -b0 / a00;
                distanceSquared = c + (b0 * s);
            }
        } else {
            f32 inverseDeterminant = 1.0f / determinant;
            s = s * inverseDeterminant;
            t = t * inverseDeterminant;
            distanceSquared = c + ((s * ((2.0f * b0) + ((a00 * s) + (a01 * t)))) + (t * ((2.0f * b1) + ((a01 * s) + (a11 * t)))));
        }
    } else if (s < 0.0f) {
        edge0 = a01 + b0;
        edge1 = a11 + b1;
        if (edge1 > edge0) {
            numerator = edge1 - edge0;
            denominator = a11 + (a00 - (2.0f * a01));
            if (numerator >= denominator) {
                s = 1.0f;
                t = 0.0f;
                distanceSquared = c + (a00 + (2.0f * b0));
            } else {
                s = numerator / denominator;
                t = 1.0f - s;
                distanceSquared = c + ((s * ((2.0f * b0) + ((a00 * s) + (a01 * t)))) + (t * ((2.0f * b1) + ((a01 * s) + (a11 * t)))));
            }
        } else {
            s = 0.0f;
            if (edge1 <= 0.0f) {
                t = 1.0f;
                distanceSquared = c + (a11 + (2.0f * b1));
            } else if (b1 >= 0.0f) {
                t = 0.0f;
                distanceSquared = c;
            } else {
                t = -b1 / a11;
                distanceSquared = c + (b1 * t);
            }
        }
    } else if (t < 0.0f) {
        edge0 = a01 + b1;
        edge1 = a00 + b0;
        if (edge1 > edge0) {
            numerator = edge1 - edge0;
            denominator = a11 + (a00 - (2.0f * a01));
            if (numerator >= denominator) {
                t = 1.0f;
                s = 0.0f;
                distanceSquared = c + (a11 + (2.0f * b1));
            } else {
                t = numerator / denominator;
                s = 1.0f - t;
                distanceSquared = c + ((s * ((2.0f * b0) + ((a00 * s) + (a01 * t)))) + (t * ((2.0f * b1) + ((a01 * s) + (a11 * t)))));
            }
        } else {
            t = 0.0f;
            if (edge1 <= 0.0f) {
                s = 1.0f;
                distanceSquared = c + (a00 + (2.0f * b0));
            } else if (b0 >= 0.0f) {
                s = 0.0f;
                distanceSquared = c;
            } else {
                s = -b0 / a00;
                distanceSquared = c + (b0 * s);
            }
        }
    } else {
        numerator = ((a11 + b1) - a01) - b0;
        if (numerator <= 0.0f) {
            s = 0.0f;
            t = 1.0f;
            distanceSquared = c + (a11 + (2.0f * b1));
        } else {
            denominator = a11 + (a00 - (2.0f * a01));
            if (numerator >= denominator) {
                s = 1.0f;
                t = 0.0f;
                distanceSquared = c + (a00 + (2.0f * b0));
            } else {
                s = numerator / denominator;
                t = 1.0f - s;
                distanceSquared = c + ((s * ((2.0f * b0) + ((a00 * s) + (a01 * t)))) + (t * ((2.0f * b1) + ((a01 * s) + (a11 * t)))));
            }
        }
    }
    if (arg2) {
        *arg2 = s;
    }
    if (arg3) {
        *arg3 = t;
    }
    return fabsf(distanceSquared);
}

float fn_1_36BC(HuVecF *a, M656Triangle *b, float *s, float *t)
{
    return sqrtf(fn_1_2E18(a, b, s, t));
}

f32 fn_1_37E8(M656Segment *arg0, M656Segment *arg1, f32 *arg2, f32 *arg3)
{
    Point3d offset;
    f32 t;
    f32 b0;
    f32 s;
    f32 c;
    f32 distanceSquared;
    f32 edge;
    f32 a00;
    f32 b1;
    f32 a11;
    f32 a01;
    f32 inverseDeterminant;
    f32 determinant;

    fn_1_4E0(&offset, &arg0->start, &arg1->start);
    a00 = fn_1_410(&arg0->delta);
    a01 = -fn_1_440(&arg0->delta, &arg1->delta);
    a11 = fn_1_410(&arg1->delta);
    b0 = fn_1_440(&offset, &arg0->delta);
    c = fn_1_410(&offset);
    determinant = fabsf((a00 * a11) - (a01 * a01));
    if (determinant >= 1.1920929e-7f) {
        b1 = -fn_1_440(&offset, &arg1->delta);
        s = (a01 * b1) - (a11 * b0);
        t = (a01 * b0) - (a00 * b1);
        if (s >= 0.0f) {
            if (s <= determinant) {
                if (t >= 0.0f) {
                    if (t <= determinant) {
                        inverseDeterminant = 1.0f / determinant;
                        s = s * inverseDeterminant;
                        t = t * inverseDeterminant;
                        distanceSquared = c + ((s * ((2.0f * b0) + ((a00 * s) + (a01 * t)))) + (t * ((2.0f * b1) + ((a01 * s) + (a11 * t)))));
                    } else {
                        t = 1.0f;
                        edge = a01 + b0;
                        if (edge >= 0.0f) {
                            s = 0.0f;
                            distanceSquared = c + (a11 + (2.0f * b1));
                        } else if (-edge >= a00) {
                            s = 1.0f;
                            distanceSquared = c + (a00 + a11) + (2.0f * (b1 + edge));
                        } else {
                            s = -edge / a00;
                            distanceSquared = c + ((2.0f * b1) + (a11 + (edge * s)));
                        }
                    }
                } else {
                    t = 0.0f;
                    if (b0 >= 0.0f) {
                        s = 0.0f;
                        distanceSquared = c;
                    } else if (-b0 >= a00) {
                        s = 1.0f;
                        distanceSquared = c + (a00 + (2.0f * b0));
                    } else {
                        s = -b0 / a00;
                        distanceSquared = c + (b0 * s);
                    }
                }
            } else if (t >= 0.0f) {
                if (t <= determinant) {
                    s = 1.0f;
                    edge = a01 + b1;
                    if (edge >= 0.0f) {
                        t = 0.0f;
                        distanceSquared = c + (a00 + (2.0f * b0));
                    } else if (-edge >= a11) {
                        t = 1.0f;
                        distanceSquared = c + (a00 + a11) + (2.0f * (b0 + edge));
                    } else {
                        t = -edge / a11;
                        distanceSquared = c + ((2.0f * b0) + (a00 + (edge * t)));
                    }
                } else {
                    edge = a01 + b0;
                    if (-edge <= a00) {
                        t = 1.0f;
                        if (edge >= 0.0f) {
                            s = 0.0f;
                            distanceSquared = c + (a11 + (2.0f * b1));
                        } else {
                            s = -edge / a00;
                            distanceSquared = c + ((2.0f * b1) + (a11 + (edge * s)));
                        }
                    } else {
                        s = 1.0f;
                        edge = a01 + b1;
                        if (edge >= 0.0f) {
                            t = 0.0f;
                            distanceSquared = c + (a00 + (2.0f * b0));
                        } else if (-edge >= a11) {
                            t = 1.0f;
                            distanceSquared = c + (a00 + a11) + (2.0f * (b0 + edge));
                        } else {
                            t = -edge / a11;
                            distanceSquared = c + ((2.0f * b0) + (a00 + (edge * t)));
                        }
                    }
                }
            } else if (-b0 < a00) {
                t = 0.0f;
                if (b0 >= 0.0f) {
                    s = 0.0f;
                    distanceSquared = c;
                } else {
                    s = -b0 / a00;
                    distanceSquared = c + (b0 * s);
                }
            } else {
                s = 1.0f;
                edge = a01 + b1;
                if (edge >= 0.0f) {
                    t = 0.0f;
                    distanceSquared = c + (a00 + (2.0f * b0));
                } else if (-edge >= a11) {
                    t = 1.0f;
                    distanceSquared = c + (a00 + a11) + (2.0f * (b0 + edge));
                } else {
                    t = -edge / a11;
                    distanceSquared = c + ((2.0f * b0) + (a00 + (edge * t)));
                }
            }
        } else if (t >= 0.0f) {
            if (t <= determinant) {
                s = 0.0f;
                if (b1 >= 0.0f) {
                    t = 0.0f;
                    distanceSquared = c;
                } else if (-b1 >= a11) {
                    t = 1.0f;
                    distanceSquared = c + (a11 + (2.0f * b1));
                } else {
                    t = -b1 / a11;
                    distanceSquared = c + (b1 * t);
                }
            } else {
                edge = a01 + b0;
                if (edge < 0.0f) {
                    t = 1.0f;
                    if (-edge >= a00) {
                        s = 1.0f;
                        distanceSquared = c + (a00 + a11) + (2.0f * (b1 + edge));
                    } else {
                        s = -edge / a00;
                        distanceSquared = c + ((2.0f * b1) + (a11 + (edge * s)));
                    }
                } else {
                    s = 0.0f;
                    if (b1 >= 0.0f) {
                        t = 0.0f;
                        distanceSquared = c;
                    } else if (-b1 >= a11) {
                        t = 1.0f;
                        distanceSquared = c + (a11 + (2.0f * b1));
                    } else {
                        t = -b1 / a11;
                        distanceSquared = c + (b1 * t);
                    }
                }
            }
        } else if (b0 < 0.0f) {
            t = 0.0f;
            if (-b0 >= a00) {
                s = 1.0f;
                distanceSquared = c + (a00 + (2.0f * b0));
            } else {
                s = -b0 / a00;
                distanceSquared = c + (b0 * s);
            }
        } else {
            s = 0.0f;
            if (b1 >= 0.0f) {
                t = 0.0f;
                distanceSquared = c;
            } else if (-b1 >= a11) {
                t = 1.0f;
                distanceSquared = c + (a11 + (2.0f * b1));
            } else {
                t = -b1 / a11;
                distanceSquared = c + (b1 * t);
            }
        }
    } else if (a01 > 0.0f) {
        if (b0 >= 0.0f) {
            s = 0.0f;
            t = 0.0f;
            distanceSquared = c;
        } else if (-b0 <= a00) {
            s = -b0 / a00;
            t = 0.0f;
            distanceSquared = c + (b0 * s);
        } else {
            b1 = -fn_1_440(&offset, &arg1->delta);
            s = 1.0f;
            edge = a00 + b0;
            if (-edge >= a01) {
                t = 1.0f;
                distanceSquared = c + (a00 + a11) + (2.0f * (b1 + (a01 + b0)));
            } else {
                t = -edge / a01;
                distanceSquared = c + (a00 + (2.0f * b0)) + (t * ((a11 * t) + (2.0f * (a01 + b1))));
            }
        }
    } else if (-b0 >= a00) {
        s = 1.0f;
        t = 0.0f;
        distanceSquared = c + (a00 + (2.0f * b0));
    } else if (b0 <= 0.0f) {
        s = -b0 / a00;
        t = 0.0f;
        distanceSquared = c + (b0 * s);
    } else {
        b1 = -fn_1_440(&offset, &arg1->delta);
        s = 0.0f;
        if (b0 >= -a01) {
            t = 1.0f;
            distanceSquared = c + (a11 + (2.0f * b1));
        } else {
            t = -b0 / a01;
            distanceSquared = c + (t * ((2.0f * b1) + (a11 * t)));
        }
    }
    if (arg2) {
        *arg2 = s;
    }
    if (arg3) {
        *arg3 = t;
    }
    return fabsf(distanceSquared);
}

float fn_1_43E4(M656Segment *a, M656Segment *b, float *s, float *t)
{
    return sqrtf(fn_1_37E8(a, b, s, t));
}

float fn_1_4510(HuVecF *point, M656Segment *segment)
{
    HuVecF delta, projection;
    float t, length;
    fn_1_4E0(&delta, point, &segment->start);
    t = fn_1_440(&delta, &segment->delta);
    if (t <= 0.0f) {
        t = 0.0f;
    } else {
        length = fn_1_410(&segment->delta);
        if (t >= length) {
            t = 1.0f;
            fn_1_4E0(&delta, &delta, &segment->delta);
        } else {
            t /= length;
            fn_1_6F8(&projection, &segment->delta, t);
            fn_1_4E0(&delta, &delta, &projection);
        }
    }
    return fn_1_410(&delta);
}

float fn_1_4608(HuVecF *point, M656Segment *segment)
{
    return sqrtf(fn_1_4510(point, segment));
}

float fn_1_47EC(M656Sphere *sphere, M656Segment *segment)
{
    float distance;
    distance = fn_1_4510(&sphere->center, segment);
    distance -= sphere->radius * sphere->radius;
    if (distance < 0.0f) {
        distance = 0.0f;
    }
    return distance;
}

double fn_1_4948(M656Sphere *sphere, M656Segment *segment)
{
    return sqrtf(fn_1_47EC(sphere, segment));
}

float fn_1_4B8C(M656Sphere *sphere, M656Capsule *capsule)
{
    float distance;
    distance = fn_1_4510(&sphere->center, &capsule->segment);
    distance -= sphere->radius * sphere->radius + capsule->radius * capsule->radius;
    if (distance < 0.0f) {
        distance = 0.0f;
    }
    return distance;
}

double fn_1_4CF8(M656Sphere *sphere, M656Capsule *capsule)
{
    return sqrtf(fn_1_4B8C(sphere, capsule));
}
