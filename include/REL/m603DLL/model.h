#ifndef MODEL_H
#define MODEL_H

#include "dolphin/mtx.h"
#include "dolphin/gx/GXStruct.h"

struct Transform {
    unsigned char unknown_00[24]; /* No code names this field. */
    Vec pos;
    Vec rot;
    Vec scale;
};

struct HsfObject_s;

struct Model {
    short modelId;
    ~Model() { fn_1_33908(-1); }

    operator short() const { return modelId; }

    void fn_1_34228(char *name);
    short fn_1_33908(short value);
    void fn_1_33950(unsigned long flags);
    void fn_1_33988(unsigned long flags);
    void fn_1_33C78(float level);
    void fn_1_33CB0(short layer);
    void fn_1_33CE8(unsigned short camera);
    bool fn_1_33D20(unsigned short camera);
    void fn_1_344F0(float time);
    void fn_1_34528(float time);
    void fn_1_339C0(Transform *transform);
    void fn_1_33A3C(Vec *position);
    Vec *fn_1_33A84(Vec *position);
    void fn_1_33ACC(Vec *rotation);
    Vec *fn_1_33B14(Vec *rotation);
    void fn_1_33B5C(float angle);
    float fn_1_33BAC();
    void fn_1_33BE8(Vec *scale);
    void fn_1_33C30(float scale);
    void fn_1_33D68(char *name, Vec *position, Vec *direction, GXColor *color);
    void fn_1_33DF8(bool moving);
    void fn_1_33E3C(Vec *color);
    void fn_1_33E80(void *hook);
    float *fn_1_33EA0();
    void fn_1_33F98();
    void fn_1_340D8();
    void fn_1_34108();
    float fn_1_345E4();
    float fn_1_34618();
    float fn_1_3464C();
    void fn_1_34680(float speed);
    int fn_1_346CC();
    void fn_1_34734();
    void fn_1_34764();
    void fn_1_34794(char *name, unsigned long attr);
    void fn_1_347D4(char *name, short hook);
    void fn_1_34814(char *name, short other);
    HsfObject_s *fn_1_33F28(char *name);
    void fn_1_34138(char *name);
    void fn_1_34170();
    Vec *fn_1_344A0(char *name, Vec *position);
    float fn_1_34560();
};


// These users share the owning-model declaration; other model users define their own view.
#ifdef M603_OWNED_MODEL_DECLARATION
extern "C" {
#include "game/hu3d.h"
}

struct OwnedModel:Model {
    explicit OwnedModel(void *);
    explicit OwnedModel(unsigned int);
    explicit OwnedModel(HU3D_MODEL_HOOK);
    OwnedModel(int,int);
    explicit OwnedModel(const OwnedModel*);
    ~OwnedModel() {}
    void apply_defaults();
    void load(unsigned int);
    short id() const { return modelId; }
    void created(short value) { modelId=value; apply_defaults(); }
};
#endif

#endif
