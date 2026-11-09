#ifndef M654_OWNED_MODEL_H
#define M654_OWNED_MODEL_H
#include "REL/m654DLL/framework/model.h"

struct Hu3DModel_s;

// Destruction gives the model handle back.
struct ReleasedModel : Model {
    ~ReleasedModel() { fn_1_33908(-1); }
};

// A model created from its data, a resource number, a drawing hook or a character.
struct OwnedModel : ReleasedModel {
    explicit OwnedModel(void *data);
    explicit OwnedModel(unsigned int dataNum);
    explicit OwnedModel(void (*hook)(Hu3DModel_s *model, Mtx *matrix));
    OwnedModel(int character, int variant);
    ~OwnedModel() {}
    void apply_defaults();
    void load(unsigned int dataNum);
    short id() const { return modelId; }
    void created(short value) { modelId = value; apply_defaults(); }
};

#endif
