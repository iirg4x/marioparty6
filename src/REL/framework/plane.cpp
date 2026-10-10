// Builds plane equations from 3D points and surface normals.
static const char rcsid[] = "$Id: plane.cpp,v 1.22 2004/09/09 12:24:38 saf Exp $";

#include <dolphin/mtx.h>

// Stores a unit normal and signed offset for the plane equation.
struct Plane {
    Point3d normal; // Unit direction perpendicular to the plane.
    f32 distanceFromOrigin; // Signed offset in the same units as the input points.

    void setFromPoints(Point3d *firstPoint, Point3d *secondPoint, Point3d *thirdPoint);
    void setFromNormalAndPoint(Point3d *inputNormal, Point3d *pointOnPlane);
};

// Builds the plane normal from three points, then stores the plane using the first point.
void Plane::setFromPoints(Point3d *firstPoint, Point3d *secondPoint, Point3d *thirdPoint)
{
    Point3d firstEdge;
    Point3d secondEdge;

    PSVECSubtract(firstPoint, secondPoint, &firstEdge);
    PSVECSubtract(secondPoint, thirdPoint, &secondEdge);
    PSVECCrossProduct(&firstEdge, &secondEdge, &normal);
    setFromNormalAndPoint(&normal, firstPoint);
}

// Normalizes the supplied normal and stores the offset for a plane through the point.
// setFromPoints calls this after deriving its normal from the three input points.
void Plane::setFromNormalAndPoint(Point3d *inputNormal, Point3d *pointOnPlane)
{
    f32 projectedDistance;

    PSVECNormalize(inputNormal, &this->normal);
    projectedDistance = PSVECDotProduct(&this->normal, pointOnPlane);
    this->distanceFromOrigin = -projectedDistance;
}
