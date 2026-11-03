// Fits a camera view around a collection of spheres and applies perspective settings.
#include <vector>
#include <cmath>

extern "C" {
#include "dolphin/mtx.h"
#include "game/hu3d.h"
}

static const char rcsid[] = "$Id: camera_ut0.cpp,v 1.23 2004/09/09 12:24:38 saf Exp $";

// Smaller of two values; the second is returned when they are equal.
template <class T> inline T cameraMin(T first, T second)
{
    return first < second ? first : second;
}

// Larger of two values; the second is returned when they are equal.
template <class T> inline T cameraMax(T first, T second)
{
    return first > second ? first : second;
}

// Stores the eye, up direction, and look target used to orient a minigame camera.
struct CameraFrame {
    Vec eye;    // Camera position in world space.
    Vec up;     // Camera's up direction.
    Vec target; // World-space point the camera looks toward.
};

// One sphere the camera must keep in view.
struct Sphere {
    Point3d center; // Center position in world space.
    f32 radius;     // Radius in world units; a nonpositive radius is left out of the fit.

    f32 getRadius() const { return radius; }
};

// Display size, fitting margins, and field of view used when a camera is fitted.
struct CameraPerspective {
    s32 width;        // Display width in pixels.
    s32 height;       // Display height in pixels.
    s32 marginLeft;   // Pixels kept clear at the left edge.
    s32 marginRight;  // Pixels kept clear at the right edge.
    s32 marginTop;    // Pixels kept clear at the top edge.
    s32 marginBottom; // Pixels kept clear at the bottom edge.
    f32 fovRadians;   // Vertical field of view in radians.
    CameraPerspective();
    void apply(s32 cameraBit);
    void read(s32 cameraBit);
};

// Working state of one fit: the spheres in camera axes and the view extents measured from them.
struct CameraFitter {
    f32 distance;              // Final distance from the fitted eye to its target.
    Mtx view;                  // World-to-camera matrix of the pose being fitted.
    std::vector<Vec> centers;  // Accepted sphere centers, rotated into camera axes.
    std::vector<f32> radii;    // Radius of each accepted center.
    f32 targetDepth;           // Lowest sphere surface on the camera's back axis; the target plane.
    f32 aspect;                // Display width over height.
    f32 halfFovTangent;        // Tangent of half the field of view.
    f32 horizontalSlope;       // Horizontal view slope inside the side margins.
    f32 verticalSlope;         // Vertical view slope inside the top and bottom margins.
    f32 left;                  // Left view extent.
    f32 right;                 // Right view extent.
    f32 top;                   // Top view extent.
    f32 bottom;                // Bottom view extent.
    f32 fitWidth;              // Fitted view width.
    f32 fitHeight;             // Fitted view height.

    CameraFitter() {}
    ~CameraFitter() {}

    void fit(CameraFrame *pose, CameraPerspective *perspective, std::vector<Sphere *> *spheres);
    s32 collect(CameraFrame *pose, std::vector<Sphere *> *spheres);
    void configure(CameraPerspective *perspective);
    void bounds();
    void reposition(CameraFrame *pose, CameraPerspective *perspective);
};

// Fits a temporary camera calculation and returns its final eye-to-target distance.
f32 fitCameraToSpheres(CameraFrame *pose, CameraPerspective *perspective,
                       std::vector<Sphere *> *spheres)
{
    CameraFitter fitter;
    fitter.fit(pose, perspective, spheres);
    return fitter.distance;
}

// Recomputes the camera only when a sphere passes the nonpositive-radius rejection.
void CameraFitter::fit(CameraFrame *pose, CameraPerspective *perspective,
                       std::vector<Sphere *> *spheres)
{
    if (collect(pose, spheres) > 0) {
        configure(perspective);
        bounds();
        reposition(pose, perspective);
    }
}

// Copies sphere centers and radii, then rotates the centers into the current camera axes.
s32 CameraFitter::collect(CameraFrame *pose, std::vector<Sphere *> *spheres)
{
    int sphereCount = spheres->size();
    if (sphereCount <= 0) {
        return 0;
    }
    C_MTXLookAt(view, &pose->eye, &pose->up, &pose->target);
    centers.clear();
    radii.clear();
    centers.reserve(sphereCount);
    radii.reserve(sphereCount);
    for (int index = 0; index < sphereCount; ++index) {
        Sphere *sphere = (*spheres)[index];
        f32 radius = sphere->getRadius();
        if (!(radius <= 0.0f)) {
            centers.push_back(sphere->center);
            radii.push_back(radius);
        }
    }
    sphereCount = centers.size();
    if (sphereCount <= 0) {
        return 0;
    }
    if (sphereCount > 1) {
        PSMTXMultVecArraySR(view, centers.begin(), centers.begin(), sphereCount);
    } else {
        PSMTXMultVecSR(view, centers.begin(), centers.begin());
    }
    return sphereCount;
}

// Calculates the available view slopes and the target plane's depth from the collected spheres.
void CameraFitter::configure(CameraPerspective *perspective)
{
    int centerCount = centers.size();
    targetDepth = centers[0].z - radii[0];
    for (int index = 1; index < centerCount; ++index) {
        targetDepth = cameraMin(targetDepth, centers[index].z - radii[index]);
    }
    aspect = f32(perspective->width) / f32(perspective->height);
    halfFovTangent = std::tan(perspective->fovRadians / 2.0f);
    horizontalSlope = aspect * (halfFovTangent *
        (f32(perspective->width - (perspective->marginLeft + perspective->marginRight)) /
         f32(perspective->width)));
    verticalSlope = halfFovTangent *
        (f32(perspective->height - (perspective->marginTop + perspective->marginBottom)) /
         f32(perspective->height));
}

// Measures the view extents including each sphere's radius at its relative depth.
void CameraFitter::bounds()
{
    int centerCount = centers.size();
    left = centers[0].x;
    right = centers[0].x;
    top = centers[0].y;
    bottom = centers[0].y;
    for (int index = 0; index < centerCount; ++index) {
        Vec &center = centers[index];
        f32 depth = center.z - targetDepth;
        if (!(depth < 0.0f)) {
            f32 horizontalRadius = horizontalSlope * depth + radii[index];
            f32 verticalRadius = verticalSlope * depth + radii[index];
            left = cameraMin(left, center.x - horizontalRadius);
            right = cameraMax(right, center.x + horizontalRadius);
            top = cameraMax(top, center.y + verticalRadius);
            bottom = cameraMin(bottom, center.y - verticalRadius);
        }
    }
}

// Expands to the display aspect and margins, then converts the fitted pose back to world axes.
void CameraFitter::reposition(CameraFrame *pose, CameraPerspective *perspective)
{
    fitWidth = ((right - left) / 2.0f) / horizontalSlope;
    fitHeight = ((top - bottom) / 2.0f) / verticalSlope;
    distance = cameraMax(fitWidth, fitHeight);
    const s32 &width = perspective->width;
    const s32 &height = perspective->height;
    const s32 &insetLeft = perspective->marginLeft;
    const s32 &insetRight = perspective->marginRight;
    const s32 &insetTop = perspective->marginTop;
    const s32 &insetBottom = perspective->marginBottom;
    s32 availableWidth = width - (insetLeft + insetRight);
    s32 availableHeight = height - (insetTop + insetBottom);
    if (fitWidth > fitHeight) {
        fitWidth = right - left;
        fitHeight = fitWidth * (f32(availableHeight) / f32(availableWidth));
        f32 middle = (top + bottom) / 2.0f;
        f32 halfHeight = fitHeight / 2.0f;
        top = middle + halfHeight;
        bottom = middle - halfHeight;
    } else if (fitHeight > fitWidth) {
        fitHeight = top - bottom;
        fitWidth = fitHeight * (f32(availableWidth) / f32(availableHeight));
        f32 middle = (left + right) / 2.0f;
        f32 halfWidth = fitWidth / 2.0f;
        left = middle - halfWidth;
        right = middle + halfWidth;
    }
    left -= fitWidth * (f32(insetLeft) / f32(availableWidth));
    right += fitWidth * (f32(insetRight) / f32(availableWidth));
    top += fitHeight * (f32(insetTop) / f32(availableHeight));
    bottom -= fitHeight * (f32(insetBottom) / f32(availableHeight));
    pose->target.x = (left + right) / 2.0f;
    pose->target.y = (top + bottom) / 2.0f;
    pose->target.z = targetDepth;
    pose->eye.x = pose->target.x;
    pose->eye.y = pose->target.y;
    pose->eye.z = pose->target.z + distance;
    PSMTXTranspose(view, view);
    PSMTXMultVecSR(view, &pose->target, &pose->target);
    PSMTXMultVecSR(view, &pose->eye, &pose->eye);
}

// Initializes the display size, fitting margins, and a 45-degree field of view.
CameraPerspective::CameraPerspective()
{
    width = 576;
    height = 480;
    marginLeft = 16;
    marginRight = 16;
    marginTop = 40;
    marginBottom = 40;
    fovRadians = 0.7853982f;
}

// Preserves clipping planes and applies the stored field of view with integer aspect division.
void CameraPerspective::apply(s32 cameraBit)
{
    f32 nearPlane;
    f32 farPlane;
    f32 fov;

    fn_80027D8C(cameraBit, &fov, &nearPlane, &farPlane);
    Hu3DCameraPerspectiveSet(cameraBit, 57.29578f * fovRadians,
                             nearPlane, farPlane, width / height);
}

// Reads the engine's field of view in degrees, discarding both clipping outputs into one scratch.
void CameraPerspective::read(s32 cameraBit)
{
    f32 unusedPlane;

    fn_80027D8C(cameraBit, &fovRadians, &unusedPlane, &unusedPlane);
    fovRadians = 0.017453292f * fovRadians;
}
