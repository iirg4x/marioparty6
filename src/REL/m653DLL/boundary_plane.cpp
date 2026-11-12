#include <vector>
#include "REL/m653DLL/boundary_plane.h"

f32 fn_1_8BC4(s32 index)
{
    return -658.2395f + (1316.479f * (f32)index) / 10.0f;
}

f32 fn_1_8C24(s32 index)
{
    return -415.0903f + (1316.4789f * (f32)index) / 10.0f;
}

void fn_1_8C84(Plane *plane, f32 x, f32 z)
{
    Vec position = {0.0f, 0.0f, 0.0f};
    position.x = x;
    position.z = z;
    Vec normalCopy = plane->normal;
    plane->setFromNormalAndPoint(&normalCopy, &position);
}

void fn_1_8D0C(s32 index)
{
    lbl_1_bss_70[0].normal.x = -1.0f;
    lbl_1_bss_70[0].normal.y = 0.0f;
    lbl_1_bss_70[0].normal.z = 0.0f;
    lbl_1_bss_70[1].normal.x = 1.0f;
    lbl_1_bss_70[1].normal.y = 0.0f;
    lbl_1_bss_70[1].normal.z = 0.0f;
    fn_1_8C84(&lbl_1_bss_70[1], fn_1_8BC4(index), 0.0f);
    fn_1_8C84(&lbl_1_bss_70[0], fn_1_8BC4(0xA - index), 0.0f);
}

void fn_1_8F8C(s32 index)
{
    lbl_1_bss_70[2].normal.x = 0.0f;
    lbl_1_bss_70[2].normal.y = 0.0f;
    lbl_1_bss_70[2].normal.z = 1.0f;
    lbl_1_bss_70[3].normal.x = 0.0f;
    lbl_1_bss_70[3].normal.y = 0.0f;
    lbl_1_bss_70[3].normal.z = -1.0f;
    fn_1_8C84(&lbl_1_bss_70[2], 0.0f, fn_1_8C24(index));
    fn_1_8C84(&lbl_1_bss_70[3], 0.0f, fn_1_8C24(0xA - index));
}

void fn_1_9210(void)
{
    fn_1_8D0C(0);
    fn_1_8F8C(0);
}

static inline f32 PlaneDot(Plane *plane, Vec *point)
{
    return PSVECDotProduct(&plane->normal, point);
}

static inline f32 PlaneDistance(Plane *plane, Vec *point)
{
    return plane->distance + PlaneDot(plane, point);
}

f32 fn_1_9568(f32 x, f32 z)
{
    Vec point = {0.0f,0.0f,0.0f};
    s32 index;

    point.x = x;
    point.z = z;
    for (index = 0; index < 4; index++) {
        if (PlaneDistance(&lbl_1_bss_70[index], &point) < 0.0f) {
            return -1000.0f;
        }
    }
    return 99.9593f;
}

void fn_1_965C() {}

void fn_1_9660(s32 mode)
{
    switch (mode) {
    case 1: fn_1_8F8C(1); break;
    case 2: fn_1_8D0C(1); break;
    case 3: fn_1_8F8C(2); break;
    case 4: fn_1_8D0C(2); break;
    }
}

static const Vec originPoint = {0.0f, 0.0f, 0.0f};

f32 fn_1_9D30(f32 x, f32 z, Vec *nearestNormal)
{
    Vec point = originPoint;
    f32 bestDistance;
    f32 planeDistance;
    s32 planeIndex;

    point.x = x;
    point.z = z;
    *nearestNormal = lbl_1_bss_70[0].normal;
    bestDistance = PlaneDistance(&lbl_1_bss_70[0], &point);
    planeIndex = 1;
    while (planeIndex < 4) {
        planeDistance = PlaneDistance(&lbl_1_bss_70[planeIndex], &point);
        if (bestDistance > planeDistance) {
            bestDistance = planeDistance;
            *nearestNormal = lbl_1_bss_70[planeIndex].normal;
        }
        planeIndex += 1;
    }
    return bestDistance;
}

static inline void addSelectedNormal(const Vec &normal, Vec *result)
{
    PSVECAdd(&normal, result, result);
}

s32 fn_1_9EF4(f32 x, f32 z, Vec *result, f32 threshold)
{
    Vec point = {0.0f, 0.0f, 0.0f};
    s32 planeIndex;
    s32 pointIndex;
    s32 count;

    point.x = x;
    point.z = z;
    std::vector<Vec> selectedPlanes;
    for (planeIndex = 0; planeIndex < 4; ++planeIndex) {
        if (!(PlaneDistance(&lbl_1_bss_70[planeIndex], &point) > threshold)) {
            selectedPlanes.push_back(lbl_1_bss_70[planeIndex].normal);
        }
    }
    count = selectedPlanes.size();
    if (count < 2) return 0;

    result->x = 0.0f;
    result->y = 0.0f;
    result->z = 0.0f;
    for (pointIndex = 0; pointIndex < count; ++pointIndex) {
        addSelectedNormal(selectedPlanes[pointIndex], result);
    }
    PSVECNormalize(result, result);
    return 1;
}

/* The four boundary planes. */
Plane lbl_1_bss_70[4];
