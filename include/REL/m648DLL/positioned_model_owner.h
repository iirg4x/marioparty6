#ifndef M648_POSITIONED_MODEL_OWNER_H
#define M648_POSITIONED_MODEL_OWNER_H

extern "C++" {
#include "REL/m648DLL/framework/model.h"
#include "REL/framework/linknode.h"
extern "C" {
#include "game/hu3d.h"
}

#include "REL/m648DLL/owned_model.h"

struct PositionedModelNode : Node {
    PositionedModelNode() {}
    ~PositionedModelNode() {}
};

class PositionedModelOwner : public PositionedModelNode {
public:
    OwnedModel model;
    Vec position;
    s32 updatesRemaining;
    PositionedModelOwner(const Vec *position);
    void fn_1_A270();
    void fn_1_A2F8(Mtx viewMatrix);
};
}
#endif
