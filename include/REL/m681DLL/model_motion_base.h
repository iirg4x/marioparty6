#ifndef REL_M681DLL_MODEL_MOTION_BASE_H
#define REL_M681DLL_MODEL_MOTION_BASE_H

extern "C" {
#include <math.h>
#include "game/hu3d.h"
}

class Model {
public:
    short modelId;
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

struct OwnedModel : Model {
    OwnedModel(const OwnedModel&);
    void created(short value);
    explicit OwnedModel(void*);
    OwnedModel(int character, int variant);
    void apply_defaults();
    void load(unsigned int resource);
    explicit OwnedModel(HU3D_MODEL_HOOK hook);
    explicit OwnedModel(unsigned int resource);
    ~OwnedModel();
};

inline OwnedModel::~OwnedModel() {}

#endif
