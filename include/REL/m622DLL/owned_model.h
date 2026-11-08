// A model handle released on destruction, and the models the game objects own.
#ifndef M622_OWNED_MODEL_H
#define M622_OWNED_MODEL_H

#include "REL/m622DLL/model.h"
extern "C" {
#include "game/hu3d.h"
}

struct ReleasedModel : Model {
    ~ReleasedModel() { fn_1_33908(-1); }
};

struct OwnedModel : ReleasedModel {
    explicit OwnedModel(const OwnedModel *source);
    explicit OwnedModel(unsigned int resource);
    explicit OwnedModel(HU3D_MODEL_HOOK hook);
    OwnedModel(int charNo, int variant);
    ~OwnedModel() {}
};

#endif
