// Hu3D model ownership and custom drawing hooks used by minigame objects.
#ifndef REL_FRAMEWORK_MODEL_H
#define REL_FRAMEWORK_MODEL_H

extern "C" {
#include "dolphin/math.h"
#include "game/hu3d.h"
}

// Stores a Hu3D handle; Replace releases any nonnegative current handle, even when it is reused.
// Destruction replaces the handle with -1.
class Model {
public:
    short modelId; // Index in Hu3DData, or -1 when no model is owned.

    inline ~Model() { Replace(-1); }
    operator short() const { return modelId; }
    short Replace(short replacement);
    void setHookData(void *owner);
};

// Holds a model created with a custom Hu3D drawing callback.
struct OwnedModel : Model {
    explicit OwnedModel(HU3D_MODEL_HOOK hook);
    ~OwnedModel();
};

inline OwnedModel::~OwnedModel() {}

#endif
