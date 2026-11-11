#ifndef M648_POSITIONED_MODEL_OWNER_H
#define M648_POSITIONED_MODEL_OWNER_H

#include "REL/framework/model.h"
#include "REL/framework/list.h"

// A ring drawn at a position for 15 updates; it deletes itself after them.
class PositionedModelOwner : public ListNode<PositionedModelOwner> {
public:
    OwnedModel model;
    Vec position;
    s32 updatesRemaining;
    PositionedModelOwner(const Vec *position);
    ~PositionedModelOwner();
    void fn_1_A270();
    void fn_1_A2F8(Mtx viewMatrix);
};
#endif
