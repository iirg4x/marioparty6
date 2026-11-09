#ifndef REL_M681DLL_OWNED_MODEL_H
#define REL_M681DLL_OWNED_MODEL_H

#include "REL/m681DLL/framework/model.h"
extern "C" {
#include "game/charman.h"
}

class OwnedModel : public Model {
public:
    explicit OwnedModel(void *);
    explicit OwnedModel(unsigned int);
    explicit OwnedModel(HU3D_MODEL_HOOK);
    OwnedModel(int, int);
    explicit OwnedModel(const OwnedModel *);
    ~OwnedModel();
    void apply_defaults();
    void load(unsigned int);
    short id() const { return modelId; }
    void created(short value) { modelId = value; apply_defaults(); }
};

inline OwnedModel::~OwnedModel() {}

#endif
