/* Test containment and perimeter intersections of projected quadrilaterals. */
#include "REL/m610DLL/geom.h"
/* Check corner containment first, then all pairs of perimeter segments. */
bool fn_1_10ED4(const QuadRegion *first, const QuadRegion *second)
{
    for (int index = 0; index < 4; ++index)
    {
        if (first->fn_1_111E8(&(*second)[index]))
            return true;
        if (second->fn_1_111E8(&(*first)[index]))
            return true;
    }
    for (int firstEdge = 0; firstEdge < 4; ++firstEdge)
    {
        const Point2D &firstBegin = (*first)[firstEdge];
        const Point2D &firstEnd = (*first)[(firstEdge + 1) & 3];
        for (int secondEdge = 0; secondEdge < 4; ++secondEdge)
        {
            const Point2D &secondBegin = (*second)[secondEdge];
            const Point2D &secondEnd = (*second)[(secondEdge + 1) & 3];
            if (fn_1_114B0(&firstBegin, &firstEnd, &secondBegin, &secondEnd))
                return true;
        }
    }
    return false;
}

/* Test a point against the two triangles forming this region. */
bool QuadRegion::fn_1_111E8(const Point2D *point) const
{
    if (fn_1_112A0(&(*this)[0], &(*this)[1], &(*this)[2], point))
        return true;
    if (fn_1_112A0(&(*this)[0], &(*this)[2], &(*this)[3], point))
        return true;
    return false;
}

/* Signed edge test used by the triangle containment query. */
static inline float signedEdgeTest(const Point2D &edge, const Point2D &offset)
{
    return edge.y * offset.x - edge.x * offset.y;
}

/* The region query accepts matching edge signs, including triangle boundaries.
 * A collapsed triangle can accept points beyond its vertices. */
unsigned char fn_1_112A0(const Point2D *first, const Point2D *second, const Point2D *third,
                         const Point2D *point)
{
    Point2D firstEdge, secondEdge, thirdEdge, fromFirst, fromSecond, fromThird;
    firstEdge.x = second->x - first->x;
    firstEdge.y = second->y - first->y;
    secondEdge.x = third->x - second->x;
    secondEdge.y = third->y - second->y;
    thirdEdge.x = first->x - third->x;
    thirdEdge.y = first->y - third->y;
    fromFirst.x = point->x - first->x;
    fromFirst.y = point->y - first->y;
    fromSecond.x = point->x - second->x;
    fromSecond.y = point->y - second->y;
    fromThird.x = point->x - third->x;
    fromThird.y = point->y - third->y;
    float firstSide = signedEdgeTest(firstEdge, fromSecond);
    float secondSide = signedEdgeTest(secondEdge, fromThird);
    float thirdSide = signedEdgeTest(thirdEdge, fromFirst);
    unsigned char inside = 0;
    unsigned char adjacentSidesAgree = 0;
    if (firstSide * secondSide >= 0.0f && secondSide * thirdSide >= 0.0f)
    {
        adjacentSidesAgree = 1;
    }
    if (adjacentSidesAgree)
    {
        if (thirdSide * firstSide >= 0.0f)
            inside = 1;
    }
    return inside;
}

/* Report whether two finite segments have an intersection on both spans. */
bool fn_1_114B0(const Point2D *first, const Point2D *second, const Point2D *third,
                const Point2D *fourth)
{
    Point2D intersection;
    bool result;
    if (fn_1_11650(first, second, third, fourth, &intersection) != 0)
        return false;
    result = false;
    if (fn_1_11558(first, second, &intersection) && fn_1_11558(third, fourth, &intersection))
        result = true;
    return result;
}

/* Check whether a point projects between the endpoints, without checking its distance from the
 * line. */
unsigned char fn_1_11558(const Point2D *first, const Point2D *second, const Point2D *point)
{
    float pointX, pointY, projection;
    float edgeX = second->x - first->x;
    float edgeY = second->y - first->y;
    pointX = point->x - first->x;
    pointY = point->y - first->y;
    projection = edgeX * pointX + edgeY * pointY;
    if (projection < 0.0f)
        return 0;
    pointX = point->x - second->x;
    pointY = point->y - second->y;
    projection = edgeX * pointX + edgeY * pointY;
    if (projection > 0.0f)
        return 0;
    return 1;
}

/* Intersect the two infinite endpoint lines; parallel lines leave the output unchanged and
 * return -1. */
int fn_1_11650(const Point2D *first, const Point2D *second, const Point2D *third,
               const Point2D *fourth, Point2D *intersection)
{
    float firstHorizontal, firstVertical, firstConstant;
    float secondHorizontal, secondVertical, secondConstant;
    float determinant, inverse;
    fn_1_1179C(first, second, &firstHorizontal, &firstVertical, &firstConstant);
    fn_1_1179C(third, fourth, &secondHorizontal, &secondVertical, &secondConstant);
    determinant = firstHorizontal * secondVertical - firstVertical * secondHorizontal;
    if (0.0f == determinant)
        return -1;
    inverse = 1.0f / determinant;
    firstHorizontal *= inverse;
    firstVertical *= inverse;
    secondHorizontal *= inverse;
    secondVertical *= inverse;
    intersection->x = secondVertical * firstConstant + (-firstVertical) * secondConstant;
    intersection->y = (-secondHorizontal) * firstConstant + firstHorizontal * secondConstant;
    return 0;
}

/* Return line parameters satisfying horizontal*x + vertical*y = constant. */
void fn_1_1179C(const Point2D *first, const Point2D *second, float *horizontal, float *vertical,
                float *constant)
{
    float absoluteX;
    float deltaX;
    float absoluteY;
    float deltaY;
    deltaX = second->x - first->x;
    if (0.0f == deltaX)
    {
        *horizontal = -1.0f;
        *vertical = 0.0f;
        *constant = -first->x;
        return;
    }
    deltaY = second->y - first->y;
    if (0.0f == deltaY)
    {
        *horizontal = 0.0f;
        *vertical = -1.0f;
        *constant = -first->y;
        return;
    }
    if (deltaX < 0.0f)
        absoluteX = -deltaX;
    else
        absoluteX = deltaX;
    if (deltaY < 0.0f)
        absoluteY = -deltaY;
    else
        absoluteY = deltaY;
    if (absoluteX > absoluteY)
    {
        *horizontal = -1.0f;
        *vertical = deltaX / deltaY;
        *constant = *vertical * first->y - first->x;
        return;
    }
    *horizontal = deltaY / deltaX;
    *vertical = -1.0f;
    *constant = *horizontal * first->x - first->y;
}
