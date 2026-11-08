#ifndef REL_M648_OWNED_MODEL_H
#define REL_M648_OWNED_MODEL_H
#include "REL/m648DLL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}

struct OwnedModel:Model {
    explicit OwnedModel(void *);
    explicit OwnedModel(unsigned int);
    explicit OwnedModel(HU3D_MODEL_HOOK);
    OwnedModel(int,int);
    OwnedModel(const OwnedModel &);
    ~OwnedModel();
    void apply_defaults();
    void load(unsigned int);
    short id() const { return modelId; }
    void created(short value) { modelId=value; apply_defaults(); }
};

inline OwnedModel::~OwnedModel() {}
#endif
