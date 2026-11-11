#ifndef SCENE_SHADOW_H
#define SCENE_SHADOW_H

#include <vector>
#include "REL/framework/game_object.h"
#include "REL/framework/camera_frame.h"

extern "C" {
#include "dolphin/mtx.h"
#include "dolphin/gx/GXStruct.h"
#include "game/hu3d.h"
}

// Display size, fitting margins, and field of view used when a camera is fitted.
struct CameraPerspective {
    int width;        // Display width in pixels.
    int height;       // Display height in pixels.
    int marginLeft;   // Pixels kept clear at the left edge.
    int marginRight;  // Pixels kept clear at the right edge.
    int marginTop;    // Pixels kept clear at the top edge.
    int marginBottom; // Pixels kept clear at the bottom edge.
    float fovRadians; // Vertical field of view in radians.
    CameraPerspective();
    // Preserves clipping planes and applies the stored field of view with integer aspect division.
    void apply(s32 cameraBit);
    // Reads the engine's field of view in degrees into fovRadians, in radians.
    void read(s32 cameraBit);
};

struct Sphere;

// Scene categories whose registered spheres a shadow camera follows.
typedef std::vector<int> ShadowCategories;
// Spheres a shadow camera must keep in view.
typedef std::vector<Sphere *> ShadowSpheres;

// Appends the registered spheres of one scene category.
void collectCategorySpheres(ShadowSpheres *, long);
// Fits a camera around the spheres and returns its final eye-to-target distance.
float fitCameraToSpheres(CameraFrame *, CameraPerspective *, ShadowSpheres *);

// Manages the shadow renderer and its camera-specific scene selections.
struct SceneShadow : GameObject {
    CameraPerspective perspective; // Fitting settings shared by all of this object's cameras.
    short cameraMask;              // One bit for each shadow camera this object drives.
    unsigned char collectCategories : 1; // Whether updates gather spheres from the categories.
    unsigned char flags : 7;
    Vec direction;                 // Normalized light direction.
    ShadowCategories categories[16]; // Scene categories selected for each camera.
    ShadowSpheres spheres[16];     // Explicit spheres waiting for each camera's next update.

    SceneShadow(short, const Vec *, float, int);
    virtual ~SceneShadow();
    void enableCategorySpheres();
    void disableCategorySpheres();
    void setHalfResolution(bool);
    void setColor(GXColor *);
    void setColor(unsigned char, unsigned char, unsigned char, unsigned char);
    void setTransparency(float);
    void setDirection(const Vec *);
    void appendSpheres(ShadowSpheres *, short);
    void setCategory(int, short);
    void replaceCategories(const ShadowCategories *, short);
    void addCategory(int, short);
    void appendCategories(ShadowCategories *, short);
    virtual void update();
};

#endif
