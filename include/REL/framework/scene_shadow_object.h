#ifndef FRAMEWORK_SCENE_SHADOW_OBJECT_H
#define FRAMEWORK_SCENE_SHADOW_OBJECT_H

#include <vector>
#include "REL/framework/game_object.h"
#include "REL/framework/camera_frame.h"

extern "C" {
#include "dolphin/mtx.h"
#include "dolphin/gx/GXStruct.h"
}

#include "REL/framework/camera_perspective.h"

struct Sphere;

// Scene categories whose registered spheres a shadow camera follows.
typedef std::vector<int> ShadowCategories;
// Spheres a shadow camera must keep in view.
typedef std::vector<Sphere *> ShadowSpheres;

// Fits a camera around the spheres and returns its final eye-to-target distance.
float fitCameraToSpheres(CameraFrame *, CameraPerspective *, ShadowSpheres *);
// Points shadow camera `mask` along `direction` at `target`, with the given width, depth and field
// of view.
void configureShadowCamera(short mask, const Vec *direction, Vec *target, float width, float depth,
                           float fov);

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
