// Shadow-camera helpers and setup used by minigame scenes.
#include <cmath>
#include "REL/framework/scene_shadow.h"

static const char rcsid[] = "$Id: scene_shadow.cpp,v 1.17.2.6 2004/07/16 11:12:51 shohyama Exp $";

// Shadow cameras use a vertical up vector except when looking straight up or down.
Vec *selectShadowCameraUp(const Vec *direction, Vec *up)
{
    if (0.0f == direction->x && 0.0f == direction->z) {
        up->x = 0.0f;
        up->y = 0.0f;
        up->z = 1.0f;
    } else {
        up->x = 0.0f;
        up->y = 1.0f;
        up->z = 0.0f;
    }
    return up;
}

// Store the scaled vector and return the destination for chaining.
inline Vec &scaleVector(const Vec *source, Vec *result, const float &factor)
{
    PSVECScale(source, result, factor);
    return *result;
}

// Replace a vector with its displacement to a target and return it for chaining.
inline Vec &subtractFrom(const Vec *target, Vec &value)
{
    PSVECSubtract(target, &value, &value);
    return value;
}

// Place the camera behind its target along the supplied normalized direction.
float calculateShadowCameraPosition(Vec *position, const Vec *direction,
                                   const Vec *target, float extent, float angle)
{
    float distance = extent / std::tan(angle);
    subtractFrom(target, scaleVector(direction, position, distance));
    return distance;
}

// Build a normalized shadow-camera frustum and place it behind the target.
void configureShadowCamera(short mask, const Vec *direction, Vec *target,
                           float width, float depth, float fov)
{
    float angle = 3.141592653589793 * (fov / 2.0f) / 180.0;
    width *= 0.5f;
    depth *= 0.5f;
    Vec position, normal, up;
    PSVECNormalize(direction, &normal);
    selectShadowCameraUp(direction, &up);
    float distance = calculateShadowCameraPosition(&position, &normal, target, width, angle);
    float nearPlane = distance - depth;
    float farPlane = distance + depth;
    fn_8002A3B0(fov, nearPlane, farPlane, mask);
    fn_8002A674(&position, &up, target, mask);
    fn_8002A8CC(480, mask);
    fn_8002A7D8(0.4f, mask);
}

// Normalize into the destination and return that stored direction.
inline Vec &normalizedDirection(const Vec *source, Vec *destination)
{
    PSVECNormalize(source, destination);
    return *destination;
}

// Start a shadow camera for each selected bit, initially using the default scene category.
SceneShadow::SceneShadow(short mask, const Vec *lightDirection, float fieldOfView, int priority)
    : GameObject(priority), cameraMask((short) mask), collectCategories(true),
      direction(normalizedDirection(lightDirection, &direction))
{
    CameraPerspective &camera = perspective;
    camera.marginLeft = camera.marginRight = camera.marginTop = camera.marginBottom = 8;
    camera.fovRadians = 3.141592653589793 * fieldOfView / 180.0;
    fn_8002A3B0(180.0 * (camera.fovRadians / 3.141592653589793), 0.1f, 1.0f, mask);
    fn_8002A8CC(480, mask);
    fn_8002A7D8(0.4f, mask);
    for (int index = 0; index < 16; ++index) {
        categories[index].clear();
        if (mask & (1U << index)) {
            categories[index].push_back(0);
        }
        spheres[index].clear();
    }
}

// Release the camera lists before unregistering this shadow scene node.
SceneShadow::~SceneShadow() {}

void SceneShadow::enableCategorySpheres() { collectCategories = true; }
void SceneShadow::disableCategorySpheres() { collectCategories = false; }

// Select the full or half-size shadow image for all of this object's cameras.
void SceneShadow::setHalfResolution(bool halfResolution)
{
    int resolution;
    if (halfResolution) {
        resolution = 240;
    } else {
        resolution = 480;
    }
    fn_8002A8CC((u16) resolution, (s16) cameraMask);
}

// Apply the color and convert its alpha byte into shadow transparency.
void SceneShadow::setColor(GXColor *color)
{
    fn_8002AA3C(color->r, color->g, color->b, (s16) cameraMask);
    fn_8002A7D8(color->a / 255.0, (s16) cameraMask);
}

// Apply explicit color bytes to all selected shadow cameras.
void SceneShadow::setColor(unsigned char red, unsigned char green,
                           unsigned char blue, unsigned char alpha)
{
    fn_8002AA3C(red, green, blue, (s16) cameraMask);
    fn_8002A7D8(alpha / 255.0, (s16) cameraMask);
}

// Apply one shadow transparency to all selected shadow cameras.
void SceneShadow::setTransparency(float level)
{
    fn_8002A7D8(level, (s16) cameraMask);
}

// Store the normalized light direction used by later updates.
void SceneShadow::setDirection(const Vec *lightDirection)
{
    PSVECNormalize(lightDirection, &direction);
}

// Add explicit spheres to the selected cameras' next shadow update.
void SceneShadow::appendSpheres(ShadowSpheres *input, short mask)
{
    short active = cameraMask & mask;
    if (active != 0) {
        for (int camera = 0; camera < 16; ++camera) {
            if (active & (1U << camera)) {
                ShadowSpheres &sequence = spheres[camera];
                sequence.insert(sequence.end(), input->begin(), input->end());
            }
        }
    }
}

// Reset each selected camera to one scene category.
void SceneShadow::setCategory(int categoryValue, short mask)
{
    short active = cameraMask & mask;
    if (active != 0) {
        for (int camera = 0; camera < 16; ++camera) {
            if (active & (1U << camera)) {
                ShadowCategories &sequence = categories[camera];
                sequence.clear();
                sequence.push_back(categoryValue);
            }
        }
    }
}

// Replace the category list used to gather geometry for the selected cameras.
void SceneShadow::replaceCategories(const ShadowCategories *input, short mask)
{
    short active = cameraMask & mask;
    if (active != 0) {
        for (int camera = 0; camera < 16; ++camera) {
            if (active & (1U << camera)) {
                ShadowCategories &sequence = categories[camera];
                sequence.clear();
                sequence = *input;
            }
        }
    }
}

// Add one category to every camera selected by both masks.
void SceneShadow::addCategory(int categoryValue, short mask)
{
    short active = cameraMask & mask;
    if (active != 0) {
        for (int camera = 0; camera < 16; ++camera) {
            if (active & (1U << camera)) {
                categories[camera].push_back(categoryValue);
            }
        }
    }
}

// Extend the scene selection of each camera selected by both masks.
void SceneShadow::appendCategories(ShadowCategories *input, short mask)
{
    short active = cameraMask & mask;
    if (active != 0) {
        for (int camera = 0; camera < 16; ++camera) {
            if (active & (1U << camera)) {
                ShadowCategories &sequence = categories[camera];
                sequence.insert(sequence.end(), input->begin(), input->end());
            }
        }
    }
}

// Gather selected spheres and refresh each nonempty shadow camera during scene update.
void SceneShadow::update()
{
    if (cameraMask != 0) {
        for (int camera = 0; camera < 16; ++camera) {
            if (cameraMask & (1U << camera)) {
                ShadowCategories &selected = categories[camera];
                ShadowSpheres &extra = spheres[camera];
                ShadowSpheres gathered;
                if (collectCategories) {
                    int categoryCount = selected.size();
                    for (int index = 0; index < categoryCount; ++index) {
                        collectCategorySpheres(&gathered, selected[index]);
                    }
                }
                gathered.insert(gathered.end(), extra.begin(), extra.end());
                if (gathered.size() == 0) {
                    continue;
                }
                CameraFrame frame;
                frame.eye.x = 0.0f;
                frame.eye.y = 0.0f;
                frame.eye.z = 0.0f;
                frame.target.x = direction.x;
                frame.target.y = direction.y;
                frame.target.z = direction.z;
                selectShadowCameraUp(&direction, &frame.up);
                float farPlane = fitCameraToSpheres(&frame, &perspective, &gathered);
                HU3D_SHADOW &shadow = Hu3DShadowBuf[camera];
                shadow.far = farPlane;
                shadow.camPos.x = frame.eye.x;
                shadow.camPos.y = frame.eye.y;
                shadow.camPos.z = frame.eye.z;
                shadow.camTarget.x = frame.target.x;
                shadow.camTarget.y = frame.target.y;
                shadow.camTarget.z = frame.target.z;
                shadow.camUp.x = frame.up.x;
                shadow.camUp.y = frame.up.y;
                shadow.camUp.z = frame.up.z;
                extra.clear();
            }
        }
    }
}
