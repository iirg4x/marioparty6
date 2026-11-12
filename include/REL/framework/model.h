// Hu3D model ownership and custom drawing hooks used by minigame objects.
#ifndef REL_FRAMEWORK_MODEL_H
#define REL_FRAMEWORK_MODEL_H

extern "C" {
#include "dolphin/mtx.h"
#include "dolphin/gx/GXStruct.h"
}

struct Hu3DModel_s;
// The drawing callback of a hooked Hu3D model, as game/hu3d.h declares it.
typedef void (*HU3D_MODEL_HOOK)(struct Hu3DModel_s *modelP, Mtx *mtx);

// Position, rotation and scale that setTransform copies to a model.
// setTransform does not read the first 0x18 bytes.
struct Transform {
    unsigned char unknown_00[24]; /* No code names this field. */
    Vec pos, rot, scale;
};

struct HsfObject_s;

// Stores a Hu3D handle; Replace releases any nonnegative current handle, even when it is reused.
// Destruction replaces the handle with -1.
class Model {
public:
    short modelId; // Index in Hu3DData, or -1 when no model is owned.

    inline ~Model() { Replace(-1); }
    operator short() const { return modelId; }
    short Replace(short replacement);
    void setAttribute(unsigned long attributes);
    void resetAttribute(unsigned long attributes);
    void setTransform(Transform *transform);
    void setPosition(Vec *position);
    Vec *getPosition(Vec *position);
    void setRotation(Vec *rotation);
    Vec *getRotation(Vec *rotation);
    void setRotationY(float angle);
    float getRotationY();
    void setScale(Vec *scale);
    void setUniformScale(float scale);
    void setTransparency(float level);
    void setLayer(short layer);
    void setCamera(unsigned short cameraBit);
    bool isInView(unsigned short cameraBit);
    void setLight(char *name, Vec *position, Vec *direction, GXColor *color);
    int setLightsDynamic(bool dynamic);
    void setAmbient(Vec *color);
    void setHookData(void *data);
    float *getMatrix();
    Mtx &matrix();
    void copy_matrix(Mtx source);
    HsfObject_s *getObject(char *name);
    void setObjectShadow(char *name);
    void resetObjectShadow(char *name);
    void resetObjectShadowMap(char *name);
    void hideObject(char *name);
    void showObject(char *name);
    void hideObjectTree(char *name);
    void showObjectTree(char *name);
    void setShadow();
    void resetShadow();
    void setShadowMap();
    void setShadowMapObject(char *name);
    void resetShadowMap();
    Vec &getObjectPosition(char *name, Vec *position);
    void setMotionTime(float time);
    void setOverlayMotionTime(float time);
    float getMotionTime();
    float getOverlayMotionTime();
    float getMotionMaxTime();
    float getOverlayMotionMaxTime();
    void setMotionSpeed(float speed);
    bool isMotionEnd();
    void updateMotion();
    void clearOverlayMotion();
    void setObjectAttribute(char *name, unsigned long attributes);
    void setHook(char *name, short hookModel);
    void placeModelAtObject(char *name, short other);
};

// Holds a model created from data, a resource, a Hu3D drawing callback or a character.
struct OwnedModel : Model {
    explicit OwnedModel(void *data);
    explicit OwnedModel(unsigned int resource);
    explicit OwnedModel(HU3D_MODEL_HOOK hook);
    OwnedModel(int character, int variant);
    OwnedModel(const OwnedModel &source);
    ~OwnedModel();
    void apply_defaults();
    void load(unsigned int resource);
    short id() const { return modelId; }
    void created(short value) { modelId = value; apply_defaults(); }
};

inline OwnedModel::~OwnedModel() {}

// Set the attributes every model created afterwards receives (model.cpp's two flags).
void enableDefaultAttributeA();
void enableDefaultAttributesB();

#endif
