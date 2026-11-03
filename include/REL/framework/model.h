// Hu3D model ownership and custom drawing hooks used by minigame objects.
#ifndef REL_FRAMEWORK_MODEL_H
#define REL_FRAMEWORK_MODEL_H

extern "C" {
#include <math.h>
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
    void fn_1_33CB0(short layer);
    void fn_1_33DF8(bool enabled);
    void fn_1_33950(unsigned long attributes);
    void fn_1_34108();
    void fn_1_34528(float);
    float fn_1_3464C();
    void fn_1_33CE8(unsigned short);
    void fn_1_33988(unsigned long);
    void fn_1_33A3C(Vec *);
    void fn_1_33ACC(Vec *);
    void fn_1_33C30(float);
    void fn_1_33F98();
    void fn_1_340D8();
    void fn_1_344F0(float);
};

// Holds a model created with a custom Hu3D drawing callback.
struct OwnedModel : Model {
    OwnedModel(const OwnedModel&);
    void created(short value);
    explicit OwnedModel(void*);
    OwnedModel(int character,int variant);
    void apply_defaults();
    void load(unsigned int resource);
    explicit OwnedModel(HU3D_MODEL_HOOK hook);
    explicit OwnedModel(unsigned int resource);
    ~OwnedModel();
};

inline OwnedModel::~OwnedModel() {}

#endif
