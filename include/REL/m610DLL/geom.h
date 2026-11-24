#ifndef REL_M610DLL_GEOM_H
#define REL_M610DLL_GEOM_H
extern "C"
{
/* Bounds checks are enabled here even when NDEBUG was defined. */
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
}
/* A point in the horizontal collision plane. */
struct Point2D
{
    float x;
    float y;
    /* Set both coordinates. */
    void set(float xValue, float yValue)
    {
        x = xValue;
        y = yValue;
    }
};
/* Four points forming the collision region. */
struct QuadRegion
{
    Point2D points[4];
    bool fn_1_111E8(const Point2D *point) const;

    /* Return a corner after checking its index. */
    const Point2D &operator[](int i) const
    {
        assert((i >= 0) && (i < 4));
        return points[i];
    }
};

bool fn_1_10ED4(const QuadRegion *first, const QuadRegion *second);
unsigned char fn_1_112A0(const Point2D *first, const Point2D *second, const Point2D *third,
    const Point2D *point);
bool fn_1_114B0(const Point2D *first, const Point2D *second, const Point2D *third,
    const Point2D *fourth);
unsigned char fn_1_11558(const Point2D *first, const Point2D *second, const Point2D *point);
int fn_1_11650(const Point2D *first, const Point2D *second, const Point2D *third,
    const Point2D *fourth, Point2D *intersection);
void fn_1_1179C(const Point2D *first, const Point2D *second, float *horizontal, float *vertical,
    float *constant);

#endif
