#ifndef REL_M611DLL_WANDERING_MODEL_H
#define REL_M611DLL_WANDERING_MODEL_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include "REL/framework/list.h"
// Owns a linked moving model and the state consumed by its motion callbacks.

struct WanderingModel : ListNode<WanderingModel> {
    void (WanderingModel::*callback)();
    Vec position;
    Vec target;
    s32 positionIndex;
    OwnedModel model;
    f32 heading;
    f32 speed;
    f32 targetSpeed;
    u8 unk40[8]; /* No code names this field. */
    s32 duration;
    f32 turnRate;
    f32 targetTurnRate;
    WanderingModel(const OwnedModel &modelSource, Vec *initialPosition, s32 slotIndex);
    void fn_1_1154();
    void applyModelTransform();
};

void fn_1_1018(WanderingModel *object);
void fn_1_1368(WanderingModel *object);

// Live wandering models.
extern ListRoot<WanderingModel> lbl_1_bss_18;

// Updates the wandering model.
void fn_1_F5C(WanderingModel *object);

// Deletes all wandering models.
void fn_1_DEC();

// Updates and rotates the wandering model list.
void fn_1_ED8();

#endif
